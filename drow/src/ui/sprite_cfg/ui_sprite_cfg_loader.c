#include <assert.h>
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui_sprite_cfg_loader_i.h"
#include "ui_sprite_cfg_resource_loader_i.h"
#include "ui_sprite_cfg_comp_loader_i.h"
#include "ui_sprite_cfg_action_loader_i.h"
#include "ui_sprite_cfg_context_i.h"

static void ui_sprite_cfg_loader_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_cfg_loader = {
    "ui_sprite_cfg_loader",
    ui_sprite_cfg_loader_clear
};

#define UI_SPRITE_2D_MODULE_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_ui_sprite_2d, __name); \
    assert(module-> __arg)

ui_sprite_cfg_loader_t
ui_sprite_cfg_loader_create(
    gd_app_context_t app,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_cfg_loader * module;
    nm_node_t module_node;

    assert(app);

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_cfg_loader));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_cfg_loader_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;

    if (cpe_hash_table_init(
            &module->m_resource_loaders,
            alloc,
            (cpe_hash_fun_t) ui_sprite_cfg_resource_loader_hash,
            (cpe_hash_eq_t) ui_sprite_cfg_resource_loader_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_cfg_resource_loader, m_hh_for_loader),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init hash table fail!", name);
        mem_free(module->m_alloc, module);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_comp_loaders,
            alloc,
            (cpe_hash_fun_t) ui_sprite_cfg_comp_loader_hash,
            (cpe_hash_eq_t) ui_sprite_cfg_comp_loader_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_cfg_comp_loader, m_hh_for_loader),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init hash table fail!", name);
        cpe_hash_table_fini(&module->m_resource_loaders);
        mem_free(module->m_alloc, module);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_action_loaders,
            alloc,
            (cpe_hash_fun_t) ui_sprite_cfg_action_loader_hash,
            (cpe_hash_eq_t) ui_sprite_cfg_action_loader_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_cfg_action_loader, m_hh_for_loader),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init hash table fail!", name);
        cpe_hash_table_fini(&module->m_comp_loaders);
        cpe_hash_table_fini(&module->m_resource_loaders);
        mem_free(module->m_alloc, module);
        return NULL;
    }

    TAILQ_INIT(&module->m_contexts);
    
    mem_buffer_init(&module->m_dump_buffer, module->m_alloc);

    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_cfg_loader);

    return module;
}

static void ui_sprite_cfg_loader_clear(nm_node_t node) {
    ui_sprite_cfg_loader_t module;
    module = (ui_sprite_cfg_loader_t)nm_node_data(node);

    ui_sprite_cfg_resource_loader_free_all(module);
    ui_sprite_cfg_comp_loader_free_all(module);
    ui_sprite_cfg_action_loader_free_all(module);

    cpe_hash_table_fini(&module->m_resource_loaders);
    cpe_hash_table_fini(&module->m_comp_loaders);
    cpe_hash_table_fini(&module->m_action_loaders);

    ui_sprite_cfg_context_free_all(module);
    
    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t ui_sprite_cfg_loader_app(ui_sprite_cfg_loader_t module) {
    return module->m_app;
}

void ui_sprite_cfg_loader_free(ui_sprite_cfg_loader_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_cfg_loader) return;
    nm_node_free(module_node);
}

ui_sprite_cfg_loader_t
ui_sprite_cfg_loader_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_cfg_loader) return NULL;
    return (ui_sprite_cfg_loader_t)nm_node_data(node);
}

ui_sprite_cfg_loader_t
ui_sprite_cfg_loader_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_cfg_loader";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_cfg_loader) return NULL;
    return (ui_sprite_cfg_loader_t)nm_node_data(node);
}

const char * ui_sprite_cfg_loader_name(ui_sprite_cfg_loader_t module) {
    return nm_node_name(nm_node_from_data(module));
}

int ui_sprite_cfg_loader_load_world_from_path(ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * src_path) {
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return -1;
    }

    return ui_sprite_cfg_loader_load_world_from_cfg(loader, world, cfg);
}

