#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/tailq_sort.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "ui_runtime_render_program_state_i.h"
#include "ui_runtime_render_program_i.h"
#include "ui_runtime_render_program_unif_i.h"
#include "ui_runtime_render_program_state_attr_i.h"
#include "ui_runtime_render_program_state_unif_i.h"

ui_runtime_render_program_state_t
ui_runtime_render_program_state_create(ui_runtime_render_t render, ui_runtime_render_program_t program) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_program_state_t program_state;

    program_state = TAILQ_FIRST(&module->m_free_program_states);
    if (program_state) {
        TAILQ_REMOVE(&module->m_free_program_states, program_state, m_next);
    }
    else {
        program_state = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_program_state));
        if (program_state == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_program_state_create: alloc fail");
            return NULL;
        }
    }

    program_state->m_render = render;
    program_state->m_program = program;
    program_state->m_is_sorted = 0;
    TAILQ_INIT(&program_state->m_attrs);
    TAILQ_INIT(&program_state->m_unifs);

    program->m_state_count++;
    
    return program_state;
}
    
ui_runtime_render_program_state_t
ui_runtime_render_program_state_create_by_buildin_program(ui_runtime_render_t render, ui_runtime_render_program_buildin_t buildin) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_program_state_t proto;
    
    proto = render->m_buildin_programs[buildin];
    if (proto == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_program_state_create: buildin %d not support", buildin);
        return NULL;
    }

    return ui_runtime_render_program_state_clone(proto);
}

ui_runtime_render_program_state_t
ui_runtime_render_program_state_clone(ui_runtime_render_program_state_t proto) {
    ui_runtime_render_t render = proto->m_render;
    ui_runtime_render_program_state_t program_state;
    ui_runtime_render_program_state_unif_t proto_unif;
    ui_runtime_render_program_state_attr_t proto_attr;

    program_state = ui_runtime_render_program_state_create(render, proto->m_program);
    if (program_state == NULL) return NULL;

    TAILQ_FOREACH(proto_unif, &proto->m_unifs, m_next) {
        if (ui_runtime_render_program_state_unif_create(program_state, proto_unif->m_unif, &proto_unif->m_data) == NULL) {
            ui_runtime_render_program_state_free(program_state);
            return NULL;
        }
    }

    TAILQ_FOREACH(proto_attr, &proto->m_attrs, m_next) {
        if (ui_runtime_render_program_state_attr_create(
                program_state, proto_attr->m_attr,
                proto_attr->m_data.m_stride,
                proto_attr->m_data.m_start_pos,
                proto_attr->m_data.m_element_count,
                proto_attr->m_data.m_element_type) == NULL)
        {
            ui_runtime_render_program_state_free(program_state);
            return NULL;
        }
    }

    program_state->m_is_sorted = proto->m_is_sorted;
    return program_state;
}

void ui_runtime_render_program_state_free(ui_runtime_render_program_state_t program_state) {
    ui_runtime_render_t render = program_state->m_render;
    ui_runtime_module_t module = render->m_module;

    while(!TAILQ_EMPTY(&program_state->m_attrs)) {
        ui_runtime_render_program_state_attr_free(TAILQ_FIRST(&program_state->m_attrs));
    }
    
    while(!TAILQ_EMPTY(&program_state->m_unifs)) {
        ui_runtime_render_program_state_unif_free(TAILQ_FIRST(&program_state->m_unifs));
    }

    assert(program_state->m_program->m_state_count > 0);
    program_state->m_program->m_state_count--;
    
    program_state->m_render = (ui_runtime_render_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_program_states, program_state, m_next);
}

void ui_runtime_render_program_state_real_free(ui_runtime_render_program_state_t program_state) {
    ui_runtime_module_t module = (ui_runtime_module_t)program_state->m_render;
    TAILQ_REMOVE(&module->m_free_program_states, program_state, m_next);
    mem_free(module->m_alloc, program_state);
}

