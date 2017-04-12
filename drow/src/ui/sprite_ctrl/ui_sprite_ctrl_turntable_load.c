#include "cpe/utils/stream_mem.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_ctrl_turntable_i.h"

static int ui_sprite_cfg_load_component_turntable_track(
    ui_sprite_ctrl_module_t module, UI_SPRITE_CTRL_TURNTABLE_DEF * def, cfg_t cfg);

int ui_sprite_ctrl_turntable_load(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_t turntable = ui_sprite_component_data(component);
    UI_SPRITE_CTRL_TURNTABLE_DEF def;
    cfg_t track_cfg;
    cfg_t slots_cfg;

    def = *ui_sprite_ctrl_turntable_def(turntable);

    if ((track_cfg = cfg_find_cfg(cfg, "track"))) {
        if (ui_sprite_cfg_load_component_turntable_track(module, &def, track_cfg) != 0) return -1;
    }

    if ((slots_cfg = cfg_find_cfg(cfg, "slots"))) {
        struct cfg_it slots_it;
        cfg_t slot_cfg;

        def.slot_count = 0;

        cfg_it_init(&slots_it, slots_cfg);
        while((slot_cfg = cfg_it_next(&slots_it))) {
            if (def.slot_count + 1 > CPE_ARRAY_SIZE(def.slots)) {
                CPE_ERROR(
                    module->m_em, "%s: create turntable component: slots overflow!!",
                    ui_sprite_ctrl_module_name(module));
                return -1;
            }

            if (cfg_try_as_float(slot_cfg, &def.slots[def.slot_count++]) != 0) {
                CPE_ERROR(
                    module->m_em, "%s: create turntable component: load slot, format error!",
                    ui_sprite_ctrl_module_name(module));
                return -1;
            }
        }
    }

    def.focuse_angle = cfg_get_float(cfg, "focuse-angle", def.focuse_angle);
    def.max_angel_step = cfg_get_float(cfg, "max-angle-step", def.max_angel_step);
    def.scale_min = cfg_get_float(cfg, "scale-range.min", def.scale_min);
    def.scale_max = cfg_get_float(cfg, "scale-range.max", def.scale_max);

    if (ui_sprite_ctrl_turntable_set_def(turntable, &def) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create turntable component: set def fail!",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    return 0;
}

static int ui_sprite_cfg_load_component_turntable_track(
    ui_sprite_ctrl_module_t module, UI_SPRITE_CTRL_TURNTABLE_DEF * def, cfg_t cfg)
{
    const char * type = cfg_get_string(cfg, "type", NULL);

    if (type == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create turntable component: load track, type not configured!",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    if (strcmp(type, "circle") == 0) {
        def->track_type = UI_SPRITE_CTRL_TURNTABLE_TRACK_TYPE_CIRCLE;
        def->track_def.circle.radius = cfg_get_float(cfg, "radius", 0.0f);
        if (def->track_def.circle.radius <= 0.0f) {
            CPE_ERROR(
                module->m_em, "%s: create turntable component: load track: circle: radius %f is error!",
                ui_sprite_ctrl_module_name(module), def->track_def.circle.radius);
            return -1;
        }
    }
    else if (strcmp(type, "ellipse") == 0) {
        def->track_type = UI_SPRITE_CTRL_TURNTABLE_TRACK_TYPE_ELLIPSE;
        def->track_def.ellipse.angle = cfg_get_float(cfg, "angle", 0.0f);
        def->track_def.ellipse.radius_x = cfg_get_float(cfg, "radius.x", 0.0f);
        def->track_def.ellipse.radius_y = cfg_get_float(cfg, "radius.y", 0.0f);

        if (def->track_def.ellipse.radius_x <= 0.0f || def->track_def.ellipse.radius_y <= 0.0f) {
            CPE_ERROR(
                module->m_em, "%s: create turntable component: load track: ellipse: radius (%f,%f) is error!",
                ui_sprite_ctrl_module_name(module), def->track_def.ellipse.radius_x, def->track_def.ellipse.radius_x);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create turntable component: load track, type %s unknown!",
            ui_sprite_ctrl_module_name(module), type);
        return -1;
    }

    return 0;
}
