#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "ui_runtime_render_state_i.h"

ui_runtime_render_state_t
ui_runtime_render_state_create(ui_runtime_render_t render, ui_runtime_render_state_t parent) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_state_t render_state;

    render_state = TAILQ_FIRST(&module->m_free_states);
    if (render_state) {
        TAILQ_REMOVE(&module->m_free_states, render_state, m_next);
    }
    else {
        render_state = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_state));
        if (render_state == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_state_create: alloc fail!");
            return NULL; 
        }
    }

    render_state->m_render = render;
    render_state->m_parent = parent;
    bzero(&render_state->m_data, sizeof(render_state->m_data));
    
    return render_state;
}

ui_runtime_render_state_t
ui_runtime_render_state_clone(ui_runtime_render_state_t proto, ui_runtime_render_state_t parent) {
    ui_runtime_render_state_t render_state = ui_runtime_render_state_create(proto->m_render, parent);
    if (render_state == NULL) return NULL;

    render_state->m_data = proto->m_data;
    
    return render_state;
}

void ui_runtime_render_state_free(ui_runtime_render_state_t state) {
    ui_runtime_module_t module = state->m_render->m_module;

    state->m_render = (ui_runtime_render_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_states, state, m_next);
}

void ui_runtime_render_state_real_free(ui_runtime_render_state_t state) {
    ui_runtime_module_t module = (ui_runtime_module_t)state->m_render;
    TAILQ_REMOVE(&module->m_free_states, state, m_next);
    mem_free(module->m_alloc, state);
}

ui_runtime_render_state_t
ui_runtime_render_state_parent(ui_runtime_render_state_t state) {
    return state->m_parent;
}

ui_runtime_render_state_data_t
ui_runtime_render_state_data(ui_runtime_render_state_t state) {
    return &state->m_data;
}

void ui_runtime_render_state_clear_tag(ui_runtime_render_state_t state, ui_runtime_render_state_tag_t tag) {
    cpe_ba_set(&state->m_data.m_bits, tag, cpe_ba_false);
}

void ui_runtime_render_state_set_scissor(ui_runtime_render_state_t state, ui_rect_t scissor) {
    cpe_ba_set(&state->m_data.m_bits, ui_runtime_render_state_tag_scissor, cpe_ba_true);
    if (scissor) {
        state->m_data.m_scissor_on = 1;
        state->m_data.m_scissor = *scissor;
    }
    else {
        state->m_data.m_scissor_on = 0;
    }
}

void ui_runtime_render_state_set_blend(ui_runtime_render_state_t state, ui_runtime_render_blend_t blend) {
    cpe_ba_set(&state->m_data.m_bits, ui_runtime_render_state_tag_blend, cpe_ba_true);
    if (blend) {
        assert(blend->m_src_factor >= 0 && blend->m_src_factor <= ui_runtime_render_zero);
        assert(blend->m_dst_factor >= 0 && blend->m_dst_factor <= ui_runtime_render_zero);
        state->m_data.m_blend_on = 1;
        state->m_data.m_blend = *blend;
    }
    else {
        state->m_data.m_blend_on = 0;
    }
}

uint8_t ui_runtime_render_state_blend_compatible(ui_runtime_render_state_t state, ui_runtime_render_blend_t blend) {
    ui_runtime_render_state_data_t d = ui_runtime_render_state_data_find_by_tag(state, ui_runtime_render_state_tag_blend);

    if (d == NULL) return 0;
    
    if (blend == NULL) {
        return d->m_blend_on == 0 ? 1 : 0;
    }
    else {
        return (d->m_blend_on
                && d->m_blend.m_src_factor == blend->m_src_factor
                && d->m_blend.m_dst_factor == blend->m_dst_factor)
            ? 1
            : 0;
    }
}

ui_runtime_render_state_data_t
ui_runtime_render_state_data_find_by_tag(ui_runtime_render_state_t state, ui_runtime_render_state_tag_t tag) {
    while(state && !cpe_ba_get(&state->m_data.m_bits, tag)) {
        state = state->m_parent;
    }

    return state ? &state->m_data : NULL;
}

uint8_t ui_runtime_render_state_compatible(ui_runtime_render_state_t l, ui_runtime_render_state_t r, uint32_t flags) {
    ui_runtime_render_state_data_t l_e, r_e;

    /*blend*/
    l_e = ui_runtime_render_state_data_find_by_tag(l, ui_runtime_render_state_tag_blend);
    r_e = ui_runtime_render_state_data_find_by_tag(r, ui_runtime_render_state_tag_blend);
    if ((l_e ?  1 : 0) != (r_e ? 1 : 0)) return 0;
    if (l_e) {
        if (l_e->m_blend_on != r_e->m_blend_on) return 0;
        
        if (l_e->m_blend_on) {
            if (l_e->m_blend.m_src_factor != r_e->m_blend.m_src_factor
                || l_e->m_blend.m_dst_factor != r_e->m_blend.m_dst_factor) return 0;
        }
    }
    
    /*scissor*/
    l_e = ui_runtime_render_state_data_find_by_tag(l, ui_runtime_render_state_tag_scissor);
    r_e = ui_runtime_render_state_data_find_by_tag(r, ui_runtime_render_state_tag_scissor);
    if ((l_e ?  1 : 0) != (r_e ? 1 : 0)) return 0;
    if (l_e) {
        if (l_e->m_scissor_on != r_e->m_scissor_on) return 0;
        
        if (l_e->m_scissor_on) {
            if (ui_rect_cmp(&l_e->m_scissor, &r_e->m_scissor) != 0) return 0;
        }
    }

    return 1;
}
