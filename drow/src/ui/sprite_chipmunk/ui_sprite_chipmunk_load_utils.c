#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui_sprite_chipmunk_load_utils.h"
#include "ui_sprite_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ui_sprite_chipmunk_build_layer_masks(ui_sprite_chipmunk_env_t env, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child;
    uint32_t mask = 0;

    cfg_it_init(&child_it, cfg);
    while((child = cfg_it_next(&child_it))) {
        const char * layer_name = cfg_as_string(child, NULL);
        if (layer_name == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_build_obj_type_masks: category format error!");
        }

        if (strcmp(layer_name, "*") == 0) return 0xFFFFFFFF;

        mask |= plugin_chipmunk_env_mask(env->m_env, layer_name);
    }

    return mask;
}

int ui_sprite_chipmunk_get_obj_type(ui_sprite_chipmunk_env_t env, chipmunk_obj_type_t * obj_type, const char * str_obj_type) {    
    if (strcmp(str_obj_type, "static") == 0) {
        *obj_type = chipmunk_obj_type_static;
        return 0;
    }
    else if (strcmp(str_obj_type, "kinematic") == 0) {
        *obj_type = chipmunk_obj_type_kinematic;
        return 0;
    }
    else if (strcmp(str_obj_type, "dynamic") == 0) {
        *obj_type = chipmunk_obj_type_dynamic;
        return 0;
    }
    else {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_get_obj_type: unknown obj type %s", str_obj_type);
        return -1;
    }
}

int ui_sprite_chipmunk_get_runing_mode(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_runing_mode_t * runint_mode, const char * str_runing_mode) {
    if (strcmp(str_runing_mode, "active") == 0) {
        *runint_mode = ui_sprite_chipmunk_runing_mode_active;
        return 0;
    }
    else if (strcmp(str_runing_mode, "passive") == 0) {
        *runint_mode = ui_sprite_chipmunk_runing_mode_passive;
        return 0;
    }
    else {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_get_runing_mode: unknown runing mode %s", str_runing_mode);
        return -1;
    }
}

int ui_sprite_chipmunk_load_fixture_data(ui_sprite_chipmunk_env_t env, CHIPMUNK_FIXTURE * fixture_data, cfg_t shape_cfg) {
    ui_sprite_chipmunk_module_t module = env->m_module;
    const char * str_value;
    cfg_t cfg_value;

    if ((str_value = cfg_get_string(shape_cfg, "collision.category", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &fixture_data->collision_category, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_load_fixture_data: load category from '%s' fail!", str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(shape_cfg, "collision.mask", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &fixture_data->collision_mask, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_load_detail: load mask from '%s' fail!", str_value);
            return -1;
        }
    }

    if ((cfg_value = cfg_find_cfg(shape_cfg, "collision.group"))) {
        fixture_data->collision_group = cfg_as_uint32(cfg_value, 0);
    }

    if ((cfg_value = cfg_find_cfg(shape_cfg, "density"))) {
        fixture_data->density = cfg_as_float(cfg_value, 0.0f);
    }
    
    if ((cfg_value = cfg_find_cfg(shape_cfg, "mass"))) {
        fixture_data->mass = cfg_as_float(cfg_value, 0.0f);
    }

    if ((cfg_value = cfg_find_cfg(shape_cfg, "elasticity"))) {
        fixture_data->elasticity = cfg_as_float(cfg_value, 0.0f);
    }

    if ((cfg_value = cfg_find_cfg(shape_cfg, "friction"))) {
        fixture_data->friction = cfg_as_float(cfg_value, 0.0f);
    }

    if ((cfg_value = cfg_find_cfg(shape_cfg, "surface-velocity"))) {
        fixture_data->surface_velocity.x = cfg_get_float(cfg_value, "x", 0.0f);
        fixture_data->surface_velocity.y = cfg_get_float(cfg_value, "y", 0.0f);
    }

    if ((cfg_value = cfg_find_cfg(shape_cfg, "is-sensor"))) {
        fixture_data->is_sensor = cfg_as_uint8(cfg_value, 0);
    }
    
    return 0;
}

