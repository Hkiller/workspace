#include <assert.h>
#include "spine/Skeleton.h"
#include "spine/RegionAttachment.h"
#include "spine/BoundingBoxAttachment.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_layout.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_page.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui/sprite_ui/ui_sprite_ui_module.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_ui_anim_bind_i.h"

ui_sprite_spine_ui_anim_bind_t
ui_sprite_spine_ui_anim_bind_create(plugin_ui_env_t env) {
    plugin_ui_animation_t animation;

    animation = plugin_ui_animation_create_by_type_name(env, UI_SPRITE_SPINE_UI_ANIM_BIND_NAME);
    if (animation == NULL) return NULL;
    
    return plugin_ui_animation_data(animation);
}

int ui_sprite_spine_ui_anim_bind_set_prefix(ui_sprite_spine_ui_anim_bind_t anim_bind, const char * prefix) {
    if (anim_bind->m_prefix) {
        mem_free(anim_bind->m_module->m_alloc, anim_bind->m_prefix);
    }

    if (prefix) {
        anim_bind->m_prefix = cpe_str_mem_dup_trim(anim_bind->m_module->m_alloc, prefix);
        if (anim_bind->m_prefix == NULL) return -1;
    }
    else {
        anim_bind->m_prefix = NULL;
    }

    return 0;
}

int ui_sprite_spine_ui_anim_bind_add_root(ui_sprite_spine_ui_anim_bind_t anim_bind, const char * root_path) {
    ui_sprite_spine_ui_module_t module = anim_bind->m_module;
    ui_sprite_spine_ui_anim_bind_root_t root;
    size_t root_len = strlen(root_path) + 1;

    root = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_spine_ui_anim_bind_root) + root_len);
    if (root == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind_add_root: alloc fail!");
        return -1;
    }

    memcpy((void*)(root + 1), root_path, root_len);
    root->m_bind = anim_bind;
    root->m_root = (void*)(root + 1);
    root->m_aspect = NULL;

    TAILQ_INSERT_TAIL(&anim_bind->m_roots, root, m_next);

    return 0;
}

int ui_sprite_spine_ui_anim_bind_init(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_bind_t anim_bind = plugin_ui_animation_data(animation);

    anim_bind->m_module = module;
    
    anim_bind->m_aspect = plugin_ui_aspect_create(plugin_ui_animation_env(animation), NULL);
    if (anim_bind->m_aspect == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind_init: create asspect fail!");
        return -1;
    }

    anim_bind->m_debug = 0;

    TAILQ_INIT(&anim_bind->m_roots);
    
    return 0;
}

void ui_sprite_spine_ui_anim_bind_free(plugin_ui_animation_t animation, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_bind_t anim_bind = plugin_ui_animation_data(animation);

    assert(anim_bind->m_aspect);
    plugin_ui_aspect_free(anim_bind->m_aspect);
    anim_bind->m_aspect = NULL;

    assert(anim_bind->m_prefix);
    free(anim_bind->m_prefix);
    anim_bind->m_prefix = NULL;

    while(!TAILQ_EMPTY(&anim_bind->m_roots)) {
        ui_sprite_spine_ui_anim_bind_root_t root = TAILQ_FIRST(&anim_bind->m_roots);
        TAILQ_REMOVE(&anim_bind->m_roots, root, m_next);

        if (root->m_aspect) {
            plugin_ui_aspect_free(root->m_aspect);
            root->m_aspect = NULL;
        }
        
        mem_free(module->m_alloc, root);
    }
}

