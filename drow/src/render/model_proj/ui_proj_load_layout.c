#include <assert.h>
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_string_table_builder.h"
#include "render/model/ui_data_layout.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "ui_proj_loader_i.h"
#include "ui_proj_utils.h"
#include "ui_proj_load_utils.h"

struct ui_proj_load_layout_ctx {
    gd_app_context_t m_app;
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    ui_string_table_builder_t m_string_table;
    mem_buffer_t m_tmp_buffer;
    error_monitor_t m_em;
    ui_data_layout_t m_layout;
    ui_data_control_t m_cur_control;
    ui_data_control_anim_t m_cur_anim;
    ui_data_control_anim_frame_t * m_anim_frames;
    int32_t m_anim_frame_pos;
    uint32_t m_anim_frame_count;
    uint32_t m_anim_frame_capacity;
    UI_CONTROL_ANIM_FRAME * m_cur_anim_frame;
    UI_CONTROL * m_control_data;
    UI_CONTROL_RES_REF * m_data_res_ref;
    UI_CONTROL_TEXT * m_data_text;
    UI_CONTROL_DOWN * m_data_down;
    UI_CONTROL_SCROLL * m_data_scroll;
    UI_CONTROL_BOX * m_data_box;
    UI_CONTROL_GROUP * m_data_group;
    UI_CONTROL_CHECK * m_data_check;
    UI_CONTROL_EDITOR * m_data_editor;
    UI_FONT_DROW * m_font_drow;
    UI_CONTROL_ADDITION * m_cur_addition;
    char m_cur_tag_name[64];
};

static void ui_proj_load_control_init_data(struct ui_proj_load_layout_ctx * ctx, uint8_t type);
static void ui_proj_load_control_check_create_anim_frame(struct ui_proj_load_layout_ctx * ctx);
static void ui_proj_load_control_common_attrs(struct ui_proj_load_layout_ctx * ctx, int nb_attributes, const xmlChar** attributes);
static void ui_proj_load_layout_start_addition_res_ref(struct ui_proj_load_layout_ctx * ctx, uint8_t layer, uint8_t usage);

