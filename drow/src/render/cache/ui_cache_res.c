#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "render/cache/ui_cache_pixel_format.h"
#include "render/cache/ui_cache_texture.h"
#include "ui_cache_res_i.h"
#include "ui_cache_res_ref_i.h"
#include "ui_cache_res_plugin_i.h"
#include "ui_cache_group_i.h"
#include "ui_cache_pixel_buf_i.h"
#include "ui_cache_sound_buf_i.h"

ui_cache_res_t ui_cache_res_create(ui_cache_manager_t mgr, ui_cache_res_type_t res_type) {
    ui_cache_res_plugin_t plugin;
    ui_cache_res_t res;

    plugin = ui_cache_res_plugin_find_by_type(mgr, res_type);
    
    res = mem_alloc(mgr->m_alloc, sizeof(struct ui_cache_res) + (plugin ? plugin->m_capacity : 0));
    if (res == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create res: alloc fail!", ui_cache_manager_name(mgr));
        return NULL;
    }

    res->m_mgr = mgr;
    res->m_res_type = res_type;
    res->m_op_id = 0;
    res->m_path[0] = 0;
    res->m_load_state = ui_cache_res_not_load;
    res->m_load_result = ui_cache_res_load_success;
    res->m_ref_count = 0;
    res->m_deleting = 0;
    
    switch(res_type) {
    case ui_cache_res_type_texture:
        res->m_texture.m_format = ui_cache_pf_unknown;
        res->m_texture.m_width = 0;
        res->m_texture.m_height = 0;
        res->m_texture.m_keep_data_buf = mgr->m_texture_data_buff_keep;
        res->m_texture.m_scale = mgr->m_texture_scale;
        res->m_texture.m_need_part_update = 0;
        res->m_texture.m_is_dirty = 0;
        res->m_texture.m_data_buff = NULL;
        break;
    case ui_cache_res_type_sound:
        res->m_sound.m_format = ui_cache_sound_format_unknown;
        res->m_sound.m_data_buff = NULL;
        break;
    case ui_cache_res_type_font:
        res->m_font.m_data_size = 0;
        res->m_font.m_data = NULL;
        break;
    default:
        CPE_ERROR(mgr->m_em, "%s: create res: res type %d unknown!", ui_cache_manager_name(mgr), res_type);
        return NULL;
    }
    
    TAILQ_INIT(&res->m_in_groups);

    TAILQ_INSERT_TAIL(&mgr->m_ress, res, m_next_for_mgr);
    
    return res;
}

void ui_cache_res_free(ui_cache_res_t res) {
    ui_cache_manager_t mgr = res->m_mgr;

    assert(res->m_ref_count == 0);

    TAILQ_REMOVE(&mgr->m_ress, res, m_next_for_mgr);

    if (res->m_path[0]) {
        cpe_hash_table_remove_by_ins(&mgr->m_texturies_by_path, res);
    }

    ui_cache_res_do_unload(res, 0);

    while(!TAILQ_EMPTY(&res->m_in_groups)) {
        ui_cache_res_ref_free(TAILQ_FIRST(&res->m_in_groups));
    }

    if (res->m_deleting) {
        TAILQ_REMOVE(&mgr->m_deleting_ress, res, m_next_for_deleting);
    }
    
    mem_free(mgr->m_alloc, res);
}

ui_cache_res_type_t ui_cache_res_type(ui_cache_res_t res) {
    return res->m_res_type;
}

const char * ui_cache_res_load_state_to_str(ui_cache_res_load_state_t load_state) {
    switch(load_state) {
    case ui_cache_res_not_load:
        return "not load";
    case ui_cache_res_wait_load:
        return "wait load";
    case ui_cache_res_loading:
        return "loading";
    case ui_cache_res_data_loaded:
        return "data loaded";
    case ui_cache_res_loaded:
        return "loaded";
    case ui_cache_res_load_fail:
        return "load fail";
    case ui_cache_res_cancel_loading:
        return "cancel loading";
    default:
        return "unknown";
    }
}

int ui_cache_res_set_path(ui_cache_res_t res, const char * path) {
    ui_cache_manager_t mgr = res->m_mgr;

    if (ui_cache_res_find_by_path(mgr, path) != NULL) {
        CPE_ERROR(mgr->m_em, "%s: res %s already exist!", ui_cache_manager_name(mgr), path);
        return -1;
    }

    if (res->m_path[0]) {
        cpe_hash_table_remove_by_ins(&mgr->m_texturies_by_path, res);
    }

    if (path) {
        cpe_str_dup(res->m_path, sizeof(res->m_path), path);
    }
    else {
        res->m_path[0] = 0;
    }

    if (res->m_path[0]) {
        cpe_hash_entry_init(&res->m_hh_for_mgr);
        if (cpe_hash_table_insert_unique(&mgr->m_texturies_by_path, res) != 0) {
            CPE_ERROR(mgr->m_em, "%s: load res %s: insert fail!", ui_cache_manager_name(mgr), path);
            res->m_path[0] = 0;
            return -1;
        }
    }

    return 0;
}