int ui_sprite_spine_ui_anim_bind_add_frame(ui_sprite_spine_ui_anim_bind_t anim_bind, plugin_ui_control_frame_t frame) {
    plugin_ui_animation_t animation = plugin_ui_animation_from_data(anim_bind);
        
    if (plugin_ui_aspect_control_frame_add(anim_bind->m_aspect, frame, 0) != 0) {
        CPE_ERROR(anim_bind->m_module->m_em, "ui_sprite_spine_ui_anim_bind_add_frame: add to aspect fail!");
        return -1;
    }

    if (plugin_ui_animation_control_create(animation, plugin_ui_control_frame_control(frame), 1) == NULL) {
        plugin_ui_aspect_control_frame_remove(anim_bind->m_aspect, frame);
        CPE_ERROR(anim_bind->m_module->m_em, "ui_sprite_spine_ui_anim_bind_add_frame: add control to animation fail!");
        return -1;
    }

    return 0;
}

static int ui_sprite_spine_ui_anim_calc_target_rect_area(
    ui_sprite_spine_ui_module_t module, ui_rect_t target_rect, ui_runtime_render_obj_t obj, spSlot * slot)
{
    spBoundingBoxAttachment * binding_box = (spBoundingBoxAttachment *)slot->attachment;
    static float s_vertices[40];
    ui_transform_t base_transform;

    base_transform = ui_runtime_render_obj_transform(obj);
    if (base_transform == NULL) return -1;

    spBoundingBoxAttachment_computeWorldVertices(binding_box, slot, s_vertices);
    
    if (binding_box->super.verticesCount < 2) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_bind_controls_binding_update: spBoundingBoxAttachment count < 2!");
        return -1;
    }

    target_rect->lt.x =  s_vertices[SP_VERTEX_X1];
    target_rect->lt.y =  - s_vertices[SP_VERTEX_Y1];
    target_rect->rb.x =  s_vertices[SP_VERTEX_X3];
    target_rect->rb.y =  - s_vertices[SP_VERTEX_Y3];
    if (target_rect->lt.x > target_rect->rb.x) {
        float t = target_rect->lt.x;
        target_rect->lt.x = target_rect->rb.x;
        target_rect->rb.x = t;
    }

    if (target_rect->lt.y > target_rect->rb.y) {
        float t = target_rect->lt.y;
        target_rect->lt.y = target_rect->rb.y;
        target_rect->rb.y = t;
    }
        
    ui_transform_inline_adj_vector_2(base_transform, &target_rect->lt);
    ui_transform_inline_adj_vector_2(base_transform, &target_rect->rb);

    return 0;
}

static int ui_sprite_spine_ui_anim_calc_target_rect_area_scale(
    ui_sprite_spine_ui_module_t module, ui_rect_t target_rect, plugin_ui_control_t control, ui_runtime_render_obj_t obj, spSlot * slot)
{
    ui_transform bone_transform;
    ui_rect area_rect;
    ui_vector_2 area_center;
    ui_data_control_t control_src;
    UI_CONTROL const * control_data;
    ui_vector_2 control_half_sz;
    ui_vector_2_t screen_adj = plugin_ui_env_screen_adj(plugin_ui_control_env(control));

    if (plugin_spine_bone_calc_transform(slot->bone, &bone_transform) != 0) return -1;
    
    control_src = plugin_ui_control_data_src(control);
    if (control_src == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_bind_controls_binding_update: control %s no src!",
            plugin_ui_control_path_dump(gd_app_tmp_buffer(module->m_app), control));
        return -1;
    }
    control_data =  ui_data_control_data(control_src);

    if (ui_sprite_spine_ui_anim_calc_target_rect_area(module, &area_rect, obj, slot) != 0) return -1;

    area_center.x = (area_rect.lt.x + area_rect.rb.x) / 2.0f;
    area_center.y = (area_rect.lt.y + area_rect.rb.y) / 2.0f;

    control_half_sz.x = control_data->basic.editor_sz.value[0] * bone_transform.m_s.x * 0.5 * screen_adj->x;
    control_half_sz.y = control_data->basic.editor_sz.value[1] * bone_transform.m_s.y * 0.5 * screen_adj->y;

    /* printf("xxxxxx: editor_sz=(%f,%f), scale=(%f,%f), screen_adj=(%f,%f), result=(%f,%f)\n", */
    /*        control_data->basic.editor_sz.value[0], control_data->basic.editor_sz.value[1], */
    /*        bone_transform.m_s.x, bone_transform.m_s.y, screen_adj->x, screen_adj->y, */
    /*        control_half_sz.x, control_half_sz.y); */
    
    target_rect->lt.x =  area_center.x - control_half_sz.x;
    target_rect->lt.y =  area_center.y - control_half_sz.y;
    target_rect->rb.x =  area_center.x + control_half_sz.x;
    target_rect->rb.y =  area_center.y + control_half_sz.y;

    return 0;
}