static void ui_proj_load_layout_startElement(
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
    struct ui_proj_load_layout_ctx * ctx = (struct ui_proj_load_layout_ctx *)(iprojutCtx);
    uint8_t i;

    for(i = UI_CONTROL_TYPE_MIN; i < UI_CONTROL_TYPE_MAX; ++i) {
        if (strcmp((const char *)localname, ui_data_proj_control_tag_name(i)) == 0) {
            ctx->m_cur_control = ui_data_control_create(ctx->m_layout, ctx->m_cur_control);
            if (ctx->m_cur_control  == NULL) {
                CPE_ERROR(ctx->m_em, "create actor fail!");
                ui_proj_load_control_init_data(ctx, 0);
                return;
            }

            ui_proj_load_control_init_data(ctx, i);
            ui_proj_load_control_common_attrs(ctx, nb_attributes, attributes);
            return;
        }
    }

    if (strcmp((const char *)localname, "RGUIPage") == 0) {
        ctx->m_cur_control = ui_data_control_create(ctx->m_layout, ctx->m_cur_control);
        if (ctx->m_cur_control  == NULL) {
            CPE_ERROR(ctx->m_em, "create actor fail!");
            ui_proj_load_control_init_data(ctx, 0);
            return;
        }

        ui_proj_load_control_init_data(ctx, ui_control_type_panel);
        ui_proj_load_control_common_attrs(ctx, nb_attributes, attributes);
        return;
    }
    
    if (strcmp((const char *)localname, "RenderPT") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_control_data->basic.render_pt);
        }
    }
    else if (strcmp((const char *)localname, "EditorPT") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->basic.editor_pt);
        }
    }
    else if (strcmp((const char *)localname, "EditorSZ") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->basic.editor_sz);
        }
    }
    else if (strcmp((const char *)localname, "EditorSZ") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->basic.editor_sz);
        }
    }
    else if (strcmp((const char *)localname, "EditorPD") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_RECT(&ctx->m_control_data->basic.editor_pd);
        }
    }
    else if (strcmp((const char *)localname, "RenderSZ") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_control_data->basic.render_sz);
        }
    }
    else if (strcmp((const char *)localname, "ClientPD") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_UNIT_RECT(&ctx->m_control_data->basic.client_pd);
        }
    }
    else if (strcmp((const char *)localname, "Pivot") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->basic.pivot);
        }
    }
    else if (strcmp((const char *)localname, "Scale") == 0) {
        if (ctx->m_cur_anim) {
            ui_proj_load_control_check_create_anim_frame(ctx);
            ctx->m_cur_anim_frame->have_scale = 1;
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_cur_anim_frame->scale.data);
        }
        else if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->basic.scale);
        }
    }
    else if (strcmp((const char *)localname, "FloatScale") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->basic.float_scale);
        }
    }
    else if (strcmp((const char *)localname, "Angle") == 0) {
        if (ctx->m_cur_anim) {
            ui_proj_load_control_check_create_anim_frame(ctx);
            ctx->m_cur_anim_frame->have_angle = 1;
            UI_R_XML_READ_ATTR_VECTOR_3(&ctx->m_cur_anim_frame->angle.data);
        }
        else if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_VECTOR_3(&ctx->m_control_data->basic.angle);
        }
    }
    else if (strcmp((const char *)localname, "Color") == 0) {
        if (ctx->m_cur_anim) {
            ui_proj_load_control_check_create_anim_frame(ctx);
            ctx->m_cur_anim_frame->have_color = 1;
            UI_R_XML_READ_ATTR_COLOR(&ctx->m_cur_anim_frame->color.data);
        }
        else if (ctx->m_data_res_ref) {
            UI_R_XML_READ_ATTR_COLOR(&ctx->m_data_res_ref->color);
        }
        else if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_COLOR(&ctx->m_control_data->basic.color);
        }
    }
    else if (strcmp((const char *)localname, "Trans") == 0) {
        if (ctx->m_cur_anim) {
            ui_proj_load_control_check_create_anim_frame(ctx);
            ctx->m_cur_anim_frame->have_trans = 1;
            UI_R_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_cur_anim_frame->trans.data);
        }
    }
    else if (strcmp((const char *)localname, "URect") == 0) {
        if (ctx->m_data_res_ref) {
            UI_R_XML_READ_ATTR_URECT(&ctx->m_data_res_ref->urect);
        }
    }
    else if (strcmp((const char *)localname, "ERect") == 0) {
        if (ctx->m_data_res_ref) {
            UI_R_XML_READ_ATTR_RECT(&ctx->m_data_res_ref->erect);
        }
    }
    else if (strcmp((const char *)localname, "GrayColor") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_ATTR_COLOR(&ctx->m_control_data->basic.gray_color);
        }
    }
    else if (strcmp((const char *)localname, "BackFrame") == 0) {
        if (ctx->m_cur_control) {
            ui_proj_load_layout_start_addition_res_ref(ctx, ui_control_frame_layer_back, ui_control_frame_usage_normal);
        }
    }
    else if (strcmp((const char *)localname, "DisableFrame") == 0) {
        if (ctx->m_cur_control) {
            ui_proj_load_layout_start_addition_res_ref(ctx, ui_control_frame_layer_back, ui_control_frame_usage_gray);
        }
    }
    else if (strcmp((const char *)localname, "FloatPicture") == 0) {
        if (ctx->m_cur_control) {
            ui_proj_load_layout_start_addition_res_ref(ctx, ui_control_frame_layer_float, ui_control_frame_usage_normal);
        }
    }
    else if (strcmp((const char *)localname, "ForeFrame") == 0) {
        if (ctx->m_cur_control) {
            ui_proj_load_layout_start_addition_res_ref(ctx, ui_control_frame_layer_tail, ui_control_frame_usage_normal);
        }
    }
    else if (strcmp((const char *)localname, "TextFontInfo") == 0) {
        if (ctx->m_data_text) {
            UI_R_XML_READ_ATTR_FONTINFO(&ctx->m_data_text->font_info);
        }
    }
    else if (strcmp((const char *)localname, "TextFontUnit") == 0) {
        if (ctx->m_data_text) {
            UI_R_XML_READ_ATTR_UNIT(&ctx->m_data_text->font_unit);
        }
    }
    else if (strcmp((const char *)localname, "TextBackDraw") == 0) {
        if (ctx->m_data_text) {
            UI_R_XML_READ_ATTR_FONTDROW(&ctx->m_data_text->back_drow);
            ctx->m_font_drow = &ctx->m_data_text->back_drow;
        }
    }
    else if (strcmp((const char *)localname, "TextBackGrap") == 0) {
        if (ctx->m_data_text) {
            UI_R_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_data_text->back_grap);
        }
    }
    else if (strcmp((const char *)localname, "TextDownDist") == 0) {
        if (ctx->m_data_down) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_data_down->down_dist);
        }
    }
    else if (strcmp((const char *)localname, "DownFrameScale") == 0) {
        if (ctx->m_data_down) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_data_down->down_scale);
        }
    }
    else if (strcmp((const char *)localname, "TextDownDraw") == 0) {
        if (ctx->m_data_down) {
            UI_R_XML_READ_ATTR_FONTDROW(&ctx->m_data_down->down_text.back_drow);
            ctx->m_font_drow = &ctx->m_data_down->down_text.back_drow;
        }
    }
    else if (strcmp((const char *)localname, "HintTextDraw") == 0) {
        if (ctx->m_data_editor) {
            UI_R_XML_READ_ATTR_FONTDROW(&ctx->m_data_editor->hint_draw);
            ctx->m_font_drow = &ctx->m_data_editor->hint_draw;
        }
    }
    else if (strcmp((const char *)localname, "DownFrame") == 0) {
        if (ctx->m_data_down) {
            ui_proj_load_layout_start_addition_res_ref(ctx, ui_control_frame_layer_back, ui_control_frame_usage_down);
        }
    }
    else if (strcmp((const char *)localname, "NormalColor") == 0) {
        if (ctx->m_font_drow) {
            UI_R_XML_READ_ATTR_COLOR(&ctx->m_font_drow->normal_color);
        }
    }
    else if (strcmp((const char *)localname, "ShadowColor") == 0) {
        if (ctx->m_font_drow) {
            UI_R_XML_READ_ATTR_COLOR(&ctx->m_font_drow->shadow_color);
        }
    }
    else if (strcmp((const char *)localname, "StrokeColor") == 0) {
        if (ctx->m_font_drow) {
            UI_R_XML_READ_ATTR_COLOR(&ctx->m_font_drow->stroke_color);
        }
    }
    else if (strcmp((const char *)localname, "VScrollBarFrame") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_data_scroll->v_scroll_bar.res);
        }
    }
    else if (strcmp((const char *)localname, "VScrollMidFrame") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_data_scroll->v_scroll_mid.res);
        }
    }
    else if (strcmp((const char *)localname, "HScrollBarFrame") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_data_scroll->h_scroll_bar.res);
        }
    }
    else if (strcmp((const char *)localname, "HScrollMidFrame") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_data_scroll->h_scroll_mid.res);
        }
    }
    else if (strcmp((const char *)localname, "LightFrame") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_data_box->light_frame.res);
        }
    }
    else if (strcmp((const char *)localname, "CurrPageFrame") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_swiper) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_control_data->data.swiper.curr_page_frame.res);
        }
    }
    else if (strcmp((const char *)localname, "GrayPageFrame") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_swiper) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_control_data->data.swiper.gray_page_frame.res);
        }
    }
    else if (strcmp((const char *)localname, "UnitHeight") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_multi_edit_box) {
            UI_R_XML_READ_ATTR_UNIT(&ctx->m_control_data->data.multi_edit_box.unit_height);
        }
        else if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_rich_text) {
            UI_R_XML_READ_ATTR_UNIT(&ctx->m_control_data->data.rich_text.unit_height);
        }
    }
    else if (strcmp((const char *)localname, "StatusFrame") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_slider) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_control_data->data.slider.status_frame.res);
        }
    }
    else if (strcmp((const char *)localname, "TurnonFrame") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_switcher) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_control_data->data.switcher.turnon_frame.res);
        }
    }
    else if (strcmp((const char *)localname, "ToggleGrap") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab) {
            UI_R_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_control_data->data.tab.toggle_grap_unit);
        }
    }
    else if (strcmp((const char *)localname, "ToggleRela") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab) {
            UI_R_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_control_data->data.tab.toggle_rela_unit);
        }
    }
    else if (strcmp((const char *)localname, "ToggleGrapReal") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->data.tab.toggle_grap_real);
        }
    }
    else if (strcmp((const char *)localname, "ToggleRelaReal") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab) {
            UI_R_XML_READ_ATTR_VECTOR_2(&ctx->m_control_data->data.tab.toggle_rela_real);
        }
    }
    else if (strcmp((const char *)localname, "Frame") == 0) {
        if (ctx->m_cur_anim) {
            ui_proj_load_control_check_create_anim_frame(ctx);
            cpe_str_dup(ctx->m_cur_tag_name, sizeof(ctx->m_cur_tag_name), (const char*)localname);
        }
        else if (ctx->m_data_res_ref) {
            UI_R_XML_READ_ATTR_URL(&ctx->m_data_res_ref->res);
        }
    }
    else if (strcmp((const char *)localname, "Icon") == 0) {
        if (ctx->m_cur_addition) {
            if (ctx->m_cur_addition->type == ui_control_addition_type_rich_element) {
                UI_R_XML_READ_ATTR_URL(&ctx->m_cur_addition->data.rich_element.icon);
            }
        }
    }
    else if (strcmp((const char *)localname, "FireAnim") == 0) {
        if (ctx->m_cur_control) {
            ui_data_control_addition_t addition = ui_data_control_addition_create(ctx->m_cur_control);
            UI_CONTROL_ADDITION * addition_data = ui_data_control_addition_data(addition);

            addition_data->type = ui_control_addition_type_animation;
            addition_data->data.animation.res.type = UI_OBJECT_TYPE_ACTOR;
            UI_R_XML_READ_ATTR_UINT32(addition_data->data.animation.res.res_id, "Actorid");
            UI_R_XML_READ_ATTR_UINT32(addition_data->data.animation.res.src_id, "Resfile");
            UI_R_XML_READ_ATTR_UINT16(addition_data->data.animation.trigger, "FireEvent");
            UI_R_XML_READ_ATTR_BOOL(addition_data->data.animation.draw_frame, "DrawFrame");
        }
    }
    else if (strcmp((const char *)localname, "Alpha") == 0) {
        if (ctx->m_cur_anim) {
            ui_proj_load_control_check_create_anim_frame(ctx);
            cpe_str_dup(ctx->m_cur_tag_name, sizeof(ctx->m_cur_tag_name), (const char*)localname);
        }
        else {
            cpe_str_dup(ctx->m_cur_tag_name, sizeof(ctx->m_cur_tag_name), (const char*)localname);
        }
    }
    else if (strcmp((const char *)localname, "ShowAnimData") == 0) {
        if (ctx->m_cur_control) {
            ctx->m_cur_anim = ui_data_control_anim_create(ctx->m_cur_control);
            ui_data_control_anim_data(ctx->m_cur_anim)->anim_type = ui_control_anim_type_show;
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->soft, "Soft");
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->loop, "Loop");
            ctx->m_anim_frame_count = 0;
        }
    }
    else if (strcmp((const char *)localname, "HideAnimData") == 0) {
        if (ctx->m_cur_control) {
            ctx->m_cur_anim = ui_data_control_anim_create(ctx->m_cur_control);
            ui_data_control_anim_data(ctx->m_cur_anim)->anim_type = ui_control_anim_type_hide;
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->soft, "Soft");
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->loop, "Loop");
            ctx->m_anim_frame_count = 0;
        }
    }
    else if (strcmp((const char *)localname, "DeadAnimData") == 0) {
        if (ctx->m_cur_control) {
            ctx->m_cur_anim = ui_data_control_anim_create(ctx->m_cur_control);
            ui_data_control_anim_data(ctx->m_cur_anim)->anim_type = ui_control_anim_type_dead;
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->soft, "Soft");
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->loop, "Loop");
            ctx->m_anim_frame_count = 0;
        }
    }
    else if (strcmp((const char *)localname, "DownAnimData") == 0) {
        if (ctx->m_cur_control) {
            ctx->m_cur_anim = ui_data_control_anim_create(ctx->m_cur_control);
            ui_data_control_anim_data(ctx->m_cur_anim)->anim_type = ui_control_anim_type_down;
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->soft, "Soft");
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->loop, "Loop");
            ctx->m_anim_frame_count = 0;
        }
    }
    else if (strcmp((const char *)localname, "RiseAnimData") == 0) {
        if (ctx->m_cur_control) {
            ctx->m_cur_anim = ui_data_control_anim_create(ctx->m_cur_control);
            ui_data_control_anim_data(ctx->m_cur_anim)->anim_type = ui_control_anim_type_rise;
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->soft, "Soft");
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->loop, "Loop");
            ctx->m_anim_frame_count = 0;
        }
    }
    else if (strcmp((const char *)localname, "UserAnimData") == 0) {
        if (ctx->m_cur_control) {
            ctx->m_cur_anim = ui_data_control_anim_create(ctx->m_cur_control);
            ui_data_control_anim_data(ctx->m_cur_anim)->anim_type = ui_control_anim_type_user;
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->soft, "Soft");
            UI_R_XML_READ_ATTR_BOOL(ui_data_control_anim_data(ctx->m_cur_anim)->loop, "Loop");
            ctx->m_anim_frame_count = 0;
        }
    }
    else if (strcmp((const char *)localname, "FrameRoot") == 0) {
        ctx->m_anim_frame_pos = -1;
    }
    else if (strcmp((const char *)localname, "ScaleRoot") == 0) {
        ctx->m_anim_frame_pos = -1;
    }
    else if (strcmp((const char *)localname, "TransRoot") == 0) {
        ctx->m_anim_frame_pos = -1;
    }
    else if (strcmp((const char *)localname, "ColorRoot") == 0) {
        ctx->m_anim_frame_pos = -1;
    }
    else if (strcmp((const char *)localname, "AlphaRoot") == 0) {
        ctx->m_anim_frame_pos = -1;
    }
    else if (strcmp((const char *)localname, "AngleRoot") == 0) {
        ctx->m_anim_frame_pos = -1;
    }
    else if (strcmp((const char *)localname, "RenderData") == 0) {
        if (ctx->m_cur_control) {
            ui_proj_load_layout_start_addition_res_ref(ctx, ui_control_frame_layer_back, ui_control_frame_usage_normal);
        }
    }
    else if (strcmp((const char *)localname, "RenderText") == 0) {
        if (ctx->m_cur_control) {
            ui_data_control_addition_t addition = ui_data_control_addition_create(ctx->m_cur_control);
            ctx->m_cur_addition = ui_data_control_addition_data(addition);
            ctx->m_cur_addition->type = ui_control_addition_type_text;
            ctx->m_cur_addition->data.text.text_key = (uint32_t)-1;
        }
    }
    else if (strcmp((const char *)localname, "RichElem") == 0) {
        if (ctx->m_cur_control) {
            ui_data_control_addition_t addition = ui_data_control_addition_create(ctx->m_cur_control);
            ctx->m_cur_addition = ui_data_control_addition_data(addition);
            ctx->m_cur_addition->type = ui_control_addition_type_rich_element;
            ctx->m_cur_addition->data.rich_element.text_key = (uint32_t)-1;
        }
    }
    else if (strcmp((const char *)localname, "TextDraw") == 0) {
        if (ctx->m_cur_addition && ctx->m_cur_addition->type == ui_control_addition_type_text) {
            UI_R_XML_READ_ATTR_FONTDROW(&ctx->m_cur_addition->data.text.text_drow);
            ctx->m_font_drow = &ctx->m_cur_addition->data.text.text_drow;
        }
        else if (ctx->m_cur_addition && ctx->m_cur_addition->type == ui_control_addition_type_rich_element) {
            UI_R_XML_READ_ATTR_FONTDROW(&ctx->m_cur_addition->data.rich_element.text_drow);
            ctx->m_font_drow = &ctx->m_cur_addition->data.rich_element.text_drow;
        }
    }
    else {
        cpe_str_dup(ctx->m_cur_tag_name, sizeof(ctx->m_cur_tag_name), (const char*)localname);
    }
}

