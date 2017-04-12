#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/moving/plugin_moving_node.h"
#include "plugin/moving/plugin_moving_plan_segment.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_moving_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_moving_obj_on_node_update(void * ctx, plugin_moving_node_t node, plugin_moving_node_event_t evt);
static void ui_sprite_moving_obj_update_top(ui_sprite_moving_obj_t obj);

ui_sprite_moving_obj_t ui_sprite_moving_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_MOVING_OBJ_NAME);
    return component ? (ui_sprite_moving_obj_t)ui_sprite_component_data(component) : NULL;
};

ui_sprite_moving_obj_t ui_sprite_moving_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_MOVING_OBJ_NAME);
    return component ? (ui_sprite_moving_obj_t)ui_sprite_component_data(component) : NULL;
};

void ui_sprite_moving_obj_free(ui_sprite_moving_obj_t moving_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(moving_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}

uint8_t ui_sprite_moving_obj_is_suspend(ui_sprite_moving_obj_t obj) {
    return obj->m_is_suspend;
}
    
void ui_sprite_moving_obj_set_is_suspend(ui_sprite_moving_obj_t obj, uint8_t is_suspend) {
    if (is_suspend) is_suspend = 1;

    if (obj->m_is_suspend != is_suspend) {
        obj->m_is_suspend = is_suspend;
        ui_sprite_moving_obj_update_top(obj);
    }
}

float ui_sprite_moving_obj_time_scale(ui_sprite_moving_obj_t obj) {
    return obj->m_time_scale;
}
    
void ui_sprite_moving_obj_set_tile_scale(ui_sprite_moving_obj_t obj, float time_scale) {
    if (obj->m_time_scale != time_scale) {
        obj->m_time_scale = time_scale;
        ui_sprite_moving_obj_update_top(obj);        
    }
}

int ui_sprite_moving_obj_push_node(
    ui_sprite_moving_obj_t obj, plugin_moving_node_t moving_node,
    void * ctx, ui_sprite_moving_obj_node_destory_fun_t destory)
{
    ui_sprite_moving_obj_node_t new_node;
    ui_sprite_moving_obj_node_t pre_node;
    float time_scale = obj->m_is_suspend ? 0.0f : obj->m_time_scale;

    assert(moving_node);
    
    pre_node = TAILQ_FIRST(&obj->m_node_stack);

    if (pre_node) plugin_moving_node_set_time_scale(pre_node->m_moving_node, 0.0f);

    new_node = ui_sprite_moving_obj_node_create(obj);
    if (new_node == NULL) {
        if (pre_node) plugin_moving_node_set_time_scale(pre_node->m_moving_node, time_scale);
        return -1;
    }

    new_node->m_moving_node = moving_node;
    new_node->m_ctx = ctx;
    new_node->m_destory = destory;

    if (plugin_moving_node_update_fun(moving_node) == NULL) {
        plugin_moving_node_set_update_fun(moving_node, ui_sprite_moving_obj_on_node_update, obj);
    }
    plugin_moving_node_set_time_scale(new_node->m_moving_node, time_scale);

    plugin_moving_node_update(new_node->m_moving_node, 0.0f);

    return 0;
}
    
int ui_sprite_moving_obj_pop_node(
    ui_sprite_moving_obj_t obj, plugin_moving_node_t moving_node)
{
    ui_sprite_moving_obj_node_t obj_node = NULL;
    uint8_t is_top_node = 0;
    
    TAILQ_FOREACH(obj_node, &obj->m_node_stack, m_next) {
        if (obj_node->m_moving_node == moving_node) break;
    }

    if (obj_node == NULL) return -1;

    if (obj_node == TAILQ_FIRST(&obj->m_node_stack)) {
        is_top_node = 1;
    }
    
    if (plugin_moving_node_update_ctx(obj_node->m_moving_node) == obj) {
        plugin_moving_node_set_update_fun(obj_node->m_moving_node, NULL, NULL);
    }
    
    ui_sprite_moving_obj_node_free(obj, obj_node);

    if (is_top_node) {
        ui_sprite_moving_obj_update_top(obj);
        
        obj_node = TAILQ_FIRST(&obj->m_node_stack);
        if(obj_node) {
            plugin_moving_node_update(obj_node->m_moving_node, 0.0f);
        }
    }

    return 0;
}

void ui_sprite_moving_obj_remove_node_by_ctx(ui_sprite_moving_obj_t obj, void * ctx) {
    ui_sprite_moving_obj_node_t obj_node, next_node;

    for(obj_node = TAILQ_FIRST(&obj->m_node_stack);
        obj_node;
        obj_node = next_node)
    {
        next_node = TAILQ_NEXT(obj_node, m_next);

        if (obj_node->m_ctx == ctx) {
            if (plugin_moving_node_update_ctx(obj_node->m_moving_node) == obj) {
                plugin_moving_node_set_update_fun(obj_node->m_moving_node, NULL, NULL);
            }
    
            ui_sprite_moving_obj_node_free(obj, obj_node);
        }
    }

    ui_sprite_moving_obj_update_top(obj);
}

int ui_sprite_moving_obj_destory_node(
    ui_sprite_moving_obj_t obj, plugin_moving_node_t moving_node)
{
    ui_sprite_moving_obj_node_t obj_node = NULL;

    TAILQ_FOREACH(obj_node, &obj->m_node_stack, m_next) {
        if (obj_node->m_moving_node == moving_node) break;
    }

    if (obj_node == NULL) return -1;

    if (obj_node->m_destory) {
        obj_node->m_destory(obj_node->m_ctx, obj_node->m_moving_node);
    }
    else {
        plugin_moving_node_free(obj_node->m_moving_node);
    }
    ui_sprite_moving_obj_node_free(obj, obj_node);
    
    ui_sprite_moving_obj_update_top(obj);

    return 0;
}

void ui_sprite_moving_obj_destory_node_by_ctx(ui_sprite_moving_obj_t obj, void * ctx) {
    ui_sprite_moving_obj_node_t obj_node, next_node;

    for(obj_node = TAILQ_FIRST(&obj->m_node_stack);
        obj_node;
        obj_node = next_node)
    {
        next_node = TAILQ_NEXT(obj_node, m_next);

        if (obj_node->m_ctx == ctx) {
            if (obj_node->m_destory) {
                obj_node->m_destory(obj_node->m_ctx, obj_node->m_moving_node);
            }
            else {
                plugin_moving_node_free(obj_node->m_moving_node);
            }
    
            ui_sprite_moving_obj_node_free(obj, obj_node);
        }
    }

    ui_sprite_moving_obj_update_top(obj);
}

static int ui_sprite_moving_obj_do_init(ui_sprite_moving_module_t module, ui_sprite_moving_obj_t obj) {
    obj->m_module = module;
    TAILQ_INIT(&obj->m_node_stack);
    obj->m_is_suspend = 0;
    obj->m_time_scale = 1.0f;

    return 0;
}

static int ui_sprite_moving_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_moving_module_t module = (ui_sprite_moving_module_t)ctx;
    ui_sprite_moving_obj_t moving_obj = (ui_sprite_moving_obj_t)ui_sprite_component_data(component);

    if (ui_sprite_moving_obj_do_init(module, moving_obj) != 0) return -1;

    return 0;
}