static int ui_sprite_spine_ui_anim_calc_target_rect_bone_scale(
    ui_sprite_spine_ui_module_t module, ui_rect_t target_rect, plugin_ui_control_t control, ui_runtime_render_obj_t obj, spSlot * slot)
{
    ui_data_control_t control_src;
    ui_transform_t base_transform;
    ui_transform bone_transform;
    UI_CONTROL const * control_data;
    ui_vector_2 bone_pos;
    ui_vector_2 control_half_sz;
    ui_vector_2_t screen_adj = plugin_ui_env_screen_adj(plugin_ui_control_env(control));

    if (plugin_spine_bone_calc_transform(slot->bone, &bone_transform) != 0) return -1;
    
    base_transform = ui_runtime_render_obj_transform(obj);
    if (base_transform == NULL) return -1;

    control_src = plugin_ui_control_src(control);
    if (control_src == NULL) {
        control_src = plugin_ui_control_template(control);
    }
    if (control_src == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_bind_controls_binding_update: control %s no src!",
            plugin_ui_control_path_dump(gd_app_tmp_buffer(module->m_app), control));
        return -1;
    }
    control_data =  ui_data_control_data(control_src);
    
    ui_transform_adj_by_parent(&bone_transform, base_transform);
    ui_transform_get_pos_2(&bone_transform, &bone_pos);

    control_half_sz.x = control_data->basic.editor_sz.value[0] * bone_transform.m_s.x * 0.5 * screen_adj->x;
    control_half_sz.y = control_data->basic.editor_sz.value[1] * bone_transform.m_s.y * 0.5 * screen_adj->y;
    
    target_rect->lt.x =  bone_pos.x - control_half_sz.x;
    target_rect->lt.y =  bone_pos.y - control_half_sz.y;
    target_rect->rb.x =  bone_pos.x + control_half_sz.x;
    target_rect->rb.y =  bone_pos.y + control_half_sz.y;

    return 0;
}

static void ui_sprite_spine_ui_anim_update_control_to_target(
    ui_sprite_spine_ui_module_t module, ui_rect_t target_rect, plugin_ui_control_t control, spSlot * slot)
{
    /* CPE_ERROR( */
    /*     module->m_em, "ui_sprite_spine_ui_bind_controls_binding_update: control %s.%s: rect=(%f,%f)-(%f,%f)", */
    /*     plugin_ui_page_name(plugin_ui_control_page(control)), */
    /*     plugin_ui_control_name(control), */
    /*     target_rect->lt.x, target_rect->lt.y, target_rect->rb.x, target_rect->rb.y); */

    /* printf("\n\n\nbegin update pace on screen\n"); */
    if (plugin_ui_control_place_on_screen(control, &target_rect->lt, target_rect) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_bind_controls_binding_update: spBoundingBoxAttachment count < 2!");
        return;
    }

    plugin_ui_control_set_alpha(control, slot->a);

    if (plugin_ui_control_need_update_cache(control)) {
        /* printf( */
        /*     "xxxx: %s.%s: update cache, size=%f,%f\n", */
        /*     plugin_ui_page_name(plugin_ui_control_page(control)), */
        /*     plugin_ui_control_name(control), */
        /*     target_rect->rb.x - target_rect->lt.x, target_rect->rb.y - target_rect->lt.y); */
        plugin_ui_control_update_cache(control, 0);
    }
    else {
        /* printf( */
        /*     "xxxx: %s.%s: skip update\n", */
        /*     plugin_ui_page_name(plugin_ui_control_page(control)), */
        /*     plugin_ui_control_name(control)); */
    }
        
    //printf("end update pace on screen\n\n\n\n");
}

