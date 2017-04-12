#include <assert.h>
#include "chipmunk/chipmunk_private.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "plugin_chipmunk_render_i.h"
#include "plugin_chipmunk_env_i.h"
#include "plugin_chipmunk_module_i.h"

struct plugin_chipmunk_render_ctx {
    ui_runtime_render_cmd_t m_batch_cmd;
    ui_runtime_render_t m_render;
    ui_rect_t m_rect;
	ui_transform_t m_transform;
};

#define CHIPMUNK_DEBUG_DRAW_POINT_LINE_SCALE (1.0f)
#define CHIPMUNK_DEBUGD_RAW_OUTLINE_WIDTH (1.0f)

static cpSpaceDebugColor s_shapeOutlineColor = {200.0f/255.0f, 210.0f/255.0f, 230.0f/255.0f, 1.0f};
static cpSpaceDebugColor s_constraintColor = {0.0f, 0.75f, 0.0f, 1.0f};
static cpSpaceDebugColor s_collisionPointColor = {1.0f, 0.0f, 0.0f, 1.0f};

int plugin_chipmunk_render_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_chipmunk_render_t obj = (plugin_chipmunk_render_t)ui_runtime_render_obj_data(render_obj);
    obj->m_env = NULL;
    return 0;
}

static cpSpaceDebugColor plugin_chipmunk_render_clolr_from_rgba(float r, float g, float b, float a){
	cpSpaceDebugColor color = {r, g, b, a};
	return color;
}

static cpSpaceDebugColor plugin_chipmunk_render_clolr_from_la(float l, float a){
	cpSpaceDebugColor color = {l, l, l, a};
	return color;
}

static void plugin_chipmunk_render_draw_poly(
    struct plugin_chipmunk_render_ctx * ctx, cpVect * i_points, uint8_t i0, uint8_t i1, uint8_t i2, ui_color_t bcolor)
{
    uint32_t abgr = ui_color_make_abgr(bcolor);
    ui_runtime_vertex_v3f_t2f_c4ub vertexs[3] = {
        { UI_VECTOR_3_INITLIZER(i_points[i0].x, i_points[i0].y, 0.0f), UI_VECTOR_2_ZERO, abgr }
        , { UI_VECTOR_3_INITLIZER(i_points[i1].x, i_points[i1].y, 0.0f), UI_VECTOR_2_ZERO, abgr }
        , { UI_VECTOR_3_INITLIZER(i_points[i2].x, i_points[i2].y, 0.0f), UI_VECTOR_2_ZERO, abgr }
    };
    struct ui_runtime_render_blend blend = { ui_runtime_render_one_minus_src_alpha, ui_runtime_render_one_minus_src_alpha };
    
    ui_runtime_render_cmd_triangle_batch_append(
        &ctx->m_batch_cmd,
        ctx->m_render, 0,
        ctx->m_transform,
        vertexs,
        NULL, ui_runtime_render_filter_linear,
        ui_runtime_render_program_buildin_color, &blend);
}
    
static void plugin_chipmunk_render_draw_fat_segment(
    cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor i_outline, cpSpaceDebugColor i_fill, cpDataPointer data)
{
	struct plugin_chipmunk_render_ctx * ctx = (struct plugin_chipmunk_render_ctx *)data;
	cpVect n = cpvnormalize(cpvrperp(cpvsub(b, a)));
	cpVect t = cpvrperp(n);
	cpFloat half = 1.0f / CHIPMUNK_DEBUG_DRAW_POINT_LINE_SCALE;
	cpFloat r = radius + half;
    cpVect v[8];
	cpVect nw;
	cpVect tw;
    ui_color outline = { i_outline.r, i_outline.g, i_outline.b, i_outline.a };
    ui_color fill = { i_fill.r, i_fill.g, i_fill.b, i_fill.a };
    ui_vector_2 line_min = UI_VECTOR_2_INITLIZER(a.x, a.y);
    ui_vector_2 line_max = UI_VECTOR_2_INITLIZER(b.x, b.y);
    
	if(r <= half){
		r = half;
		fill = outline;
	}

    //TODO
    //ui_runtime_render_draw_basic_line_box(ctx->m_render, ctx->m_rect, ctx->m_transform, &line_min, &line_max, &outline);

	nw = (cpvmult(n, r));
	tw = (cpvmult(t, r));

	v[0] = cpvsub(b, cpvadd(nw, tw)); // { 1.0, -1.0}
	v[1] = cpvadd(b, cpvsub(nw, tw)); // { 1.0,  1.0}
	v[2] = cpvsub(b, nw); // { 0.0, -1.0}
	v[3] = cpvadd(b, nw); // { 0.0,  1.0}
	v[4] = cpvsub(a, nw); // { 0.0, -1.0}
	v[5] = cpvadd(a, nw); // { 0.0,  1.0}
	v[6] = cpvsub(a, cpvsub(nw, tw)); // {-1.0, -1.0}
	v[7] = cpvadd(a, cpvadd(nw, tw)); // {-1.0,  1.0}

    plugin_chipmunk_render_draw_poly(ctx, v, 0, 1, 2, &fill);
    plugin_chipmunk_render_draw_poly(ctx, v, 3, 1, 2, &fill);
    plugin_chipmunk_render_draw_poly(ctx, v, 3, 4, 2, &fill);
    plugin_chipmunk_render_draw_poly(ctx, v, 3, 4, 5, &fill);
    plugin_chipmunk_render_draw_poly(ctx, v, 6, 4, 5, &fill);
    plugin_chipmunk_render_draw_poly(ctx, v, 6, 7, 5, &fill);
}

