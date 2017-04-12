#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui_sprite_cfg_loader_i.h"

int ui_sprite_cfg_loader_load_entity_from_path(
    ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, const char * src_path)
{
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return -1;
    }

    return ui_sprite_cfg_loader_load_entity_from_cfg(loader, entity, cfg);
}

static int ui_sprite_cfg_loader_load_entity_groups(ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, cfg_t cfg) {
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    struct cfg_it join_group_it;
    cfg_t child_cfg;

    cfg_it_init(&join_group_it, cfg);
    while((child_cfg = cfg_it_next(&join_group_it))) {
        const char * join_group_name = cfg_as_string(child_cfg, NULL);
        ui_sprite_group_t join_group;

        if (join_group_name == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): join group format error!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        join_group = ui_sprite_group_find_by_name(world, join_group_name);
        if (join_group == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): join  group %s not exist!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                join_group_name);
            return -1;
        }

        if (ui_sprite_group_add_entity(join_group, entity) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): join to group %s fail!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                join_group_name);
            return -1;
        }
    }

    return 0;
}

int ui_sprite_cfg_loader_load_entity_from_cfg(
    ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, cfg_t cfg)
{
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, cfg);

    if ((child_cfg = cfg_find_cfg(cfg, "import"))) {
        const char * import_from = cfg_as_string(child_cfg, NULL);
        cfg_t import_from_cfg = NULL;

        if (import_from == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): import format error!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        import_from_cfg = ui_sprite_cfg_loader_find_cfg(loader, import_from);
        if (import_from_cfg == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): import: path %s not exist!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), import_from);
            return -1;
        }

        if (ui_sprite_cfg_loader_load_entity_from_cfg(loader, entity, import_from_cfg) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): import: from %s fail!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), import_from);
            return -1;
        }
    }

    if ((child_cfg = cfg_find_cfg(cfg, "debug"))) {
        ui_sprite_entity_set_debug(entity, cfg_as_uint8(child_cfg, 0));
    }

    if ((child_cfg = cfg_find_cfg(cfg, "update-priority"))) {
        ui_sprite_entity_set_update_priority(entity, cfg_as_int8(child_cfg, 0));
    }

    if ((child_cfg = cfg_find_cfg(cfg, "join-groups"))) {
        if (ui_sprite_cfg_loader_load_entity_groups(loader, entity, child_cfg) != 0) return -1;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "components"))) {
        if (ui_sprite_cfg_loader_load_components_from_cfg(loader, entity, child_cfg) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): load components fail!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    if ((child_cfg = cfg_find_cfg(cfg, "attributes"))) {
        const char * attributes = cfg_as_string(child_cfg, NULL);

        if (attributes == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): attributes format error!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        if (ui_sprite_entity_bulk_set_attrs(entity, attributes, NULL) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: load entity %d(%s): set attributes(%s) fail!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), attributes);
            return -1;
        }
    }

    return 0;
}