static int
ui_sprite_spine_ui_anim_bind_do_bind(
    ui_sprite_spine_ui_module_t module, plugin_ui_animation_t animation, ui_sprite_spine_ui_anim_bind_t anim_bind, 
    plugin_ui_env_t env, plugin_ui_control_t anim_control,
    plugin_spine_obj_t spine_obj, const char * prefix, ui_sprite_spine_ui_anim_bind_root_t root)
{
    struct spSkeleton* skeleton;
    struct plugin_spine_attachment_it attachment_it;
    plugin_spine_data_attachment_t attachment;
    uint16_t prefix_len;
    plugin_ui_control_t root_control;
    
    assert(root->m_root);

    if (root->m_root[0] == '^') {
        root_control = plugin_ui_control_find_child_by_path(anim_control, root->m_root);
        if (root_control == NULL) return -1;
    }
    else {
        root_control = plugin_ui_control_find_by_path(env, root->m_root);
        if (root_control == NULL) return -1;
    }

	skeleton = plugin_spine_obj_skeleton(spine_obj);

    prefix_len = strlen(prefix);

    CPE_INFO(
        module->m_em, "spine_bin_controls: anim %s ==> page %s: prefix=%s",
        plugin_ui_animation_name(animation),
        plugin_ui_page_name(plugin_ui_control_page(root_control)),
        anim_bind->m_prefix);

    plugin_spine_skin_attachments(&attachment_it, skeleton->skin ? skeleton->skin : skeleton->data->defaultSkin);
    
    while((attachment = plugin_spine_attachment_it_next(&attachment_it))) {
        struct spAttachment * sp_attachment = plugin_spine_data_attachment_attachment(attachment);
        spBoundingBoxAttachment * binding_box = NULL;
        spSlot* slot;
        ui_sprite_spine_ui_anim_bind_way_t way;
        plugin_ui_control_t control;
        plugin_ui_animation_control_t anim_control;
        ui_sprite_spine_ui_anim_bind_control_t binding;
        const char * slot_name;
        
        if (sp_attachment->type != SP_ATTACHMENT_BOUNDING_BOX) continue;

        binding_box = (spBoundingBoxAttachment *)sp_attachment;

        slot_name = binding_box->super.super.name;
        way = ui_sprite_spine_ui_anim_bind_area_scale;
        if (slot_name[0] == '+') {
            slot_name++;
            way = ui_sprite_spine_ui_anim_bind_area;
        }
            
        if (!cpe_str_start_with(slot_name, prefix)) continue;

        slot = skeleton->slots[plugin_spine_data_attachment_slot_index(attachment)];
        
        control = plugin_ui_control_find_child_by_path(root_control, slot_name + prefix_len);
        if (control == NULL) {
            if (anim_bind->m_debug) {
                CPE_INFO(module->m_em, "    %s ignore", slot_name + prefix_len);
            }
            continue;
        }

        anim_control = plugin_ui_animation_control_create(animation, control, 0);
        if (anim_control == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind: create binding fail!");
            return -1;
        }

        binding = plugin_ui_animation_control_data(anim_control);
        binding->m_slot = slot;
        binding->m_way = way;

        if (plugin_ui_aspect_control_add(root->m_aspect, control, 0) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind: add bind control to aspect fail!");
            return -1;
        }
        
        if (anim_bind->m_debug) {
            CPE_INFO(module->m_em, "    %s ok", binding_box->super.super.name + prefix_len);
        }
    }

    return 0;
}

