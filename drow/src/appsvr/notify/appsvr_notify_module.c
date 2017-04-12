#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "appsvr_notify_module_i.h"
#include "appsvr_notify_schedule_i.h"
#include "appsvr_notify_activate_i.h"
#include "appsvr_notify_tag_i.h"
#include "appsvr_notify_tag_schedule_i.h"
#include "appsvr_notify_adapter_i.h"

static void appsvr_notify_module_clear(nm_node_t node);
static ptr_int_t appsvr_notify_module_tick(void * ctx, ptr_int_t arg, float delta_s);

struct nm_node_type s_nm_node_type_appsvr_notify_module = {
    "appsvr_notify_module",
    appsvr_notify_module_clear
};

/* static struct { */
/*     const char * name;  */
/*     int (*init)(appsvr_notify_module_t module); */
/*     void (*fini)(appsvr_notify_module_t module); */
/* } s_auto_reg_products[] = { */
/*     { "executor", appsvr_notify_executor_init, appsvr_notify_executor_fini }     */
/* }; */

appsvr_notify_module_t
appsvr_notify_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_notify_module_t module;
    nm_node_t module_node;
    
    assert(app);

    if (name == NULL) name = "appsvr_notify_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_notify_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_notify_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    
    module->m_schedule_count = 0;
    module->m_schedule_max_id = 0;
    
    TAILQ_INIT(&module->m_adapters);
    TAILQ_INIT(&module->m_tags);
    TAILQ_INIT(&module->m_free_tag_schedules);
    TAILQ_INIT(&module->m_schedules_to_process);
    TAILQ_INIT(&module->m_schedules);
    TAILQ_INIT(&module->m_free_schedules);
    TAILQ_INIT(&module->m_free_activates);

    if (gd_app_tick_add(module->m_app, appsvr_notify_module_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "appsvr_notify_module: add tick fail!");
        nm_node_free(module_node);
        return NULL;
    }
    
    if (appsvr_notify_tag_create(module, "default") == NULL) {
        CPE_ERROR(em, "appsvr_notify_module_create: create default tag fail!");
        gd_app_tick_remove(module->m_app, appsvr_notify_module_tick, module);
        nm_node_free(module_node);
        return NULL;
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_appsvr_notify_module);

    return module;
}

static void appsvr_notify_module_clear(nm_node_t node) {
    appsvr_notify_module_t module = nm_node_data(node);

    gd_app_tick_remove(module->m_app, appsvr_notify_module_tick, module);
    
    while(!TAILQ_EMPTY(&module->m_schedules)) {
        appsvr_notify_schedule_free(TAILQ_FIRST(&module->m_schedules));
    }
    assert(TAILQ_EMPTY(&module->m_schedules_to_process));
    
    while(!TAILQ_EMPTY(&module->m_tags)) {
        appsvr_notify_tag_free(TAILQ_FIRST(&module->m_tags));
    }
    
    while(!TAILQ_EMPTY(&module->m_adapters)) {
        appsvr_notify_adapter_free(TAILQ_FIRST(&module->m_adapters));
    }

    while(!TAILQ_EMPTY(&module->m_free_tag_schedules)) {
        appsvr_notify_tag_schedule_real_free(TAILQ_FIRST(&module->m_free_tag_schedules));
    }

    while(!TAILQ_EMPTY(&module->m_free_schedules)) {
        appsvr_notify_schedule_real_free(TAILQ_FIRST(&module->m_free_schedules));
    }

    while(!TAILQ_EMPTY(&module->m_free_activates)) {
        appsvr_notify_activate_real_free(TAILQ_FIRST(&module->m_free_activates));
    }
}

gd_app_context_t appsvr_notify_module_app(appsvr_notify_module_t module) {
    return module->m_app;
}

void appsvr_notify_module_free(appsvr_notify_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_notify_module) return;
    nm_node_free(module_node);
}

appsvr_notify_module_t
appsvr_notify_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_notify_module) return NULL;
    return (appsvr_notify_module_t)nm_node_data(node);
}

appsvr_notify_module_t
appsvr_notify_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_notify_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_notify_module) return NULL;
    return (appsvr_notify_module_t)nm_node_data(node);
}

const char * appsvr_notify_module_name(appsvr_notify_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

static void appsvr_notify_module_clear_in_tag(void * ctx, const char * value) {
    appsvr_notify_module_t module = ctx;
    appsvr_notify_tag_t tag;

    tag = appsvr_notify_tag_find(module, value);
    if (tag == NULL) return;

    appsvr_notify_tag_clear(tag);
}

void appsvr_notify_module_clear_in_tags(appsvr_notify_module_t module, const char * tags) {
    cpe_str_list_for_each(tags, ':', appsvr_notify_module_clear_in_tag, module);
}

static ptr_int_t appsvr_notify_module_tick(void * ctx, ptr_int_t arg, float delta_s) {
    appsvr_notify_module_t module = ctx;

    appsvr_notify_schedule_tick(module);

    return 0;
}

EXPORT_DIRECTIVE
int appsvr_notify_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_notify_module_t ad;
    
    ad =
        appsvr_notify_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ad == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_notify_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_notify_module_t appsvr_notify_module;

    appsvr_notify_module = appsvr_notify_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_notify_module) {
        appsvr_notify_module_free(appsvr_notify_module);
    }
}
