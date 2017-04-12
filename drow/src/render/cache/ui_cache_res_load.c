#include <assert.h>
#ifndef _MSC_VER
#include <sched.h>
#else
#include <windows.h>
#endif
#include "cpe/pal/pal_stdio.h"
#include "ui_cache_res_i.h"
#include "ui_cache_res_plugin_i.h"
#include "ui_cache_pixel_buf_i.h"
#include "ui_cache_task_i.h"

int ui_cache_res_load_sync(ui_cache_res_t res, const char * root) {
    ui_cache_manager_t mgr = res->m_mgr;
    ui_cache_res_plugin_t plugin;
    
CHECK_AGAIN:
    switch(res->m_load_state) {
    case ui_cache_res_not_load:
        break;
    case ui_cache_res_wait_load:
    case ui_cache_res_loading:
    case ui_cache_res_cancel_loading:
        pthread_mutex_lock(&mgr->m_task_mutex);

        if (res->m_load_state == ui_cache_res_wait_load) {
            res->m_op_id++;
            res->m_load_state = ui_cache_res_not_load;
            pthread_mutex_unlock(&mgr->m_task_mutex);
            goto CHECK_AGAIN;
        }
        else if (res->m_load_state == ui_cache_res_loading) {
            pthread_mutex_unlock(&mgr->m_task_mutex);
#ifndef _MSC_VER
            sched_yield();
#else
            Sleep(0);
#endif
            ui_cache_task_main_thread_process(mgr, 0, 0.0f);
            goto CHECK_AGAIN;
        }
        else if (res->m_load_state == ui_cache_res_cancel_loading) {
            res->m_load_state = ui_cache_res_loading;
            pthread_mutex_unlock(&mgr->m_task_mutex);
            
#ifndef _MSC_VER
            sched_yield();
#else
            Sleep(0);
#endif
            ui_cache_task_main_thread_process(mgr, 0, 0.0f);
            goto CHECK_AGAIN;
        }
        else {
            pthread_mutex_unlock(&mgr->m_task_mutex);
            goto CHECK_AGAIN;
        }
    case ui_cache_res_data_loaded: {
        ui_cache_res_plugin_t plugin;

        switch(res->m_res_type) {
        case ui_cache_res_type_texture:
            assert(res->m_texture.m_data_buff);
            if (ui_cache_texture_load_from_buf(res, res->m_texture.m_data_buff) != 0) {
                CPE_ERROR(mgr->m_em, "res %s: load from buf fail", res->m_path);
                ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
                res->m_texture.m_data_buff = NULL;
                res->m_load_result = ui_cache_res_internal_error;
                return -1;
            }
            else {
                res->m_load_state = ui_cache_res_loaded;
            }
            break;
        case ui_cache_res_type_sound:
            res->m_load_state = ui_cache_res_loaded;
            break;
        case ui_cache_res_type_font:
            res->m_load_state = ui_cache_res_loaded;
            break;
        default:
            CPE_ERROR(
                mgr->m_em, "%s: res %s: unknown res type %d",
                ui_cache_manager_name(mgr), res->m_path, res->m_res_type);
            assert(0);
            return -1;
        }

        assert(res->m_load_state != ui_cache_res_data_loaded);
        
        plugin = ui_cache_res_plugin_find_by_type(mgr, res->m_res_type);
        if (plugin && plugin->m_on_load) {
            if (plugin->m_on_load(plugin->m_ctx, res) != 0) {
                CPE_ERROR(
                    mgr->m_em, "%s: res %s: plugin %s do load fail",
                    ui_cache_manager_name(mgr), res->m_path, plugin->m_name);
                ui_cache_res_do_unload(res, 0);
                return -1;
            }
            if (mgr->m_debug) {
                CPE_INFO(mgr->m_em, "cache: %s: loaded\n", ui_cache_res_path(res));
            }
        }
        
        goto CHECK_AGAIN;
    }
    case ui_cache_res_loaded:
        if (res->m_res_type == ui_cache_res_type_texture && res->m_texture.m_data_buff) {
            ui_cache_res_plugin_t plugin;
            
            if (ui_cache_texture_load_from_buf(res, res->m_texture.m_data_buff) != 0) {
                CPE_ERROR(mgr->m_em, "res %s: load from buf fail", res->m_path);
                ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
                res->m_texture.m_data_buff = NULL;
                res->m_load_result = ui_cache_res_internal_error;
                return -1;
            }

            plugin = ui_cache_res_plugin_find_by_type(mgr, res->m_res_type);
            if (plugin && plugin->m_on_load) {
                if (plugin->m_on_load(plugin->m_ctx, res) != 0) {
                    CPE_ERROR(
                        mgr->m_em, "%s: res %s: plugin %s do load fail",
                        ui_cache_manager_name(mgr), res->m_path, plugin->m_name);
                    ui_cache_res_do_unload(res, 0);
                    return -1;
                }
                if (mgr->m_debug) {
                    CPE_INFO(mgr->m_em, "cache: %s: reloaded\n", ui_cache_res_path(res));
                }
            }
        }
        return 0;
    case ui_cache_res_load_fail:
        CPE_ERROR(
            mgr->m_em, "%s: res %s: already faild, reason=%d",
            ui_cache_manager_name(mgr), res->m_path, res->m_load_result);
        return -1;
    default:
        CPE_ERROR(
            mgr->m_em, "%s: res %s: unknown load state %d",
            ui_cache_manager_name(mgr), res->m_path, res->m_load_state);
        return -1;
    }

    if(res->m_res_type == ui_cache_res_type_texture) {
        if (res->m_texture.m_data_buff == NULL) {
            if (res->m_path[0]) {
                if (ui_cache_texture_do_load(mgr, res, root) != 0) {
                    res->m_load_state = ui_cache_res_load_fail;
                    return -1;
                }
            }
        }

        if (res->m_texture.m_data_buff) {
            if (ui_cache_texture_load_from_buf(res, res->m_texture.m_data_buff) != 0) {
                CPE_ERROR(mgr->m_em, "res %s: load from buf fail", res->m_path);
                ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
                res->m_texture.m_data_buff = NULL;
                res->m_load_result = ui_cache_res_internal_error;
                return -1;
            }
        }
        
        res->m_load_result = ui_cache_res_load_success;
        res->m_load_state = ui_cache_res_loaded;
    }
    else if(res->m_res_type == ui_cache_res_type_sound) {
        if (ui_cache_sound_do_load(mgr, res, root) != 0) {
            res->m_load_state = ui_cache_res_load_fail;
            return -1;
        }

        res->m_load_result = ui_cache_res_load_success;
        res->m_load_state = ui_cache_res_loaded;
    }
    else if(res->m_res_type == ui_cache_res_type_font) {
        if (ui_cache_font_do_load(mgr, res, root) != 0) {
            res->m_load_state = ui_cache_res_load_fail;
            return -1;
        }

        res->m_load_result = ui_cache_res_load_success;
        res->m_load_state = ui_cache_res_loaded;
    }
    else {
        CPE_ERROR(
            mgr->m_em, "%s: res %s: unknown res type %d",
            ui_cache_manager_name(mgr), res->m_path, res->m_res_type);
        assert(0);
    }

    plugin = ui_cache_res_plugin_find_by_type(mgr, res->m_res_type);
    if (plugin && plugin->m_on_unload) {
        if (plugin->m_on_load(plugin->m_ctx, res) != 0) {
            CPE_ERROR(
                mgr->m_em, "%s: res %s: plugin %s do load fail",
                ui_cache_manager_name(mgr), res->m_path, plugin->m_name);
            res->m_load_state = ui_cache_res_data_loaded;
            ui_cache_res_do_unload(res, 0);
            return -1;
        }
    }

    if (res->m_mgr->m_debug) {
        CPE_INFO(res->m_mgr->m_em, "res %s: loaded(sync)", res->m_path);
    }
    
    return 0;
}

