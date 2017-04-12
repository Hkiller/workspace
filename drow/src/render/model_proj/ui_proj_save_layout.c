#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "render/model/ui_data_layout.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_proj_save_utils.h"
#include "ui_proj_utils.h"

struct ui_proj_save_control_ctx {
    ui_data_control_t m_control;
    UI_CONTROL * m_control_data;
};

static const char * ui_data_proj_control_type_name(uint8_t control_type);

static int ui_data_proj_save_control_i(
    write_stream_t s, ui_data_control_t control, uint8_t level, error_monitor_t em)
{
    struct ui_data_control_it control_it;
    ui_data_control_t child_control;
    struct ui_proj_save_control_ctx ctx;
    int rv = 0;
    const char * tag_name;

    /*得到TagName */
    tag_name = ui_data_proj_control_tag_name(ui_data_control_type(control));
    if (tag_name == NULL) {
        CPE_ERROR(em, "ui_data_proj_save_control: not support type %d", ui_data_control_type(control));
        return -1;
    }

    if (level + 1 > 64) {
        CPE_ERROR(em, "ui_data_proj_save_control: level %d overflow!", level);
        return -1;
    }

    ctx.m_control = control;
    ctx.m_control_data = ui_data_control_data(control);
    
    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s ID=\"%u\" Name=\"%s\" Control=\"%s\"",
        tag_name,
        ctx.m_control_data->id,
        ui_data_control_msg(control, ctx.m_control_data->name_id),
        ui_data_proj_control_type_name(ui_data_control_type(control)));
    if (ctx.m_control_data) {
        stream_printf(
            s, " TemplateLink=\"%s\" TemplateLinkCtrl=\"%s\"",
            ctx.m_control_data->basic.is_link ? "True" : "False",
            ui_data_control_msg(ctx.m_control, ctx.m_control_data->basic.link_control_id));
    }
    stream_printf(s, ">\n");

    ui_data_proj_save_unit_vector_2(s, level + 1, "RenderPT", &ctx.m_control_data->basic.render_pt);
    ui_data_proj_save_vector_2(s, level + 1, "EditorPT", &ctx.m_control_data->basic.editor_pt);
    ui_data_proj_save_str(s, level + 1, "UserText", ui_data_control_msg(control, ctx.m_control_data->basic.user_text_id));
    ui_data_proj_save_int(s, level + 1, "AlignHorz", ctx.m_control_data->basic.align_horz);
    ui_data_proj_save_int(s, level + 1, "AlignVert", ctx.m_control_data->basic.align_vert);
    ui_data_proj_save_bool(s, level + 1, "Visible", ctx.m_control_data->basic.visible);

    if (!ctx.m_control_data->basic.is_link) {
        ui_data_proj_save_bool(s, level + 1, "Template", 0);
        ui_data_proj_save_vector_2(s, level + 1, "EditorSZ", &ctx.m_control_data->basic.editor_sz);
        ui_data_proj_save_rect(s, level + 1, "EditorPD", &ctx.m_control_data->basic.editor_pd);
        ui_data_proj_save_unit_vector_2(s, level + 1, "RenderSZ", &ctx.m_control_data->basic.render_sz);
        ui_data_proj_save_unit_rect(s, level + 1, "ClientPD", &ctx.m_control_data->basic.client_pd);
        ui_data_proj_save_vector_2(s, level + 1, "Pivot", &ctx.m_control_data->basic.pivot);
        ui_data_proj_save_vector_2(s, level + 1, "Scale", &ctx.m_control_data->basic.scale);
        ui_data_proj_save_vector_3(s, level + 1, "Angle", &ctx.m_control_data->basic.angle);
        ui_data_proj_save_float(s, level + 1, "Alpha", ctx.m_control_data->basic.alpha);
        ui_data_proj_save_color(s, level + 1, "Color", &ctx.m_control_data->basic.color);
        ui_data_proj_save_color(s, level + 1, "GrayColor", &ctx.m_control_data->basic.gray_color);
        ui_data_proj_save_int(s, level + 1, "DrawAlign", ctx.m_control_data->basic.draw_align);
        /* 	RXmlHelper::ExportArray(node, RGUIKey::BackFrame,			mBackFrame); */
        /*  RXmlHelper::ExportArray(node, RGUIKey::ForeFrame,         mForeFrame); */
        ui_data_proj_save_bool(s, level + 1, "DrawColor", ctx.m_control_data->basic.draw_color);
        ui_data_proj_save_bool(s, level + 1, "DrawFrame", ctx.m_control_data->basic.draw_frame);
        ui_data_proj_save_bool(s, level + 1, "DrawInner", ctx.m_control_data->basic.draw_inner);
        ui_data_proj_save_bool(s, level + 1, "Enable", ctx.m_control_data->basic.enable);
        ui_data_proj_save_bool(s, level + 1, "AlwaysTop", ctx.m_control_data->basic.always_top);
        ui_data_proj_save_bool(s, level + 1, "AcceptPTLS", ctx.m_control_data->basic.accept_pt_ls);
        ui_data_proj_save_bool(s, level + 1, "AcceptSZLS", ctx.m_control_data->basic.accept_sz_ls);
        ui_data_proj_save_bool(s, level + 1, "AcceptHits", ctx.m_control_data->basic.accept_hits);
        ui_data_proj_save_bool(s, level + 1, "AcceptClip", ctx.m_control_data->basic.accept_clip);
        ui_data_proj_save_bool(s, level + 1, "ParentClip", ctx.m_control_data->basic.parent_clip);
        ui_data_proj_save_bool(s, level + 1, "AcceptMove", ctx.m_control_data->basic.accept_move);
        /* 	RXmlHelper::ExportValue(node, RGUIKey::ShowAnimData,		mShowAnimData); */
        /* 	RXmlHelper::ExportValue(node, RGUIKey::HideAnimData,		mHideAnimData); */
        /* 	RXmlHelper::ExportValue(node, RGUIKey::DeadAnimData,		mDeadAnimData); */
        /* 	RXmlHelper::ExportValue(node, RGUIKey::DownAnimData,		mDownAnimData); */
        /* 	RXmlHelper::ExportValue(node, RGUIKey::RiseAnimData,		mRiseAnimData); */
        /* 	RXmlHelper::ExportValue(node, RGUIKey::UserAnimData,		mUserAnimData); */
        ui_data_proj_save_bool(s, level + 1, "FireAnimClip", ctx.m_control_data->basic.fire_anim_clip);
        ui_data_proj_save_str(s, level + 1, "ShowSFXFile", ui_data_control_msg(ctx.m_control, ctx.m_control_data->basic.show_sfx_file_id));
        ui_data_proj_save_str(s, level + 1, "HideSFXFile", ui_data_control_msg(ctx.m_control, ctx.m_control_data->basic.hide_sfx_file_id));
        ui_data_proj_save_str(s, level + 1, "DnSFXFile", ui_data_control_msg(ctx.m_control, ctx.m_control_data->basic.down_sfx_file_id));
        ui_data_proj_save_str(s, level + 1, "UpSFXFile", ui_data_control_msg(ctx.m_control, ctx.m_control_data->basic.rise_sfx_file_id));
        ui_data_proj_save_str(s, level + 1, "PushSFXFile", ui_data_control_msg(ctx.m_control, ctx.m_control_data->basic.push_sfx_file_id));
        ui_data_proj_save_bool(s, level + 1, "ForceClip", ctx.m_control_data->basic.force_clip);
	}

    if (ctx.m_control_data->type == ui_control_type_window) {
        ui_data_proj_save_bool(s, level + 1, "Modal", ctx.m_control_data->data.window.mudal);
        ui_data_proj_save_str(s, level + 1, "OkayCtrl", "");
        ui_data_proj_save_str(s, level + 1, "DenyCtrl", "");
        ui_data_proj_save_str(s, level + 1, "HideCtrl", "");
        ui_data_proj_save_str(s, level + 1, "HeadCtrl", "");
        ui_data_proj_save_str(s, level + 1, "TextCtrl", "");
        ui_data_proj_save_bool(s, level + 1, "StartVisible", ctx.m_control_data->data.window.start_show);
        ui_data_proj_save_str(s, level + 1, "ClassName", "UserWindow");
        ui_data_proj_save_bool(s, level + 1, "DialogTemplate", ctx.m_control_data->data.window.dialog_template);
    }

    if (rv) return rv;

    /*遍历子控件 */
    ui_data_control_childs(&control_it, control);
    child_control = ui_data_control_it_next(&control_it);
    if (child_control) {
        stream_putc_count(s, ' ', (level + 1) * 4);
        stream_printf(s, "<Children>\n");

        for(; child_control; child_control = ui_data_control_it_next(&control_it)) {
            if (ui_data_proj_save_control_i(s, child_control, level + 2, em) != 0) {
                rv = -1;
            }
        }

        stream_putc_count(s, ' ', (level + 1) * 4);
        stream_printf(s, "</Children>\n");
    }

    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "</%s>\n", tag_name);

    return rv;
}

static int ui_data_proj_save_layout_i(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    struct vfs_write_stream fs;
    ui_data_layout_t layout = ui_data_src_product(src);
    ui_data_control_t root_control;
    write_stream_t s = (write_stream_t)&fs;

    vfs_write_stream_init(&fs, fp);
    
    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

    root_control = ui_data_layout_root(layout);
    if (root_control == NULL) {
        CPE_ERROR(em, "ui_data_proj_save_layout: no root control!");
        return -1;
    }

    if (ui_data_proj_save_control_i(s, root_control, 0, em) != 0) {
        return -1;
    }

    return 0;
}

int ui_data_proj_save_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    if (ui_data_proj_save_gen_meta_file(root, src, em) != 0) return -1;    
    return ui_data_src_save_to_file(src, root, ui_data_proj_postfix(ui_data_src_type(src)), ui_data_proj_save_layout_i, ctx, em);
}

static const char * ui_data_proj_control_type_name(uint8_t control_type) {
    switch (control_type) {
    case ui_control_type_window:
        return "RGUIWindow";
    default:
        return ui_data_proj_control_tag_name(control_type);
    }
}
