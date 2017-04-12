#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_cfg_comp_loader_i.h"

int ui_sprite_cfg_loader_load_components_from_path(
    ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, const char * src_path)
{
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return -1;
    }

    return ui_sprite_cfg_loader_load_components_from_cfg(loader, entity, cfg);
}

int ui_sprite_cfg_loader_load_components_from_cfg(ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, cfg_t cfg) {
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;

    cfg_it_init(&child_cfg_it, cfg);

    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        ui_sprite_component_t component;
        const char * component_type;
        cfg_t component_args;

        if (cfg_type(child_cfg) == CPE_CFG_TYPE_STRING) {
            component_type = cfg_as_string(child_cfg, NULL);
            component_args = NULL;
        }
        else {
            child_cfg = cfg_child_only(child_cfg);
            if (child_cfg == NULL) {
                CPE_ERROR(
                    loader->m_em, "entity %d(%s): load componetns: format error!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }

            component_type = cfg_name(child_cfg);
            component_args = child_cfg;
        }

        component = 
            ui_sprite_cfg_loader_load_component_from_cfg(
                loader, entity, component_type, component_args);
        if (component == NULL) return -1;
    }

    return 0;
}

ui_sprite_component_t
ui_sprite_cfg_loader_load_component_from_path(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_entity_t entity, const char * comp_name, const char * src_path)
{
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return NULL;
    }

    return ui_sprite_cfg_loader_load_component_from_cfg(loader, entity, comp_name, cfg);
}

ui_sprite_component_t
ui_sprite_cfg_loader_load_component_from_cfg(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_entity_t entity, const char * comp_name, cfg_t cfg)
{
    ui_sprite_cfg_comp_loader_t comp_loader;
    ui_sprite_component_t comp;

    comp_loader = ui_sprite_cfg_comp_loader_find(loader, comp_name);
    if (comp_loader == NULL) {
        CPE_ERROR(loader->m_em, "%s: comp loader %s not exist!", ui_sprite_cfg_loader_name(loader), comp_name);
        return NULL;
    }

    comp =  ui_sprite_component_find(entity, comp_name);
    if (comp == NULL) {
        comp = ui_sprite_component_create(entity, comp_name);
        if (comp == NULL) { 
            CPE_ERROR(
                loader->m_em, "%s: comp %s create in entity %d(%s) fail!",
                ui_sprite_cfg_loader_name(loader), comp_name,
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return NULL;
        }
    }

    if (comp_loader->m_fun(comp_loader->m_ctx, comp, cfg) != 0) {
        if (loader->m_debug) {
            CPE_ERROR(
                loader->m_em, "%s: comp %s load from [%s] fail!",
                ui_sprite_cfg_loader_name(loader), comp_name, cfg_dump_inline(cfg, &loader->m_dump_buffer));
        }
        else {
            CPE_ERROR(
                loader->m_em, "%s: comp %s load fail!",
                ui_sprite_cfg_loader_name(loader), comp_name);
        }

        return NULL;
    }

    return comp;
}


int ui_sprite_cfg_loader_add_comp_loader(ui_sprite_cfg_loader_t loader, const char * name, ui_sprite_cfg_load_comp_fun_t fun, void * ctx) {
    ui_sprite_cfg_comp_loader_t comp = ui_sprite_cfg_comp_loader_create(loader, name, ctx, fun);

    if (comp == NULL) {
        CPE_ERROR(loader->m_em, "add comp loader %s fail!", name);
        return -1;
    }
    
    return 0;
}

int ui_sprite_cfg_loader_remove_comp_loader(ui_sprite_cfg_loader_t loader, const char * name) {
    ui_sprite_cfg_comp_loader_t comp = ui_sprite_cfg_comp_loader_find(loader, name);
    if (comp == NULL) return -1;

    ui_sprite_cfg_comp_loader_free(comp);
    return 0;
}

/*impl*/
ui_sprite_cfg_comp_loader_t
ui_sprite_cfg_comp_loader_create(
    ui_sprite_cfg_loader_t loader, const char * name,
    void * ctx, ui_sprite_cfg_load_comp_fun_t fun)
{
    ui_sprite_cfg_comp_loader_t comp;
    size_t name_len = strlen(name) + 1;

    comp = mem_alloc(loader->m_alloc, sizeof(struct ui_sprite_cfg_comp_loader) + name_len);
    if (comp == NULL) {
        CPE_ERROR(loader->m_em, "create ui_sprite_cfg_comp_loader_loader: alloc fail!");
        return NULL;
    }

    memcpy(comp + 1, name, name_len);

    comp->m_loader = loader;
    comp->m_name = (char *)(comp + 1);
    comp->m_fun = fun;
    comp->m_ctx = ctx;

    cpe_hash_entry_init(&comp->m_hh_for_loader);
    if (cpe_hash_table_insert_unique(&loader->m_comp_loaders, comp) != 0) {
        CPE_ERROR(loader->m_em, "create ui_sprite_cfg_comp_loader: name %s duplicate!", name);
        mem_free(loader->m_alloc, comp);
        return NULL;
    }

    return comp;
}

void ui_sprite_cfg_comp_loader_free(ui_sprite_cfg_comp_loader_t comp) {
    ui_sprite_cfg_loader_t loader = comp->m_loader;

    cpe_hash_table_remove_by_ins(&loader->m_comp_loaders, comp);

    mem_free(loader->m_alloc, comp);
}

ui_sprite_cfg_comp_loader_t ui_sprite_cfg_comp_loader_find(ui_sprite_cfg_loader_t loader, const char * name) {
    struct ui_sprite_cfg_comp_loader key;
    key.m_name = name;
    return cpe_hash_table_find(&loader->m_comp_loaders, &key);
}

void ui_sprite_cfg_comp_loader_free_all(ui_sprite_cfg_loader_t loader) {
    struct cpe_hash_it comp_loader_it;
    ui_sprite_cfg_comp_loader_t comp_loader;

    cpe_hash_it_init(&comp_loader_it, &loader->m_comp_loaders);

    comp_loader = cpe_hash_it_next(&comp_loader_it);
    while (comp_loader) {
        ui_sprite_cfg_comp_loader_t next = cpe_hash_it_next(&comp_loader_it);
        ui_sprite_cfg_comp_loader_free(comp_loader);
        comp_loader = next;
    }
}

uint32_t ui_sprite_cfg_comp_loader_hash(const ui_sprite_cfg_comp_loader_t loader) {
    return cpe_hash_str(loader->m_name, strlen(loader->m_name));
}

int ui_sprite_cfg_comp_loader_eq(const ui_sprite_cfg_comp_loader_t l, const ui_sprite_cfg_comp_loader_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
