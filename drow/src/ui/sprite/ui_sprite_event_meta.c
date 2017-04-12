#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_data_entry.h"
#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui_sprite_event_meta_i.h"
#include "ui_sprite_data_build_i.h"

ui_sprite_event_meta_t
ui_sprite_event_meta_create(ui_sprite_repository_t repo, const char * name, LPDRMETA dr_meta) {
    ui_sprite_event_meta_t meta;

    if (dr_meta == NULL) {
        size_t name_len;

        name_len = strlen(name) + 1;

        meta = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_event_meta) + name_len);
        if (meta == NULL) {
            CPE_ERROR(repo->m_em, "create event meta %s: alloc fail !", name);
            return NULL;
        }

        meta->m_name = (char*)(meta + 1);
        memcpy(meta + 1, name, name_len);
    }
    else {
        meta = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_event_meta));
        if (meta == NULL) {
            CPE_ERROR(repo->m_em, "create event meta %s: alloc fail !", dr_meta_name(dr_meta));
            return NULL;
        }

        meta->m_name = dr_meta_name(dr_meta);
    }

    meta->m_meta = dr_meta;
    meta->m_debug_level = 1;

    cpe_hash_entry_init(&meta->m_hh_for_repo);
    if (cpe_hash_table_insert_unique(&repo->m_event_metas, meta) != 0) {
        CPE_ERROR(repo->m_em, "create event meta %s: name duplicate!", dr_meta_name(dr_meta));
        mem_free(repo->m_alloc, meta);
        return NULL;
    }

    return meta;
}

void ui_sprite_event_meta_free(ui_sprite_repository_t repo, ui_sprite_event_meta_t meta) {
    cpe_hash_table_remove_by_ins(&repo->m_event_metas, meta);
    mem_free(repo->m_alloc, meta);
}

void ui_sprite_event_meta_free_all(ui_sprite_repository_t repo) {
    struct cpe_hash_it meta_it;
    ui_sprite_event_meta_t meta;

    cpe_hash_it_init(&meta_it, &repo->m_event_metas);

    meta = cpe_hash_it_next(&meta_it);
    while (meta) {
        ui_sprite_event_meta_t next = cpe_hash_it_next(&meta_it);
        ui_sprite_event_meta_free(repo, meta);
        meta = next;
    }
}

ui_sprite_event_meta_t ui_sprite_event_meta_find(ui_sprite_repository_t repo, const char * name) {
    struct ui_sprite_event_meta key;
    key.m_name = name;
    return cpe_hash_table_find(&repo->m_event_metas, &key);
}

static int ui_sprite_event_meta_build_event_set_entry(
    ui_sprite_event_t event,
    ui_sprite_repository_t repo,
    ui_sprite_world_t world, ui_sprite_entity_t entity,
    char * def, dr_data_source_t data_source)
{
    char * p = strchr(def, '=');
    char * arg_name = def;
    char * arg_value;
    int to_entry_start;
    struct dr_data_entry to;

    if (p == NULL) {
        CPE_ERROR(repo->m_em, "build event: event %s: arg def %s format error!", dr_meta_name(event->meta), def);
        return -1;
    }

    *((char*)cpe_str_trim_tail(p, arg_name)) = 0;
    arg_value = (char*)cpe_str_trim_head(p + 1);

    to.m_entry = dr_meta_find_entry_by_path_ex(event->meta, arg_name, &to_entry_start);
    if (to.m_entry == NULL) {
        CPE_ERROR(repo->m_em, "build event: event %s: entry %s not exist!", dr_meta_name(event->meta), arg_name);
        return -1;
    }

    to.m_data = ((char *)event->data) + to_entry_start;
    to.m_size = dr_entry_size(to.m_entry);

    if (ui_sprite_data_build(&to, arg_value, world, entity, data_source) != 0) {
        CPE_ERROR(repo->m_em, "build event: event %s: entry %s: set value fail!", dr_meta_name(event->meta), arg_name);
        return -1;
    }

    return 0;
}

