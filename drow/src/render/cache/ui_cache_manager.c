#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "ui_cache_manager_i.h"
#include "ui_cache_color_i.h"
#include "ui_cache_group_i.h"
#include "ui_cache_res_i.h"
#include "ui_cache_res_plugin_i.h"
#include "ui_cache_worker_i.h"
#include "ui_cache_task_i.h"

static void ui_cache_manager_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_cache_manager = {
    "ui_cache_manager",
    ui_cache_manager_clear
};

ui_cache_manager_t
ui_cache_manager_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, uint32_t task_capacity, error_monitor_t em) {
    struct ui_cache_manager * cache_mgr;
    nm_node_t cache_mgr_node;

    assert(app);

    if (name == NULL) name = "ui_cache_manager";

    cache_mgr_node = nm_group_create(
        gd_app_nm_mgr(app), name,
        sizeof(struct ui_cache_manager)
        + sizeof(struct ui_cache_task) * task_capacity
        + sizeof(struct ui_cache_back_task) * task_capacity);
    if (cache_mgr_node == NULL) return NULL;

    cache_mgr = (ui_cache_manager_t)nm_node_data(cache_mgr_node);

    cache_mgr->m_app = app;
    cache_mgr->m_alloc = alloc;
    cache_mgr->m_em = em;
    cache_mgr->m_debug = 0;
    cache_mgr->m_texture_data_buff_keep = 0;
    cache_mgr->m_texture_scale = 0;
    
    cache_mgr->m_task_capacity = task_capacity;
    cache_mgr->m_task_wp = 0;
    cache_mgr->m_task_rp = 0;
    cache_mgr->m_tasks = (struct ui_cache_task *)(cache_mgr + 1);

    cache_mgr->m_back_task_wp = 0;
    cache_mgr->m_back_task_rp = 0;
    cache_mgr->m_back_tasks = (struct ui_cache_back_task *)(cache_mgr->m_tasks + task_capacity);

    bzero(cache_mgr->m_res_plugins, sizeof(cache_mgr->m_res_plugins));

    if (cpe_hash_table_init(
            &cache_mgr->m_colors,
            alloc,
            (cpe_hash_fun_t) ui_cache_color_hash,
            (cpe_hash_eq_t) ui_cache_color_eq,
            CPE_HASH_OBJ2ENTRY(ui_cache_color, m_hh),
            -1) != 0)
    {
        nm_node_free(cache_mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &cache_mgr->m_texturies_by_path,
            alloc,
            (cpe_hash_fun_t) ui_cache_res_path_hash,
            (cpe_hash_eq_t) ui_cache_res_path_eq,
            CPE_HASH_OBJ2ENTRY(ui_cache_res, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&cache_mgr->m_colors);
        nm_node_free(cache_mgr_node);
        return NULL;
    }

    if (pthread_mutex_init(&cache_mgr->m_task_mutex, NULL) != 0) {
        CPE_ERROR(cache_mgr->m_em, "%s: create: init mutex fail, errno=%d (%s)", name, errno, strerror(errno));
        cpe_hash_table_fini(&cache_mgr->m_texturies_by_path);
        cpe_hash_table_fini(&cache_mgr->m_colors);
        nm_node_free(cache_mgr_node);
        return NULL;
    }

    if (pthread_cond_init(&cache_mgr->m_task_cond, NULL) != 0) {
        CPE_ERROR(cache_mgr->m_em, "%s: create: init mutex fail, errno=%d (%s)", name, errno, strerror(errno));
        pthread_mutex_destroy(&cache_mgr->m_task_mutex);
        cpe_hash_table_fini(&cache_mgr->m_texturies_by_path);
        cpe_hash_table_fini(&cache_mgr->m_colors);
        nm_node_free(cache_mgr_node);
        return NULL;
    }

    if (gd_app_tick_add(app, ui_cache_task_main_thread_process, cache_mgr, 0) != 0) {
        CPE_ERROR(cache_mgr->m_em, "%s: create: add tick func fail", name);
        pthread_cond_destroy(&cache_mgr->m_task_cond);
        pthread_mutex_destroy(&cache_mgr->m_task_mutex);
        cpe_hash_table_fini(&cache_mgr->m_texturies_by_path);
        cpe_hash_table_fini(&cache_mgr->m_colors);
        nm_node_free(cache_mgr_node);
        return NULL;
    }

    TAILQ_INIT(&cache_mgr->m_ress);
    TAILQ_INIT(&cache_mgr->m_deleting_ress);
    TAILQ_INIT(&cache_mgr->m_groups);
    TAILQ_INIT(&cache_mgr->m_workers);

    mem_buffer_init(&cache_mgr->m_dump_buffer, alloc);

    nm_node_set_type(cache_mgr_node, &s_nm_node_type_ui_cache_manager);

    /*以下会自动调用clear */
    if (ui_cache_color_load_defaults(cache_mgr) != 0) {
        nm_node_free(cache_mgr_node);
        return NULL;
    }

    return cache_mgr;
}

static void ui_cache_manager_clear(nm_node_t node) {
    ui_cache_manager_t cache_mgr;
    uint8_t i;
    
    cache_mgr = nm_node_data(node);

    gd_app_tick_remove(cache_mgr->m_app, ui_cache_task_main_thread_process, cache_mgr);

    ui_cache_worker_free_all(cache_mgr);

    ui_cache_group_free_all(cache_mgr);

    ui_cache_res_free_all(cache_mgr);
    cpe_hash_table_fini(&cache_mgr->m_texturies_by_path);
    assert(TAILQ_EMPTY(&cache_mgr->m_ress));
    assert(TAILQ_EMPTY(&cache_mgr->m_deleting_ress));

    ui_cache_color_free_all(cache_mgr);
    cpe_hash_table_fini(&cache_mgr->m_colors);
    
    for(i = 0; i < CPE_ARRAY_SIZE(cache_mgr->m_res_plugins); ++i) {
        if (cache_mgr->m_res_plugins[i]) {
            ui_cache_res_plugin_free(cache_mgr->m_res_plugins[i]);
        }
        assert(cache_mgr->m_res_plugins[i] == NULL);
    }

    mem_buffer_clear(&cache_mgr->m_dump_buffer);

    pthread_cond_destroy(&cache_mgr->m_task_cond);
    pthread_mutex_destroy(&cache_mgr->m_task_mutex);
}

gd_app_context_t ui_cache_manager_app(ui_cache_manager_t cache_mgr) {
    return cache_mgr->m_app;
}

void ui_cache_manager_free(ui_cache_manager_t cache_mgr) {
    nm_node_t cache_mgr_node;
    assert(cache_mgr);

    cache_mgr_node = nm_node_from_data(cache_mgr);
    if (nm_node_type(cache_mgr_node) != &s_nm_node_type_ui_cache_manager) return;
    nm_node_free(cache_mgr_node);
}

ui_cache_manager_t
ui_cache_manager_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_cache_manager) return NULL;
    return (ui_cache_manager_t)nm_node_data(node);
}

