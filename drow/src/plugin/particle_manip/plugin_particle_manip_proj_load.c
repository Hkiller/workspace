#include <assert.h>
#include "libxml/xmlstring.h"
#include "libxml/parser.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_math.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_string_table_builder.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_particle_manip_i.h"
#include "plugin_particle_manip_load_utils.h"
#include "plugin_particle_manip_utils.h"

struct ui_proj_load_particle_ctx {
    plugin_particle_manip_t m_manip;
    ui_data_src_t m_src;
    error_monitor_t m_em;
    ui_string_table_builder_t m_string_table;
    plugin_particle_data_t m_particle;
    plugin_particle_data_emitter_t m_cur_emitter;
    UI_PARTICLE_EMITTER * m_emitter_data;
    plugin_particle_data_mod_t m_cur_mod;
    UI_PARTICLE_MOD * m_mod_data;
    struct {
        UI_PARTICLE_MOD_ACCEL_ATTRACT * m_accel_attract;
        UI_PARTICLE_MOD_ACCEL_DAMPING * m_accel_damping;
        UI_PARTICLE_MOD_ACCEL_SEED * m_accel_seed;
        UI_PARTICLE_MOD_ACCEL_SINE * m_accel_sine;
        UI_PARTICLE_MOD_COLOR_CURVED * m_color_curved;
        UI_PARTICLE_MOD_COLOR_CURVED_ALPHA * m_color_curved_alpha;
        UI_PARTICLE_MOD_COLOR_FIXED * m_color_fixed;
        UI_PARTICLE_MOD_COLOR_OVER_LIFE * m_color_over_life;
        UI_PARTICLE_MOD_COLOR_SEED * m_color_seed;
        UI_PARTICLE_MOD_LIFETIME_SEED * m_lifetime_seed;
        UI_PARTICLE_MOD_LOCATION_ORBIT * m_location_orbit;
        UI_PARTICLE_MOD_LOCATION_SEED * m_location_seed;
        UI_PARTICLE_MOD_ROTATION2D_SEED * m_rotation2d_seed;
        UI_PARTICLE_MOD_SIZE_CURVED_UNIFORM * m_size_curved_uniform;
        UI_PARTICLE_MOD_SIZE_CURVED * m_size_curved;
        UI_PARTICLE_MOD_SIZE_UNIFORM_OVER_LIFE * m_size_uniform_over_life;
        UI_PARTICLE_MOD_SIZE_OVER_LIFE * m_size_over_life;
        UI_PARTICLE_MOD_SIZE_SEED * m_size_seed;
        UI_PARTICLE_MOD_SIZE_UNIFORM_SEED * m_size_uniform_seed;
        UI_PARTICLE_MOD_TEXCOORD_FLIPBOOK_UV * m_texcoord_flipbook_uv;
        UI_PARTICLE_MOD_TEXCOORD_SCROLL_ANIM * m_texcoord_scroll_anim;
        UI_PARTICLE_MOD_TEXCOORD_TILE_SUB_TEX * m_texcoord_tile_sub_tex;
        UI_PARTICLE_MOD_UBER_CIRCLE_SPAWN * m_uber_circle_spawn;
        UI_PARTICLE_MOD_UBER_ELLIPSE_SPAWN * m_uber_ellipse_spawn;
        UI_PARTICLE_MOD_VELOCITY_ATTRACT * m_velocity_attract;
        UI_PARTICLE_MOD_VELOCITY_SEED * m_velocity_seed;
        UI_PARTICLE_MOD_VELOCITY_THRESHOLD_STOP * m_velocity_threshold_stop;
    } m_mod;
    plugin_particle_data_curve_t m_curve_chanel;
    char m_cur_tag_name[64];
};

