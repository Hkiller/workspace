#include "cpe/cfg/cfg_read.h"
#include "ui_sprite_cfg_resource_loader_i.h"

ui_sprite_world_res_t
ui_sprite_cfg_loader_load_resource_from_path(ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * res_name, const char * src_path) {
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return NULL;
    }

    return ui_sprite_cfg_loader_load_resource_from_cfg(loader, world, res_name, cfg);
}

ui_sprite_world_res_t
ui_sprite_cfg_loader_load_resource_from_cfg(ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * res_name, cfg_t cfg) {
    ui_sprite_cfg_resource_loader_t resource_loader;
    ui_sprite_world_res_t world_res;

    resource_loader = ui_sprite_cfg_resource_loader_find(loader, res_name);
    if (resource_loader == NULL) {
        CPE_ERROR(loader->m_em, "%s: resource loader %s not exist!", ui_sprite_cfg_loader_name(loader), res_name);
        return NULL;
    }

    world_res = resource_loader->m_fun(resource_loader->m_ctx, world, cfg);
    if (world_res == NULL) {
        if (loader->m_debug) {
            CPE_ERROR(
                loader->m_em, "%s: resource %s load from [%s] fail!",
                ui_sprite_cfg_loader_name(loader), res_name, cfg_dump_inline(cfg, &loader->m_dump_buffer));
        }
        else {
            CPE_ERROR(
                loader->m_em, "%s: resource %s load fail!",
                ui_sprite_cfg_loader_name(loader), res_name);
        }

        return NULL;
    }

    return world_res;
}

int ui_sprite_cfg_loader_add_resource_loader(ui_sprite_cfg_loader_t loader, const char * name, ui_sprite_cfg_load_resource_fun_t fun, void * ctx) {
    ui_sprite_cfg_resource_loader_t resource = ui_sprite_cfg_resource_loader_create(loader, name, ctx, fun);

    if (resource == NULL) {
        CPE_ERROR(loader->m_em, "add resource loader %s fail!", name);
        return -1;
    }

    return 0;
}

int ui_sprite_cfg_loader_remove_resource_loader(ui_sprite_cfg_loader_t loader, const char * name) {
    ui_sprite_cfg_resource_loader_t resource = ui_sprite_cfg_resource_loader_find(loader, name);
    if (resource == NULL) return -1;

    ui_sprite_cfg_resource_loader_free(resource);
    return 0;
}


/*impl*/
ui_sprite_cfg_resource_loader_t
ui_sprite_cfg_resource_loader_create(
    ui_sprite_cfg_loader_t loader, const char * name,
    void * ctx, ui_sprite_cfg_load_resource_fun_t fun)
{
    ui_sprite_cfg_resource_loader_t comp;
    size_t name_len = strlen(name) + 1;

    comp = mem_alloc(loader->m_alloc, sizeof(struct ui_sprite_cfg_resource_loader) + name_len);
    if (comp == NULL) {
        CPE_ERROR(loader->m_em, "create ui_sprite_cfg_resource_loader_loader: alloc fail!");
        return NULL;
    }

    memcpy(comp + 1, name, name_len);

    comp->m_loader = loader;
    comp->m_name = (char *)(comp + 1);
    comp->m_fun = fun;
    comp->m_ctx = ctx;

    cpe_hash_entry_init(&comp->m_hh_for_loader);
    if (cpe_hash_table_insert_unique(&loader->m_resource_loaders, comp) != 0) {
        CPE_ERROR(loader->m_em, "create ui_sprite_cfg_resource_loader: name %s duplicate!", name);
        mem_free(loader->m_alloc, comp);
        return NULL;
    }

    return comp;
}

void ui_sprite_cfg_resource_loader_free(ui_sprite_cfg_resource_loader_t comp) {
    ui_sprite_cfg_loader_t loader = comp->m_loader;

    cpe_hash_table_remove_by_ins(&loader->m_resource_loaders, comp);

    mem_free(loader->m_alloc, comp);
}

ui_sprite_cfg_resource_loader_t ui_sprite_cfg_resource_loader_find(ui_sprite_cfg_loader_t loader, const char * name) {
    struct ui_sprite_cfg_resource_loader key;
    key.m_name = name;
    return cpe_hash_table_find(&loader->m_resource_loaders, &key);
}

void ui_sprite_cfg_resource_loader_free_all(ui_sprite_cfg_loader_t loader) {
    struct cpe_hash_it resource_loader_it;
    ui_sprite_cfg_resource_loader_t resource_loader;

    cpe_hash_it_init(&resource_loader_it, &loader->m_resource_loaders);

    resource_loader = cpe_hash_it_next(&resource_loader_it);
    while (resource_loader) {
        ui_sprite_cfg_resource_loader_t next = cpe_hash_it_next(&resource_loader_it);
        ui_sprite_cfg_resource_loader_free(resource_loader);
        resource_loader = next;
    }
}

uint32_t ui_sprite_cfg_resource_loader_hash(const ui_sprite_cfg_resource_loader_t loader) {
    return cpe_hash_str(loader->m_name, strlen(loader->m_name));
}

int ui_sprite_cfg_resource_loader_eq(const ui_sprite_cfg_resource_loader_t l, const ui_sprite_cfg_resource_loader_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