uint8_t ui_sprite_spine_ui_anim_bind_update(plugin_ui_animation_t animation, void * ctx, float delta_s) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_bind_t anim_bind = plugin_ui_animation_data(animation);
    struct plugin_ui_control_frame_it frame_it;
    plugin_ui_control_frame_t frame;
    plugin_ui_control_t anim_control;
    plugin_ui_env_t env;
    ui_sprite_spine_ui_anim_bind_root_t root;
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;
    plugin_spine_obj_t spine_obj;

    /*准备环境数据 */
    if (anim_bind->m_prefix == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind: no prefix!");
        return 0;
    }
    
    plugin_ui_aspect_control_frames(&frame_it, anim_bind->m_aspect);
    frame = plugin_ui_control_frame_it_next(&frame_it);
    if (frame == NULL) {
        if (anim_bind->m_debug) {
            CPE_INFO(module->m_em, "ui_sprite_spine_ui_anim_bind: no frame, auto stop!");
        }
        return 0;
    }

    anim_control = plugin_ui_control_frame_control(frame);
    env = plugin_ui_control_env(anim_control);

    render_obj_ref = plugin_ui_control_frame_render_obj_ref(frame);
    render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);

    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_ui_anim_bind: %s is not spine obj",
            plugin_ui_control_frame_dump(&module->m_dump_buffer, frame));
        return 0;
    }

    spine_obj = ui_runtime_render_obj_data(render_obj);

    /*检查控件绑定 */
    TAILQ_FOREACH(root, &anim_bind->m_roots, m_next) {
        struct plugin_ui_control_it control_it;
        plugin_ui_control_t control;

        if (root->m_aspect == NULL) {
            root->m_aspect = plugin_ui_aspect_create(env, NULL);
            if (root->m_aspect == NULL) continue;
        }

        plugin_ui_aspect_controls(&control_it, root->m_aspect);
        control = plugin_ui_control_it_next(&control_it);
        if (control == NULL) {
            ui_sprite_spine_ui_anim_bind_do_bind(module, animation, anim_bind, env, anim_control, spine_obj, anim_bind->m_prefix, root);
        }

        plugin_ui_aspect_controls(&control_it, root->m_aspect);

        /*更新绑定的控件 */
        plugin_ui_aspect_controls(&control_it, root->m_aspect);
        while((control = plugin_ui_control_it_next(&control_it))) {
            plugin_ui_animation_control_t a_control;
            ui_sprite_spine_ui_anim_bind_control_t binding;

            if (control == anim_control) continue;
        
            a_control = plugin_ui_animation_control_create(animation, control, 0);
            assert(a_control);

            binding = plugin_ui_animation_control_data(a_control);
            assert(binding->m_slot);

            if(binding->m_slot->attachment == NULL || binding->m_slot->attachment->type != SP_ATTACHMENT_BOUNDING_BOX) {
                plugin_ui_control_set_visible(control, 0);
            }
            else {
                ui_rect target_rect;

                plugin_ui_control_set_visible(control, 1);

                switch(binding->m_way) {
                case ui_sprite_spine_ui_anim_bind_area:
                    if (ui_sprite_spine_ui_anim_calc_target_rect_area(module, &target_rect, render_obj, binding->m_slot) == 0) {
                        ui_sprite_spine_ui_anim_update_control_to_target(module, &target_rect, control, binding->m_slot);
                    }
                    break;
                case ui_sprite_spine_ui_anim_bind_area_scale:
                    if (ui_sprite_spine_ui_anim_calc_target_rect_area_scale(module, &target_rect, control, render_obj, binding->m_slot) == 0) {
                        ui_sprite_spine_ui_anim_update_control_to_target(module, &target_rect, control, binding->m_slot);
                    }
                    break;
                case ui_sprite_spine_ui_anim_bind_bone_scale:
                    if (ui_sprite_spine_ui_anim_calc_target_rect_bone_scale(module, &target_rect, control, render_obj, binding->m_slot) == 0) {
                        ui_sprite_spine_ui_anim_update_control_to_target(module, &target_rect, control, binding->m_slot);
                    }
                    break;
                default:
                    assert(0);
                    break;
                }
            }
        }
    }
    
    return 1;
}

