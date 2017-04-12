#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_json.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "render/model/ui_object_ref.h"
#include "ui_data_src_group_i.h"
#include "ui_data_src_group_item_i.h"
#include "ui_data_src_i.h"
#include "ui_data_src_src_i.h"

ui_data_src_group_t ui_data_src_group_create(ui_data_mgr_t mgr) {
    ui_data_src_group_t group;

    group = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_src_group));
    if (group == NULL) {
        CPE_ERROR(mgr->m_em, "ui_data_src_group: create: alloc fail");
        return NULL;
    }

    group->m_mgr = mgr;
    group->m_item_count = 0;
    TAILQ_INIT(&group->m_items);

    return group;
}

void ui_data_src_group_free(ui_data_src_group_t group) {
    ui_data_mgr_t mgr = group->m_mgr;

    ui_data_src_group_clear(group);
    
    mem_free(mgr->m_alloc, group);
}

void ui_data_src_group_clear(ui_data_src_group_t group) {
    while(!TAILQ_EMPTY(&group->m_items)) {
        ui_data_src_group_item_free(TAILQ_FIRST(&group->m_items));
    }
}

ui_data_mgr_t ui_data_src_group_mgr(ui_data_src_group_t group) {
    return group->m_mgr;
}

int ui_data_src_group_add_src(ui_data_src_group_t group, ui_data_src_t src) {
    return ui_data_src_group_item_create(group, src) == NULL ? -1 : 0;
}

int ui_data_src_group_add_src_by_path(ui_data_src_group_t group, const char * src_path, ui_data_src_type_t src_type) {
    ui_data_src_t src = ui_data_src_find_by_path(group->m_mgr, src_path, src_type);
    if (src == NULL) {
        CPE_ERROR(
            group->m_mgr->m_em, "src %s type %s not exist!",
            src_path, ui_data_src_type_name(src_type));
        return -1;
    }

    return ui_data_src_group_add_src(group, src);
}

int ui_data_src_group_add_src_by_id(ui_data_src_group_t group, uint32_t src_id) {
    ui_data_src_t src = ui_data_src_find_by_id(group->m_mgr, src_id);
    if (src == NULL) {
        CPE_ERROR(group->m_mgr->m_em, "src %u not exist!", src_id);
        return -1;
    }

    return ui_data_src_group_add_src(group, src);
}

int ui_data_src_group_add_src_by_url(ui_data_src_group_t group, UI_OBJECT_URL const * url) {
    ui_data_src_t src;

    //printf("xxxx: add by url %s\n", dr_json_dump_inline(&group->m_mgr->m_dump_buffer, url, sizeof(*url), group->m_mgr->m_meta_object_url));
    src = ui_data_src_find_by_url(group->m_mgr, url);
    if (src == NULL) {
        CPE_ERROR(
            group->m_mgr->m_em, "src url{%s} not exist!",
            dr_json_dump_inline(&group->m_mgr->m_dump_buffer, url, sizeof(*url), group->m_mgr->m_meta_object_url));
        return -1;
    }
    
    return ui_data_src_group_add_src(group, src);
}

int ui_data_src_group_add_src_by_res(ui_data_src_group_t group, const char * res) {
    UI_OBJECT_URL buf;
    UI_OBJECT_URL * url;
    int rv;
    const char * args = NULL;
    const char * args_end = NULL;
    
    if (res[0] == '[') {
        args = res + 1;
        args_end = cpe_str_char_not_in_pair(args, ']', "{[(", ")]}");
        if (args_end == NULL) {
            CPE_ERROR(group->m_mgr->m_em, "ui_data_src_group_add_src_by_res: res %s format error!", res);
            return -1;
        }

        res = args_end + 1;
        if (res[0] == 0) return 0;
    }

    url = ui_object_ref_parse(res, &buf, group->m_mgr->m_em);
    if (url == NULL) {
        CPE_ERROR(group->m_mgr->m_em, "ui_data_src_group_add_src_by_res: res %s format error!", res);
        return -1;
    }

    rv = ui_data_src_group_add_src_by_url(group, url);

    if (args) {
        char buf[128];
        if (url->type == UI_OBJECT_TYPE_SKELETON) {
            if (cpe_str_read_arg_range(buf, sizeof(buf), args, args_end, "state", ',', '=') == 0) {
                if (buf[0] == 0) {
                    if (ui_data_src_group_add_src_by_path(group, url->data.skeleton.src.data.by_path.path, ui_data_src_type_spine_state_def) != 0) {
                        rv = -1;
                    }
                }
                else if (strcmp(buf, "auto") == 0) {
                }
                else {
                    if (ui_data_src_group_add_src_by_path(group, buf, ui_data_src_type_spine_state_def) != 0) {
                        rv = -1;
                    }
                }
            }
        }
    }
    
    return rv;
}

int ui_data_src_group_add_src_by_group(ui_data_src_group_t group, ui_data_src_group_t from_group) {
    ui_data_src_group_item_t item;
    int rv = 0;

    TAILQ_FOREACH(item, &from_group->m_items, m_next_for_group) {
        if (ui_data_src_group_add_src(group, item->m_src) != 0) rv = -1;
    }

    return rv;
}

