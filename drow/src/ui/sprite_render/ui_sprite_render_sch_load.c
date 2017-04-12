#include "cpe/utils/stream_mem.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_def_i.h"
#include "ui_sprite_render_group_i.h"

static int ui_sprite_cfg_load_component_render_sch_groups(ui_sprite_render_module_t module, ui_sprite_render_sch_t render_sch, cfg_t cfg);
static int ui_sprite_cfg_load_component_render_sch_resources(ui_sprite_render_module_t module, ui_sprite_render_sch_t render_sch, cfg_t cfg);

int ui_sprite_render_sch_load(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_sch_t render_sch = ui_sprite_component_data(component);
    const char * str_value;

    if ((str_value = cfg_get_string(cfg, "layer", NULL))) {
        ui_sprite_render_sch_set_default_layer_by_name(render_sch, str_value);
    }

    if ((str_value = cfg_get_string(cfg, "render-priority", NULL))) {
        ui_sprite_render_sch_set_render_priority(render_sch, atof(str_value));
    }

    if (ui_sprite_cfg_load_component_render_sch_groups(module, render_sch, cfg_find_cfg(cfg, "groups")) != 0) {
        return -1;
    }

    if (ui_sprite_cfg_load_component_render_sch_resources(module, render_sch, cfg_find_cfg(cfg, "resources")) != 0) {
        return -1;
    }

    return 0;
}

static uint8_t ui_sprite_2d_transform_pos_adj_policy_from_str(const char * str_policy) {
	if (strcmp(str_policy, "scale") == 0) {
		return UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE;
	}
	else if (strcmp(str_policy, "flip") == 0) {
		return UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP;
	}
	else if (strcmp(str_policy, "angle") == 0) {
		return UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_ANGLE;
	}
	else {
		return 0;
	}
}

