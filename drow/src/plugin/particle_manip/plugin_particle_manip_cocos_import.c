#include <assert.h>
#include "cpe/pal/pal_math.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/plist/plist_cfg.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_src.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin/particle_manip/plugin_particle_manip_import.h"

#define LOWER_OF_RANGE(_a, _b) (((_a) > (_b)) ? ((_a) - (_b)) : 0)
#define UPPER_OF_RANGE(_a, _b) ((_a) + (_b))
#define UPPER_OF_RANGE_MAX(_a, _b, _max) (((_a) + (_b)) > (_max) ? (_max) : ((_a) + (_b)))

static UI_PARTICLE_MOD *
plugin_particle_import_cocos_particle_carete_mod(ui_ed_obj_t emitter_obj, uint8_t mod_type, error_monitor_t em) {
    ui_ed_obj_t mod_obj;
    UI_PARTICLE_MOD * mod_data;

    mod_obj = ui_ed_obj_new(emitter_obj, ui_ed_obj_type_particle_mod);
    if (mod_obj == NULL) {
        CPE_ERROR(em, "import cocos particle: create mod fail!");
        return NULL;
    }

    mod_data = ui_ed_obj_data(mod_obj);
    mod_data->type = mod_type;

    return mod_data;
}

static int plugin_particle_import_cocos_particle_size(ui_ed_obj_t emitter_obj, cfg_t cfg, error_monitor_t em) {
    UI_PARTICLE_MOD * mod;
    float start_size = cfg_get_float(cfg, "startParticleSize", 0.0f);
    float start_size_variance = cfg_get_float(cfg, "startParticleSizeVariance", 0.0f);
    float finish_size = cfg_get_float(cfg, "finishParticleSize", 0.0f);
    float finish_size_variance = cfg_get_float(cfg, "finishParticleSizeVariance", 0.0f);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_size_uniform_seed, em);
    if (mod == NULL) return -1;

    mod->data.size_uniform_seed.min_base_size = LOWER_OF_RANGE(start_size, start_size_variance);
    mod->data.size_uniform_seed.max_base_size = UPPER_OF_RANGE(start_size, start_size_variance);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_size_uniform_over_life, em);
    if (mod == NULL) return -1;

    mod->data.size_uniform_over_life.min_base_size = 1;
    mod->data.size_uniform_over_life.max_base_size = (finish_size + finish_size_variance) / (start_size + start_size_variance);
    mod->data.size_uniform_over_life.fade_begin_time = 0;

    return 0;
}

static int plugin_particle_import_cocos_particle_color(ui_ed_obj_t emitter_obj, cfg_t cfg, error_monitor_t em) {
    UI_PARTICLE_MOD * mod;
    //float start_color_a = cfg_get_float(cfg, "startColorAlpha", 0.0f);
    float start_color_r = cfg_get_float(cfg, "startColorRed", 0.0f);
    float start_color_g = cfg_get_float(cfg, "startColorGreen", 0.0f);
    float start_color_b = cfg_get_float(cfg, "startColorBlue", 0.0f);
    //float start_color_a_d = cfg_get_float(cfg, "startColorVarianceAlpha", 0.0f);
    float start_color_r_d = cfg_get_float(cfg, "startColorVarianceRed", 0.0f);
    float start_color_g_d = cfg_get_float(cfg, "startColorVarianceGreen", 0.0f);
    float start_color_b_d = cfg_get_float(cfg, "startColorVarianceBlue", 0.0f);

    //float finish_color_a = cfg_get_float(cfg, "finishColorAlpha", 0.0f);
    float finish_color_r = cfg_get_float(cfg, "finishColorRed", 0.0f);
    float finish_color_g = cfg_get_float(cfg, "finishColorGreen", 0.0f);
    float finish_color_b = cfg_get_float(cfg, "finishColorBlue", 0.0f);
    //float finish_color_a_d = cfg_get_float(cfg, "finishColorVarianceAlpha", 0.0f);
    float finish_color_r_d = cfg_get_float(cfg, "finishColorVarianceRed", 0.0f);
    float finish_color_g_d = cfg_get_float(cfg, "finishColorVarianceGreen", 0.0f);
    float finish_color_b_d = cfg_get_float(cfg, "finishColorVarianceBlue", 0.0f);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_color_over_life, em);
    if (mod == NULL) return -1;

    mod->data.color_over_life.max_base_color.a = 1.0; //LOWER_OF_RANGE(start_color_a, start_color_a_d);
    mod->data.color_over_life.max_base_color.r = LOWER_OF_RANGE(start_color_r, start_color_r_d);
    mod->data.color_over_life.max_base_color.g = LOWER_OF_RANGE(start_color_g, start_color_g_d);
    mod->data.color_over_life.max_base_color.b = LOWER_OF_RANGE(start_color_b, start_color_b_d);
    mod->data.color_over_life.min_base_color.a = 1.0; //UPPER_OF_RANGE_MAX(finish_color_a, finish_color_a_d, 1.0f);
    mod->data.color_over_life.min_base_color.r = UPPER_OF_RANGE_MAX(finish_color_r, finish_color_r_d, 1.0f);
    mod->data.color_over_life.min_base_color.g = UPPER_OF_RANGE_MAX(finish_color_g, finish_color_g_d, 1.0f);
    mod->data.color_over_life.min_base_color.b = UPPER_OF_RANGE_MAX(finish_color_b, finish_color_b_d, 1.0f);
    mod->data.color_over_life.fade_begin_time = 0;

    return 0;
}