static void ui_proj_load_layout_on_control_done(struct ui_proj_load_layout_ctx * ctx) {
    assert(ctx->m_cur_control);

    if (ctx->m_data_text) {
        if (ctx->m_data_text->text_key == (uint32_t)-1) {
            if (ctx->m_data_text->text_id) {
                CPE_INFO(
                    ctx->m_em, "control %s text no text key!",
                    ui_string_table_builder_msg_get(ctx->m_string_table, ui_data_control_data(ctx->m_cur_control)->name_id));
            }
            ctx->m_data_text->text_key = 0;
        }
    }

    if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab_page) {
        if (ctx->m_control_data->data.tab_page.toggle_text_key == (uint32_t)-1) {
            if (ctx->m_control_data->data.tab_page.toggle_text_id) {
                CPE_INFO(
                    ctx->m_em, "control %s toggle_text no text key!",
                    ui_string_table_builder_msg_get(ctx->m_string_table, ui_data_control_data(ctx->m_cur_control)->name_id));
            }
            ctx->m_control_data->data.tab_page.toggle_text_key = 0;
        }
    }
}

static void ui_proj_load_layout_start_addition_res_ref(struct ui_proj_load_layout_ctx * ctx, uint8_t layer, uint8_t usage) {
    ui_data_control_addition_t addition;
    UI_CONTROL_ADDITION_DATA_RES_REF * addition_res_ref;
            
    addition = ui_data_control_addition_create(ctx->m_cur_control);
    ctx->m_cur_addition = ui_data_control_addition_data(addition);
    ctx->m_cur_addition->type = ui_control_addition_type_res_ref;

    addition_res_ref = &ctx->m_cur_addition->data.res_ref;
    addition_res_ref->usage = usage;
    addition_res_ref->layer = layer;
    ctx->m_data_res_ref = &addition_res_ref->frame;
}

