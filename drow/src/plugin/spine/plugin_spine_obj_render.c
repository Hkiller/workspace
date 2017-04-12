#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "spine/Skeleton.h"
#include "spine/Attachment.h"
#include "spine/RegionAttachment.h"
#include "spine/MeshAttachment.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_cmd_triangles.h"
#include "render/runtime/ui_runtime_render_state.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_material.h"
#include "render/runtime/ui_runtime_render_material_utils.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin_spine_obj_i.h"

static const unsigned short quadTriangles[6] = {0, 1, 2, 2, 3, 0};
float g_plugin_spine_world_vertices[1000];

int plugin_spine_obj_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj, ui_runtime_render_t render, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t i_blend, ui_transform_t input_transform)
{
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
	spSkeleton* skeleton;
	const float* uvs = NULL;
	int verticesCount = 0;
	const unsigned short * index = NULL;
	int indexCount = 0;
	float r = 0, g = 0, b = 0, a = 0;
    int i;
	int slot_pos;
    ui_transform s;
    ui_vector_3 s_scale;

    if (obj->m_skeleton == NULL) return -1;
    if (obj->m_need_update) {
        plugin_spine_obj_do_update(ctx, render_obj, 0.0f);
    }
    
    if (input_transform) {
        s = *input_transform;
    }
    else {
        s = UI_TRANSFORM_IDENTITY;
    }

    if (cpe_float_cmp(s.m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0 || cpe_float_cmp(s.m_s.y, 0.0f, UI_FLOAT_PRECISION) == 0) return 0;

    s_scale = s.m_s;
    s_scale.y *= -1.0f;
    ui_transform_set_scale(&s, &s_scale);

    skeleton = obj->m_skeleton;
	assert(skeleton);

	for(slot_pos = 0; slot_pos < skeleton->slotsCount; ++slot_pos) {
		spSlot* slot = skeleton->drawOrder[slot_pos];
		ui_cache_res_t texture;

		if (slot->attachment == NULL) continue;
        
		texture = NULL;

		switch (slot->attachment->type) {
		case SP_ATTACHMENT_REGION: {
			spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
			spRegionAttachment_computeWorldVertices(attachment, slot->bone, g_plugin_spine_world_vertices);
			texture = ((spAtlasRegion*)attachment->rendererObject)->page->rendererObject;
			uvs = attachment->uvs;
			verticesCount = 4;
			index = quadTriangles;
			indexCount = 6;
			r = attachment->r;
			g = attachment->g;
			b = attachment->b;
			a = attachment->a;

			break;
        }
		case SP_ATTACHMENT_MESH: {
			spMeshAttachment* attachment = (spMeshAttachment*)slot->attachment;
			spMeshAttachment_computeWorldVertices(attachment, slot, g_plugin_spine_world_vertices);
			spMeshAttachment_updateUVs(attachment);
			texture = ((spAtlasRegion*)attachment->rendererObject)->page->rendererObject;
			uvs = attachment->uvs;
			verticesCount = attachment->super.worldVerticesLength >> 1;
			index = attachment->triangles;
			indexCount = attachment->trianglesCount;
			r = attachment->r;
			g = attachment->g;
			b = attachment->b;
			a = attachment->a;
			break;
        }
        default:
            break;
		}

		if (texture) {
            ui_runtime_render_material_t material;
            ui_runtime_vertex_v3f_t2f_c4ub_t vetex_buf;
            uint32_t color_v;
            ui_color mix_color;
            uint8_t premultiplied_alpha = 1;
            struct ui_runtime_render_blend blend;
            ui_runtime_render_cmd_t cmd;
            ui_runtime_render_program_buildin_t program = ui_runtime_render_program_buildin_multiply;

            mix_color.a = skeleton->a * slot->a * a;
            mix_color.r = skeleton->r * slot->r * r * mix_color.a;
            mix_color.g = skeleton->g * slot->g * g * mix_color.a;
            mix_color.b = skeleton->b * slot->b * b * mix_color.a;
            
            switch(slot->data->blendMode) {
            case SP_BLEND_MODE_ADDITIVE:
                blend.m_src_factor = ui_runtime_render_one;
                blend.m_dst_factor = ui_runtime_render_one;
                break;
            case SP_BLEND_MODE_MULTIPLY:
                blend.m_src_factor = ui_runtime_render_dst_color;
                blend.m_dst_factor = ui_runtime_render_one_minus_src_alpha;
                break;
            case SP_BLEND_MODE_SCREEN:
                blend.m_src_factor = ui_runtime_render_one;
                blend.m_dst_factor = ui_runtime_render_one_minus_src_color;
                break;
            case SP_BLEND_MODE_NORMAL:
            default:
                blend.m_src_factor = ui_runtime_render_one;
                blend.m_dst_factor = ui_runtime_render_one_minus_src_alpha;
                break;
            }

            material = ui_runtime_render_material_create_from_buildin_program(render, program);
            if (material == NULL) continue;

            if (ui_runtime_render_material_set_texture_dft(
                    material, texture,
                    ui_runtime_render_filter_linear,
                    ui_runtime_render_filter_linear,
                    ui_runtime_render_texture_clamp_to_edge,
                    ui_runtime_render_texture_clamp_to_edge,
                    0) != 0)
            {
                ui_runtime_render_material_free(material);
                continue;
            }
            
            /*填写顶点数据 */
            color_v = ui_color_make_abgr(&mix_color);

            vetex_buf = ui_runtime_render_alloc_buf(render, ui_runtime_render_buff_vertex_v3f_t2f_c4b, verticesCount);
            if (vetex_buf == NULL) {
                CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_render: alloc verticesCount buf in op fail, count=%d", verticesCount);
                ui_runtime_render_material_free(material);
                return -1;
            }
            
            for(i = 0; i < verticesCount; ++i) {
                ui_runtime_vertex_v3f_t2f_c4ub_t vertex = vetex_buf + i;
                ui_vector_2 p;
                p.x = g_plugin_spine_world_vertices[i * 2 + 0];
                p.y = g_plugin_spine_world_vertices[i * 2 + 1];

                ui_transform_inline_adj_vector_2(&s, &p);

                vertex->m_pos.x = p.x;
                vertex->m_pos.y = p.y;
                vertex->m_pos.z = 0.0f;
                vertex->m_uv.x = uvs[i * 2 + 0];
                vertex->m_uv.y = uvs[i * 2 + 1];
                vertex->m_c = color_v;
            }

            cmd = ui_runtime_render_cmd_triangles_create(
                    render,
                    0.0f,
                    material,
                    ui_runtime_render_buff_inline(vetex_buf, ui_runtime_render_buff_vertex_v3f_t2f_c4b, verticesCount),
                    ui_runtime_render_buff_inline(index, ui_runtime_render_buff_index_uint16, indexCount),
                    NULL, 0);
            if (cmd == NULL) {
                ui_runtime_render_material_free(material);
                return -1;
            }

            ui_runtime_render_state_set_blend(ui_runtime_render_cmd_render_state(cmd), &blend);
		}
	}

	if (plugin_spine_obj_debug_slots(obj)) {
        ui_matrix_4x4_t transform;
		int i, n;
		uint32_t debug_color = ui_color_make_abgr_from_value(1.0f, 1.0f, 1.0f, 0.5f);
		struct ui_runtime_vertex_v3f_t2f_c4ub points[4] = {
            { UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), debug_color},
            { UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), debug_color},
            { UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), debug_color},
            { UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), debug_color}
        };

        transform = ui_transform_calc_matrix_4x4(&s);
        
		for (i = 0, n = skeleton->slotsCount; i < n; i++) {
			spSlot* slot = skeleton->drawOrder[i];
			spRegionAttachment* attachment;

			if (!slot->attachment || slot->attachment->type != SP_ATTACHMENT_REGION) continue;

			attachment = (spRegionAttachment*)slot->attachment;
			spRegionAttachment_computeWorldVertices(attachment, slot->bone, g_plugin_spine_world_vertices);

			points[0].m_pos.x = + g_plugin_spine_world_vertices[SP_VERTEX_X1];
			points[0].m_pos.y = - g_plugin_spine_world_vertices[SP_VERTEX_Y1];
			points[1].m_pos.x = + g_plugin_spine_world_vertices[SP_VERTEX_X2];
			points[1].m_pos.y = - g_plugin_spine_world_vertices[SP_VERTEX_Y2];
			points[2].m_pos.x = + g_plugin_spine_world_vertices[SP_VERTEX_X3];
			points[2].m_pos.y = - g_plugin_spine_world_vertices[SP_VERTEX_Y3];
			points[3].m_pos.x = + g_plugin_spine_world_vertices[SP_VERTEX_X4];
			points[3].m_pos.y = - g_plugin_spine_world_vertices[SP_VERTEX_Y4];
            
            /* ui_runtime_render_add_ha_rect( */
            /*     render, transform, NULL, points, */
            /*     ui_runtime_render_program_type_vertex, GL_NEAREST, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */
		}
	}

	if (plugin_spine_obj_debug_bones(obj)) {
        ui_matrix_4x4_t transform;
        int i;
		uint32_t debug_color = ui_color_make_abgr_from_value(1.0f, 0.0f, 0.0f, 1.0f);
		struct ui_runtime_vertex_v3f_t2f_c4ub points[2] = {
            { UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), debug_color},
            { UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 0.0f), UI_VECTOR_2_INITLIZER(0.0f, 0.0f), debug_color}
        };

        transform = ui_transform_calc_matrix_4x4(&s);
        
		for (i = 0; i < skeleton->bonesCount; i++) {
			spBone *bone = skeleton->bones[i];
			points[0].m_pos.x = + bone->worldX;
			points[0].m_pos.y = - bone->worldY;
			points[1].m_pos.x = points[0].m_pos.x + bone->data->length * bone->x;
			points[1].m_pos.y = points[0].m_pos.y - bone->data->length * bone->y;

            /* ui_runtime_render_add_ha_line(render, transform, points); */
		}

		debug_color = ui_color_make_abgr_from_value(0.0f, 0.0f, 1.0f, 1.0f);
		for (i = 0; i < skeleton->bonesCount; i++) {
			spBone *bone = skeleton->bones[i];
			points[0].m_pos.x = + bone->worldX - 2;
			points[0].m_pos.y = - bone->worldY - 2;
            points[0].m_c = debug_color;
			points[1].m_pos.x = + bone->worldX + 2;
			points[1].m_pos.y = - bone->worldY + 2;
            points[1].m_c = debug_color;

            /* ui_runtime_render_add_ha_line(render, transform, points); */

			if (i == 0) debug_color = ui_color_make_abgr_from_value(0.0f, 1.0f, 0.0f, 1.0f);
		}
	}

    return 0;
}