static void plugin_chipmunk_render_draw_segment(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data) {
	plugin_chipmunk_render_draw_fat_segment(a, b, 0.0f, color, color, data);
}

static void plugin_chipmunk_render_draw_circle(
    cpVect p, cpFloat a, cpFloat radius, cpSpaceDebugColor i_outline, cpSpaceDebugColor i_fill, cpDataPointer data)
{
	struct plugin_chipmunk_render_ctx * ctx = (struct plugin_chipmunk_render_ctx *)data;
    ui_color outline = { i_outline.r, i_outline.g, i_outline.b, i_outline.a };
    ui_color fill = { i_fill.r, i_fill.g, i_fill.b, i_fill.a };
	int count = 30;
    ui_vector_2 line_min = UI_VECTOR_2_INITLIZER(p.x, p.y);
    ui_vector_2 line_max = UI_VECTOR_2_INITLIZER(
        p.x + (radius) * cpe_cos_radians(a),
        p.y + (radius) * cpe_sin_radians(a)
        );
    int i;
    
	for (i = 0; i < count; i++) {
		float x1 = p.x + radius * cos(i * 2 * M_PI / count);
		float y1 = p.y + radius * sin(i * 2 * M_PI / count);
		float x2 = p.x + radius * cos((i+1) * 2 * M_PI / count);
		float y2 = p.y + radius * sin((i+1) * 2 * M_PI / count);
		cpVect points[3] = {
            { p.x, p.y },
            { x1, y1 },
            { x2, y2 }, 
        };

        plugin_chipmunk_render_draw_poly(ctx, points, 0, 1, 2, &fill);

        /*TODO */
        /*ui_runtime_render_draw_basic_line(ctx->m_render, ctx->m_rect, ctx->m_transform, points + 1, points + 2, &outline);*/
	}

    //TODO:
    //ui_runtime_render_draw_basic_line(ctx->m_render, ctx->m_rect, ctx->m_transform, &line_min, &line_max, &outline);
}