static void ui_proj_load_layout_on_addition_done(struct ui_proj_load_layout_ctx * ctx) {
    assert(ctx->m_cur_addition);
    assert(ctx->m_cur_control);

    if (ctx->m_cur_addition->type == ui_control_addition_type_res_ref) {
        ctx->m_cur_addition = NULL;
        ctx->m_data_res_ref = NULL;
    }
    else if(ctx->m_cur_addition->type == ui_control_addition_type_text) {
        if (ctx->m_cur_addition->data.text.text_key == (uint32_t)-1) {
            if (ctx->m_cur_addition->data.text.text_id) {
                CPE_INFO(
                    ctx->m_em, "control %s addition no text key!",
                    ui_string_table_builder_msg_get(ctx->m_string_table, ui_data_control_data(ctx->m_cur_control)->name_id));
            }
            ctx->m_cur_addition->data.text.text_key = 0;
        }
    }
    else if (ctx->m_cur_addition->type == ui_control_addition_type_rich_element) {
        if (ctx->m_cur_addition->data.rich_element.text_key == (uint32_t)-1) {
            if (ctx->m_cur_addition->data.rich_element.text_id) {
                CPE_INFO(
                    ctx->m_em, "control %s addition no text key!",
                    ui_string_table_builder_msg_get(ctx->m_string_table, ui_data_control_data(ctx->m_cur_control)->name_id));
            }
            ctx->m_cur_addition->data.rich_element.text_key = 0;
        }
    }
    
    ctx->m_cur_addition = NULL;
}

