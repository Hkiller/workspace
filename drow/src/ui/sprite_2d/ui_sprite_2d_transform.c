#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_rect.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui_sprite_2d_transform_i.h"
#include "ui_sprite_2d_part_i.h"

ui_sprite_2d_transform_t ui_sprite_2d_transform_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_2D_TRANSFORM_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_2d_transform_t ui_sprite_2d_transform_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_2D_TRANSFORM_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_2d_transform_free(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_2D_TRANSFORM_NAME);
    if (component) {
        ui_sprite_component_free(component);
    }
}

static int ui_sprite_2d_transform_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_2d_transform_t transform = ui_sprite_component_data(component);

    bzero(transform, sizeof(*transform));

    transform->m_module = ctx;
    transform->m_data.transform.scale.x = 1.0f;
    transform->m_data.transform.scale.y = 1.0f;
    
    TAILQ_INIT(&transform->m_parts);

    return 0;
}

static void ui_sprite_2d_transform_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_2d_transform_t transform = ui_sprite_component_data(component);
    
    while(!TAILQ_EMPTY(&transform->m_parts)) {
        ui_sprite_2d_part_free(TAILQ_FIRST(&transform->m_parts));
    }
}

static int ui_sprite_2d_transform_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_2d_module_t module = (ui_sprite_2d_module_t)ctx;
    ui_sprite_2d_transform_t from_transform = (ui_sprite_2d_transform_t)ui_sprite_component_data(from);
    ui_sprite_2d_transform_t to_transform = (ui_sprite_2d_transform_t)ui_sprite_component_data(to);

    to_transform->m_module = module;
    to_transform->m_data = from_transform->m_data;

    TAILQ_INIT(&to_transform->m_parts);

    return 0;
}

static int ui_sprite_2d_transform_load(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_2d_transform_t transform = ui_sprite_component_data(component);
    cfg_t child_cfg;

    if ((child_cfg = cfg_find_cfg(cfg, "pos"))) {
        ui_vector_2 pos = ui_sprite_2d_transform_origin_pos(transform);
        pos.x = cfg_get_float(child_cfg, "x", pos.x);
        pos.y = cfg_get_float(child_cfg, "y", pos.y);
        ui_sprite_2d_transform_set_origin_pos(transform, pos);
    }

    if ((child_cfg = cfg_find_cfg(cfg, "flip"))) {
        uint8_t flip_x = cfg_get_uint8(child_cfg, "x", ui_sprite_2d_transform_flip_x(transform));
        uint8_t flip_y = cfg_get_uint8(child_cfg, "y", ui_sprite_2d_transform_flip_y(transform));
        ui_sprite_2d_transform_set_flip(transform, flip_x, flip_y);
    }

    if ((child_cfg = cfg_find_cfg(cfg, "scale"))) {
        ui_vector_2 scale = ui_sprite_2d_transform_scale_pair(transform);
        scale.x = cfg_get_float(child_cfg, "x", scale.x);
        scale.y = cfg_get_float(child_cfg, "y", scale.y);
        ui_sprite_2d_transform_set_scale_pair(transform, scale);
    }

    if ((child_cfg = cfg_find_cfg(cfg, "angle"))) {
        ui_sprite_2d_transform_set_angle(
            transform,
            cfg_as_float(child_cfg, ui_sprite_2d_transform_angle(transform)));
    }

	if ((child_cfg = cfg_find_cfg(cfg, "rect"))) {
		ui_rect rect;
		rect.lt.x = cfg_get_float(child_cfg, "lt.x", 0.0f);
		rect.lt.y = cfg_get_float(child_cfg, "lt.y", 0.0f);
		rect.rb.x = cfg_get_float(child_cfg, "rb.x", 0.0f);
		rect.rb.y = cfg_get_float(child_cfg, "rb.y", 0.0f);
		ui_sprite_2d_transform_merge_rect(transform, &rect);
	}

    return 0;
}