int ui_sprite_spine_ui_anim_bind_enter(plugin_ui_animation_t animation, void * ctx) {
    return ui_sprite_spine_ui_anim_bind_update(animation, ctx, 0.0f) ? 0 : -1;
}

void ui_sprite_spine_ui_anim_bind_exit(plugin_ui_animation_t animation, void * ctx) {
}

int ui_sprite_spine_ui_anim_bind_setup(
    plugin_ui_animation_t animation, void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_anim_bind_t anim_bind = plugin_ui_animation_data(animation);
    const char * str_value;
    uint8_t have_root = 0;
    
    if (frame == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind_setup: spine-ui-bind only support work on frame!");
        return -1;
    }

    while((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-bind.root", ',', '='))) {
        if (ui_sprite_spine_ui_anim_bind_add_root(anim_bind, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind_setup: add root %s fail!", str_value);
            return -1;
        }

        have_root = 1;
    }

    if (!have_root) {
        str_value = plugin_ui_page_name(plugin_ui_control_page(control));
        if (ui_sprite_spine_ui_anim_bind_add_root(anim_bind, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind_setup: add root %s(default) fail!", str_value);
            return -1;
        }
    }

    if((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-bind.debug", ',', '='))) {
        anim_bind->m_debug = atoi(str_value);
    }

    str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "spine-ui-bind.prefix", ',', '=');
    if (str_value == NULL) str_value = "ctrl_";
    if (ui_sprite_spine_ui_anim_bind_set_prefix(anim_bind, str_value) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_ui_anim_bind_setup: set prefix %s fail!", str_value);
        return -1;
    }
    

    return ui_sprite_spine_ui_anim_bind_add_frame(anim_bind, frame);
}

int ui_sprite_spine_ui_anim_bind_control_attach(plugin_ui_animation_control_t animation_control, void * ctx) {
    ui_sprite_spine_ui_anim_bind_control_t binding;

    binding = plugin_ui_animation_control_data(animation_control);
    binding->m_slot = NULL;
    
    return 0;
}

void ui_sprite_spine_ui_anim_bind_control_detach(plugin_ui_animation_control_t animation_control, void * ctx) {
    ui_sprite_spine_ui_anim_bind_control_t binding;

    binding = plugin_ui_animation_control_data(animation_control);
    binding->m_slot = NULL;
}

int ui_sprite_spine_ui_anim_bind_regist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_create(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_BIND_NAME, module,
            /*animation*/
            sizeof(struct ui_sprite_spine_ui_anim_bind),
            ui_sprite_spine_ui_anim_bind_init,
            ui_sprite_spine_ui_anim_bind_free,
            ui_sprite_spine_ui_anim_bind_enter,
            ui_sprite_spine_ui_anim_bind_exit,
            ui_sprite_spine_ui_anim_bind_update,
            /*control*/
            sizeof(struct ui_sprite_spine_ui_anim_bind_control),
            ui_sprite_spine_ui_anim_bind_control_attach,
            ui_sprite_spine_ui_anim_bind_control_detach,
            /*setup*/
            ui_sprite_spine_ui_anim_bind_setup);
    
    return meta ? 0 : -1;
}

void ui_sprite_spine_ui_anim_bind_unregist(ui_sprite_spine_ui_module_t module) {
    plugin_ui_animation_meta_t meta =
        plugin_ui_animation_meta_find(
            ui_sprite_ui_module_ui_module(module->m_ui_module),
            UI_SPRITE_SPINE_UI_ANIM_BIND_NAME);
    if (meta) {
        plugin_ui_animation_meta_free(meta);
    }
}

const char * UI_SPRITE_SPINE_UI_ANIM_BIND_NAME = "spine-ui-bind";