static void ui_proj_load_layout_endElement(
        void* iprojutCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    uint8_t i;
    struct ui_proj_load_layout_ctx * ctx = (struct ui_proj_load_layout_ctx *)(iprojutCtx);
    ctx->m_cur_tag_name[0] = 0;

    if (ctx->m_cur_control == NULL) return;

    if (strcmp((const char *)localname, "BackFrame") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }
    else if (strcmp((const char *)localname, "DisableFrame") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }
    else if (strcmp((const char *)localname, "FloatPicture") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }
    else if (strcmp((const char *)localname, "ForeFrame") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }
    else if (strcmp((const char *)localname, "DownFrame") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }
    else if (strcmp((const char *)localname, "TextBackDraw") == 0) {
        ctx->m_font_drow = NULL;
    }
    else if (strcmp((const char *)localname, "TextDownDraw") == 0) {
        ctx->m_font_drow = NULL;
    }
    else if (strcmp((const char *)localname, "HintTextDraw") == 0) {
        ctx->m_font_drow = NULL;
    }
    else if (strcmp((const char *)localname, "ShowAnimData") == 0) {
        ctx->m_cur_anim = NULL;
    }
    else if (strcmp((const char *)localname, "HideAnimData") == 0) {
        ctx->m_cur_anim = NULL;
    }
    else if (strcmp((const char *)localname, "DeadAnimData") == 0) {
        ctx->m_cur_anim = NULL;
    }
    else if (strcmp((const char *)localname, "DownAnimData") == 0) {
        ctx->m_cur_anim = NULL;
    }
    else if (strcmp((const char *)localname, "RiseAnimData") == 0) {
        ctx->m_cur_anim = NULL;
    }
    else if (strcmp((const char *)localname, "UserAnimData") == 0) {
        ctx->m_cur_anim = NULL;
    }
    else if (strcmp((const char *)localname, "FrameRoot") == 0) {
        ctx->m_cur_anim_frame = NULL;
    }
    else if (strcmp((const char *)localname, "ScaleRoot") == 0) {
        ctx->m_cur_anim_frame = NULL;
    }
    else if (strcmp((const char *)localname, "TransRoot") == 0) {
        ctx->m_cur_anim_frame = NULL;
    }
    else if (strcmp((const char *)localname, "ColorRoot") == 0) {
        ctx->m_cur_anim_frame = NULL;
    }
    else if (strcmp((const char *)localname, "AlphaRoot") == 0) {
        ctx->m_cur_anim_frame = NULL;
    }
    else if (strcmp((const char *)localname, "AngleRoot") == 0) {
        ctx->m_cur_anim_frame = NULL;
    }
    else if (strcmp((const char *)localname, "RenderData") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }
    else if (strcmp((const char *)localname, "RenderText") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }
    else if (strcmp((const char *)localname, "RichElem") == 0) {
        ui_proj_load_layout_on_addition_done(ctx);
    }

    for(i = UI_CONTROL_TYPE_MIN; i < UI_CONTROL_TYPE_MAX; ++i) {
        if (strcmp((const char *)localname, ui_data_proj_control_tag_name(i)) == 0) {
            ui_proj_load_layout_on_control_done(ctx);
            ctx->m_cur_control = ui_data_control_parent(ctx->m_cur_control);
            ui_proj_load_control_init_data(ctx, 0);
            return;
        }
    }

    if (strcmp((const char *)localname, "RGUIPage") == 0) {
        ctx->m_cur_control = ui_data_control_parent(ctx->m_cur_control);
        ui_proj_load_control_init_data(ctx, 0);
        return;
    }
}