static int plugin_particle_import_cocos_particle_speed(ui_ed_obj_t emitter_obj, cfg_t cfg, error_monitor_t em) {
    UI_PARTICLE_MOD * mod;
    float speed = cfg_get_float(cfg, "speed", 0.0f);
    float speed_d = cfg_get_float(cfg, "speedVariance", 0.0f);
    float angle = cfg_get_float(cfg, "angle", 0.0f) * (float)M_PI / 180.0f;
    float speed_min = LOWER_OF_RANGE(speed, speed_d);
    float speed_max = UPPER_OF_RANGE(speed, speed_d);
    float angle_sin = sinf(angle);
    float angle_cos = cosf(angle);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_velocity_seed, em);
    if (mod == NULL) return -1;

    mod->data.velocity_seed.min_base_velocity.value[0] = speed_min * angle_cos;
    mod->data.velocity_seed.min_base_velocity.value[1] = - speed_min * angle_sin;
    mod->data.velocity_seed.min_base_velocity.value[2] = 0;

    mod->data.velocity_seed.max_base_velocity.value[0] = speed_max * angle_cos;
    mod->data.velocity_seed.max_base_velocity.value[1] = - speed_max * angle_sin;
    mod->data.velocity_seed.max_base_velocity.value[2] = 0;

    mod->data.velocity_seed.min_multiplier = 1.0;
    mod->data.velocity_seed.max_multiplier = 1.0;

    return 0;
}

static int plugin_particle_import_cocos_particle_accel(ui_ed_obj_t emitter_obj, cfg_t cfg, error_monitor_t em) {
    UI_PARTICLE_MOD * mod;
    float gravity_x = cfg_get_float(cfg, "gravityx", 0.0f);
    float gravity_y = cfg_get_float(cfg, "gravityy", 0.0f);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_accel_seed, em);
    if (mod == NULL) return -1;

    mod->data.accel_seed.min_base.value[0] = mod->data.accel_seed.max_base.value[0] = gravity_x;
    mod->data.accel_seed.min_base.value[1] = mod->data.accel_seed.max_base.value[1] = gravity_y;

    return 0;
}

static int plugin_particle_import_cocos_particle_location(ui_ed_obj_t emitter_obj, cfg_t cfg, error_monitor_t em) {
    UI_PARTICLE_MOD * mod;
    float diff_x = cfg_get_float(cfg, "sourcePositionVariancex", 0.0f);
    float diff_y = cfg_get_float(cfg, "sourcePositionVariancey", 0.0f);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_location_seed, em);
    if (mod == NULL) return -1;

    mod->data.location_seed.min_base_location.value[0] = - diff_x;
    mod->data.location_seed.max_base_location.value[0] = diff_x;

    mod->data.location_seed.min_base_location.value[1] = - diff_y;
    mod->data.location_seed.max_base_location.value[1] = diff_y;

    return 0;
}

static int plugin_particle_import_cocos_particle_lifetime(ui_ed_obj_t emitter_obj, cfg_t cfg, error_monitor_t em) {
    UI_PARTICLE_MOD * mod;
    float lifetime = cfg_get_float(cfg, "particleLifespan", 0.0f);
    float lifetime_variance = cfg_get_float(cfg, "particleLifespanVariance", 0.0f);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_lifetime_seed, em);
    if (mod == NULL) return -1;

    mod->data.lifetime_seed.min_base_time = LOWER_OF_RANGE(lifetime, lifetime_variance);
    mod->data.lifetime_seed.max_base_time = UPPER_OF_RANGE(lifetime, lifetime_variance);

    return 0;
}