ui_runtime_render_program_t
ui_runtime_render_program_state_program(ui_runtime_render_program_state_t program_state) {
    return program_state->m_program;
}

static ui_runtime_render_program_state_attr_t
ui_runtime_render_program_state_attr_next(struct ui_runtime_render_program_state_attr_it * it) {
    ui_runtime_render_program_state_attr_t * data = (ui_runtime_render_program_state_attr_t *)(it->m_data);
    ui_runtime_render_program_state_attr_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_runtime_render_program_state_attrs(ui_runtime_render_program_state_t program_state, ui_runtime_render_program_state_attr_it_t it) {
    *(ui_runtime_render_program_state_attr_t *)(it->m_data) = TAILQ_FIRST(&program_state->m_attrs);
    it->next = ui_runtime_render_program_state_attr_next;
}

static ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_next(struct ui_runtime_render_program_state_unif_it * it) {
    ui_runtime_render_program_state_unif_t * data = (ui_runtime_render_program_state_unif_t *)(it->m_data);
    ui_runtime_render_program_state_unif_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_runtime_render_program_state_unifs(ui_runtime_render_program_state_t program_state, ui_runtime_render_program_state_unif_it_t it) {
    *(ui_runtime_render_program_state_unif_t *)(it->m_data) = TAILQ_FIRST(&program_state->m_unifs);
    it->next = ui_runtime_render_program_state_unif_next;
}

static int ui_runtime_render_program_state_unif_cmp(
    ui_runtime_render_program_state_unif_t l, ui_runtime_render_program_state_unif_t r, void * ctx)
{
    return l->m_unif < r->m_unif
        ? -1
        : l->m_unif > r->m_unif
        ? 1
        : 0;
}

static int ui_runtime_render_program_state_attr_cmp(
    ui_runtime_render_program_state_attr_t l, ui_runtime_render_program_state_attr_t r, void * ctx)
{
    return l->m_attr < r->m_attr
        ? -1
        : l->m_attr > r->m_attr
        ? 1
        : 0;
}

uint8_t ui_runtime_render_program_state_compatible(
    ui_runtime_render_program_state_t l, ui_runtime_render_program_state_t r, uint32_t flags)
{
    ui_runtime_render_program_state_unif_t l_unif;
    ui_runtime_render_program_state_unif_t r_unif;
    ui_runtime_render_program_state_attr_t l_attr;
    ui_runtime_render_program_state_attr_t r_attr;
    
    if (l->m_program != r->m_program) return 0;

    if (!l->m_is_sorted) {
        l->m_is_sorted = 1;
        TAILQ_SORT(
            &l->m_unifs, ui_runtime_render_program_state_unif, ui_runtime_render_program_state_unif_list,
            m_next, ui_runtime_render_program_state_unif_cmp, NULL);
        TAILQ_SORT(
            &l->m_attrs, ui_runtime_render_program_state_attr, ui_runtime_render_program_state_attr_list,
            m_next, ui_runtime_render_program_state_attr_cmp, NULL);
    }

    if (!r->m_is_sorted) {
        r->m_is_sorted = 1;
        TAILQ_SORT(
            &r->m_unifs, ui_runtime_render_program_state_unif, ui_runtime_render_program_state_unif_list,
            m_next, ui_runtime_render_program_state_unif_cmp, NULL);
        TAILQ_SORT(
            &r->m_attrs, ui_runtime_render_program_state_attr, ui_runtime_render_program_state_attr_list,
            m_next, ui_runtime_render_program_state_attr_cmp, NULL);
    }

    for(l_unif = TAILQ_FIRST(&l->m_unifs), r_unif = TAILQ_FIRST(&r->m_unifs);
        l_unif;
        l_unif = TAILQ_NEXT(l_unif, m_next), r_unif = TAILQ_NEXT(r_unif, m_next))
    {
        /*忽略mvp */
        if ((flags & ui_runtime_render_render_env_compatible_ignore_mvp)
            && (l_unif->m_unif
                == ui_runtime_render_program_unif_buildin(
                    l->m_program, ui_runtime_render_program_unif_matrix_mvp))) continue;
            
        if (ui_runtime_render_program_state_unif_data_cmp(&l_unif->m_data, &r_unif->m_data) != 0) return 0;
    }
    if (r_unif != NULL) return 0;
    
    for(l_attr = TAILQ_FIRST(&l->m_attrs), r_attr = TAILQ_FIRST(&r->m_attrs);
        l_attr;
        l_attr = TAILQ_NEXT(l_attr, m_next), r_attr = TAILQ_NEXT(r_attr, m_next))
    {
        if (ui_runtime_render_program_state_attr_data_cmp(&l_attr->m_data, &r_attr->m_data) != 0) return 0;
    }
    if (r_attr != NULL) return 0;
    
    return 1;
}

int ui_runtime_render_program_state_set_attrs_by_buf_type(
    ui_runtime_render_program_state_t program_state, ui_runtime_render_buff_type_t buff_type)
{
    switch(buff_type) {
    case ui_runtime_render_buff_vertex_v3f_t2f_c4b:
        if (ui_runtime_render_program_state_attr_create_by_id_if_exist(
                program_state, ui_runtime_render_program_attr_position, sizeof(struct ui_runtime_vertex_v3f_t2f_c4ub),
                CPE_ENTRY_START(ui_runtime_vertex_v3f_t2f_c4ub, m_pos), 3, CPE_DR_TYPE_FLOAT) != 0
            || ui_runtime_render_program_state_attr_create_by_id_if_exist(
                program_state, ui_runtime_render_program_attr_texcoord, sizeof(struct ui_runtime_vertex_v3f_t2f_c4ub),
                CPE_ENTRY_START(ui_runtime_vertex_v3f_t2f_c4ub, m_uv), 2, CPE_DR_TYPE_FLOAT) != 0
            || ui_runtime_render_program_state_attr_create_by_id_if_exist(
                program_state, ui_runtime_render_program_attr_color, sizeof(struct ui_runtime_vertex_v3f_t2f_c4ub),
                CPE_ENTRY_START(ui_runtime_vertex_v3f_t2f_c4ub, m_c), 4, CPE_DR_TYPE_UINT8) != 0
            )
        {
            CPE_ERROR(
                program_state->m_render->m_module->m_em,
                "ui_runtime_render_program_state_set_attrs_by_buf_type: create attr fail");
            return -1;
        }
        break;
    case ui_runtime_render_buff_index_uint16:
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_attrs_by_buf_type: not support index_uint16");
        return -1;
    default:
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_attrs_by_buf_type: unkonwn type %d", buff_type);
        return -1;
    };
    
    return 0;
}

int ui_runtime_render_program_state_set_unifs_buildin(ui_runtime_render_program_state_t program_state, ui_transform_t mv) {
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    ui_transform_t m;
    
    if ((unif = ui_runtime_render_program_unif_buildin(program_state->m_program, ui_runtime_render_program_unif_matrix_p))) {
        data.m_type = ui_runtime_render_program_unif_m16;

        if ((m = ui_runtime_render_matrix(program_state->m_render, ui_runtime_render_matrix_stack_projection))) {
            data.m_data.m_m16 = *m;
        }
        else {
            data.m_data.m_m16 = UI_TRANSFORM_IDENTITY;
        }
        
        if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
            CPE_ERROR(
                program_state->m_render->m_module->m_em,
                "ui_runtime_render_program_state_set_unifs_buildin: create projection unif data fail");
            return -1;
        }
    }

    if ((unif = ui_runtime_render_program_unif_buildin(program_state->m_program, ui_runtime_render_program_unif_matrix_mv))) {
        data.m_type = ui_runtime_render_program_unif_m16;
        if (mv) {
            data.m_data.m_m16 = *mv;
        }
        else {
            data.m_data.m_m16 = UI_TRANSFORM_IDENTITY;
        }
        
        if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
            CPE_ERROR(
                program_state->m_render->m_module->m_em,
                "ui_runtime_render_program_state_set_unifs_buildin: create projection unif data fail");
            return -1;
        }
    }

    if ((unif = ui_runtime_render_program_unif_buildin(program_state->m_program, ui_runtime_render_program_unif_matrix_mvp))) {
        m = ui_runtime_render_matrix(program_state->m_render, ui_runtime_render_matrix_stack_projection);
        
        data.m_type = ui_runtime_render_program_unif_m16;

        if (mv && m) {
            data.m_data.m_m16 = *mv;
            ui_transform_adj_by_parent(&data.m_data.m_m16, m);
        }
        else if (mv) {
            data.m_data.m_m16 = *mv;
        }
        else if (m) {
            data.m_data.m_m16 = *m;
        }
        else {
            data.m_data.m_m16 = UI_TRANSFORM_IDENTITY;
        }
        
        if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
            CPE_ERROR(
                program_state->m_render->m_module->m_em,
                "ui_runtime_render_program_state_set_unifs_buildin: create mvp unif data fail");
            return -1;
        }
    }
    
    return 0;
}