static void ui_proj_load_particle_startElement(
        void* iprojutCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI,
        int nb_namespaces,
        const xmlChar** namespaces,
        int nb_attributes,
        int nb_defaulted,
        const xmlChar** attributes)
{
    struct ui_proj_load_particle_ctx * ctx = (struct ui_proj_load_particle_ctx *)(iprojutCtx);

    if (strcmp((const char *)localname, "RParticleSpriteSRC") == 0) {
        ctx->m_emitter_data = NULL;
        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        ctx->m_curve_chanel = NULL;

        ctx->m_cur_emitter = plugin_particle_data_emitter_create(ctx->m_particle);
        if (ctx->m_cur_emitter == NULL) {
            CPE_ERROR(ctx->m_em, "create particle emitter: create fail!");
            return;
        }

        ctx->m_emitter_data = plugin_particle_data_emitter_data(ctx->m_cur_emitter);
        assert(ctx->m_emitter_data);;

        dr_meta_set_defaults(
            ctx->m_emitter_data, sizeof(*ctx->m_emitter_data), plugin_particle_data_emitter_meta(ctx->m_manip->m_particle_module),
            DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);

        UI_R_XML_TRY_READ_ATTR_STRING_ID(ctx->m_emitter_data->name_id, "Name");
    }
    else if (strcmp((const char *)localname, "MOD") == 0) {
        char mod_type_name[64];
        uint32_t mod_type;

        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        bzero(&ctx->m_mod, sizeof(ctx->m_mod));

        if (ctx->m_cur_emitter == NULL) return;

        UI_R_XML_READ_ATTR_STRING(mod_type_name, "Type");

        mod_type = plugin_particle_manip_proj_particle_mod_type(mod_type_name);
        if (mod_type == 0) {
            CPE_ERROR(ctx->m_em, "create particle_mod: type %s unknown!", mod_type_name);
            return;
        }

        ctx->m_cur_mod = plugin_particle_data_mod_create(ctx->m_cur_emitter);
        if (ctx->m_cur_mod  == NULL) {
            CPE_ERROR(ctx->m_em, "create mod fail!");
            return;
        }
        ctx->m_mod_data = plugin_particle_data_mod_data(ctx->m_cur_mod);
        assert(ctx->m_mod_data);

        ctx->m_mod_data->type = mod_type;

        switch(ctx->m_mod_data->type) {
        case ui_particle_mod_type_accel_attract:
            ctx->m_mod.m_accel_attract = &ctx->m_mod_data->data.accel_attract;
            break;
        case ui_particle_mod_type_accel_damping:
            ctx->m_mod.m_accel_damping = &ctx->m_mod_data->data.accel_damping;
            break;
        case ui_particle_mod_type_accel_seed:
            ctx->m_mod.m_accel_seed = &ctx->m_mod_data->data.accel_seed;
            break;
        case ui_particle_mod_type_accel_sine:
            ctx->m_mod.m_accel_sine = &ctx->m_mod_data->data.accel_sine;
            break;
        case ui_particle_mod_type_color_curved:
            ctx->m_mod.m_color_curved = &ctx->m_mod_data->data.color_curved;
            break;
        case ui_particle_mod_type_color_curved_alpha:
            ctx->m_mod.m_color_curved_alpha = &ctx->m_mod_data->data.color_curved_alpha;
            break;
        case ui_particle_mod_type_color_fixed:
            ctx->m_mod.m_color_fixed = &ctx->m_mod_data->data.color_fixed;
            break;
        case ui_particle_mod_type_color_over_life:
            ctx->m_mod.m_color_over_life = &ctx->m_mod_data->data.color_over_life;
            break;
        case ui_particle_mod_type_color_seed:
            ctx->m_mod.m_color_seed = &ctx->m_mod_data->data.color_seed;
            break;
        case ui_particle_mod_type_lifetime_seed:
            ctx->m_mod.m_lifetime_seed = &ctx->m_mod_data->data.lifetime_seed;
            break;
        case ui_particle_mod_type_location_orbit:
            ctx->m_mod.m_location_orbit = &ctx->m_mod_data->data.location_orbit;
            break;
        case ui_particle_mod_type_location_seed:
            ctx->m_mod.m_location_seed = &ctx->m_mod_data->data.location_seed;
            break;
        case ui_particle_mod_type_rotation2d_seed:
            ctx->m_mod.m_rotation2d_seed = &ctx->m_mod_data->data.rotation2d_seed;
            break;
        case ui_particle_mod_type_size_curved_uniform:
            ctx->m_mod.m_size_curved_uniform = &ctx->m_mod_data->data.size_curved_uniform;
            break;
        case ui_particle_mod_type_size_uniform_over_life:
            ctx->m_mod.m_size_uniform_over_life = &ctx->m_mod_data->data.size_uniform_over_life;
            break;
        case ui_particle_mod_type_size_curved:
            ctx->m_mod.m_size_curved = &ctx->m_mod_data->data.size_curved;
            break;
        case ui_particle_mod_type_size_over_life:
            ctx->m_mod.m_size_over_life = &ctx->m_mod_data->data.size_over_life;
            break;
        case ui_particle_mod_type_size_seed:
            ctx->m_mod.m_size_seed = &ctx->m_mod_data->data.size_seed;
            break;
        case ui_particle_mod_type_size_uniform_seed:
            ctx->m_mod.m_size_uniform_seed = &ctx->m_mod_data->data.size_uniform_seed;
            break;
        case ui_particle_mod_type_texcoord_flipbook_uv:
            ctx->m_mod.m_texcoord_flipbook_uv = &ctx->m_mod_data->data.texcoord_flipbook_uv;
            break;
        case ui_particle_mod_type_texcoord_scroll_anim:
            ctx->m_mod.m_texcoord_scroll_anim = &ctx->m_mod_data->data.texcoord_scroll_anim;
            break;
        case ui_particle_mod_type_texcoord_tile_sub_tex:
            ctx->m_mod.m_texcoord_tile_sub_tex = &ctx->m_mod_data->data.texcoord_tile_sub_tex;
            break;
        case ui_particle_mod_type_uber_circle_spawn:
            ctx->m_mod.m_uber_circle_spawn = &ctx->m_mod_data->data.uber_circle_spawn;
            break;
        case ui_particle_mod_type_uber_ellipse_spawn:
            ctx->m_mod.m_uber_ellipse_spawn = &ctx->m_mod_data->data.uber_ellipse_spawn;
            break;
        case ui_particle_mod_type_velocity_attract:
            ctx->m_mod.m_velocity_attract = &ctx->m_mod_data->data.velocity_attract;
            break;
        case ui_particle_mod_type_velocity_seed:
            ctx->m_mod.m_velocity_seed = &ctx->m_mod_data->data.velocity_seed;
            break;
        case ui_particle_mod_type_velocity_threshold_stop:
            ctx->m_mod.m_velocity_threshold_stop = &ctx->m_mod_data->data.velocity_threshold_stop;
            break;
        }
    }
    else if (strcmp((const char *)localname, "CurveR") == 0) {
        if (ctx->m_mod.m_color_curved) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_color_curved->chanel_r_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveG") == 0) {
        if (ctx->m_mod.m_color_curved) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_color_curved->chanel_g_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveB") == 0) {
        if (ctx->m_mod.m_color_curved) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_color_curved->chanel_b_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveA") == 0) {
        if (ctx->m_mod.m_color_curved) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_color_curved->chanel_a_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
        else if (ctx->m_mod.m_color_curved_alpha) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_color_curved_alpha->chanel_a_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveX") == 0) {
        if (ctx->m_mod.m_size_curved) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_size_curved->chanel_x_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveY") == 0) {
        if (ctx->m_mod.m_size_curved) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_size_curved->chanel_y_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveSize") == 0) {
        if (ctx->m_mod.m_size_curved_uniform) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_size_curved_uniform->chanel_size_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveAttractPower") == 0) {
        if (ctx->m_mod.m_velocity_attract) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_velocity_attract->power_chanel_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "AttractCurvedPowerAdj") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_CURVED) {
            ctx->m_curve_chanel = plugin_particle_data_curve_create(ctx->m_particle, 0);
            if (ctx->m_curve_chanel) {
                ctx->m_mod.m_accel_attract->attract_data.curved.power_adj_curve_id = plugin_particle_data_curve_id(ctx->m_curve_chanel);
            }
        }
    }
    else if (strcmp((const char *)localname, "CurveKey") == 0) {
        if (ctx->m_curve_chanel) {
            UI_CURVE_POINT * point;
            float angle_l, angle_r;

            point = plugin_particle_data_curve_point_append(ctx->m_curve_chanel);
            if (point == NULL) {
                CPE_ERROR(ctx->m_em, "curved chanel overflow!");
                return;
            }

            UI_R_XML_READ_ATTR_UINT8(point->interp, "Mode");
            UI_R_XML_READ_ATTR_FLOAT(point->key, "Key");
            UI_R_XML_READ_ATTR_FLOAT(point->ret, "Ret");

            UI_R_XML_READ_ATTR_FLOAT(angle_l, "Enter");
            UI_R_XML_READ_ATTR_FLOAT(angle_r, "Leave");

            point->enter_tan = tan(angle_l / 180.0f * M_PI);
            point->leave_tan = tan(angle_r / 180.0f * M_PI);
        }
    }
    else {
        cpe_str_dup(ctx->m_cur_tag_name, sizeof(ctx->m_cur_tag_name), (const char*)localname);
    }
}

