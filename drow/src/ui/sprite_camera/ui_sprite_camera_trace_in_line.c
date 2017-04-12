#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_camera_trace_in_line_i.h"
#include "ui_sprite_camera_env_i.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

ui_sprite_camera_trace_in_line_t ui_sprite_camera_trace_in_line_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_TRACE_IN_LINE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_trace_in_line_free(ui_sprite_camera_trace_in_line_t trace_in_line) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(trace_in_line);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_camera_trace_in_line_on_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_trace_in_line_t trace_in_line = ctx;
    ui_sprite_camera_module_t module = trace_in_line->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
	UI_SPRITE_EVT_CAMERA_TRACE_IN_LINE const * evt_data = evt->data;
    ui_vector_2 screen_pos;
    enum ui_sprite_camera_trace_type trace_type;
    ui_vector_2 world_pos_a;
    ui_vector_2 world_pos_b;

    if (camera->m_trace_type != ui_sprite_camera_trace_none) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): camera-trace-in-line enter: on event: remove %s scope trace!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                camera->m_trace_type == ui_sprite_camera_trace_by_x ? "x" : "y");
        }

        ui_sprite_camera_env_remove_trace(camera);
    }

    if (strcmp(evt_data->trace_type, "x") == 0) {
        trace_type = ui_sprite_camera_trace_by_x;
    }
    else if (strcmp(evt_data->trace_type, "y") == 0) {
        trace_type = ui_sprite_camera_trace_by_y;
    }
    else if (strcmp(evt_data->trace_type, "noop") == 0) {
        trace_type = ui_sprite_camera_trace_none;
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-trace-in-line enter: on event: not support trace type %s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->trace_type);
        return;
    }

    if (ui_sprite_camera_env_pos_of_entity(
            &world_pos_a,
            world, 0, evt_data->pos_a_entity, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN) != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-trace-in-line enter: on event: get pos of entity %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_a_entity);
        return;
    }

    if (ui_sprite_camera_env_pos_of_entity(
            &world_pos_b,
            world, 0, evt_data->pos_b_entity, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN) != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-trace-in-line enter: on event: get pos of entity %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_b_entity);
        return;
    }

    screen_pos.x = evt_data->screen_pos.x;
    screen_pos.y = evt_data->screen_pos.y;

    if (ui_sprite_camera_env_set_trace(camera, trace_type, screen_pos, world_pos_a, world_pos_b) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-trace-in-line enter: on event: set trace fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;

    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): camera-trace-in-line enter: on event: set trace success!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
    }
}

static int ui_sprite_camera_trace_in_line_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_trace_in_line_t trace_in_line = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_camera_module_t module = ctx;
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

	if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_camera_trace_in_line", ui_sprite_camera_trace_in_line_on_event, trace_in_line) != 0)
	{    
		CPE_ERROR(
            module->m_em, "entity %d(%s): camera-trace-in-line: enter: add eventer handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
		return -1;
	}

    return 0;
}

static void ui_sprite_camera_trace_in_line_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-trace-in-line enter: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (camera->m_trace_type != ui_sprite_camera_trace_none) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): camera-trace-in-line enter: on exit: remove %s scope trace!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                camera->m_trace_type == ui_sprite_camera_trace_by_x ? "x" : "y");
        }
        ui_sprite_camera_env_remove_trace(camera);
    }
}

static int ui_sprite_camera_trace_in_line_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_trace_in_line_t trace_in_line = ui_sprite_fsm_action_data(fsm_action);

    bzero(trace_in_line, sizeof(*trace_in_line));

    trace_in_line->m_module = ctx;

    return 0;
}

static void ui_sprite_camera_trace_in_line_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    
}

static int ui_sprite_camera_trace_in_line_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_camera_trace_in_line_init(to, ctx)) return -1;
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_camera_trace_in_line_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_camera_trace_in_line_t trace_in_line = ui_sprite_camera_trace_in_line_create(fsm_state, name);

	return ui_sprite_fsm_action_from_data(trace_in_line);
}

int ui_sprite_camera_trace_in_line_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_TRACE_IN_LINE_NAME, sizeof(struct ui_sprite_camera_trace_in_line));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera trace_in_line register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_trace_in_line_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_trace_in_line_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_trace_in_line_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_trace_in_line_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_trace_in_line_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_TRACE_IN_LINE_NAME, ui_sprite_camera_trace_in_line_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_trace_in_line_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_TRACE_IN_LINE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera trace_in_line unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_TRACE_IN_LINE_NAME);
    }
}

const char * UI_SPRITE_CAMERA_TRACE_IN_LINE_NAME = "camera-trace-in-line";