static void plugin_chipmunk_render_draw_polygon(
    int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor i_outline, cpSpaceDebugColor i_fill, cpDataPointer data)
{
    struct plugin_chipmunk_render_ctx * ctx = (struct plugin_chipmunk_render_ctx *)data;
	struct ExtrudeVerts {cpVect offset, n;};
	size_t bytes = sizeof(struct ExtrudeVerts)*count;
	struct ExtrudeVerts *extrude = (struct ExtrudeVerts *)alloca(bytes);
    ui_color outline = { i_outline.r, i_outline.g, i_outline.b, i_outline.a };
    ui_color fill = { i_fill.r, i_fill.g, i_fill.b, i_fill.a };
    int i,j;
    cpFloat inset;
    cpFloat outset;
    cpVect points[3];
    
	memset(extrude, 0, bytes);
	for(i=0; i<count; i++){
		cpVect v0 = verts[(i-1+count)%count];
		cpVect v1 = verts[i];
		cpVect v2 = verts[(i+1)%count];

		cpVect n1 = cpvnormalize(cpvrperp(cpvsub(v1, v0)));
		cpVect n2 = cpvnormalize(cpvrperp(cpvsub(v2, v1)));

		cpVect offset = cpvmult(cpvadd(n1, n2), 1.0/(cpvdot(n1, n2) + 1.0f));
		struct ExtrudeVerts v = {offset, n2}; extrude[i] = v;
	}

	inset = -cpfmax(0.0f, 1.0f/CHIPMUNK_DEBUG_DRAW_POINT_LINE_SCALE - radius);
	for(i=0; i<count-2; i++){
		cpVect v[3];
        v[0] = cpvadd(verts[  0], cpvmult(extrude[  0].offset, inset));
		v[1] = cpvadd(verts[i+1], cpvmult(extrude[i+1].offset, inset));
		v[2] = cpvadd(verts[i+2], cpvmult(extrude[i+2].offset, inset));

        plugin_chipmunk_render_draw_poly(ctx, v, 0, 1, 2, &fill);
	}

	outset = 1.0f/CHIPMUNK_DEBUG_DRAW_POINT_LINE_SCALE + radius - inset;
	for(i=0, j=count-1; i<count; j=i, i++){
		cpVect vA = verts[i];
		cpVect vB = verts[j];

		cpVect nA = extrude[i].n;
		cpVect nB = extrude[j].n;

		cpVect offsetA = extrude[i].offset;
		cpVect offsetB = extrude[j].offset;

		cpVect innerA = cpvadd(vA, cpvmult(offsetA, inset));
		cpVect innerB = cpvadd(vB, cpvmult(offsetB, inset));

		cpVect inner0 = innerA;
		cpVect inner1 = innerB;
		cpVect outer0 = cpvadd(innerA, cpvmult(nB, outset));
		cpVect outer1 = cpvadd(innerB, cpvmult(nB, outset));
		cpVect outer2 = cpvadd(innerA, cpvmult(offsetA, outset));
		cpVect outer3 = cpvadd(innerA, cpvmult(nA, outset));

		/* cpVect n0 = nA; */
		/* cpVect n1 = nB; */
		/* cpVect offset0 = offsetA; */

		points[0] = cpv(inner0.x, inner0.y);
		points[1] = cpv(inner1.x, inner1.y);
		points[2] = cpv(outer1.x, outer1.y);
        plugin_chipmunk_render_draw_poly(ctx, points, 0, 0, 0, &outline);
        
		points[0] = cpv(inner0.x, inner0.y);
		points[1] = cpv(outer0.x, outer0.y);
		points[2] = cpv(outer1.x, outer1.y);
        plugin_chipmunk_render_draw_poly(ctx, points, 0, 0, 0, &outline);

		points[0] = cpv(inner0.x, inner0.y);
		points[1] = cpv(outer0.x, outer0.y);
		points[2] = cpv(outer2.x, outer2.y);
        plugin_chipmunk_render_draw_poly(ctx, points, 0, 0, 0, &outline);

		points[0] = cpv(inner0.x, inner0.y);
		points[1] = cpv(outer2.x, outer2.y);
		points[2] = cpv(outer3.x, outer3.y);
        plugin_chipmunk_render_draw_poly(ctx, points, 0, 0, 0, &outline);
	}
}

static void plugin_chipmunk_render_draw_dot(cpFloat size, cpVect pos, cpSpaceDebugColor i_color, cpDataPointer data) {
    struct plugin_chipmunk_render_ctx * ctx = (struct plugin_chipmunk_render_ctx *)data;
    ui_color color = { i_color.r, i_color.g, i_color.b, i_color.a };

	float r = size * 0.5f / CHIPMUNK_DEBUG_DRAW_POINT_LINE_SCALE;
    ui_vector_2 lt = UI_VECTOR_2_INITLIZER(pos.x - r, pos.y - r);
    ui_vector_2 rb = UI_VECTOR_2_INITLIZER(pos.x + r, pos.y + r);

    //TODO:
    //ui_runtime_render_draw_basic_line_box(ctx->m_render, ctx->m_rect, ctx->m_transform, &lt, &rb, &color);
}

static cpSpaceDebugColor plugin_chipmunk_render_color_for_shape(cpShape *shape, cpDataPointer data) {
	if(cpShapeGetSensor(shape)) {
		return plugin_chipmunk_render_clolr_from_la(1.0f, 0.1f);
	}
    else {
		cpBody *body = cpShapeGetBody(shape);

		if(body && cpBodyIsSleeping(body)) {
			return plugin_chipmunk_render_clolr_from_la(0.2f, 0.5f);
		}
        else if(body && shape->space && body->sleeping.idleTime > shape->space->sleepTimeThreshold) {
			return plugin_chipmunk_render_clolr_from_la(0.66f, 0.5f);
		}
        else {
			uint32_t val;
			float r, g, b;
			float max;
			float min;
			float intensity;

            val = (uint32_t)shape->hashid;
			val = (val+0x7ed55d16) + (val<<12);
			val = (val^0xc761c23c) ^ (val>>19);
			val = (val+0x165667b1) + (val<<5);
			val = (val+0xd3a2646c) ^ (val<<9);
			val = (val+0xfd7046c5) + (val<<3);
			val = (val^0xb55a4f09) ^ (val>>16);
			
			r = (float)((val>>0) & 0xFF);
			g = (float)((val>>8) & 0xFF);
			b = (float)((val>>16) & 0xFF);
			
			max = (float)cpfmax(cpfmax(r, g), b);
			min = (float)cpfmin(cpfmin(r, g), b);
			intensity = (body && cpBodyGetType(body) == CP_BODY_TYPE_STATIC ? 0.15f : 0.75f);
			
			// Saturate and scale the color
			if(min == max) {
				return plugin_chipmunk_render_clolr_from_rgba(intensity, 0.0f, 0.0f, 1.0f);
			}
            else {
				float coef = (float)intensity/(max - min);
				return plugin_chipmunk_render_clolr_from_rgba(
					(r - min)*coef,
					(g - min)*coef,
					(b - min)*coef,
					0.5f
				);
			}
		}
	}
}