int ui_runtime_render_program_state_set_unif_f(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, float f)
{
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    
    unif = ui_runtime_render_program_unif_find(program_state->m_program, unif_name);
    if (unif == NULL) {
        CPE_ERROR(program_state->m_render->m_module->m_em, "ui_runtime_render_program_state_set_unif: unif %s not exist", unif_name);
        return -1;
    }

    if (unif->m_type != ui_runtime_render_program_unif_f) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: unif %s type %s not float",
            unif_name, ui_runtime_render_program_unif_type_to_str(unif->m_type));
        return -1;
    }

    data.m_type = unif->m_type;
    data.m_data.m_f = f;

    if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: create unif data fail");
        return -1;
    }

    return 0;
}

int ui_runtime_render_program_state_set_unif_i(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, int32_t i)
{
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    
    unif = ui_runtime_render_program_unif_find(program_state->m_program, unif_name);
    if (unif == NULL) {
        CPE_ERROR(program_state->m_render->m_module->m_em, "ui_runtime_render_program_state_set_unif: unif %s not exist", unif_name);
        return -1;
    }

    if (unif->m_type != ui_runtime_render_program_unif_i) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: unif %s type %s not int",
            unif_name, ui_runtime_render_program_unif_type_to_str(unif->m_type));
        return -1;
    }

    data.m_type = unif->m_type;
    data.m_data.m_i = i;

    if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: create unif data fail");
        return -1;
    }

    return 0;
}    