int ui_cache_res_load_async(ui_cache_res_t res, const char * root) {
    ui_cache_manager_t mgr = res->m_mgr;
    ui_cache_res_plugin_t plugin;
    
CHECK_AGAIN:
    switch(res->m_load_state) {
    case ui_cache_res_loaded:
    case ui_cache_res_wait_load:
    case ui_cache_res_loading:
        return 0;
    case ui_cache_res_cancel_loading:
        pthread_mutex_lock(&mgr->m_task_mutex);
        if (res->m_load_state == ui_cache_res_cancel_loading) {
            res->m_load_state = ui_cache_res_loading;
            pthread_mutex_unlock(&mgr->m_task_mutex);
            goto CHECK_AGAIN;
        }
        else {
            pthread_mutex_unlock(&mgr->m_task_mutex);
            goto CHECK_AGAIN;
        }
        break;
    case ui_cache_res_load_fail:
    case ui_cache_res_not_load:
        if (res->m_path[0] == 0) {
            res->m_load_state = ui_cache_res_data_loaded;
        }
        break;
    case ui_cache_res_data_loaded:
        switch(res->m_res_type) {
        case ui_cache_res_type_texture:
            if(res->m_texture.m_data_buff) {
                if (ui_cache_texture_load_from_buf(res, res->m_texture.m_data_buff) != 0) {
                    CPE_ERROR(mgr->m_em, "res %s: load from buf fail", res->m_path);
                    ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
                    res->m_texture.m_data_buff = NULL;
                    res->m_load_result = ui_cache_res_internal_error;
                    return -1;
                }
            }
            
            res->m_load_state = ui_cache_res_loaded;
            break;
        case ui_cache_res_type_sound:
            res->m_load_state = ui_cache_res_loaded;
            break;
        case ui_cache_res_type_font:
            res->m_load_state = ui_cache_res_loaded;
            break;
        default:
            CPE_ERROR(
                mgr->m_em, "%s: res %s: unknown res type %d",
                ui_cache_manager_name(mgr), res->m_path, res->m_res_type);
            assert(0);
            return -1;
        }

        plugin = ui_cache_res_plugin_find_by_type(mgr, res->m_res_type);
        if (plugin && plugin->m_on_load) {
            if (plugin->m_on_load(plugin->m_ctx, res) != 0) {
                CPE_ERROR(
                    mgr->m_em, "%s: res %s: plugin %s do load fail",
                    ui_cache_manager_name(mgr), res->m_path, plugin->m_name);
                ui_cache_res_do_unload(res, 0);
                return -1;
            }
            if (mgr->m_debug) {
                CPE_INFO(mgr->m_em, "cache: %s: loaded\n", ui_cache_res_path(res));
            }
        }
        
        assert(res->m_load_state != ui_cache_res_data_loaded);
        goto CHECK_AGAIN;
    default:
        assert(0);
        return -1;
    }

    if (res->m_mgr->m_debug) {
        CPE_INFO(res->m_mgr->m_em, "res %s: load begin (async)", res->m_path);
    }
    
    if (ui_cache_add_load_res_task(mgr, res, root) != 0) {
        assert(res->m_load_state == ui_cache_res_not_load || res->m_load_state == ui_cache_res_load_fail);
        return -1;
    }

    return 0;
}