ui_cache_res_t ui_cache_res_find_by_path(ui_cache_manager_t mgr, const char * path) {
    struct ui_cache_res key;

    cpe_str_dup(key.m_path, sizeof(key.m_path), path);

    return cpe_hash_table_find(&mgr->m_texturies_by_path, &key);
}

ui_cache_res_t ui_cache_res_create_by_path(ui_cache_manager_t mgr, const char * path) {
    ui_cache_res_type_t res_type;
    const char * suffix;
    ui_cache_res_t res;
    
    suffix = file_name_suffix(path);
    if (suffix == NULL) {
        CPE_ERROR(mgr->m_em, "%s: collect: res %s no suffix!", ui_cache_manager_name(mgr), path);
        return NULL;
    }

    if (strcmp(suffix, "ogg") == 0) {
        res_type = ui_cache_res_type_sound;
    }
    else if (strcmp(suffix, "ttf") == 0) {
        res_type = ui_cache_res_type_font;
    }
    else if (strcmp(suffix, "png") == 0 || strcmp(suffix, "pzd") == 0) {
        res_type = ui_cache_res_type_texture;
    }
    else {
        CPE_ERROR(
            mgr->m_em, "%s: collect: res %s suffix %s not support!",
            ui_cache_manager_name(mgr), path, suffix);
        return NULL;
    }

    res = ui_cache_res_create(mgr, res_type);

    return res;
}

ui_cache_res_t ui_cache_res_check_create_by_path(ui_cache_manager_t mgr, const char * path) {
    ui_cache_res_t res = ui_cache_res_find_by_path(mgr, path);
    if (res) return res;
    return ui_cache_res_create_by_path(mgr, path);
}

void ui_cache_res_free_all(ui_cache_manager_t mgr) {
    while(!TAILQ_EMPTY(&mgr->m_ress)) {
        ui_cache_res_free(TAILQ_FIRST(&mgr->m_ress));
    }
}

const char * ui_cache_res_path(ui_cache_res_t res) {
    return res->m_path;
}

uint32_t ui_cache_res_ref_count(ui_cache_res_t res) {
    return res->m_ref_count;
}

void ui_cache_res_ref_inc(ui_cache_res_t res) {
    res->m_ref_count++;
}

void ui_cache_res_ref_dec(ui_cache_res_t res) {
    assert(res->m_ref_count > 0);
    res->m_ref_count--;
}

void ui_cache_res_unload_data(ui_cache_res_t res) {
    switch(res->m_res_type) {
    case ui_cache_res_type_texture:
        if (res->m_texture.m_data_buff) {
            ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
            res->m_texture.m_data_buff = NULL;
        }
        break;
    case ui_cache_res_type_sound:
        if (res->m_sound.m_data_buff) {
            ui_cache_sound_buf_free(res->m_sound.m_data_buff);
            res->m_sound.m_data_buff = NULL;
        }
        break;
    case ui_cache_res_type_font:
        if (res->m_font.m_data) {
            mem_free(res->m_mgr->m_alloc, res->m_font.m_data);
            res->m_font.m_data = NULL;
        }
        res->m_font.m_data_size = 0;
        break;
    default:
        CPE_ERROR(
            res->m_mgr->m_em, "%s: unload res: res type %d unknown!",
            ui_cache_manager_name(res->m_mgr), res->m_res_type);
        assert(0);
        break;
    }
}

void ui_cache_res_do_unload(ui_cache_res_t res, uint8_t is_external_unload) {
    //TOassert(res->m_ref_count == 0);

    if (res->m_load_state == ui_cache_res_loaded) {
        ui_cache_res_plugin_t plugin = ui_cache_res_plugin_find_by_type(res->m_mgr, res->m_res_type);
        if (plugin && plugin->m_on_unload) plugin->m_on_unload(plugin->m_ctx, res, is_external_unload);
    }

    if (is_external_unload) {
        switch(res->m_res_type) {
        case ui_cache_res_type_texture:
            res->m_load_state = res->m_texture.m_data_buff ? ui_cache_res_data_loaded :  ui_cache_res_not_load;
            break;
        case ui_cache_res_type_sound:
            res->m_load_state = res->m_sound.m_data_buff ? ui_cache_res_data_loaded :  ui_cache_res_not_load;
            break;
        case ui_cache_res_type_font:
            res->m_load_state = res->m_font.m_data ? ui_cache_res_data_loaded :  ui_cache_res_not_load;
            break;
        default:
            assert(0);
            res->m_load_state = ui_cache_res_not_load;
            break;
        }
    }
    else {
        ui_cache_res_unload_data(res);
        res->m_load_state = ui_cache_res_not_load;
    }
}

uint8_t ui_cache_res_is_loading(ui_cache_res_t res) {
    switch(res->m_load_state) {
    case ui_cache_res_wait_load:
    case ui_cache_res_loading:
    case ui_cache_res_data_loaded:
        return 1;
    default:
        return 0;
    }
}