int ui_runtime_render_program_state_set_unif_v2(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_vector_2_t v2)
{
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    
    unif = ui_runtime_render_program_unif_find(program_state->m_program, unif_name);
    if (unif == NULL) {
        CPE_ERROR(program_state->m_render->m_module->m_em, "ui_runtime_render_program_state_set_unif: unif %s not exist", unif_name);
        return -1;
    }

    if (unif->m_type != ui_runtime_render_program_unif_v2) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: unif %s type %s not vector2",
            unif_name, ui_runtime_render_program_unif_type_to_str(unif->m_type));
        return -1;
    }

    data.m_type = unif->m_type;
    data.m_data.m_v2 = *v2;

    if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: create unif data fail");
        return -1;
    }

    return 0;
}

int ui_runtime_render_program_state_set_unif_v3(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_vector_3_t v3)
{
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    
    unif = ui_runtime_render_program_unif_find(program_state->m_program, unif_name);
    if (unif == NULL) {
        CPE_ERROR(program_state->m_render->m_module->m_em, "ui_runtime_render_program_state_set_unif: unif %s not exist", unif_name);
        return -1;
    }

    if (unif->m_type != ui_runtime_render_program_unif_v3) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: unif %s type %s not vector3",
            unif_name, ui_runtime_render_program_unif_type_to_str(unif->m_type));
        return -1;
    }

    data.m_type = unif->m_type;
    data.m_data.m_v3 = *v3;

    if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: create unif data fail");
        return -1;
    }

    return 0;
}