static void ui_proj_load_particle_endElement(
        void* iprojutCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct ui_proj_load_particle_ctx * ctx = (struct ui_proj_load_particle_ctx *)(iprojutCtx);
    ctx->m_cur_tag_name[0] = 0;

    if (strcmp((const char *)localname, "MOD") == 0) {
        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        ctx->m_curve_chanel = NULL;
    }
    else if (strcmp((const char *)localname, "RParticleSpriteSRC") == 0) {
        ctx->m_cur_emitter = NULL;
        ctx->m_emitter_data = NULL;
        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        ctx->m_curve_chanel = NULL;
    }
    else if (strcmp((const char *)localname, "CurveR") == 0
             || strcmp((const char *)localname, "CurveG") == 0
             || strcmp((const char *)localname, "CurveB") == 0
             || strcmp((const char *)localname, "CurveA") == 0
             || strcmp((const char *)localname, "CurveSize") == 0
             || strcmp((const char *)localname, "CurveX") == 0
             || strcmp((const char *)localname, "CurveY") == 0
             || strcmp((const char *)localname, "CurveAttractPower") == 0
             || strcmp((const char *)localname, "AttractCurvedPowerAdj") == 0
        )
    {
        ctx->m_curve_chanel = NULL;
    }
}

static void ui_proj_load_particle_characters(void * iprojutCtx, const xmlChar *ch, int len) {
    struct ui_proj_load_particle_ctx * ctx = (struct ui_proj_load_particle_ctx *)(iprojutCtx);

    if (strcmp(ctx->m_cur_tag_name, "SpawnRate") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->spawn_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxAmount") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_emitter_data->max_amount);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "XFormMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->xform_mod);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BoundMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->bound_mod);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AutoUpDir") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_emitter_data->auto_up_dir);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "InitDelayTime") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->init_delay_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LoopDelayTime") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->loop_delay_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Duration") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->duration);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TimeScale") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->time_scale);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "RepeatTimes") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_emitter_data->repeat_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxExtraBrust") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_emitter_data->max_extra_brust);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinExtraBrust") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_emitter_data->min_extra_brust);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Scale") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->scale);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ChildFxFile") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->child_fx_file_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "WaitChildFX") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_emitter_data->wait_child_fx);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "PassOnColor") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_emitter_data->pass_on_color);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TilingU") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->tiling_u);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TilingV") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->tiling_v);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BlendMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->blend_mode);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "FilterMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->filter_mode);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "CustomSrcMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_emitter_data->custom_src_factor);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "CustomDstMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_emitter_data->custom_des_factor);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasBiasX") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_x);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasBiasY") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_y);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasSizeX") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_w);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasSizeY") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_h);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ColAtlasBiasX") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->collision_atlas_x);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ColAtlasBiasY") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->collision_atlas_y);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ColAtlasSizeX") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->collision_atlas_w);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ColAtlasSizeY") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->collision_atlas_h);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Origin") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->origin);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseTexel") == 0) {
        if (ctx->m_emitter_data) {
            char * p;
            char str_buf[128];
            
            UI_R_XML_READ_VALUE_STRING(str_buf);

            if ((p = strstr(str_buf, "//"))) {
                size_t left_len = strlen(p) - 1;
                memmove(p, p + 1, left_len);
                p[left_len] = 0;
            }

            ctx->m_emitter_data->texture_id = ui_string_table_builder_msg_alloc(ctx->m_string_table, str_buf);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "UserData") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->user_text_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseAccelX") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->min_base.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseAccelY") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->min_base.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseAccelZ") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->min_base.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseAccelX") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->max_base.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseAccelY") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->max_base.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseAccelZ") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->max_base.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseSize") == 0) {
        if (ctx->m_mod.m_size_uniform_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_seed->min_base_size);
        }
        else if (ctx->m_mod.m_size_uniform_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_over_life->min_base_size);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseSize") == 0) {
        if (ctx->m_mod.m_size_uniform_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_seed->max_base_size);
        }
        else if (ctx->m_mod.m_size_uniform_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_over_life->max_base_size);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLifetime") == 0) {
        if (ctx->m_mod.m_lifetime_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_lifetime_seed->min_base_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLifetime") == 0) {
        if (ctx->m_mod.m_lifetime_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_lifetime_seed->max_base_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "FadeBeginTime") == 0) {
        if (ctx->m_mod.m_size_uniform_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_over_life->fade_begin_time);
        }
        else if (ctx->m_mod.m_size_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_over_life->fade_begin_time);
        }
        else if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->fade_begin_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLocationX") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->min_base_location.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLocationY") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->min_base_location.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLocationZ") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->min_base_location.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLocationX") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->max_base_location.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLocationY") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->max_base_location.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLocationZ") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->max_base_location.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AngleDelta") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->angle_delta);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->angle_delta);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "CircleRadius") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->circle_radius);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinRadialAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_radial_accel);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->min_radial_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxRadialAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_radial_accel);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->max_radial_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinTangentAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_tangent_accel);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->min_tangent_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxTangentAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_tangent_accel);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->max_tangent_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinRadialVelocity") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_radial_velocity);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->min_radial_velocity);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxRadialVelocity") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_radial_velocity);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->max_radial_velocity);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinArcAngle") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_arc_angle);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->min_arc_angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxArcAngle") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_arc_angle);
        }
        else if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->max_arc_angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DistributeDelta") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->distribute_delta);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "InitialAngle") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->initial_angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Framerate") == 0) {
        if (ctx->m_mod.m_texcoord_flipbook_uv) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_texcoord_flipbook_uv->frame_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Loop") == 0) {
        if (ctx->m_mod.m_texcoord_flipbook_uv) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_mod.m_texcoord_flipbook_uv->loop);
        }
        else if (ctx->m_mod.m_texcoord_scroll_anim) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_mod.m_texcoord_scroll_anim->loop);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "StartTile") == 0) {
        if (ctx->m_mod.m_texcoord_flipbook_uv) {
            UI_R_XML_READ_VALUE_INT32(ctx->m_mod.m_texcoord_flipbook_uv->start_tile_index);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinSpinRate") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->min_spin_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxSpinRate") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->max_spin_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinInitSpin") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->min_init_spin);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxInitSpin") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->max_init_spin);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseSizeX") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->min_base_size.value[0]);
        }
        else if (ctx->m_mod.m_size_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_over_life->min_base_size.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseSizeY") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->min_base_size.value[1]);
        }
        else if (ctx->m_mod.m_size_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_over_life->min_base_size.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseSizeX") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->max_base_size.value[0]);
        }
        else if (ctx->m_mod.m_size_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_over_life->max_base_size.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseSizeY") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->max_base_size.value[1]);
        }
        else if (ctx->m_mod.m_size_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_over_life->max_base_size.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorA") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.a);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.a);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorR") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.r);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.r);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorG") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.g);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.g);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorB") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.b);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.b);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorA") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.a);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.a);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorR") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.r);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.r);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorG") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.g);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.g);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorB") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.b);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.b);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseVelocityX") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_base_velocity.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseVelocityY") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_base_velocity.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseVelocityZ") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_base_velocity.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseVelocityX") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_base_velocity.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseVelocityY") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_base_velocity.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseVelocityZ") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_base_velocity.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinMultiplier") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_multiplier);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxMultiplier") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_multiplier);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractLocationX") == 0) {
        if (ctx->m_mod.m_accel_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->location.value[0]);
        }
        else if (ctx->m_mod.m_velocity_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_attract->attract_location.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractLocationY") == 0) {
        if (ctx->m_mod.m_accel_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->location.value[1]);
        }
        else if (ctx->m_mod.m_velocity_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_attract->attract_location.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractLocationZ") == 0) {
        if (ctx->m_mod.m_accel_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->location.value[2]);
        }
        else if (ctx->m_mod.m_velocity_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_attract->attract_location.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractPower") == 0) {
        if (ctx->m_mod.m_accel_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->power);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractKillZone") == 0) {
        if (ctx->m_mod.m_accel_attract) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->kill_zone);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DampingSize") == 0) {
        if (ctx->m_mod.m_accel_damping) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_damping->size);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseAccelX") == 0) {
        if (ctx->m_mod.m_accel_sine) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_sine->base.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseAccelY") == 0) {
        if (ctx->m_mod.m_accel_sine) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_sine->base.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseAccelZ") == 0) {
        if (ctx->m_mod.m_accel_sine) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_sine->base.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Cycle") == 0) {
        if (ctx->m_mod.m_accel_sine) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_sine->cycle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseColorA") == 0) {
        if (ctx->m_mod.m_color_fixed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_fixed->base_color.a);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseColorR") == 0) {
        if (ctx->m_mod.m_color_fixed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_fixed->base_color.r);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseColorG") == 0) {
        if (ctx->m_mod.m_color_fixed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_fixed->base_color.g);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseColorB") == 0) {
        if (ctx->m_mod.m_color_fixed) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_fixed->base_color.b);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OrbitRate") == 0) {
        if (ctx->m_mod.m_location_orbit) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_orbit->orbit_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OrbitX") == 0) {
        if (ctx->m_mod.m_location_orbit) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_orbit->orbit_x);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OrbitY") == 0) {
        if (ctx->m_mod.m_location_orbit) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_orbit->orbit_y);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OrbitOffset") == 0) {
        if (ctx->m_mod.m_location_orbit) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_orbit->orbit_offset);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "SpeedU") == 0) {
        if (ctx->m_mod.m_texcoord_scroll_anim) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_texcoord_scroll_anim->speed_u);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "SpeedV") == 0) {
        if (ctx->m_mod.m_texcoord_scroll_anim) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_texcoord_scroll_anim->speed_v);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TileIndex") == 0) {
        if (ctx->m_mod.m_texcoord_tile_sub_tex) {
            UI_R_XML_READ_VALUE_INT32(ctx->m_mod.m_texcoord_tile_sub_tex->tile_index);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "EllipseAxisX") == 0) {
        if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->ellipse_axis_x);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "EllipseAxisY") == 0) {
        if (ctx->m_mod.m_uber_ellipse_spawn) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_ellipse_spawn->ellipse_axis_y);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinStopThreshold") == 0) {
        if (ctx->m_mod.m_velocity_threshold_stop) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_threshold_stop->min_stop_threshold);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxStopThreshold") == 0) {
        if (ctx->m_mod.m_velocity_threshold_stop) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_threshold_stop->max_stop_threshold);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "IsDraw") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_emitter_data->is_render);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OnParticleStart") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->on_particle_begin_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OnParticleEnd") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->on_particle_end_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OnEmitterStart") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->on_emitter_begin_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OnEmitterEnd") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->on_emitter_end_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DeadAnim") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->dead_anim_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "IsSingleFollow") == 0) {
        if (ctx->m_mod.m_accel_attract) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_mod.m_accel_attract->is_location_local);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "StartConditionTime") == 0) {
        if (ctx->m_mod_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod_data->begin_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "StartConditionDis") == 0) {
        if (ctx->m_mod_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod_data->begin_distance);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "EndConditionTime") == 0) {
        if (ctx->m_mod_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod_data->end_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "EndConditionDis") == 0) {
        if (ctx->m_mod_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod_data->end_distance);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractMode") == 0) {
        if (ctx->m_mod.m_accel_attract) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_mod.m_accel_attract->attract_type);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAIMovePowerMaxVelocity") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_MOVE) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_move.max_velocity);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAIMovePowerMaxAccel") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_MOVE) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_move.max_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAISpringK") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_SPRING) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_spring.K);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAISpringMaxDistance") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_SPRING) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_spring.max_distance);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAISpringBaseDistance") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_SPRING) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_spring.base_distance);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAIGravitationG") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_GRAVITATION) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_gravitation.G);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAIGravitationMaxDistance") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_GRAVITATION) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_gravitation.max_distance);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AttractAIGravitationMinDistance") == 0) {
        if (ctx->m_mod.m_accel_attract && ctx->m_mod.m_accel_attract->attract_type == UI_PARTICLE_ATTRACT_MOD_TYPE_AI_GRAVITATION) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_attract->attract_data.ai_gravitation.min_distance);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ParticleRepeatTimes") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_emitter_data->particle_repeat_times);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "UseMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_emitter_data->use_state);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "GroupInfo") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->group_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BindEmitters") == 0) {
        if (ctx->m_emitter_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_emitter_data->bind_emitters_id);
        }
    }
}

