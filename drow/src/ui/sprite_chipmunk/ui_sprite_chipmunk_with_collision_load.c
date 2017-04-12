#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_collision_i.h"
#include "ui_sprite_chipmunk_env_i.h"
#include "ui_sprite_chipmunk_load_utils.h"

static ui_sprite_chipmunk_with_collision_shape_t
ui_sprite_chipmunk_with_collision_shape_load(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_env_t env,
    ui_sprite_chipmunk_with_collision_src_t src, cfg_t cfg);

static ui_sprite_chipmunk_with_collision_src_t
ui_sprite_chipmunk_with_collision_src_load(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_env_t env,
    ui_sprite_chipmunk_with_collision_t with_collision, cfg_t cfg);

ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_collision_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_collision_t with_collision = (ui_sprite_chipmunk_with_collision_t)ui_sprite_chipmunk_with_collision_create(fsm_state, name);
    ui_sprite_world_t world = ui_sprite_fsm_action_to_world(ui_sprite_fsm_action_from_data(with_collision));
    ui_sprite_chipmunk_env_t env = ui_sprite_chipmunk_env_find(world);
    struct cfg_it bodies_it;
    cfg_t body_cfg;

    if (with_collision == NULL) {
        CPE_ERROR(module->m_em, "%s: with_collision action: load: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    cfg_it_init(&bodies_it, cfg_find_cfg(cfg, "bodies"));
    while((body_cfg = cfg_it_next(&bodies_it))) {
        if (ui_sprite_chipmunk_with_collision_src_load(module, env, with_collision, body_cfg) == NULL) return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(with_collision);
}

static ui_sprite_chipmunk_with_collision_src_t
ui_sprite_chipmunk_with_collision_src_load(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_env_t env,
    ui_sprite_chipmunk_with_collision_t with_collision, cfg_t body_cfg)
{
    struct cfg_it shapes_it;
    cfg_t shape_cfg;
    const char * str_value;
    ui_sprite_chipmunk_with_collision_src_t src;

    src = ui_sprite_chipmunk_with_collision_src_create(with_collision);
    if (src == NULL) {
        CPE_ERROR(
            module->m_em, "%s: with_collision action: create src fail!",
            ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(body_cfg, "load-from", NULL))) {
        cpe_str_dup(src->m_res, sizeof(src->m_res), str_value);
    }
        
    if ((str_value = cfg_get_string(body_cfg, "name", NULL))) {
        cpe_str_dup(src->m_name, sizeof(src->m_name), str_value);
    }
    
    cfg_it_init(&shapes_it, cfg_find_cfg(body_cfg, "shapes"));
    while((shape_cfg = cfg_it_next(&shapes_it))) {
        if (ui_sprite_chipmunk_with_collision_shape_load(module, env, src, shape_cfg) == NULL) return NULL;
    }

    src->m_is_main = cfg_get_uint8(body_cfg, "is-main", 0);

    if (ui_sprite_chipmunk_load_body_attrs(env, &src->m_body_attrs, body_cfg) != 0) {
        CPE_ERROR(
            module->m_em, "%s: with_collision action: load body attrs fail!",
            ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    return src;
}

ui_sprite_chipmunk_with_collision_shape_t
ui_sprite_chipmunk_with_collision_shape_load(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_env_t env,
    ui_sprite_chipmunk_with_collision_src_t src, cfg_t shape_cfg)
{
    const char * shape_type;
    CHIPMUNK_FIXTURE * fixture_data;
    ui_sprite_chipmunk_with_collision_shape_t shape;

    shape = ui_sprite_chipmunk_with_collision_shape_create(src);
    if (shape == NULL) return NULL;
    
    fixture_data =  &shape->m_data;
        
    shape_type = cfg_get_string(shape_cfg, "type", NULL);
    if (shape_type == NULL) {
        CPE_ERROR(
            module->m_em, "%s: with_collision action: shape shaoe fail!",
            ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_with_collision_shape_free(shape);
        return NULL;
    }

    shape_type = cpe_str_trim_head((char*)shape_type);
    if (shape_type[0] == ':') {
        if (ui_sprite_chipmunk_with_collision_shape_set_shape_def(shape, shape_type) != 0) {
            CPE_ERROR(
                module->m_em, "%s: with_collision action: set shape type %s fail!",
                ui_sprite_chipmunk_module_name(module), shape_type);
            ui_sprite_chipmunk_with_collision_shape_free(shape);
            return NULL;
        }
    }
    else {
        if (ui_sprite_chipmunk_load_fixture_shape(env, fixture_data, shape_cfg) != 0) {
            CPE_ERROR(
                module->m_em, "%s: with_collision action: load shape fail fail!",
                ui_sprite_chipmunk_module_name(module));
            ui_sprite_chipmunk_with_collision_shape_free(shape);
            return NULL;
        }
    }

    if (ui_sprite_chipmunk_load_fixture_data(env, fixture_data, shape_cfg) != 0) {
        CPE_ERROR(
            module->m_em, "%s: with_collision action: load shape detail fail!",
            ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_with_collision_shape_free(shape);
        return NULL;
    }

    if (ui_sprite_chipmunk_with_collision_shape_set_density(shape, cfg_get_string(shape_cfg, "density", NULL)) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_mass(shape, cfg_get_string(shape_cfg, "mass", NULL)) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_elasticity(shape, cfg_get_string(shape_cfg, "elasticity", NULL)) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_friction(shape, cfg_get_string(shape_cfg, "friction", NULL)) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_collision_group(shape, cfg_get_string(shape_cfg, "collision_group", NULL)) != 0
        )
    {
        CPE_ERROR(
            module->m_em, "%s: with_collision action: load str calc fail!",
            ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_with_collision_shape_free(shape);
        return NULL;
    }

    return shape;
}