static void ui_proj_load_layout_characters(void * iprojutCtx, const xmlChar *ch, int len) {
    struct ui_proj_load_layout_ctx * ctx = (struct ui_proj_load_layout_ctx *)(iprojutCtx);

    if (strcmp(ctx->m_cur_tag_name, "UserText") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_control_data->basic.user_text_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AlignHorz") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_control_data->basic.align_horz);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AlignVert") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_control_data->basic.align_vert);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Visible") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.visible);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Frame") == 0) {
        if (ctx->m_cur_anim_frame) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_cur_anim_frame->frame);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Alpha") == 0) {
        if (ctx->m_cur_anim_frame) {
            ctx->m_cur_anim_frame->have_alpha = 1;
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_cur_anim_frame->alpha.data);
        }
        else if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_control_data->basic.alpha);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawAlign") == 0) {
        if (ctx->m_control_data->basic.draw_align) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_control_data->basic.draw_align);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawColor") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.draw_color);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawFrame") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.draw_frame);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawInner") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.draw_inner);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Enable") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.enable);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ForceClip") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.force_clip);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AlwaysTop") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.always_top);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptPTLS") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_pt_ls);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptSZLS") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_sz_ls);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptHits") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_hits);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptClip") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_clip);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ParentClip") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.parent_clip);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptMove") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_move);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptDoubleClick") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_double_click);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptClickMove") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_click_move);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptGloablDownColor") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_global_down_color);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptFloat") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.accept_float);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "FireAnimClip") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->basic.fire_anim_clip);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ExtraTemplate") == 0) {
        if (ctx->m_control_data) {
            char template_name[256];
            UI_R_XML_READ_VALUE_STRING(template_name);
            if (template_name[0] == '/') {
                ctx->m_control_data->basic.is_link = 1;
                ctx->m_control_data->basic.link_control_id =
                    ui_string_table_builder_msg_alloc(ctx->m_string_table, template_name + 1);
            }
            else if (template_name[0]) {
                mem_buffer_t b = gd_app_tmp_buffer(ctx->m_app);
                ui_data_src_t p = ui_data_src_parent(ctx->m_src);
                char * path = template_name;

                while((cpe_str_start_with(path, "../"))) {
                    if (p == NULL) {
                        CPE_ERROR(ctx->m_em, "ExtraTemplate %s path error", template_name);
                        return;
                    }

                    p = ui_data_src_parent(p);
                    path += 3;
                }
                
                mem_buffer_clear_data(b);
                if (p) {
                    ui_data_src_path_dump(b, p);
                    mem_buffer_strcat(b, "/");
                }
                mem_buffer_strcat(b, path);

                ctx->m_control_data->basic.is_link = 1;
                ctx->m_control_data->basic.link_control_id =
                    ui_string_table_builder_msg_alloc(ctx->m_string_table, mem_buffer_make_continuous(b, 0));
            }
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ShowSFXFile") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_control_data->basic.show_sfx_file_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "HideSFXFile") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_control_data->basic.hide_sfx_file_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DnSFXFile") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_control_data->basic.down_sfx_file_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "UpSFXFile") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_control_data->basic.rise_sfx_file_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "PushSFXFile") == 0) {
        if (ctx->m_control_data) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_control_data->basic.push_sfx_file_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "VScrollSoft") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_scroll->v_scroll_soft);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "HScrollSoft") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_scroll->h_scroll_soft);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "VScrollAutoHide") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_scroll->v_scroll_auto_hide);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "HScrollAutoHide") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_scroll->h_scroll_auto_hide);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "CheckFrame") == 0) {
        if (ctx->m_data_check) {
            ctx->m_data_res_ref = &ctx->m_data_check->check_frame;
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Text") == 0) {
        if (ctx->m_cur_addition && ctx->m_cur_addition->type == ui_control_addition_type_text) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_cur_addition->data.text.text_id);
        }
        else if (ctx->m_cur_addition && ctx->m_cur_addition->type == ui_control_addition_type_rich_element) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_cur_addition->data.rich_element.text_id);
        }
        else if (ctx->m_data_text) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_data_text->text_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TextKey") == 0) {
        if (ctx->m_cur_addition && ctx->m_cur_addition->type == ui_control_addition_type_text) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_cur_addition->data.text.text_key);
        }
        else if (ctx->m_cur_addition && ctx->m_cur_addition->type == ui_control_addition_type_rich_element) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_cur_addition->data.rich_element.text_key);
        }
        else if (ctx->m_data_text) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_data_text->text_key);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TextSL") == 0) {
        if (ctx->m_data_text) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_text->text_sl);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TextAlign") == 0) {
        if (ctx->m_data_text) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_data_text->text_align);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Group") == 0) {
        if (ctx->m_data_group) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_data_group->group);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Checked") == 0) {
        if (ctx->m_data_group) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_check->checked);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxLength") == 0) {
        if (ctx->m_data_editor) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_data_editor->max_length);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Password") == 0) {
        if (ctx->m_data_editor) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_editor->is_passwd);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ReadOnly") == 0) {
        if (ctx->m_data_editor) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_editor->is_read_only);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "NumberOnly") == 0) {
        if (ctx->m_data_editor) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_editor->is_number_only);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "HintText") == 0) {
        if (ctx->m_data_editor) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_data_editor->hint_text_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "HorzGrap") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_VALUE_INT32(ctx->m_data_box->horz_grap);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "VertGrap") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_VALUE_INT32(ctx->m_data_box->vert_grap);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LightFrameShow") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_box->light_frame_show);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ItemFreeSize") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_data_box->item_free_size);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LeftBtnTemplate") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_button, ctx->m_data_scroll->left_btn_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "RightBtnTemplate") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_button, ctx->m_data_scroll->right_btn_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TopBtnTemplate") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_button, ctx->m_data_scroll->up_btn_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BottomBtnTemplate") == 0) {
        if (ctx->m_data_scroll) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_button, ctx->m_data_scroll->down_btn_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ItemTemplate") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_list_box_adv_item, ctx->m_data_box->item_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "HeadTemplate") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_list_box_adv_item, ctx->m_data_box->head_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TailTemplate") == 0) {
        if (ctx->m_data_box) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_list_box_adv_item, ctx->m_data_box->tail_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Modal") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_window) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->data.window.mudal);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "StartVisible") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_window) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->data.window.start_show);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DialogTemplate") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_window) {
            UI_R_XML_READ_VALUE_BOOL(ctx->m_control_data->data.window.dialog_template);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DropListBoxTemplate") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_combo_box) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_comb_box_drop_list, ctx->m_control_data->data.combo_box.drop_list_box_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DropPushBtnTemplate") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_combo_box) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_toggle, ctx->m_control_data->data.combo_box.drop_push_btn_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Index") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_picture_cond) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.picture_cond.index);
        }
        else if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_label_condition) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.label_condition.index);
        }
        else if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_rich_label) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.rich_label.index);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ColCount") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_list_box_col) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.list_box_col.col_count);
        }
        else if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_comb_box_drop_list) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.comb_box_drop_list.col_count);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "RowCount") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_list_box_row) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.list_box_row.row_count);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxDropItem") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_comb_box_drop_list) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.comb_box_drop_list.max_drop_item);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LineHeight") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_multi_edit_box) {
            UI_R_XML_READ_VALUE_INT32(ctx->m_control_data->data.multi_edit_box.line_height);
        }
        else if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_rich_text) {
            UI_R_XML_READ_VALUE_INT32(ctx->m_control_data->data.rich_text.line_height);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "SliderRange") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_slider) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.slider.slider_range);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MidBtnTemplate") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_slider) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_button, ctx->m_control_data->data.slider.mid_btn_template_id);
        }
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_switcher) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_button, ctx->m_control_data->data.switcher.mid_btn_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ToggleTemplate") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_toggle, ctx->m_control_data->data.tab.toggle_template_id);
        }
        else if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab_page) {
            UI_R_XML_READ_VALUE_TEMPLATE(ui_control_type_toggle, ctx->m_control_data->data.tab_page.toggle_template_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ToggleDockStyle") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_control_data->data.tab.toggle_dock);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ToggleText") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab_page) {
            UI_R_XML_READ_VALUE_STRING_ID(ctx->m_control_data->data.tab_page.toggle_text_id);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ToggleTextKey") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_tab_page) {
            UI_R_XML_READ_VALUE_UINT32(ctx->m_control_data->data.tab_page.toggle_text_key);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Type") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_progress) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_control_data->data.progress.type);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Mode") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_progress) {
            UI_R_XML_READ_VALUE_UINT8(ctx->m_control_data->data.progress.mode);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Speed") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_progress) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_control_data->data.progress.speed);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Progress") == 0) {
        if (ctx->m_control_data && ctx->m_control_data->type == ui_control_type_progress) {
            UI_R_XML_READ_VALUE_FLOAT(ctx->m_control_data->data.progress.cur_progress);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "IsText") == 0) {
        if (ctx->m_cur_addition && ctx->m_cur_addition->type == ui_control_addition_type_res_ref) {
            uint8_t is_text;
            UI_R_XML_READ_VALUE_BOOL(is_text);
            if (is_text) {
                ctx->m_cur_addition->data.res_ref.layer = ui_control_frame_layer_text;
            }
        }
    }
}