int ui_sprite_2d_transform_regist(ui_sprite_2d_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_2D_TRANSFORM_NAME, sizeof(struct ui_sprite_2d_transform));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRANSFORM_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_data_meta(
        meta,
        module->m_meta_transform_data,
        CPE_ENTRY_START(ui_sprite_2d_transform, m_data),
        CPE_ENTRY_SIZE(ui_sprite_2d_transform, m_data));

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_2d_transform_init, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_2d_transform_fini, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_2d_transform_copy, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_comp_loader(
                module->m_loader, UI_SPRITE_2D_TRANSFORM_NAME, ui_sprite_2d_transform_load, module)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: %s register: register loader fail",
                ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRANSFORM_NAME);
            ui_sprite_component_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_transform_unregist(ui_sprite_2d_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_2D_TRANSFORM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRANSFORM_NAME);
        return;
    }

    ui_sprite_component_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_2D_TRANSFORM_NAME);
    }
}

ui_vector_2 ui_sprite_2d_transform_local_pos(ui_sprite_2d_transform_t transform, uint8_t pos_policy, uint8_t adj_type) {
    ui_vector_2 r;

    switch(pos_policy) {
    case UI_SPRITE_2D_TRANSFORM_POS_F_TOP_LEFT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT);
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_TOP_CENTER:
        pos_policy = 
            transform->m_data.transform.flip_y
            ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER
            : UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_TOP_RIGHT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT);
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_CENTER_LEFT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT
            : UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_CENTER_RIGHT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT
            : UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_LEFT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT);
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_CENTER:
        pos_policy = 
            transform->m_data.transform.flip_y
            ? UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER
            : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_RIGHT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT);
        break;
    default:
        break;
    }

    switch(pos_policy) {
    case UI_SPRITE_2D_TRANSFORM_POS_ORIGIN:
        r.x = 0;
        r.y = 0;
        return r;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT:
        r.x = transform->m_data.transform.rect.lt.x;
        r.y = transform->m_data.transform.rect.lt.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER:
        r.x = (transform->m_data.transform.rect.lt.x + transform->m_data.transform.rect.rb.x) / 2.0f;
        r.y = transform->m_data.transform.rect.lt.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT:
        r.x = transform->m_data.transform.rect.rb.x;
        r.y = transform->m_data.transform.rect.lt.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT:
        r.x = transform->m_data.transform.rect.lt.x;
        r.y = (transform->m_data.transform.rect.lt.y + transform->m_data.transform.rect.rb.y) / 2.0f;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER:
        r.x = (transform->m_data.transform.rect.lt.x + transform->m_data.transform.rect.rb.x) / 2.0f;
        r.y = (transform->m_data.transform.rect.lt.y + transform->m_data.transform.rect.rb.y) / 2.0f;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT:
        r.x = transform->m_data.transform.rect.rb.x;
        r.y = (transform->m_data.transform.rect.lt.y + transform->m_data.transform.rect.rb.y) / 2.0f;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT:
        r.x = transform->m_data.transform.rect.lt.x;
        r.y = transform->m_data.transform.rect.rb.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER:
        r.x = (transform->m_data.transform.rect.lt.x + transform->m_data.transform.rect.rb.x) / 2.0f;
        r.y = transform->m_data.transform.rect.rb.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT:
        r.x = transform->m_data.transform.rect.rb.x;
        r.y = transform->m_data.transform.rect.rb.y;
        break;
    default: {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        CPE_ERROR(
            transform->m_module->m_em, "entity %d(%s): transform: get pos policy %d is unknown",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), pos_policy);

        r.x = 0.0f;
        r.y = 0.0f;
        return r;
    }
    }

    if (adj_type) {
        r = ui_sprite_2d_transform_adj_local_pos(transform, r, adj_type);
    }

    return r;
}

ui_vector_2 ui_sprite_2d_transform_world_pos(ui_sprite_2d_transform_t transform, uint8_t pos_policy, uint8_t adj_type) {
    ui_vector_2 r = ui_sprite_2d_transform_local_pos(transform, pos_policy, adj_type);

    r.x += transform->m_data.transform.pos.x;
    r.y += transform->m_data.transform.pos.y;

    return r;
}

ui_vector_2 ui_sprite_2d_transform_origin_pos(ui_sprite_2d_transform_t transform) {
    ui_vector_2 r = UI_VECTOR_2_INITLIZER(transform->m_data.transform.pos.x, transform->m_data.transform.pos.y);
    return r;
}