int ui_sprite_event_meta_build_event(
    ui_sprite_event_t event,
    ui_sprite_repository_t repo, 
    ui_sprite_world_t world,
    ui_sprite_entity_t entity,
    char * event_args, dr_data_source_t data_source)
{
    char * p;

    while((p = (char*)cpe_str_char_not_in_pair(event_args, ',', "{[(", ")]}"))) {
        char * arg_begin = event_args;
        char * arg_end = (char*)cpe_str_trim_tail(p, arg_begin);

        *arg_end = 0;
        event_args = (char*)cpe_str_trim_head(p + 1);

        if (ui_sprite_event_meta_build_event_set_entry(event, repo, world, entity, arg_begin, data_source) != 0) {
            return -1;
        }
    }

    if (*event_args != 0) {
        *(char *)cpe_str_trim_tail(event_args + strlen(event_args), event_args) = 0;
        if (ui_sprite_event_meta_build_event_set_entry(event, repo, world, entity, event_args, data_source) != 0) {
            return -1;
        }
    }

    return 0;
}

int ui_sprite_repository_register_event(ui_sprite_repository_t repo, LPDRMETA meta) {
    ui_sprite_event_meta_t event_meta;
    event_meta = ui_sprite_event_meta_find(repo, dr_meta_name(meta));
    if (event_meta) {
        if (event_meta->m_meta) {
            CPE_ERROR(repo->m_em, "create event meta %s: name duplicate!", dr_meta_name(meta));
        }
        else {
            event_meta->m_meta = meta;
        }
    }
    else {
        event_meta = ui_sprite_event_meta_create(repo, NULL, meta);
    }

    return event_meta ? 0 : -1;
}

uint8_t ui_sprite_repository_event_debug_level(ui_sprite_repository_t repo, LPDRMETA meta) {
    ui_sprite_event_meta_t event_meta = ui_sprite_event_meta_find(repo, dr_meta_name(meta));
    return event_meta ? event_meta->m_debug_level : 1;
}

int ui_sprite_repository_register_event_debug_level(ui_sprite_repository_t repo, const char * name, uint8_t level) {
    ui_sprite_event_meta_t event_meta;

    event_meta = ui_sprite_event_meta_create(repo, name, NULL);
    if (event_meta == NULL) {
        CPE_ERROR(repo->m_em, "set event meta %s debug level %d: name duplicate!", name, level);
        return -1;
    }

    event_meta->m_debug_level = level;

    return 0;
}


void ui_sprite_repository_unregister_event(ui_sprite_repository_t repo, const char * name) {
    ui_sprite_event_meta_t event_meta = ui_sprite_event_meta_find(repo, name);
    if (event_meta == NULL) {
        if (event_meta->m_debug_level == 1) {
            ui_sprite_event_meta_free(repo, event_meta);
        }
        else {
            event_meta->m_meta = NULL;
        }
    }
}

LPDRMETA ui_sprite_repository_find_event(ui_sprite_repository_t repo, const char * name) {
    ui_sprite_event_meta_t event_meta = ui_sprite_event_meta_find(repo, name);
    return event_meta ? event_meta->m_meta : NULL;
}

int ui_sprite_repository_register_events_by_prefix(ui_sprite_repository_t repo, LPDRMETALIB metalib, const char * prefix) {
    int i;

    for(i = 0; i < dr_lib_meta_num(metalib); ++i) {
        LPDRMETA meta = dr_lib_meta_at(metalib, i);
        const char * meta_name = dr_meta_name(meta);
        if (strstr(meta_name, prefix) == meta_name) {
            if (ui_sprite_repository_register_event(repo, meta) != 0) {
                int j;

                /* 已经注册的事件需要删除 */
                for(j = 0; j < i; ++j) {
                    LPDRMETA remove_meta = dr_lib_meta_at(metalib, j);
                    const char * remove_meta_name = dr_meta_name(remove_meta);
                    if (strstr(remove_meta_name, prefix) == remove_meta_name) {
                        ui_sprite_repository_unregister_event(repo, remove_meta_name);
                    }
                }

                return -1;
            }
        }
    }

    return 0;
}

void ui_sprite_repository_unregister_events_by_prefix(ui_sprite_repository_t repo, LPDRMETALIB metalib, const char * prefix) {
    int i;

    for(i = 0; i < dr_lib_meta_num(metalib); ++i) {
        LPDRMETA meta = dr_lib_meta_at(metalib, i);
        const char * meta_name = dr_meta_name(meta);
        if (strstr(meta_name, prefix) == meta_name) {
            ui_sprite_repository_unregister_event(repo, meta_name);
        }
    }
}

uint32_t ui_sprite_event_meta_hash(const ui_sprite_event_meta_t meta) {
    return cpe_hash_str(meta->m_name, strlen(meta->m_name));
}

int ui_sprite_event_meta_eq(const ui_sprite_event_meta_t l, const ui_sprite_event_meta_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