int ui_runtime_render_program_state_set_unif_v4(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_vector_4_t v4)
{
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    
    unif = ui_runtime_render_program_unif_find(program_state->m_program, unif_name);
    if (unif == NULL) {
        CPE_ERROR(program_state->m_render->m_module->m_em, "ui_runtime_render_program_state_set_unif: unif %s not exist", unif_name);
        return -1;
    }

    if (unif->m_type != ui_runtime_render_program_unif_v4) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: unif %s type %s not vector4",
            unif_name, ui_runtime_render_program_unif_type_to_str(unif->m_type));
        return -1;
    }

    data.m_type = unif->m_type;
    data.m_data.m_v4 = *v4;

    if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: create unif data fail");
        return -1;
    }

    return 0;
}
    
int ui_runtime_render_program_state_set_unif_m16(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_transform_t m16)
{
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    
    unif = ui_runtime_render_program_unif_find(program_state->m_program, unif_name);
    if (unif == NULL) {
        CPE_ERROR(program_state->m_render->m_module->m_em, "ui_runtime_render_program_state_set_unif: unif %s not exist", unif_name);
        return -1;
    }

    if (unif->m_type != ui_runtime_render_program_unif_m16) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: unif %s type %s not matrix16",
            unif_name, ui_runtime_render_program_unif_type_to_str(unif->m_type));
        return -1;
    }

    data.m_type = unif->m_type;
    data.m_data.m_m16 = *m16;

    if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: create unif data fail");
        return -1;
    }

    return 0;
}
    
int ui_runtime_render_program_state_set_unif_texture(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t texture_idx)
{
    ui_runtime_render_program_unif_t unif;
    struct ui_runtime_render_program_state_unif_data data;
    
    unif = ui_runtime_render_program_unif_find(program_state->m_program, unif_name);
    if (unif == NULL) {
        CPE_ERROR(program_state->m_render->m_module->m_em, "ui_runtime_render_program_state_set_unif: unif %s not exist", unif_name);
        return -1;
    }

    if (unif->m_type != ui_runtime_render_program_unif_texture) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: unif %s type %s not texture",
            unif_name, ui_runtime_render_program_unif_type_to_str(unif->m_type));
        return -1;
    }

    data.m_type = unif->m_type;
    data.m_data.m_tex.m_res = res;
    data.m_data.m_tex.m_min_filter = min_filter;
    data.m_data.m_tex.m_mag_filter = mag_filter;
    data.m_data.m_tex.m_wrap_s = wrap_s;
    data.m_data.m_tex.m_wrap_t = wrap_t;
    data.m_data.m_tex.m_texture_idx = texture_idx;

    if (ui_runtime_render_program_state_unif_create(program_state, unif, &data) == NULL) {
        CPE_ERROR(
            program_state->m_render->m_module->m_em,
            "ui_runtime_render_program_state_set_unif: create unif data fail");
        return -1;
    }

    return 0;
}

int ui_runtime_render_program_state_set_unif_texture_dft(
    ui_runtime_render_program_state_t program_state,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t texture_idx)
{
    char unif_name[32];
    snprintf(unif_name, sizeof(unif_name), "texture%d", texture_idx);
    return ui_runtime_render_program_state_set_unif_texture(
        program_state, unif_name, res, min_filter, mag_filter, wrap_s, wrap_t, texture_idx);
}

ui_runtime_render_program_state_unif_data_t
ui_runtime_render_program_state_find_unif_texture_dft(
    ui_runtime_render_program_state_t program_state, uint8_t texture_idx)
{
    ui_runtime_render_program_state_unif_t unif;
    char unif_name[32];
    snprintf(unif_name, sizeof(unif_name), "texture%d", texture_idx);

    unif = ui_runtime_render_program_state_unif_find_by_name(program_state, unif_name);
    return unif ? &unif->m_data : NULL;
}