void ui_sprite_2d_transform_set_origin_pos(ui_sprite_2d_transform_t transform, ui_vector_2 pos) {
    cpe_assert_float_sane(pos.x);
    cpe_assert_float_sane(pos.y);
    
    if (pos.x != transform->m_data.transform.pos.x || pos.y != transform->m_data.transform.pos.y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.pos.x = pos.x;
        transform->m_data.transform.pos.y = pos.y;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.pos");
        }
    }
}

ui_vector_2 ui_sprite_2d_transform_scale_pair(ui_sprite_2d_transform_t transform) {
    ui_vector_2 r = UI_VECTOR_2_INITLIZER(transform->m_data.transform.scale.x, transform->m_data.transform.scale.y);
    return r;
}

void ui_sprite_2d_transform_set_scale_pair(ui_sprite_2d_transform_t transform, ui_vector_2 scale) {
    cpe_assert_float_sane(scale.x);
    cpe_assert_float_sane(scale.y);
    
    if (scale.x != transform->m_data.transform.scale.x || scale.y != transform->m_data.transform.scale.y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.scale.x = scale.x;
        transform->m_data.transform.scale.y = scale.y;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.scale");
        }
    }
}

void ui_sprite_2d_transform_set_scale(ui_sprite_2d_transform_t transform, float scale) {
    ui_vector_2 scale_pair = UI_VECTOR_2_INITLIZER(scale, scale);
    ui_sprite_2d_transform_set_scale_pair(transform, scale_pair);
}

float ui_sprite_2d_transform_angle(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.angle;
}

void ui_sprite_2d_transform_set_angle(ui_sprite_2d_transform_t transform, float angle) {
    if (angle != transform->m_data.transform.angle) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.angle = angle;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.angle");
        }
    }
}

void ui_sprite_2d_transform_merge_rect_world(ui_sprite_2d_transform_t transform, ui_rect const * i_rect) {
    ui_rect rect = * i_rect;
    rect.lt.x -= transform->m_data.transform.pos .x;
    rect.lt.y -= transform->m_data.transform.pos.y;
    rect.rb.x -= transform->m_data.transform.pos.x;
    rect.rb.y -= transform->m_data.transform.pos.y;
    ui_sprite_2d_transform_merge_rect(transform, &rect);
}

ui_rect ui_sprite_2d_transform_rect_world(ui_sprite_2d_transform_t transform) {
    ui_rect r = UI_RECT_INITLIZER(
        transform->m_data.transform.rect.lt.x + transform->m_data.transform.pos.x,
        transform->m_data.transform.rect.lt.y + transform->m_data.transform.pos.y,
        transform->m_data.transform.rect.rb.x + transform->m_data.transform.pos.x,
        transform->m_data.transform.rect.rb.y + transform->m_data.transform.pos.y);
    return r;
}

void ui_sprite_2d_transform_merge_rect(ui_sprite_2d_transform_t transform, ui_rect const * rect) {
    ui_rect origin_rect = UI_RECT_INITLIZER(
        transform->m_data.transform.rect.lt.x, transform->m_data.transform.rect.lt.y,
        transform->m_data.transform.rect.rb.x, transform->m_data.transform.rect.rb.y);
    ui_rect new_rect = origin_rect;
    ui_sprite_2d_rect_merge(&new_rect, rect);

    if (ui_rect_cmp(&new_rect, &origin_rect) != 0) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);
        
        transform->m_data.transform.rect.lt.x = new_rect.lt.x;
        transform->m_data.transform.rect.lt.y = new_rect.lt.y;
        transform->m_data.transform.rect.rb.x = new_rect.rb.x;
        transform->m_data.transform.rect.rb.y = new_rect.rb.y;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.rect");
        }
    }
}

ui_rect ui_sprite_2d_transform_rect(ui_sprite_2d_transform_t transform) {
    ui_rect r = UI_RECT_INITLIZER(
        transform->m_data.transform.rect.lt.x, transform->m_data.transform.rect.lt.y,
        transform->m_data.transform.rect.rb.x, transform->m_data.transform.rect.rb.y);

    return r;
}

void ui_sprite_2d_transform_set_rect(ui_sprite_2d_transform_t transform, ui_rect const * rect) {
    ui_rect origin_rect = UI_RECT_INITLIZER(
        transform->m_data.transform.rect.lt.x, transform->m_data.transform.rect.lt.y,
        transform->m_data.transform.rect.rb.x, transform->m_data.transform.rect.rb.y);
    
    if (ui_rect_cmp(rect, &origin_rect) != 0) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);
        
        transform->m_data.transform.rect.lt.x = rect->lt.x;
        transform->m_data.transform.rect.lt.y = rect->lt.y;
        transform->m_data.transform.rect.rb.x = rect->rb.x;
        transform->m_data.transform.rect.rb.y = rect->rb.y;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.rect");
        }
    }
}

