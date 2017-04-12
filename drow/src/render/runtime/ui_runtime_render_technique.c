#include <assert.h>
#include "ui_runtime_render_technique_i.h"
#include "ui_runtime_render_pass_i.h"
#include "ui_runtime_render_state_i.h"

ui_runtime_render_technique_t
ui_runtime_render_technique_create(ui_runtime_render_material_t material) {
    ui_runtime_module_t module = material->m_render->m_module;
    ui_runtime_render_technique_t technique;

    technique = TAILQ_FIRST(&module->m_free_techniques);
    if (technique) {
        TAILQ_REMOVE(&module->m_free_techniques, technique, m_next);
    }
    else {
        technique = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_technique));
        if (technique == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_technique: create: alloc fail!");
            return NULL;
        }
    }

    technique->m_material = material;
    technique->m_render_state = NULL;
    technique->m_pass_count = 0;
    TAILQ_INIT(&technique->m_passes);

    TAILQ_INSERT_TAIL(&material->m_techniques, technique, m_next);
    
    return technique;
}

ui_runtime_render_technique_t
ui_runtime_render_technique_clone(ui_runtime_render_material_t material, ui_runtime_render_technique_t proto) {
    ui_runtime_render_technique_t technique;
    ui_runtime_render_pass_t proto_pass;

    technique = ui_runtime_render_technique_create(material);
    if (technique == NULL) return technique;

    if (proto->m_render_state) {
        technique->m_render_state = ui_runtime_render_state_clone(proto->m_render_state, NULL);
        if (technique->m_render_state == NULL) {
            ui_runtime_render_technique_free(technique);
            return NULL;
        }
    }

    TAILQ_FOREACH(proto_pass, &proto->m_passes, m_next) {
        ui_runtime_render_pass_t pass = ui_runtime_render_pass_clone(technique, proto_pass);
        if (pass == NULL) {
            ui_runtime_render_technique_free(technique);
            return NULL;
        }

        if (technique->m_render_state && pass->m_render_state) {
            pass->m_render_state->m_parent = technique->m_render_state;
        }
    }
    
    return technique;
}

void ui_runtime_render_technique_free(ui_runtime_render_technique_t technique) {
    ui_runtime_module_t module = technique->m_material->m_render->m_module;

    while(!TAILQ_EMPTY(&technique->m_passes)) {
        ui_runtime_render_pass_free(TAILQ_FIRST(&technique->m_passes));
    }

    if (technique->m_render_state) ui_runtime_render_state_free(technique->m_render_state);
    
    TAILQ_REMOVE(&technique->m_material->m_techniques, technique, m_next);
    if (technique->m_material->m_cur_technique == technique) {
        technique->m_material->m_cur_technique = NULL;
    }
    assert(technique->m_pass_count == 0);
    
    technique->m_material = (ui_runtime_render_material_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_techniques, technique, m_next);
}

void ui_runtime_render_technique_real_free(ui_runtime_render_technique_t technique) {
    ui_runtime_module_t module = (ui_runtime_module_t)technique->m_material;
    TAILQ_REMOVE(&module->m_free_techniques, technique, m_next);
    mem_free(module->m_alloc, technique);
}

ui_runtime_render_material_t
ui_runtime_render_technique_material(ui_runtime_render_technique_t technique) {
    return technique->m_material;
}

ui_runtime_render_state_t
ui_runtime_render_technique_render_state_check_create(ui_runtime_render_technique_t technique) {
    if (technique->m_render_state == NULL) {
        ui_runtime_render_t render = technique->m_material->m_render;
        ui_runtime_render_state_t parent = ui_runtime_render_material_render_state(technique->m_material);
        ui_runtime_render_pass_t pass;

        technique->m_render_state = ui_runtime_render_state_create(render, parent);
        if (technique->m_render_state == NULL) {
            CPE_ERROR(render->m_module->m_em, "ui_runtime_render_technique_set_render_state: alloc fail");
            return NULL;
        }

        TAILQ_FOREACH(pass, &technique->m_passes, m_next) {
            if (pass->m_render_state) {
                pass->m_render_state->m_parent = technique->m_render_state;
            }
        }
    }

    return technique->m_render_state;
}

int ui_runtime_render_technique_set_render_state(ui_runtime_render_technique_t technique, ui_runtime_render_state_data_t render_state_data) {
    ui_runtime_render_state_t render_state = ui_runtime_render_technique_render_state_check_create(technique);
    if (render_state == NULL) return -1;
    render_state->m_data = *render_state_data;
    return 0;
}

ui_runtime_render_state_t
ui_runtime_render_technique_render_state(ui_runtime_render_technique_t technique) {
    if (technique->m_render_state) return technique->m_render_state;
    if (technique->m_material->m_render_state) return technique->m_material->m_render_state;
    return technique->m_material->m_render_state_parent;
}

void ui_runtime_render_technique_set_parent_render_state(
    ui_runtime_render_technique_t technique, ui_runtime_render_state_t parent)
{
    if (technique->m_render_state) {
        technique->m_render_state->m_parent = parent;
    }
    else {
        ui_runtime_render_pass_t pass;

        TAILQ_FOREACH(pass, &technique->m_passes, m_next) {
            if (pass->m_render_state) {
                pass->m_render_state->m_parent = parent;
            }
        }
    }
}

uint8_t ui_runtime_render_technique_pass_count(ui_runtime_render_technique_t technique) {
    return technique->m_pass_count;
}

static ui_runtime_render_pass_t
ui_runtime_render_technique_pass_next(struct ui_runtime_render_pass_it * it) {
    ui_runtime_render_pass_t * data = (ui_runtime_render_pass_t *)(it->m_data);
    ui_runtime_render_pass_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_runtime_render_technique_passes(ui_runtime_render_technique_t technique, ui_runtime_render_pass_it_t it) {
    *(ui_runtime_render_pass_t *)(it->m_data) = TAILQ_FIRST(&technique->m_passes);
    it->next = ui_runtime_render_technique_pass_next;
}