void ui_cache_res_unload(ui_cache_res_t res, uint8_t is_external_unload) {
CHECK_AGAIN:
    switch(res->m_load_state) {
    case ui_cache_res_not_load:
        break;
    case ui_cache_res_wait_load:
    case ui_cache_res_loading:
        pthread_mutex_lock(&res->m_mgr->m_task_mutex);
        if (res->m_load_state == ui_cache_res_wait_load) {
            switch(res->m_res_type) {
            case ui_cache_res_type_texture:
                if (res->m_texture.m_data_buff) {
                    ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
                    res->m_texture.m_data_buff = NULL;
                }
                break;
            case ui_cache_res_type_sound:
                if (res->m_sound.m_data_buff) {
                    ui_cache_sound_buf_free(res->m_sound.m_data_buff);
                    res->m_sound.m_data_buff = NULL;
                }
                break;
            case ui_cache_res_type_font:
                if (res->m_font.m_data) {
                    mem_free(res->m_mgr->m_alloc, res->m_font.m_data);
                    res->m_font.m_data = NULL;
                }
                res->m_font.m_data_size = 0;
                break;
            default:
                CPE_ERROR(res->m_mgr->m_em, "res %s: unload (in wait load): unknown res type %d", res->m_path, res->m_res_type);
                break;
            }
            
            res->m_load_state = ui_cache_res_not_load;
            pthread_mutex_unlock(&res->m_mgr->m_task_mutex);

            if (res->m_mgr->m_debug) {
                CPE_INFO(res->m_mgr->m_em, "res %s: unload (in wait load)", res->m_path);
            }
            
            break;
        }
        else if (res->m_load_state == ui_cache_res_loading) {
            res->m_load_state = ui_cache_res_cancel_loading;
            pthread_mutex_unlock(&res->m_mgr->m_task_mutex);

            if (res->m_mgr->m_debug) {
                CPE_INFO(res->m_mgr->m_em, "res %s: cancel loading", res->m_path);
            }

            break;
        }
        else {
            pthread_mutex_unlock(&res->m_mgr->m_task_mutex);
            goto CHECK_AGAIN;
        }
    case ui_cache_res_data_loaded:
    case ui_cache_res_loaded: {
        if (res->m_mgr->m_debug) {
            CPE_INFO(
                res->m_mgr->m_em, "res %s: unload (in %s)",
                res->m_path,
                res->m_load_state == ui_cache_res_data_loaded ? "data loaded" : "loaded");
        }

        ui_cache_res_do_unload(res, is_external_unload);
        break;
    }
    case ui_cache_res_load_fail:
        res->m_load_state = ui_cache_res_not_load;            
        break;
    default:
        break;
    }
}

uint8_t ui_cache_res_in_group(ui_cache_res_t res, ui_cache_group_t group) {
    ui_cache_res_ref_t ref;

    TAILQ_FOREACH(ref, &res->m_in_groups, m_next_for_res) {
        if (ref->m_group == group) return 1;
    }

    return 0;
}

ui_cache_res_using_state_t ui_cache_res_using_state(ui_cache_res_t res) {
    ui_cache_res_ref_t ref;
    ui_cache_res_using_state_t using_state = ui_cache_res_using_state_free;
    
    TAILQ_FOREACH(ref, &res->m_in_groups, m_next_for_res) {
        if (ref->m_group->m_res_using_state > using_state) {
            using_state = ref->m_group->m_res_using_state;
        }
    }

    return using_state;
}

void ui_cache_res_tag_delete(ui_cache_res_t res) {
    assert(!res->m_deleting);
    res->m_deleting = 1;
    TAILQ_INSERT_TAIL(&res->m_mgr->m_deleting_ress, res, m_next_for_deleting);
}

void ui_cache_res_free_deleting(ui_cache_manager_t mgr) {
    while(!TAILQ_EMPTY(&mgr->m_deleting_ress)) {
        ui_cache_res_free(TAILQ_FIRST(&mgr->m_deleting_ress));
    }
}

ui_cache_res_load_result_t ui_cache_res_load_result(ui_cache_res_t res) {
    return res->m_load_result;
}

ui_cache_res_load_state_t ui_cache_res_load_state(ui_cache_res_t res) {
    return res->m_load_state;
}

uint32_t ui_cache_res_path_hash(const ui_cache_res_t res) {
    return cpe_hash_str(res->m_path, strlen(res->m_path));
}

int ui_cache_res_path_eq(const ui_cache_res_t l, const ui_cache_res_t r) {
    return strcmp(l->m_path, r->m_path) == 0;
}

const char * ui_cache_res_type_to_str(ui_cache_res_type_t res_type) {
    switch(res_type) {
    case ui_cache_res_type_texture:
        return "texture";
    case ui_cache_res_type_sound:
        return "sound";
    case ui_cache_res_type_font:
        return "font";
    default:
        return "unknown";
    }
}