int plugin_chipmunk_render_do_render(
    void * i_ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t render, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_chipmunk_render_t obj = (plugin_chipmunk_render_t)ui_runtime_render_obj_data(render_obj);
    struct plugin_chipmunk_render_ctx ctx;
    cpSpace * space;
    
	if (obj->m_env == NULL) return -1;

    ctx.m_batch_cmd = NULL;
    ctx.m_render = render;
    ctx.m_rect = clip_rect;
	ctx.m_transform = transform;

    space = (cpSpace*)plugin_chipmunk_env_space(obj->m_env);
    assert(space);

	cpSpaceDebugDrawOptions drawOptions = {
		plugin_chipmunk_render_draw_circle,
		plugin_chipmunk_render_draw_segment,
		plugin_chipmunk_render_draw_fat_segment,
		plugin_chipmunk_render_draw_polygon,
		plugin_chipmunk_render_draw_dot,
		(cpSpaceDebugDrawFlags)(
            CP_SPACE_DEBUG_DRAW_SHAPES
            | CP_SPACE_DEBUG_DRAW_CONSTRAINTS
            | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS
            ),
		s_shapeOutlineColor,
		plugin_chipmunk_render_color_for_shape,
		s_constraintColor,
		s_collisionPointColor,
		&ctx,
	};
	
    cpSpaceDebugDraw(space, &drawOptions);
    ui_runtime_render_cmd_triangle_batch_commit(&ctx.m_batch_cmd, render);

    return 0;
}

void plugin_chipmunk_render_set_env(plugin_chipmunk_render_t render, plugin_chipmunk_env_t env) {
    render->m_env = env;
}

int plugin_chipmunk_render_regist(plugin_chipmunk_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "chipmunk", 0, sizeof(struct plugin_chipmunk_render), module,
                plugin_chipmunk_render_do_init,
                NULL,
                NULL,
                NULL,
                NULL,
                plugin_chipmunk_render_do_render,
                NULL,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "plugin_chipmunk_render_regist: register render obj fail");
            return -1;
        }
    }

    return 0;
}

void plugin_chipmunk_render_unregist(plugin_chipmunk_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "chipmunk"))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}

void plugin_chipmunk_render_shape(ui_runtime_render_t render, ui_rect_t clip_rect, ui_transform_t transform, void * shape) {
    struct plugin_chipmunk_render_ctx ctx;
	cpBody *body = ((cpShape*)shape)->body;
	cpDataPointer data = &ctx;
	cpSpaceDebugColor fill_color = plugin_chipmunk_render_color_for_shape((cpShape*)shape, data);

    ctx.m_batch_cmd = NULL;
    ctx.m_render = render;
    ctx.m_rect = clip_rect;
	ctx.m_transform = transform;

	switch(((cpShape*)shape)->klass->type) {
    case CP_CIRCLE_SHAPE: {
        cpCircleShape *circle = (cpCircleShape *)shape;
        if (body) {
            plugin_chipmunk_render_draw_circle(circle->tc, body->a, circle->r, s_shapeOutlineColor, fill_color, data);
        }
        break;
    }
    case CP_SEGMENT_SHAPE: {
        cpSegmentShape *seg = (cpSegmentShape *)shape;
        plugin_chipmunk_render_draw_fat_segment(seg->ta, seg->tb, seg->r, s_shapeOutlineColor, fill_color, data);
        break;
    }
    case CP_POLY_SHAPE: {
        cpPolyShape *poly = (cpPolyShape *)shape;
			
        int count = poly->count;
        struct cpSplittingPlane *planes = poly->planes;
        cpVect *verts = (cpVect *)alloca(count*sizeof(cpVect));
        int i;
        
        for(i=0; i<count; i++) verts[i] = planes[i].v0;
        plugin_chipmunk_render_draw_polygon(count, verts, poly->r, s_shapeOutlineColor, fill_color, data);
        break;
    }
    default:
        break;
	}

    ui_runtime_render_cmd_triangle_batch_commit(&ctx.m_batch_cmd, render);
}