int ui_sprite_cfg_loader_load_world_from_cfg(ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, cfg_t cfg) {
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;

    cfg_it_init(&child_cfg_it, cfg_find_cfg(cfg, "groups"));
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        const char * group_name = cfg_get_string(child_cfg, "name", NULL);
        struct cfg_it join_group_it;
        ui_sprite_group_t group;

        if (group_name == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load word: group no name!",
                ui_sprite_cfg_loader_name(loader));
            return -1;
        }

        group = ui_sprite_group_create(world, group_name);
        if (group == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load word: create group %s fail!",
                ui_sprite_cfg_loader_name(loader), group_name);
            return -1;
        }

        cfg_it_init(&join_group_it, cfg_find_cfg(child_cfg, "join-groups"));
        while((child_cfg = cfg_it_next(&join_group_it))) {
            const char * join_group_name = cfg_as_string(child_cfg, NULL);
            ui_sprite_group_t join_group;

            if (join_group_name == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: load word: create group %s: join group format error!",
                    ui_sprite_cfg_loader_name(loader), group_name);
                return -1;
            }

            join_group = ui_sprite_group_find_by_name(world, join_group_name);
            if (join_group == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: load word: create group %s: join to group %s not exist!",
                    ui_sprite_cfg_loader_name(loader), group_name, join_group_name);
                return -1;
            }

            if (ui_sprite_group_add_group(join_group, group) != 0) {
                CPE_ERROR(
                    loader->m_em, "%s: load word: create group %s: join to group %s fail!",
                    ui_sprite_cfg_loader_name(loader), group_name, join_group_name);
                return -1;
            }
        }
    }

    cfg_it_init(&child_cfg_it, cfg_find_cfg(cfg, "resources"));
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        const char * res_name;
        cfg_t res_cfg;
        ui_sprite_world_res_t world_res;

        if (cfg_type(child_cfg) == CPE_CFG_TYPE_STRING) {
            res_name = cfg_as_string(child_cfg, NULL);
            res_cfg = NULL;

            assert(res_name);
        }
        else if (cfg_type(child_cfg) == CPE_CFG_TYPE_STRUCT) {
            child_cfg = cfg_child_only(child_cfg);
            if (child_cfg == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: load word: resource config format error!",
                    ui_sprite_cfg_loader_name(loader));
                return -1;
            }

            res_name = cfg_name(child_cfg);
            res_cfg = child_cfg;
        }
        else {
            CPE_ERROR(
                loader->m_em, "%s: load word: resource config format error!",
                ui_sprite_cfg_loader_name(loader));
            return -1;
        }

        world_res = ui_sprite_cfg_loader_load_resource_from_cfg(loader, world, res_name, res_cfg);
        if (world_res == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load word: create resource %s fail!",
                ui_sprite_cfg_loader_name(loader), res_name);
            return -1;
        }
    }

    cfg_it_init(&child_cfg_it, cfg_find_cfg(cfg, "protos"));
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        ui_sprite_entity_t proto_entity;

        proto_entity = ui_sprite_entity_proto_create(world, cfg_name(child_cfg));
        if (proto_entity == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load word: create proto entity %s error!",
                ui_sprite_cfg_loader_name(loader), cfg_name(child_cfg));
            return -1;
        }

        if (ui_sprite_cfg_loader_load_entity_from_cfg(loader, proto_entity, child_cfg) != 0) {
            ui_sprite_entity_free(proto_entity);
            return -1;
        }
    }

    cfg_it_init(&child_cfg_it, cfg_find_cfg(cfg, "entities"));
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        ui_sprite_entity_t entity;

        entity = ui_sprite_entity_create(world, cfg_name(child_cfg), cfg_get_string(child_cfg, "proto", NULL));
        if (entity == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load word: create entity %s error!",
                ui_sprite_cfg_loader_name(loader), cfg_name(child_cfg));
            return -1;
        }

        if (ui_sprite_cfg_loader_load_entity_from_cfg(loader, entity, child_cfg) != 0) {
            ui_sprite_entity_free(entity);
            return -1;
        }
    }

    return 0;
}

cfg_t ui_sprite_cfg_loader_find_cfg(ui_sprite_cfg_loader_t loader, const char * path) {
    ui_sprite_cfg_context_t context;

    TAILQ_FOREACH(context, &loader->m_contexts, m_next_for_loader) {
        cfg_t r = cfg_find_cfg(context->m_cfg, path);
        if (r) return r;
    }
    
    return NULL;
}

/*module*/
EXPORT_DIRECTIVE
int ui_sprite_cfg_loader_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_cfg_loader_t ui_sprite_cfg_loader;

    ui_sprite_cfg_loader =
        ui_sprite_cfg_loader_create(
            app,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_cfg_loader == NULL) return -1;

    if (cfg_get_int8(cfg, "load-default", 1)) {
        if (ui_sprite_cfg_loader_init_default_loaders(ui_sprite_cfg_loader) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: init default loaders fail",
                ui_sprite_cfg_loader_name(ui_sprite_cfg_loader));
            ui_sprite_cfg_loader_free(ui_sprite_cfg_loader);
            return -1;
        }
    }

    ui_sprite_cfg_loader->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_cfg_loader->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_cfg_loader_name(ui_sprite_cfg_loader));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_cfg_loader_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_cfg_loader_t ui_sprite_cfg_loader;

    ui_sprite_cfg_loader = ui_sprite_cfg_loader_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_cfg_loader) {
        ui_sprite_cfg_loader_free(ui_sprite_cfg_loader);
    }
}

