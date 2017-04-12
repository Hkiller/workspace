#include <assert.h>
#include "ui_runtime_render_pass_i.h"
#include "ui_runtime_render_state_i.h"
#include "ui_runtime_render_program_state_i.h"

ui_runtime_render_pass_t
ui_runtime_render_pass_create(ui_runtime_render_technique_t technique) {
    ui_runtime_render_t render = technique->m_material->m_render;
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_pass_t pass;

    pass = TAILQ_FIRST(&module->m_free_passes);
    if (pass) {
        TAILQ_REMOVE(&module->m_free_passes, pass, m_next);
    }
    else {
        pass = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_pass));
        if (pass == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_pass: create: alloc fail!");
            return NULL;
        }
    }

    pass->m_technique = technique;
    pass->m_render_state = NULL;
    pass->m_program_state = NULL;

    technique->m_pass_count++;
    TAILQ_INSERT_TAIL(&technique->m_passes, pass, m_next);
    
    return pass;
}

ui_runtime_render_pass_t
ui_runtime_render_pass_clone(ui_runtime_render_technique_t technique, ui_runtime_render_pass_t proto) {
    ui_runtime_render_pass_t pass;

    pass = ui_runtime_render_pass_create(technique);
    if (pass == NULL) return NULL;

    if (proto->m_render_state) {
        assert(pass->m_render_state == NULL);
        pass->m_render_state = ui_runtime_render_state_clone(proto->m_render_state, NULL);
    }

    if (proto->m_program_state) {
        assert(pass->m_program_state == NULL);
        pass->m_program_state = ui_runtime_render_program_state_clone(proto->m_program_state);
    }
    
    return pass;
}

void ui_runtime_render_pass_free(ui_runtime_render_pass_t pass) {
    ui_runtime_render_t render = pass->m_technique->m_material->m_render;
    ui_runtime_module_t module = render->m_module;

    if (pass->m_render_state) ui_runtime_render_state_free(pass->m_render_state);
    if (pass->m_program_state) ui_runtime_render_program_state_free(pass->m_program_state);
    
    assert(pass->m_technique->m_pass_count > 0);
    pass->m_technique->m_pass_count--;
    TAILQ_REMOVE(&pass->m_technique->m_passes, pass, m_next);

    pass->m_technique = (ui_runtime_render_technique_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_passes, pass, m_next);
}

void ui_runtime_render_pass_real_free(ui_runtime_render_pass_t pass) {
    ui_runtime_module_t module = (ui_runtime_module_t)pass->m_technique;
    TAILQ_REMOVE(&module->m_free_passes, pass, m_next);
    mem_free(module->m_alloc, pass);
}

ui_runtime_render_technique_t ui_runtime_render_pass_technique(ui_runtime_render_pass_t pass) {
    return pass->m_technique;
}

ui_runtime_render_t ui_runtime_render_pass_render(ui_runtime_render_pass_t pass) {
    return pass->m_technique->m_material->m_render;
}

ui_runtime_render_state_t
ui_runtime_render_pass_render_state_check_create(ui_runtime_render_pass_t pass) {
    if (pass->m_render_state == NULL) {
        ui_runtime_render_t render = pass->m_technique->m_material->m_render;
        ui_runtime_render_state_t parent = ui_runtime_render_technique_render_state(pass->m_technique);

        pass->m_render_state = ui_runtime_render_state_create(render, parent);
        if (pass->m_render_state == NULL) {
            CPE_ERROR(render->m_module->m_em, "ui_runtime_render_pass_set_render_state: alloc fail");
            return NULL;
        }
    }

    return pass->m_render_state;
}

int ui_runtime_render_pass_set_render_state(ui_runtime_render_pass_t pass, ui_runtime_render_state_data_t render_state_data) {
    ui_runtime_render_state_t state = ui_runtime_render_pass_render_state_check_create(pass);
    if (state == NULL) return -1;

    pass->m_render_state->m_data = *render_state_data;
    return 0;
}

ui_runtime_render_state_t
ui_runtime_render_pass_render_state(ui_runtime_render_pass_t pass) {
    if (pass->m_render_state) return pass->m_render_state;
    if (pass->m_technique->m_render_state) return pass->m_technique->m_render_state;
    if (pass->m_technique->m_material->m_render_state) return pass->m_technique->m_material->m_render_state;
    return pass->m_technique->m_material->m_render_state_parent;
}

void ui_runtime_render_pass_set_program_state(ui_runtime_render_pass_t pass, ui_runtime_render_program_state_t program_state) {
    pass->m_program_state = program_state;
}

ui_runtime_render_program_state_t
ui_runtime_render_pass_program_state(ui_runtime_render_pass_t pass) {
    return pass->m_program_state;
}

uint8_t ui_runtime_render_pass_compatible(ui_runtime_render_pass_t l, ui_runtime_render_pass_t r, uint32_t flags) {
    return (ui_runtime_render_state_compatible(
                ui_runtime_render_pass_render_state(l),
                ui_runtime_render_pass_render_state(r),
                flags)
            && ui_runtime_render_program_state_compatible(
                l->m_program_state,
                r->m_program_state,
                flags)
        ) ? 1 : 0;
}