static int ui_sprite_cfg_load_component_render_sch_groups(ui_sprite_render_module_t module, ui_sprite_render_sch_t render_sch, cfg_t cfg) {
    struct cfg_it group_cfg_it;
    cfg_t group_cfg;
    int32_t index = 0;
    ui_sprite_render_group_t group;

    cfg_it_init(&group_cfg_it, cfg);
    while((group_cfg = cfg_it_next(&group_cfg_it))) {
        const char * group_name;
        const char * base_pos_str;
		cfg_t base_pos_adj_policy_cfg;
        cfg_t pos_cfg;
        cfg_t scale_cfg;
        const char * bind_part_name;
        ui_transform local_trans = UI_TRANSFORM_IDENTITY;
        
        if ((group_name = cfg_get_string(group_cfg, "name", NULL)) == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create render_sch component: group (%d) name not configured!",
                ui_sprite_render_module_name(module), index);
            return -1;
        }

        group = ui_sprite_render_group_find_by_name(render_sch, group_name);
        if (group == NULL) {
            group = ui_sprite_render_group_create(render_sch, group_name);
            if (group == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create render_sch component: group %s: create fail!",
                    ui_sprite_render_module_name(module), group_name);
                return -1;
            }
        }

        if ((bind_part_name = cfg_get_string(group_cfg, "binding-part", NULL))) {
            if (ui_sprite_render_group_set_binding_part(group, bind_part_name) != 0) {
                CPE_ERROR(
                    module->m_em, "%s: create render_sch component: group %s: binding part %s error!",
                    ui_sprite_render_module_name(module), group_name, bind_part_name);
                return -1;
            }
        }

        local_trans = *ui_sprite_render_group_local_trans(group);

        if ((pos_cfg = cfg_find_cfg(group_cfg, "pos-adj"))) {
            ui_vector_2 old_pos;
            ui_vector_2 pos_adj;

            ui_transform_get_pos_2(&local_trans, &old_pos);
            
            pos_adj.x = cfg_get_float(pos_cfg, "x", old_pos.x);
            pos_adj.y = cfg_get_float(pos_cfg, "y", old_pos.y);

            ui_transform_set_pos_2(&local_trans, &pos_adj);
        }

        if ((scale_cfg = cfg_find_cfg(group_cfg, "scale-adj"))) {
            ui_vector_3 scale_adj;

            scale_adj.x = cfg_get_float(scale_cfg, "x", local_trans.m_s.x);
            scale_adj.y = cfg_get_float(scale_cfg, "y", local_trans.m_s.y);
            scale_adj.z = local_trans.m_s.z;
            
            ui_transform_set_scale(&local_trans, &scale_adj);
        }

        if ((scale_cfg = cfg_find_cfg(group_cfg, "angle-adj"))) {
            ui_quaternion q;
            ui_quaternion_set_z_radians(&q, cpe_math_angle_to_radians(cfg_get_float(group_cfg, "angle-adj", 0.0f)));
            ui_transform_set_quation(&local_trans, &q);
        }
        ui_sprite_render_group_set_local_trans(group, &local_trans);

        ui_sprite_render_group_set_accept_scale(
            group,
            cfg_get_uint8(group_cfg, "accept-scale", ui_sprite_render_group_accept_scale(group)));

        ui_sprite_render_group_set_adj_accept_scale(
            group,
            cfg_get_uint8(group_cfg, "pos-adj-accept-scale", ui_sprite_render_group_adj_accept_scale(group)));

		ui_sprite_render_group_set_accept_rotate(
			group,
			cfg_get_uint8(group_cfg, "accept-rotate", ui_sprite_render_group_accept_rotate(group)));

		ui_sprite_render_group_set_accept_flip(
			group,
			cfg_get_uint8(group_cfg, "accept-flip", ui_sprite_render_group_accept_flip(group)));
        
        ui_sprite_render_group_set_adj_render_priority(
            group,
            cfg_get_float(group_cfg, "adj-render-priority", ui_sprite_render_group_adj_render_priority(group)));
        
        if ((base_pos_str = cfg_get_string(group_cfg, "base-pos", NULL))) {
            uint8_t base_pos = ui_sprite_2d_transform_pos_policy_from_str(base_pos_str);
            if (base_pos == 0) {
                CPE_ERROR(
                    module->m_em, "%s: create render_sch component: group %s: base-pos %s error!",
                    ui_sprite_render_module_name(module), group_name, base_pos_str);
                return -1;
            }

            ui_sprite_render_group_set_base_pos(group, base_pos);
        }

		if ((base_pos_adj_policy_cfg = cfg_find_cfg(group_cfg, "base-pos-adj-policy"))) {
			uint8_t policy = 0;
			struct cfg_it policy_it;
			cfg_t policy_cfg;

			cfg_it_init(&policy_it, base_pos_adj_policy_cfg);
			while((policy_cfg = cfg_it_next(&policy_it))) {
				policy |= ui_sprite_2d_transform_pos_adj_policy_from_str(cfg_as_string(policy_cfg, NULL));
			}

			ui_sprite_render_group_set_base_pos_adj_policy(group, policy);
		}

        ++index;
    }

    return 0;
}

static int ui_sprite_cfg_load_component_render_sch_resources(
    ui_sprite_render_module_t module, ui_sprite_render_sch_t render_sch, cfg_t cfg)
{
    struct cfg_it render_sch_cfg_it;
    cfg_t render_sch_cfg;
    int32_t index = 0;

    cfg_it_init(&render_sch_cfg_it, cfg);
    while((render_sch_cfg = cfg_it_next(&render_sch_cfg_it))) {
        const char * render_sch_res;
        const char * render_sch_name;

        if ((render_sch_name = cfg_get_string(render_sch_cfg, "name", NULL)) == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create render_sch component: (%d) name not configured!",
                ui_sprite_render_module_name(module), index);
            return -1;
        }

        if ((render_sch_res = cfg_get_string(render_sch_cfg, "res", NULL)) == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create render_sch component: (%d) res not configured!",
                ui_sprite_render_module_name(module), index);
            return -1;
        }

        if (ui_sprite_render_def_create(
                render_sch, render_sch_name, render_sch_res, cfg_get_uint8(render_sch_cfg, "auto-start", 0))
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "%s: create render_sch component: (%d) add render_sch %s ==> %s fail!",
                ui_sprite_render_module_name(module), index,
                render_sch_name, render_sch_res);
            return -1;
        }

        ++index;
    }

    return 0;
}