uint8_t ui_sprite_2d_transform_flip_x(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.flip_x;
}

uint8_t ui_sprite_2d_transform_flip_y(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.flip_y;
}

void ui_sprite_2d_transform_set_flip(ui_sprite_2d_transform_t transform, uint8_t flip_x, uint8_t flip_y) {
    assert(flip_x == 0 || flip_x == 1);
    assert(flip_y == 0 || flip_y == 1);

    if (transform->m_data.transform.flip_x != flip_x) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.flip_x = flip_x;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.flip_x");
        }
    }

    if (transform->m_data.transform.flip_y != flip_y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.flip_y = flip_y;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.flip_y");
        }
    }
}

static const char * s_pos_policy_defs[] = {
    "unknown-pos-policy",
    "origin",
    "top-left",
    "top-center",
    "top-right",
    "center-left",
    "center",
    "center-right",
    "bottom-left",
    "bottom-center",
    "bottom-right",
};

uint8_t ui_sprite_2d_transform_pos_policy_from_str(const char * str_pos_policy) {
    uint8_t i;

    if (str_pos_policy[0] == 0) return UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;

    for(i = 1; i < CPE_ARRAY_SIZE(s_pos_policy_defs); ++i) {
        if (strcmp(s_pos_policy_defs[i], str_pos_policy) == 0) return i;
    }

    return 0;
}

const char * ui_sprite_2d_transform_pos_policy_to_str(uint8_t pos_policy) {
    return s_pos_policy_defs[pos_policy >= CPE_ARRAY_SIZE(s_pos_policy_defs) ? 0 : pos_policy];
}

float ui_sprite_2d_transform_adj_angle_by_flip(ui_sprite_2d_transform_t transform, float angle) {
    assert(angle >= -180.f && angle <= 180.f);

    if (transform->m_data.transform.flip_x) {
		angle = cpe_math_angle_regular(180.0f - angle);
    }

    if (transform->m_data.transform.flip_y) {
		angle = cpe_math_angle_regular(0.0 - angle);
    }

    return angle;
}

float ui_sprite_2d_transform_adj_radians_by_flip(ui_sprite_2d_transform_t transform, float radians) {
	assert(radians >= -M_PI && radians <= M_PI);
	if (transform->m_data.transform.flip_x) {
		radians = (float)cpe_math_radians_diff((float)M_PI, radians);
	}

	if (transform->m_data.transform.flip_y) {
		radians = (float)cpe_math_radians_diff(0, radians);
	}

    return radians;
}

ui_vector_2
ui_sprite_2d_transform_adj_world_pos(ui_sprite_2d_transform_t transform, ui_vector_2 pos, uint8_t adj_type)
{
    pos.x -= transform->m_data.transform.pos.x;
    pos.y -= transform->m_data.transform.pos.y;

    pos = ui_sprite_2d_transform_adj_local_pos(transform, pos, adj_type);

    pos.x += transform->m_data.transform.pos.x;
    pos.y += transform->m_data.transform.pos.y;

    return pos;
}

ui_vector_2
ui_sprite_2d_transform_adj_local_pos(ui_sprite_2d_transform_t transform, ui_vector_2 pos, uint8_t adj_type) {
    if (adj_type & UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP) {
        if (transform->m_data.transform.flip_x) pos.x *= -1.0f;
        if (transform->m_data.transform.flip_y) pos.y *= -1.0f;
    }

    if (adj_type & UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE) {
        pos.x *= transform->m_data.transform.scale.x;
        pos.y *= transform->m_data.transform.scale.y;
    }

    if (adj_type & UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_ANGLE && transform->m_data.transform.angle) {
        float distance = cpe_math_distance(0, 0, pos.x, pos.y);
        float radians = cpe_math_radians(0, 0, pos.x, pos.y);

        radians = cpe_math_radians_add(radians, cpe_math_angle_to_radians(transform->m_data.transform.angle));

        pos.x = distance * cos(radians);
        pos.y = distance * sin(radians);
    }

    return pos;
}