int ui_data_src_group_remove_src(ui_data_src_group_t group, ui_data_src_t src) {
    ui_data_src_group_item_t item;

    TAILQ_FOREACH(item, &src->m_items, m_next_for_src) {
        if (item->m_group == group) {
            ui_data_src_group_item_free(item);
            return 0;
        }
    }

    return -1;
}

static ui_data_src_t ui_data_src_group_src_next(struct ui_data_src_it * it) {
    ui_data_src_group_item_t r = *(ui_data_src_group_item_t *)it->m_data;

    if (r) {
        *(ui_data_src_group_item_t *)it->m_data = TAILQ_NEXT(r, m_next_for_group);
    }

    return r ? r->m_src : NULL;
}

void ui_data_src_group_srcs(ui_data_src_it_t src_it, ui_data_src_group_t group) {
    *(ui_data_src_group_item_t *)src_it->m_data = TAILQ_FIRST(&group->m_items);
    src_it->next = ui_data_src_group_src_next;
}

static int ui_data_src_group_load_dir(ui_data_src_group_t group, ui_data_src_t d) {
    if (d->m_type == ui_data_src_type_dir) {
        ui_data_src_t child;

        TAILQ_FOREACH(child, &d->m_childs, m_next_for_parent) {
            if (ui_data_src_group_load_dir(group, child) != 0) return -1;
        }

        return 0;
    }
    else {
        return ui_data_src_group_add_src(group, d);
    }
}

static int ui_data_src_group_load_deps(ui_data_src_group_t group, ui_data_src_t src) {
    ui_data_src_src_t dep_src;
    int rv = 0;
    
    ui_data_src_update_using(src);
    
    TAILQ_FOREACH(dep_src, &src->m_using_srcs, m_next_for_user) {
        if (ui_data_src_group_add_src(group, dep_src->m_using_src) != 0) {
            rv = -1;
        }
    }
    
    return rv;
}

int ui_data_src_group_load_all(ui_data_src_group_t group) {
    ui_data_src_group_item_t item;
    int rv = 0;
    
    for(item = TAILQ_FIRST(&group->m_items); item != TAILQ_END(&group->m_items); item = TAILQ_NEXT(item, m_next_for_group)) {
        if (ui_data_src_type(item->m_src) == ui_data_src_type_dir) {
            if (ui_data_src_group_load_dir(group, item->m_src) != 0) {
                rv = -1;
            }
        }
        else {
            if (!ui_data_src_is_loaded(item->m_src)) {
                if (ui_data_src_load(item->m_src, group->m_mgr->m_em) != 0) {
                    CPE_ERROR(
                        group->m_mgr->m_em, "src %s load fail!",
                        ui_data_src_path_dump(&group->m_mgr->m_dump_buffer, item->m_src));
                    rv = -1;
                }
            }

            if (ui_data_src_is_loaded(item->m_src) && !item->m_is_expand) {
                item->m_is_expand = 1;

                if (ui_data_src_group_load_deps(group, item->m_src) != 0) {
                    rv = -1;
                }
            }
        }
    }

    return rv;
}

int ui_data_src_group_expand_dir(ui_data_src_group_t group) {
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    int rv = 0;
    ui_data_src_group_t remove_group = NULL;
    ui_data_src_group_t add_group = NULL;
    
    ui_data_src_group_srcs(&src_it, group);
    while((src = ui_data_src_it_next(&src_it))) {
        struct ui_data_src_it child_src_it;
        ui_data_src_t child_src;
        
        if (ui_data_src_type(src) != ui_data_src_type_dir) continue;

        if (remove_group == NULL) remove_group = ui_data_src_group_create(group->m_mgr);
        ui_data_src_group_add_src(remove_group, src);

        if (add_group == NULL) add_group = ui_data_src_group_create(group->m_mgr);
        ui_data_src_all_childs(&child_src_it, src);
        while((child_src = ui_data_src_it_next(&child_src_it))) {
            if (ui_data_src_type(child_src) == ui_data_src_type_dir) continue;

            ui_data_src_group_add_src(add_group, child_src);
        }
    }

    if (remove_group) {
        ui_data_src_group_srcs(&src_it, remove_group);
        while((src = ui_data_src_it_next(&src_it))) {
            ui_data_src_group_remove_src(group, src);
        }
        ui_data_src_group_free(remove_group);
    }

    if (add_group) {
        ui_data_src_group_add_src_by_group(group, add_group);
        ui_data_src_group_free(add_group);
    }
    
    return rv;
}

int ui_data_src_group_collect_ress(ui_data_src_group_t group, ui_cache_group_t res_group) {
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    int rv = 0;
    
    ui_data_src_group_srcs(&src_it, group);
    while((src = ui_data_src_it_next(&src_it))) {
        struct ui_cache_res_it res_it;
        ui_cache_res_t res;
        
        ui_data_src_using_ress(src, &res_it);
        while((res = ui_cache_res_it_next(&res_it))) {
            if (ui_cache_group_add_res(res_group, res) != 0) rv = -1;
        }
    }

    return rv;
}