int ui_sprite_chipmunk_load_body_attrs(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_body_attrs_t body_attrs, cfg_t body_cfg) {
    ui_sprite_chipmunk_module_t module = env->m_module;
    const char * str_value;
    cfg_t cfg_value;

    if ((str_value = cfg_get_string(body_cfg, "collision.category", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &body_attrs->m_category, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: load body attrs: category %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
        body_attrs->m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_CATEGORY;
    }

    if ((str_value = cfg_get_string(body_cfg, "collision.mask", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &body_attrs->m_mask, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: load body attrs: mask %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
        body_attrs->m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_MASK;
    }

    if ((cfg_value = cfg_find_cfg(body_cfg, "collision.group"))) {
        body_attrs->m_group = cfg_as_uint32(cfg_value, 0);
        body_attrs->m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_GROUP;
    }

    if ((cfg_value = cfg_find_cfg(body_cfg, "mass"))) {
        body_attrs->m_mass = cfg_as_float(cfg_value, 0);
        body_attrs->m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_MASS;
    }

    if ((cfg_value = cfg_find_cfg(body_cfg, "moment"))) {
        body_attrs->m_moment = cfg_as_float(cfg_value, 0);
        body_attrs->m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_MOMENT;
    }

    if ((cfg_value = cfg_find_cfg(body_cfg, "is-free"))) {
        body_attrs->m_is_free = cfg_as_uint8(cfg_value, body_attrs->m_is_free);
    }

    if ((cfg_value = cfg_find_cfg(body_cfg, "is-main"))) {
        body_attrs->m_is_main = cfg_as_uint8(cfg_value, body_attrs->m_is_main);
    }
    
    if ((str_value = cfg_get_string(body_cfg, "type", NULL))) {
        if (ui_sprite_chipmunk_get_obj_type(env, &body_attrs->m_type, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: load body attrs: type %s error!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }

        body_attrs->m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_TYPE;
    }

    if ((str_value = cfg_get_string(body_cfg, "runing-mode", NULL))) {
        if (ui_sprite_chipmunk_get_runing_mode(env, &body_attrs->m_runing_mode, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: load body attrs: load obj runing mode '%s' fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }
    else {
        if (body_attrs->m_runing_mode != ui_sprite_chipmunk_runing_mode_active
            && body_attrs->m_runing_mode != ui_sprite_chipmunk_runing_mode_passive)
        {
            body_attrs->m_runing_mode = ui_sprite_chipmunk_runing_mode_active;
        }
    }

    if ((cfg_value = cfg_find_cfg(body_cfg, "gravity"))) {
        if (ui_sprite_chipmunk_load_gravity(env, &body_attrs->m_gravity, cfg_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: load body attrs: load gravity fail!",
                ui_sprite_chipmunk_module_name(module));
            return -1;
        }
    }

    
    return 0;
}

int ui_sprite_chipmunk_load_gravity(ui_sprite_chipmunk_env_t env, UI_SPRITE_CHIPMUNK_GRAVITY * gravity, cfg_t body_cfg) {
    cfg_t child_cfg;
    float ptm = plugin_chipmunk_env_ptm(env->m_env);

    if ((child_cfg = cfg_find_cfg(body_cfg, "fix-value"))) {
        gravity->type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_FIX_VALUE;
        gravity->data.fix_value.gravity.x = cfg_get_float(child_cfg, "x", 0.0f) * ptm;
        gravity->data.fix_value.gravity.y = cfg_get_float(child_cfg, "y", 0.0f) * ptm;
    }
    else if ((child_cfg = cfg_find_cfg(body_cfg, "fix-size-value"))) {
        gravity->type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_FIX_SIZE_VALUE;
        gravity->data.fix_size_value.gravity = cfg_as_float(child_cfg, 0.0f) * ptm;
    }
    else if ((child_cfg = cfg_find_cfg(body_cfg, "adj-value"))) {
        gravity->type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_ADJ_VALUE;
        gravity->data.adj_value.adj_value = cfg_as_float(child_cfg, 0.0f) * ptm;
    }
    else if ((child_cfg = cfg_find_cfg(body_cfg, "native"))) {
        gravity->type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_NATIVE;
    }
    else {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_load_gravity: unknown type!");
        return -1;
    }

    return 0;
}

#define ui_sprite_chipmunk_load_fixture_shape_read_float(__arg, __str_arg, __dft) \
    if ((str_value = cpe_str_read_and_remove_arg(values, __str_arg, ',', '='))) { \
        if (cpe_str_to_float(&fixture_data->fixture_data. __arg , str_value)) { \
            CPE_ERROR(                                                  \
                module->m_em,                                           \
                "ui_sprite_chipmunk_load_fixture_shape_from_str: read arg %s from %s fail!", \
                __str_arg, str_value);                                  \
            return -1;                                                  \
        }                                                               \
    }                                                                   \
    else {                                                              \
        fixture_data->fixture_data. __arg = __dft;    \
    }
    
int ui_sprite_chipmunk_load_fixture_shape_from_str(
    ui_sprite_chipmunk_module_t module, CHIPMUNK_FIXTURE * fixture_data, const char * shape_def)
{
    char str_buf[256];
    char * values;
    char * shape_type;
    char * str_value;
    
    cpe_str_dup(str_buf, sizeof(str_buf), shape_def);
    values = strchr(str_buf, ':');
    if (values == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_load_fixture_shape_from_str: shape def %s format error!", shape_def);
        return -1;
    }

    * cpe_str_trim_tail(values, str_buf) = 0;

    shape_type = str_buf;
    values = values + 1;
    
    if (strcmp(shape_type, "box") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_box;
        ui_sprite_chipmunk_load_fixture_shape_read_float(box.lt.x, "lt.x", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(box.lt.y, "lt.y", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(box.rb.x, "rb.x", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(box.rb.y, "rb.y", 0.0f);
    }
    else if (strcmp(shape_type, "circle") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_circle;
        ui_sprite_chipmunk_load_fixture_shape_read_float(circle.radius, "radius", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(circle.position.x, "center.x", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(circle.position.y, "center.y", 0.0f);
    }
    else if (strcmp(shape_type, "entity-rect") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_entity_rect;
        ui_sprite_chipmunk_load_fixture_shape_read_float(entity_rect.adj.x, "adj.x", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(entity_rect.adj.y, "adj.y", 0.0f);
    }
    else if (strcmp(shape_type, "sector") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_sector;
        ui_sprite_chipmunk_load_fixture_shape_read_float(sector.center.x, "center.x", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(sector.center.y, "center.y", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(sector.radius, "radius", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(sector.angle_start, "angle-start", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(sector.angle_range, "angle-range", 0.0f);
        ui_sprite_chipmunk_load_fixture_shape_read_float(sector.angle_step, "angle-step", 3.0f);
    }
    else {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_load_fixture_shape_from_str: shape type %s is unknown!", shape_type);
        return -1;
    }

    return 0;
}

#define ui_sprite_chipmunk_load_shape_read_float(__arg, __str_arg, __dft) \
    if ((str_value = cpe_str_read_and_remove_arg(values, __str_arg, ',', '='))) { \
        if (cpe_str_to_float(&shape_data->shape_data. __arg , str_value)) { \
            CPE_ERROR(                                                  \
                module->m_em,                                           \
                "ui_sprite_chipmunk_load_shape_from_str: read arg %s from %s fail!", \
                __str_arg, str_value);                                  \
            return -1;                                                  \
        }                                                               \
    }                                                                   \
    else {                                                              \
        shape_data->shape_data. __arg = __dft;    \
    }
    
    
int ui_sprite_chipmunk_load_shape_from_str(ui_sprite_chipmunk_module_t module, CHIPMUNK_SHAPE * shape_data, const char * shape_def) {
    char str_buf[256];
    char * values;
    char * shape_type;
    char * str_value;
    
    cpe_str_dup(str_buf, sizeof(str_buf), shape_def);
    values = strchr(str_buf, ':');
    assert(values);

    * cpe_str_trim_tail(values, str_buf) = 0;

    shape_type = str_buf;
    values = values + 1;
    
    if (strcmp(shape_type, "box") == 0) {
        shape_data->shape_type = chipmunk_fixture_type_box;
        ui_sprite_chipmunk_load_shape_read_float(box.lt.x, "lt.x", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(box.lt.y, "lt.y", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(box.rb.x, "rb.x", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(box.rb.y, "rb.y", 0.0f);
    }
    else if (strcmp(shape_type, "circle") == 0) {
        shape_data->shape_type = chipmunk_fixture_type_circle;
        ui_sprite_chipmunk_load_shape_read_float(circle.radius, "radius", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(circle.position.x, "center.x", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(circle.position.y, "center.y", 0.0f);
    }
    else if (strcmp(shape_type, "entity-rect") == 0) {
        shape_data->shape_type = chipmunk_fixture_type_entity_rect;
        ui_sprite_chipmunk_load_shape_read_float(entity_rect.adj.x, "adj.x", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(entity_rect.adj.y, "adj.y", 0.0f);
    }
    else if (strcmp(shape_type, "sector") == 0) {
        shape_data->shape_type = chipmunk_fixture_type_sector;
        ui_sprite_chipmunk_load_shape_read_float(sector.center.x, "center.x", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(sector.center.y, "center.y", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(sector.radius, "radius", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(sector.angle_start, "angle-start", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(sector.angle_range, "angle-range", 0.0f);
        ui_sprite_chipmunk_load_shape_read_float(sector.angle_step, "angle-step", 3.0f);
    }
    else {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_load_shape_from_str: shape type %s is unknown!", shape_type);
        return -1;
    }

    return 0;
}

int ui_sprite_chipmunk_load_fixture_shape(ui_sprite_chipmunk_env_t env, CHIPMUNK_FIXTURE * fixture_data, cfg_t shape_cfg) {
    const char * shape_type;
    
    shape_type = cfg_get_string(shape_cfg, "type", NULL);
    if (shape_type == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_load_fixture_shape: shape type not configured!");
        return -1;
    }
    
    if (strcmp(shape_type, "box") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_box;
        fixture_data->fixture_data.box.lt.x = cfg_get_float(shape_cfg, "lt.x", 0.0f);
        fixture_data->fixture_data.box.lt.y = cfg_get_float(shape_cfg, "lt.y", 0.0f);
        fixture_data->fixture_data.box.rb.x = cfg_get_float(shape_cfg, "rb.x", 0.0f);
        fixture_data->fixture_data.box.rb.y = cfg_get_float(shape_cfg, "rb.y", 0.0f);
    }
    else if (strcmp(shape_type, "circle") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_circle;
        fixture_data->fixture_data.circle.radius = cfg_get_float(shape_cfg, "radius", 0.0f);
        fixture_data->fixture_data.circle.position.x = cfg_get_float(shape_cfg, "center.x", 0.0f);
        fixture_data->fixture_data.circle.position.y = cfg_get_float(shape_cfg, "center.y", 0.0f);
    }
    else if (strcmp(shape_type, "entity-rect") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_entity_rect;
        fixture_data->fixture_data.entity_rect.adj.x = cfg_get_float(shape_cfg, "adj.x", 0.0f);
        fixture_data->fixture_data.entity_rect.adj.y = cfg_get_float(shape_cfg, "adj.y", 0.0f);
    }
    else if (strcmp(shape_type, "sector") == 0) {
        fixture_data->fixture_type = chipmunk_fixture_type_sector;
        fixture_data->fixture_data.sector.center.x = cfg_get_float(shape_cfg, "center.x", 0.0f);
        fixture_data->fixture_data.sector.center.y = cfg_get_float(shape_cfg, "center.y", 0.0f);
        fixture_data->fixture_data.sector.radius = cfg_get_float(shape_cfg, "radius", 0.0f);
        fixture_data->fixture_data.sector.angle_start = cfg_get_float(shape_cfg, "angle-start", 0.0f);
        fixture_data->fixture_data.sector.angle_range = cfg_get_float(shape_cfg, "angle-range", 0.0f);
        fixture_data->fixture_data.sector.angle_step = cfg_get_float(shape_cfg, "angle-step", 3.0f);
    }
    else {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_chipmunk_load_fixture_shape: shape type %s is unknown!", shape_type);
        return -1;
    }

    return 0;
}
    
#ifdef __cplusplus
}
#endif