static void ui_sprite_moving_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_moving_obj_t moving_obj = (ui_sprite_moving_obj_t)ui_sprite_component_data(component);

    assert(TAILQ_EMPTY(&moving_obj->m_node_stack));
}

static int ui_sprite_moving_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_moving_module_t module = (ui_sprite_moving_module_t)ctx;
    ui_sprite_moving_obj_t to_moving_obj = (ui_sprite_moving_obj_t)ui_sprite_component_data(to);

    if (ui_sprite_moving_obj_do_init(module, to_moving_obj) != 0) return -1;
    
    return 0;
}

static void ui_sprite_moving_obj_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_moving_obj_t moving_obj = (ui_sprite_moving_obj_t)ui_sprite_component_data(component);

    while(!TAILQ_EMPTY(&moving_obj->m_node_stack)) {
        ui_sprite_moving_obj_node_t obj_node = TAILQ_FIRST(&moving_obj->m_node_stack);
        if (obj_node->m_destory) {
            obj_node->m_destory(obj_node->m_ctx, obj_node->m_moving_node);
        }
        else {
            plugin_moving_node_free(obj_node->m_moving_node);
        }

        ui_sprite_moving_obj_node_free(moving_obj, obj_node);
    }
}

static int ui_sprite_moving_obj_enter(ui_sprite_component_t component, void * ctx) {
    return 0;
}

