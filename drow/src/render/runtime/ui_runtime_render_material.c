#include <assert.h>
#include "ui_runtime_render_material_i.h"
#include "ui_runtime_render_technique_i.h"
#include "ui_runtime_render_pass_i.h"
#include "ui_runtime_render_state_i.h"

ui_runtime_render_material_t
ui_runtime_render_material_create(ui_runtime_render_t render) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_material_t material;

    material = TAILQ_FIRST(&module->m_free_materials);
    if (material) {
        TAILQ_REMOVE(&module->m_free_materials, material, m_next);
    }
    else {
        material = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_material));
        if (material == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_material: create: alloc fail!");
            return NULL;
        }
    }

    material->m_render = render;
    TAILQ_INIT(&material->m_techniques);
    material->m_render_state_parent = NULL;
    material->m_render_state = NULL;
    material->m_cur_technique = NULL;
    
    TAILQ_INSERT_TAIL(&render->m_materials, material, m_next);
    
    return material;
}

ui_runtime_render_material_t
ui_runtime_render_material_clone(ui_runtime_render_material_t proto) {
    ui_runtime_render_material_t material;
    ui_runtime_render_technique_t proto_technique;

    material = ui_runtime_render_material_create(proto->m_render);
    if (material == NULL) return NULL;

    if (proto->m_render_state) {
        material->m_render_state = ui_runtime_render_state_clone(proto->m_render_state, NULL);
        if (material->m_render_state == NULL) {
            ui_runtime_render_material_free(material);
            return NULL;
        }
    }
    
    TAILQ_FOREACH(proto_technique, &proto->m_techniques, m_next) {
        ui_runtime_render_technique_t technique = ui_runtime_render_technique_clone(material, proto_technique);
        if (technique == NULL) {
            ui_runtime_render_material_free(material);
            return NULL;
        }

        if (material->m_render_state) {
            ui_runtime_render_technique_set_parent_render_state(technique, material->m_render_state);
        }
        
        if (proto_technique == proto->m_cur_technique) {
            material->m_cur_technique = technique;
        }
    }
    
    return material;
}

void ui_runtime_render_material_free(ui_runtime_render_material_t material) {
    ui_runtime_module_t module = material->m_render->m_module;

    while(!TAILQ_EMPTY(&material->m_techniques)) {
        ui_runtime_render_technique_free(TAILQ_FIRST(&material->m_techniques));
    }
    assert(material->m_cur_technique == NULL);

    if (material->m_render_state) ui_runtime_render_state_free(material->m_render_state);
    
    TAILQ_REMOVE(&material->m_render->m_materials, material, m_next);

    material->m_render = (ui_runtime_render_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_materials, material, m_next);
}

void ui_runtime_render_material_real_free(ui_runtime_render_material_t material) {
    ui_runtime_module_t module = (ui_runtime_module_t)material->m_render;
    TAILQ_REMOVE(&module->m_free_materials, material, m_next);
    mem_free(module->m_alloc, material);
}

ui_runtime_render_t ui_runtime_render_material_render(ui_runtime_render_material_t material) {
    return material->m_render;
}

ui_runtime_render_technique_t
ui_runtime_render_material_cur_technique(ui_runtime_render_material_t material) {
    return material->m_cur_technique;
}

void ui_runtime_render_material_set_cur_technique(
    ui_runtime_render_material_t material, ui_runtime_render_technique_t technique)
{
    material->m_cur_technique = technique;
}

ui_runtime_render_state_t
ui_runtime_render_material_check_create_render_state(ui_runtime_render_material_t material) {
    if (material->m_render_state == NULL) {
        ui_runtime_render_t render = material->m_render;
        ui_runtime_render_technique_t technique;

        material->m_render_state = ui_runtime_render_state_create(render, material->m_render_state_parent);
        if (material->m_render_state == NULL) {
            CPE_ERROR(render->m_module->m_em, "ui_runtime_render_material_set_render_state: alloc fail");
            return NULL;
        }

        TAILQ_FOREACH(technique, &material->m_techniques, m_next) {
            ui_runtime_render_technique_set_parent_render_state(technique, material->m_render_state);
        }
    }

    return material->m_render_state;
}

int ui_runtime_render_material_set_render_state(ui_runtime_render_material_t material, ui_runtime_render_state_data_t render_state_data) {
    ui_runtime_render_state_t render_state = ui_runtime_render_material_check_create_render_state(material);
    if (render_state == NULL) return -1;
    render_state->m_data = *render_state_data;
    return 0;
}

ui_runtime_render_state_t
ui_runtime_render_material_render_state(ui_runtime_render_material_t material) {
    if (material->m_render_state) return material->m_render_state;
    return material->m_render_state_parent;
}

void ui_runtime_render_material_set_parent_render_state(
    ui_runtime_render_material_t material, ui_runtime_render_state_t parent)
{
    material->m_render_state_parent = parent;
    
    if (material->m_render_state) {
        material->m_render_state->m_parent = parent;
    }
    else {
        ui_runtime_render_technique_t technique;
        
        TAILQ_FOREACH(technique, &material->m_techniques, m_next) {
            ui_runtime_render_technique_set_parent_render_state(technique, parent);
        }
    }
}

static ui_runtime_render_technique_t
ui_runtime_render_material_technique_next(struct ui_runtime_render_technique_it * it) {
    ui_runtime_render_technique_t * data = (ui_runtime_render_technique_t *)(it->m_data);
    ui_runtime_render_technique_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_runtime_render_material_techniques(ui_runtime_render_material_t material, ui_runtime_render_technique_it_t it) {
    *(ui_runtime_render_technique_t *)(it->m_data) = TAILQ_FIRST(&material->m_techniques);
    it->next = ui_runtime_render_material_technique_next;
}