static int plugin_particle_import_cocos_particle_rotate(ui_ed_obj_t emitter_obj, cfg_t cfg, error_monitor_t em) {
    UI_PARTICLE_MOD * mod;
    float start_rotate = cfg_get_float(cfg, "rotationStart", 0.0f);
    float start_rotate_d = cfg_get_float(cfg, "rotationStartVariance", 0.0f);
    float finish_rotate = cfg_get_float(cfg, "rotationEnd", 0.0f);
    //float finish_rotate_d = cfg_get_float(cfg, "rotationEndVariance", 0.0f);
    float lifetime = cfg_get_float(cfg, "particleLifespan", 0.0f);

    mod = plugin_particle_import_cocos_particle_carete_mod(emitter_obj, ui_particle_mod_type_rotation2d_seed, em);
    if (mod == NULL) return -1;

    mod->data.rotation2d_seed.min_init_spin = LOWER_OF_RANGE(start_rotate, start_rotate_d);
    mod->data.rotation2d_seed.max_init_spin = UPPER_OF_RANGE(start_rotate, start_rotate_d);

    if (lifetime <= 0.0f) lifetime = 1;

    mod->data.rotation2d_seed.min_spin_rate
        = mod->data.rotation2d_seed.max_spin_rate
        = (finish_rotate - start_rotate) / lifetime;

    return 0;
}

int plugin_particle_manip_import_cocos_particle(
    ui_ed_mgr_t ed_mgr, const char * particle_path,
    const char * plist, const char * pic, error_monitor_t em)
{
    cfg_t root_cfg;
    ui_ed_src_t particle_src;
    ui_ed_obj_t src_obj;
    ui_ed_obj_t emitter_obj;
    UI_PARTICLE_EMITTER * emitter_data;

    /*读取配置文件 */
    root_cfg = plist_cfg_load_dict_from_file(plist, em);
    if (root_cfg == NULL) {
        CPE_ERROR(em, "import cocos particle: read plist %s fail!", plist);
        return -1;
    }

    /* do { */
    /*     struct write_stream_file ws = CPE_WRITE_STREAM_FILE_INITIALIZER(stdout, NULL); */
    /*     cfg_print(root_cfg, (write_stream_t)&ws, 4, 4); */
        
    /* } while(0); */

    /*创建particle */
    particle_src = ui_ed_src_check_create(ed_mgr, particle_path, ui_data_src_type_particle);
    if (particle_src == NULL) {
        CPE_ERROR(em, "import cocos particle: check create particle at %s fail!", particle_path);
        cfg_free(root_cfg);
        return -1;
    }

    src_obj = ui_ed_src_root_obj(particle_src);
    assert(src_obj);

    ui_ed_obj_remove_childs(src_obj);

    /*创建Emitter*/
    emitter_obj = ui_ed_obj_new(src_obj, ui_ed_obj_type_particle_emitter);
    if (emitter_obj == NULL) {
        CPE_ERROR(em, "import cocos particle: check emitter!");
        cfg_free(root_cfg);
        return -1;
    }

    emitter_data =ui_ed_obj_data(emitter_obj);

    emitter_data->duration = cfg_get_float(root_cfg, "duration", 0.0f);
    if (emitter_data->duration < 0.0f) emitter_data->duration = 0.0f;

    emitter_data->max_amount = cfg_get_uint32(root_cfg, "maxParticles", 0);
    emitter_data->texture_id = ui_ed_src_msg_alloc(particle_src, pic);

    if (plugin_particle_import_cocos_particle_lifetime(emitter_obj, root_cfg, em) != 0
        || plugin_particle_import_cocos_particle_location(emitter_obj, root_cfg, em) != 0
        || plugin_particle_import_cocos_particle_accel(emitter_obj, root_cfg, em) != 0
        || plugin_particle_import_cocos_particle_size(emitter_obj, root_cfg, em) != 0
        || plugin_particle_import_cocos_particle_color(emitter_obj, root_cfg, em) != 0
        || plugin_particle_import_cocos_particle_speed(emitter_obj, root_cfg, em) != 0
        || plugin_particle_import_cocos_particle_rotate(emitter_obj, root_cfg, em) != 0
        )
    {
        cfg_free(root_cfg);
        return -1;
    }

    cfg_free(root_cfg);
    return 0;
}
