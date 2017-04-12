#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_responser_i.h"
#include "ui_sprite_touch_box_i.h"
#include "ui_sprite_touch_mgr_i.h"

ui_sprite_touch_touchable_t ui_sprite_touch_touchable_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_TOUCH_TOUCHABLE_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_touch_touchable_t ui_sprite_touch_touchable_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_TOUCH_TOUCHABLE_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_touch_touchable_free(ui_sprite_touch_touchable_t touch_touchable) {
    ui_sprite_component_t component = ui_sprite_component_from_data(touch_touchable);
    ui_sprite_component_free(component);
};

int ui_sprite_touch_touchable_is_point_in(ui_sprite_touch_touchable_t touchable, ui_vector_2 world_pt) {
    ui_sprite_touch_env_t env = touchable->m_env;
    ui_sprite_entity_t entity;
    ui_sprite_touch_box_t box;

    if (TAILQ_EMPTY(&touchable->m_boxes)) return 1;

    entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

    TAILQ_FOREACH(box, &touchable->m_boxes, m_next_for_touchable) {
        if (ui_sprite_touch_box_check_pt_in(box, entity, world_pt)) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    env->m_mgr->m_em, "entity %d(%s): touch check success",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            }
            return 1;
        }
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            env->m_mgr->m_em, "entity %d(%s): touch check fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
    }

    return 0;
}

void ui_sprite_touch_touchable_set_z(ui_sprite_touch_touchable_t touch_touchable, float z) {
    touch_touchable->m_z = z; 
}

float ui_sprite_touch_touchable_z(ui_sprite_touch_touchable_t touch_touchable) {
    return touch_touchable->m_z;
}

static void ui_sprite_touch_show_boxes(ui_sprite_touch_touchable_t touch) {
    /* ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touch)); */
    /* ui_sprite_touch_box_t box; */
    /* ui_sprite_render_sch_t anim = ui_sprite_render_sch_find(entity); */

    /* if (anim == NULL) { */
    /*     CPE_ERROR( */
    /*         touch->m_mgr->m_em, "%d(%s): touchable: show boxes: no anim!", */
    /*         ui_sprite_entity_id(entity), ui_sprite_entity_name(entity)); */
    /*     return; */
    /* } */

    /* TAILQ_FOREACH(box, &touch->m_boxes, m_next_for_touchable) { */
    /*     ui_sprite_render_anim_t anim; */
    /*     char buf[128]; */
    /*     snprintf(buf, sizeof(buf), "BOX: lt.x=%f, lt.y=%f, rb.x=%f, rb.y=%f, color=1", box->m_lt.x, box->m_lt.y, box->m_rb.x, box->m_rb.y); */

    /*     assert(box->m_box_id == 0); */

    /*     anim = ui_sprite_render_sch_start_anim(anim, "", buf, NULL); */
    /*     if (anim == NUL) { */
    /*         CPE_ERROR( */
    /*             touch->m_mgr->m_em, "%d(%s): touchable: show boxes: start '%s' fail!", */
    /*             ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), buf); */
    /*     } */
    /*     else { */
    /*         box->m_box_id == ui_sprite_render_anim_id(anim); */
    /*     } */
    /* } */
}

static void ui_sprite_touch_hide_boxes(ui_sprite_touch_touchable_t touch) {
    /* ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(touch)); */
    /* ui_sprite_touch_box_t box; */
    /* ui_sprite_render_sch_t anim = ui_sprite_render_sch_find(entity); */

    /* if (anim == NULL) return; */

    /* TAILQ_FOREACH(box, &touch->m_boxes, m_next_for_touchable) { */
    /*     if (box->m_box_id != 0) { */
    /*         ui_sprite_render_anim_t anim = ui_sprite_render_anim_find_by_id(anim, box->m_box_id); */
    /*         if (anim) ui_sprite_render_anim_free(anim); */
    /*         box->m_box_id = 0; */
    /*     } */
    /* } */
}

static int ui_sprite_touch_touchable_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);

    if (ui_sprite_entity_debug(entity)) {
        ui_sprite_touch_show_boxes(touch);
    }

    return 0;
}

static void ui_sprite_touch_touchable_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);

    ui_sprite_touch_hide_boxes(touch);
}

static int ui_sprite_touch_touchable_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);
    ui_sprite_touch_env_t env = ui_sprite_touch_env_find(world);

    if (env == NULL) {
        CPE_ERROR(
            mgr->m_em, "entity %d(%s): Touchable: init: no render env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
    }
    
    touch->m_env = env;
    touch->m_z = 0.0f;
    TAILQ_INIT(&touch->m_responsers);
    TAILQ_INIT(&touch->m_boxes);

    return 0;
}

static void ui_sprite_touch_touchable_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_touch_touchable_t touch = ui_sprite_component_data(component);

    while(!TAILQ_EMPTY(&touch->m_boxes)) {
        ui_sprite_touch_box_free(TAILQ_FIRST(&touch->m_boxes));
    }

    assert(TAILQ_EMPTY(&touch->m_responsers));
}

static int ui_sprite_touch_touchable_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_touchable_t to_touch = ui_sprite_component_data(to);
    ui_sprite_entity_t to_entity = ui_sprite_component_entity(to);
    ui_sprite_touch_touchable_t from_touch = ui_sprite_component_data(from);
    ui_sprite_touch_box_t from_box;

    if (ui_sprite_touch_touchable_init(to, ctx) != 0) return -1;

    to_touch->m_z = from_touch->m_z;

    TAILQ_FOREACH(from_box, &from_touch->m_boxes, m_next_for_touchable) {
        ui_sprite_touch_box_t to_box =
            ui_sprite_touch_box_create(to_touch, &from_box->m_shape);
        if (to_box == NULL) {
            CPE_ERROR(
                mgr->m_em, "entity %d(%s): Touchable: copy: box create fail!",
                ui_sprite_entity_id(to_entity), ui_sprite_entity_name(to_entity));
            ui_sprite_touch_touchable_fini(to, ctx);
            return -1;
        }
    }

    return 0;
}

int ui_sprite_touch_touchable_regist(ui_sprite_touch_mgr_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(module->m_repo, UI_SPRITE_TOUCH_TOUCHABLE_NAME, sizeof(struct ui_sprite_touch_touchable));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch component register: meta create fail",
            ui_sprite_touch_mgr_name(module));
        return -1;
    }

    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_touch_touchable_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_touch_touchable_exit, module);
    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_touch_touchable_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_touch_touchable_copy, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_touch_touchable_fini, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_comp_loader(
                module->m_loader, UI_SPRITE_TOUCH_TOUCHABLE_NAME, ui_sprite_touch_touchable_load, module)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: %s register: register loader fail",
                ui_sprite_touch_mgr_name(module), UI_SPRITE_TOUCH_TOUCHABLE_NAME);
            ui_sprite_component_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_touch_touchable_unregist(ui_sprite_touch_mgr_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_TOUCH_TOUCHABLE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch component unregister: meta not exist",
            ui_sprite_touch_mgr_name(module));
        return;
    }

    ui_sprite_component_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_TOUCH_TOUCHABLE_NAME);
    }
}

const char * UI_SPRITE_TOUCH_TOUCHABLE_NAME = "Touchable";