ui_cache_manager_t
ui_cache_manager_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "ui_cache_manager";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_cache_manager) return NULL;
    return (ui_cache_manager_t)nm_node_data(node);
}

const char * ui_cache_manager_name(ui_cache_manager_t cache_mgr) {
    return nm_node_name(nm_node_from_data(cache_mgr));
}

uint32_t ui_cache_manager_runing_task_count(ui_cache_manager_t cache_mgr) {
    uint16_t rp = cache_mgr->m_task_rp;

    if (cache_mgr->m_task_wp < rp) {
        return cache_mgr->m_task_wp + cache_mgr->m_task_capacity - rp;
    }
    else {
        return cache_mgr->m_task_wp - rp;
    }
}

uint16_t ui_cache_manager_loading_count(ui_cache_manager_t mgr) {
    ui_cache_res_t res;
    uint16_t loading_count = 0;

    TAILQ_FOREACH(res, &mgr->m_ress, m_next_for_mgr) {
        switch(res->m_load_state) {
        case ui_cache_res_wait_load:
        case ui_cache_res_loading:
        case ui_cache_res_cancel_loading:
            loading_count++;
            break;
        default:
            break;
        }
    }

    return loading_count;
}

uint8_t ui_cache_manager_texture_data_buff_keep(ui_cache_manager_t cache_mgr) {
    return cache_mgr->m_texture_data_buff_keep;
}

void ui_cache_manager_set_texture_data_buff_keep(ui_cache_manager_t cache_mgr, uint8_t keep) {
    cache_mgr->m_texture_data_buff_keep = keep;
}

uint8_t ui_cache_manager_texture_scale(ui_cache_manager_t cache_mgr) {
    return cache_mgr->m_texture_scale;
}

void ui_cache_manager_set_texture_scale(ui_cache_manager_t cache_mgr, uint8_t scale) {
    cache_mgr->m_texture_scale = scale;
}

static ui_cache_res_t ui_cache_manager_res_next(struct ui_cache_res_it * it) {
    ui_cache_res_t * data = (ui_cache_res_t *)(it->m_data);
    ui_cache_res_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_mgr);
    return r;
}

void ui_cache_manager_ress(ui_cache_manager_t mgr, ui_cache_res_it_t it) {
    *(ui_cache_res_t *)(it->m_data) = TAILQ_FIRST(&mgr->m_ress);
    it->next = ui_cache_manager_res_next;
}

EXPORT_DIRECTIVE
int ui_cache_manager_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_cache_manager_t ui_cache_manager;
    uint32_t worker_count;
    uint32_t i;
    uint32_t task_capacity;

    task_capacity = cfg_get_uint32(cfg, "task-capacity", 512);

    ui_cache_manager =
        ui_cache_manager_create(
            app,
            gd_app_alloc(app),
            gd_app_module_name(module),
            task_capacity,
            gd_app_em(app));
    if (ui_cache_manager == NULL) return -1;

    ui_cache_manager->m_debug = cfg_get_int32(cfg, "debug", ui_cache_manager->m_debug);

    worker_count = cfg_get_uint32(cfg, "worker-count", 1);
    for(i = 0; i < worker_count; ++i) {
        ui_cache_worker_t worker = ui_cache_worker_create(ui_cache_manager);
        if (worker == NULL) {
            ui_cache_manager_free(ui_cache_manager);
            return -1;
        }
    }

    if (ui_cache_manager->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done, task-capacity=%d, worker=%d",
            ui_cache_manager_name(ui_cache_manager), task_capacity, worker_count);
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_cache_manager_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_cache_manager_t ui_cache_manager;

    ui_cache_manager = ui_cache_manager_find_nc(app, gd_app_module_name(module));
    if (ui_cache_manager) {
        ui_cache_manager_free(ui_cache_manager);
    }
}