static void ui_proj_load_particle_structed_error(void * iprojutCtx, xmlErrorPtr err) {
    struct ui_proj_load_particle_ctx * ctx = (struct ui_proj_load_particle_ctx *)(iprojutCtx);

    if (err->code == XML_ERR_DOCUMENT_END) {
        ((xmlParserCtxtPtr)err->ctxt)->wellFormed = 1;
        xmlCtxtResetLastError(err->ctxt);
    }
    else {
        CPE_ERROR_SET_LEVEL(
            ctx->m_em,
            err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

        CPE_ERROR_SET_LINE(ctx->m_em, err->line);

        cpe_error_do_notify(ctx->m_em, "(%d) %s", err->code, err->message);
    }
}

static xmlSAXHandler g_ui_proj_load_particle_callbacks = {
    NULL /* internalSubsetSAXFunc internalSubset */
    , NULL /* isStandaloneSAXFunc isStandalone */
    , NULL /* hasInternalSubsetSAXFunc hasInternalSubset */
    , NULL /* hasExternalSubsetSAXFunc hasExternalSubset */
    , NULL /* resolveEntitySAXFunc resolveEntity */
    , NULL /* getEntitySAXFunc getEntity */
    , NULL /* entityDeclSAXFunc entityDecl */
    , NULL /* notationDeclSAXFunc notationDecl */
    , NULL /* attributeDeclSAXFunc attributeDecl */
    , NULL /* elementDeclSAXFunc elementDecl */
    , NULL /* uprojarsedEntityDeclSAXFunc uprojarsedEntityDecl */
    , NULL /* setDocumentLocatorSAXFunc setDocumentLocator */
    , NULL /* startDocumentSAXFunc startDocument */
    , NULL /* endDocumentSAXFunc endDocument */
    , NULL /* startElementSAXFunc startElement */
    , NULL /* endElementSAXFunc endElement */
    , NULL /* referenceSAXFunc reference */
    , ui_proj_load_particle_characters /* charactersSAXFunc characters */
    , NULL /* ignorableWhitespaceSAXFunc ignorableWhitespace */
    , NULL /* processingInstructionSAXFunc processingInstruction */
    , NULL /* commentSAXFunc comment */
    , NULL /* warningSAXFunc warning */
    , NULL /* errorSAXFunc error */
    , NULL /* fatalErrorSAXFunc fatalError; unused error() get all the errors * */
    , NULL /* getParameterEntitySAXFunc getParameterEntity */
    , NULL /* cdataBlockSAXFunc cdataBlock */
    , NULL /* externalSubsetSAXFunc externalSubset */
    , XML_SAX2_MAGIC /* unsigned int initialized */
    , NULL /* void *_private */
    , ui_proj_load_particle_startElement /* startElementNsSAX2Func startElementNs */
    , ui_proj_load_particle_endElement /* endElementNsSAX2Func endElementNs */
    , ui_proj_load_particle_structed_error /* xmlStructuredErrorFunc serror */
};

void plugin_particle_proj_load_particle_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    plugin_particle_manip_t manip = p;
    struct ui_proj_load_particle_ctx ctx;
    struct mem_buffer path_buff;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    const char * path;

    bzero(&ctx, sizeof(ctx));
    ctx.m_manip = p;
    ctx.m_src = src;
    ctx.m_em = em;

    ctx.m_particle = plugin_particle_data_create(manip->m_particle_module, src);
    if (ctx.m_particle == NULL) {
        CPE_ERROR(em, "proj load particle: create particle fail");
        return;
    }

    ctx.m_string_table = ui_data_src_strings_build_begin(src);
    if (ctx.m_string_table == NULL) {
        CPE_ERROR(em, "proj load particle: create string table builder fail");
        plugin_particle_data_free(ctx.m_particle);
        return;
    }
    
    mem_buffer_init(&path_buff, NULL);

    stream_printf((write_stream_t)&stream, "%s/", ui_data_src_data(ui_data_mgr_src_root(mgr)));
    ui_data_src_path_print((write_stream_t)&stream, src);
    stream_printf((write_stream_t)&stream, ".%s", "particle");
    stream_putc((write_stream_t)&stream, 0);
    path = mem_buffer_make_continuous(&path_buff, 0);

    if (xmlSAXUserParseFile(&g_ui_proj_load_particle_callbacks, &ctx, path) < 0) {
        CPE_ERROR(em, "parse fail!");
        plugin_particle_data_free(ctx.m_particle);
        mem_buffer_clear(&path_buff);
        return;
    }

    mem_buffer_clear(&path_buff);
}

int plugin_particle_proj_load_particle(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        plugin_particle_proj_load_particle_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        plugin_particle_proj_load_particle_i(ctx, mgr, src, &logError);
    }

    return ret;
}

void plugin_particle_manip_install_proj_loader(plugin_particle_manip_t manip) {
    ui_data_mgr_set_loader(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_particle,
        plugin_particle_proj_load_particle,
        manip);
}