ui_vector_2 ui_sprite_2d_transform_world_to_local(ui_sprite_2d_transform_t transform, ui_vector_2 world_pt) {
    ui_vector_2 r;
    float distance;
    float radians;

    assert(transform->m_data.transform.scale.x > 0.0f && transform->m_data.transform.scale.y > 0.0f);

    r.x = world_pt.x - transform->m_data.transform.pos.x;
    r.y = world_pt.y - transform->m_data.transform.pos.y;

    if (transform->m_data.transform.angle != 0.0f) {
        distance = cpe_math_distance(0.0f, 0.0f, r.x, r.y);
        if (distance > 0.01f) {
            radians = cpe_math_radians(0.0f, 0.0f, r.x, r.y);
            radians = cpe_math_radians_diff(radians, cpe_math_angle_to_radians(transform->m_data.transform.angle));
            radians = ui_sprite_2d_transform_adj_radians_by_flip(transform, radians);

            r.x = distance * cos(radians);
            r.y = distance * sin(radians);
        }
    }
    
    r.x /= transform->m_data.transform.scale.x;
    r.y /= transform->m_data.transform.scale.y;

    /* printf("xxx: world-pt=(%f,%f), entity=(%f,%f), result=(%f,%f)\n", */
    /*        world_pt.x, world_pt.y, */
    /*        transform->m_data.transform.pos.x, transform->m_data.transform.pos.y, */
    /*        r.x, r.y); */
    
    return r;
}

ui_vector_2 ui_sprite_2d_transform_local_to_world(ui_sprite_2d_transform_t transform, ui_vector_2 local_pt) {
    ui_vector_2 r;
    float distance;
    float radians;

    distance = cpe_math_distance(0, 0, local_pt.x, local_pt.y);

    radians = distance > 0.01f ? cpe_math_radians(0, 0, local_pt.x, local_pt.y) : 0.0f;

    radians = cpe_math_radians_add(radians, cpe_math_angle_to_radians(transform->m_data.transform.angle));

    radians = ui_sprite_2d_transform_adj_radians_by_flip(transform, radians);

    r.x = transform->m_data.transform.pos.x + distance * cos(radians) * transform->m_data.transform.scale.x;
    r.y = transform->m_data.transform.pos.y + distance * sin(radians) * transform->m_data.transform.scale.y;

    return r;
}

int ui_sprite_2d_transform_calc_trans(ui_sprite_2d_transform_t transform, ui_transform_t trans) {
    ui_vector_3 s = UI_VECTOR_3_INITLIZER(transform->m_data.transform.scale.x, transform->m_data.transform.scale.y, 1.0f);
    ui_quaternion q;
    ui_vector_2 p = UI_VECTOR_2_INITLIZER(transform->m_data.transform.pos.x, transform->m_data.transform.pos.y);

    if (transform->m_data.transform.flip_x) s.x *= -1.0f;
    if (transform->m_data.transform.flip_y) s.y *= -1.0f;

    *trans = UI_TRANSFORM_IDENTITY;

    ui_quaternion_set_z_radians(&q, cpe_math_angle_to_radians(transform->m_data.transform.angle));

    ui_transform_set_pos_2(trans, &p);
    ui_transform_set_quation_scale(trans, &q, &s);

    return (cpe_float_cmp(transform->m_data.transform.scale.x, 0.0f, UI_FLOAT_PRECISION) == 0
            || cpe_float_cmp(transform->m_data.transform.scale.y, 0.0f, UI_FLOAT_PRECISION) == 0)
        ? -1
        : 0;
}

void ui_sprite_2d_transform_set_trans(ui_sprite_2d_transform_t transform, ui_transform_t trans) {
    ui_vector_2 s;
    ui_vector_2 p;

    s.x = trans->m_s.x;
    s.y = trans->m_s.y;
    ui_sprite_2d_transform_set_scale_pair(transform, s);

    ui_transform_get_pos_2(trans, &p);
    ui_sprite_2d_transform_set_origin_pos(transform, p);

    ui_sprite_2d_transform_set_angle(transform, cpe_math_radians_to_angle(ui_transform_calc_angle_z_rad(trans)));
}

const char * UI_SPRITE_2D_TRANSFORM_NAME = "Transform";