static void ui_proj_load_layout_structed_error(void * iprojutCtx, xmlErrorPtr err) {
    struct ui_proj_load_layout_ctx * ctx = (struct ui_proj_load_layout_ctx *)(iprojutCtx);

    CPE_ERROR_SET_LEVEL(
        ctx->m_em,
        err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

    CPE_ERROR_SET_LINE(ctx->m_em, err->line);

    cpe_error_do_notify(ctx->m_em, "(%d) %s", err->code, err->message);
}

static xmlSAXHandler g_ui_proj_load_layout_callbacks = {
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
    , ui_proj_load_layout_characters /* charactersSAXFunc characters */
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
    , ui_proj_load_layout_startElement /* startElementNsSAX2Func startElementNs */
    , ui_proj_load_layout_endElement /* endElementNsSAX2Func endElementNs */
    , ui_proj_load_layout_structed_error /* xmlStructuredErrorFunc serror */
};

void ui_data_proj_load_layout_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    ui_proj_loader_t loader = p;
    struct ui_proj_load_layout_ctx ctx;
    struct mem_buffer path_buff;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    const char * path;

    ctx.m_app = loader->m_app;
    ctx.m_mgr = mgr;
    ctx.m_src = src;
    ctx.m_em = em;
    ctx.m_tmp_buffer = gd_app_tmp_buffer(ui_data_mgr_app(mgr));
    ctx.m_layout = ui_data_layout_create(mgr, src);
    if (ctx.m_layout == NULL) {
        CPE_ERROR(em, "create layout fail");
        return;
    }
    ctx.m_string_table = ui_string_table_builder_create(NULL, em);
    if (ctx.m_string_table == NULL) {
        CPE_ERROR(em, "create string table builder fail");
        ui_data_layout_free(ctx.m_layout);
        return;
    }
    ctx.m_cur_control = NULL;
    ctx.m_cur_anim = NULL;
    ctx.m_anim_frames = NULL;
    ctx.m_anim_frame_capacity = 0;
    ctx.m_anim_frame_count = 0;
    
    mem_buffer_init(&path_buff, NULL);

    stream_printf((write_stream_t)&stream, "%s/", ui_data_src_data(ui_data_mgr_src_root(mgr)));
    ui_data_src_path_print((write_stream_t)&stream, src);
    stream_printf((write_stream_t)&stream, ".%s", ui_data_proj_postfix(ui_data_src_type(src)));
    stream_putc((write_stream_t)&stream, 0);
    path = mem_buffer_make_continuous(&path_buff, 0);

    if (!file_exist(path, em)) {
        mem_buffer_clear_data(&path_buff);
        stream_printf((write_stream_t)&stream, "%s/", ui_data_src_data(ui_data_mgr_src_root(mgr)));
        ui_data_src_path_print((write_stream_t)&stream, src);
        stream_printf((write_stream_t)&stream, ".npTemplate");
        stream_putc((write_stream_t)&stream, 0);
        path = mem_buffer_make_continuous(&path_buff, 0);
    }

    if (xmlSAXUserParseFile(&g_ui_proj_load_layout_callbacks, &ctx, path) < 0) {
        CPE_ERROR(em, "parse fail!");
        ui_data_layout_free(ctx.m_layout);
        ui_string_table_builder_free(ctx.m_string_table);
        if (ctx.m_anim_frames) {
            mem_free(NULL, ctx.m_anim_frames);
            ctx.m_anim_frames = NULL;
        }
        mem_buffer_clear(&path_buff);
        return;
    }

    if (ui_string_table_builder_build(ctx.m_string_table, ui_data_layout_strings(ctx.m_layout)) != 0) {
        CPE_ERROR(em, "build string table fail!");
        ui_data_layout_free(ctx.m_layout);
        ui_string_table_builder_free(ctx.m_string_table);
        if (ctx.m_anim_frames) {
            mem_free(NULL, ctx.m_anim_frames);
            ctx.m_anim_frames = NULL;
        }
        mem_buffer_clear(&path_buff);
        return;
    }
    
    if (ctx.m_anim_frames) {
        mem_free(NULL, ctx.m_anim_frames);
        ctx.m_anim_frames = NULL;
    }

    ui_string_table_builder_free(ctx.m_string_table);
    
    mem_buffer_clear(&path_buff);
}

int ui_data_proj_load_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_proj_load_layout_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_proj_load_layout_i(ctx, mgr, src, &logError);
    }

    return ret;
}