static void ui_sprite_moving_obj_update_top(ui_sprite_moving_obj_t obj) {
    ui_sprite_moving_obj_node_t top_node;

    if ((top_node = TAILQ_FIRST(&obj->m_node_stack))) {
        float time_scale = obj->m_is_suspend ? 0.0f : obj->m_time_scale;
        plugin_moving_node_set_time_scale(top_node->m_moving_node, time_scale);
    }
}
    
static int ui_sprite_moving_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg) {
    return 0;
}

static void ui_sprite_moving_obj_on_node_update(void * ctx, plugin_moving_node_t node, plugin_moving_node_event_t evt) {
    ui_sprite_moving_obj_t obj = ctx;
    ui_sprite_moving_module_t module = obj->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(ctx));

    if (evt == plugin_moving_node_event_state_updated) {
        switch(plugin_moving_node_state(node)) {
        case plugin_moving_node_state_init:
        case plugin_moving_node_state_working: {
            ui_vector_2_t pos = plugin_moving_node_pos(node);
            ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
                    
            if (transform == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): moving-obj: node update: entity no transform",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return;
            }
                
            ui_sprite_2d_transform_set_origin_pos(transform, *pos);
            break;
        }
        case plugin_moving_node_state_done:
            ui_sprite_moving_obj_destory_node(obj, node);
            break;
        default:
            CPE_ERROR(
                module->m_em, "entity %d(%s): moving-obj: node update: unkonwn state %d",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), plugin_moving_node_state(node));
            break;
        }
    }
    else if (evt == plugin_moving_node_event_segment_begin) {
        plugin_moving_plan_segment_t segment;
        MOVING_PLAN_SEGMENT const * segment_data;

        segment = plugin_moving_node_cur_segment(node);
        assert(segment);

        segment_data = plugin_moving_plan_segment_data(segment);
        assert(segment);

        if (segment_data->on_begin[0]) {
            ui_sprite_entity_build_and_send_event(entity, segment_data->on_begin, NULL);
        }
    }
    else if (evt == plugin_moving_node_event_segment_end) {
		plugin_moving_plan_segment_t segment;
		MOVING_PLAN_SEGMENT const * segment_data;

		segment = plugin_moving_node_cur_segment(node);
		assert(segment);

		segment_data = plugin_moving_plan_segment_data(segment);
		assert(segment);

		if (segment_data->on_end[0]) {
			ui_sprite_entity_build_and_send_event(entity, segment_data->on_end, NULL);
		}
    }
}

int ui_sprite_moving_obj_regist(ui_sprite_moving_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_MOVING_OBJ_NAME, sizeof(struct ui_sprite_moving_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_moving_module_name(module), UI_SPRITE_MOVING_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_moving_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_moving_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_moving_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_moving_obj_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_moving_obj_fini, module);

    if (ui_sprite_cfg_loader_add_comp_loader(
            module->m_loader, UI_SPRITE_MOVING_OBJ_NAME, ui_sprite_moving_obj_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_moving_module_name(module), UI_SPRITE_MOVING_OBJ_NAME);
        ui_sprite_component_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_moving_obj_unregist(ui_sprite_moving_module_t module) {
    ui_sprite_component_meta_t meta;

    if ((meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_MOVING_OBJ_NAME))) {
        ui_sprite_component_meta_free(meta);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_moving_module_name(module), UI_SPRITE_MOVING_OBJ_NAME);
    }

    if (ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_MOVING_OBJ_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_moving_module_name(module), UI_SPRITE_MOVING_OBJ_NAME);
    }
}

const char * UI_SPRITE_MOVING_OBJ_NAME = "MovingObj";

#ifdef __cplusplus
}
#endif