static void ui_proj_load_control_init_data(struct ui_proj_load_layout_ctx * ctx, uint8_t control_type) {
    ctx->m_control_data = ctx->m_cur_control ? ui_data_control_data(ctx->m_cur_control) : NULL;
    ctx->m_cur_anim = NULL;
    ctx->m_cur_anim_frame = NULL;
    ctx->m_data_res_ref = NULL;
    ctx->m_data_text = NULL;
    ctx->m_data_down = NULL;
    ctx->m_data_scroll = NULL;
    ctx->m_data_box = NULL;
    ctx->m_data_group = NULL;
    ctx->m_data_check = NULL;
    ctx->m_data_editor = NULL;
    ctx->m_font_drow = NULL;
    ctx->m_cur_addition = NULL;

    if (ctx->m_control_data) {
        if (control_type) ctx->m_control_data->type = control_type;
        
        switch(ctx->m_control_data->type) {
        case ui_control_type_label:
            ctx->m_data_text = &ctx->m_control_data->data.label.text;
            break;
        case ui_control_type_button:
            ctx->m_data_text = &ctx->m_control_data->data.button.text;
            ctx->m_data_down = &ctx->m_control_data->data.button.down;
            break;
        case ui_control_type_toggle:
            ctx->m_data_text = &ctx->m_control_data->data.toggle.text;
            ctx->m_data_down = &ctx->m_control_data->data.toggle.down;
            ctx->m_data_group = &ctx->m_control_data->data.toggle.group;
            break;
        case ui_control_type_progress:
            ctx->m_data_text = &ctx->m_control_data->data.progress.text;
            break;
        case ui_control_type_check_box:
            ctx->m_data_text = &ctx->m_control_data->data.check_box.text;
            ctx->m_data_down = &ctx->m_control_data->data.check_box.down;
            ctx->m_data_check = &ctx->m_control_data->data.radio_box.check;
            break;
        case ui_control_type_radio_box:
            ctx->m_data_text = &ctx->m_control_data->data.radio_box.text;
            ctx->m_data_down = &ctx->m_control_data->data.radio_box.down;
            ctx->m_data_check = &ctx->m_control_data->data.radio_box.check;
            ctx->m_data_group = &ctx->m_control_data->data.radio_box.group;
            break;
        case ui_control_type_combo_box:
            ctx->m_data_text = &ctx->m_control_data->data.combo_box.text;
            break;
        case ui_control_type_edit_box:
            ctx->m_data_text = &ctx->m_control_data->data.edit_box.text;
            ctx->m_data_editor = &ctx->m_control_data->data.edit_box.editor;
            break;
        case ui_control_type_multi_edit_box:
            ctx->m_data_text = &ctx->m_control_data->data.multi_edit_box.text;
            ctx->m_data_editor = &ctx->m_control_data->data.multi_edit_box.editor;
            break;
        case ui_control_type_label_condition:
            ctx->m_data_text = &ctx->m_control_data->data.label_condition.text;
            break;
        case ui_control_type_rich_label:
            ctx->m_data_text = &ctx->m_control_data->data.rich_label.text;
            break;
        case ui_control_type_rich_text:
            ctx->m_data_text = &ctx->m_control_data->data.rich_text.text;
            break;
        case ui_control_type_list_box_col:
            ctx->m_data_scroll = &ctx->m_control_data->data.list_box_col.scroll;
            ctx->m_data_box = &ctx->m_control_data->data.list_box_col.box;
            break;
        case ui_control_type_list_box_row:
            ctx->m_data_scroll = &ctx->m_control_data->data.list_box_row.scroll;
            ctx->m_data_box = &ctx->m_control_data->data.list_box_row.box;
            break;
        case ui_control_type_scroll_panel:
            ctx->m_data_scroll = &ctx->m_control_data->data.scroll_panel.scroll;
            break;
        case ui_control_type_comb_box_drop_list:
            ctx->m_data_scroll = &ctx->m_control_data->data.comb_box_drop_list.scroll;
            ctx->m_data_box = &ctx->m_control_data->data.comb_box_drop_list.box;
            break;
        case ui_control_type_tab_page:
            ctx->m_control_data->data.tab_page.toggle_text_key = -1;
            break;
        default:
            break;
        }

        if (ctx->m_data_text) {
            ctx->m_data_text->text_key = (uint32_t)-1;
        }
    }
}

static void ui_proj_load_control_common_attrs(
    struct ui_proj_load_layout_ctx * ctx,
    int nb_attributes,
    const xmlChar** attributes)
{
    uint32_t id;
    assert(ctx->m_control_data);

    UI_R_XML_READ_ATTR_UINT32(id, "ID");
    ui_data_control_set_id(ctx->m_cur_control, id);

    UI_R_XML_READ_ATTR_STRING_ID(ctx->m_control_data->name_id, "Name");
    UI_R_XML_READ_ATTR_BOOL(ctx->m_control_data->basic.is_link, "TemplateLink");

    UI_R_XML_READ_ATTR_TEMPLATE(ctx->m_control_data->basic.link_control_id, ctx->m_control_data->type, "TemplateLinkCtrl");
}

static void ui_proj_load_control_check_create_anim_frame(struct ui_proj_load_layout_ctx * ctx) {
    if (ctx->m_anim_frame_pos + 1 < ctx->m_anim_frame_count) {
        ctx->m_anim_frame_pos++;
        assert(ctx->m_anim_frame_pos >= 0);
        ctx->m_cur_anim_frame = ui_data_control_anim_frame_data(ctx->m_anim_frames[ctx->m_anim_frame_pos]);
        return;
    }

    if (ctx->m_anim_frame_count + 1 > ctx->m_anim_frame_capacity) {
        uint32_t new_capacity = ctx->m_anim_frame_capacity < 16 ? 16 : ctx->m_anim_frame_capacity * 2;
        ui_data_control_anim_frame_t * new_buff = mem_alloc(NULL, sizeof(ui_data_control_anim_frame_t) * new_capacity);

        if (ctx->m_anim_frame_count > 0) {
            memcpy(new_buff, ctx->m_anim_frames, sizeof(ui_data_control_anim_frame_t) * ctx->m_anim_frame_count);
        }

        mem_free(NULL, ctx->m_anim_frames);
        ctx->m_anim_frames = new_buff;
        ctx->m_anim_frame_capacity = new_capacity;
    }

    ctx->m_anim_frames[ctx->m_anim_frame_count] = ui_data_control_anim_frame_create(ctx->m_cur_anim);
    ctx->m_cur_anim_frame = ui_data_control_anim_frame_data(ctx->m_anim_frames[ctx->m_anim_frame_count]);

    ctx->m_anim_frame_count++;
    ctx->m_anim_frame_pos++;

    assert(ctx->m_anim_frame_pos + 1 == ctx->m_anim_frame_count);
}
