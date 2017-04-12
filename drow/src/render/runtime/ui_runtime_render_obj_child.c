#include "ui_runtime_render_obj_child_i.h"

ui_runtime_render_obj_child_t
ui_runtime_render_obj_child_create(ui_runtime_render_obj_t obj, ui_runtime_render_obj_ref_t child_obj_ref, void const * tag, uint8_t auto_render) {
    ui_runtime_module_t module = obj->m_module;
    ui_runtime_render_obj_child_t child;

    child = TAILQ_FIRST(&module->m_free_obj_childs);
    if (child) {
        TAILQ_REMOVE(&module->m_free_obj_childs, child, m_next_for_obj);
    }
    else {
        child = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_obj_child));
        if (child == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_obj_child_create: alloc fail!");
            return NULL;
        }
    }

    child->m_obj = obj;
    child->m_child_obj_ref = child_obj_ref;
    child->m_tag = tag;
    child->m_auto_render = auto_render;

    if (tag) {
        cpe_hash_entry_init(&child->m_hh_for_module);
        if (cpe_hash_table_insert(&module->m_obj_childs, child) != 0) {
            CPE_ERROR(
                module->m_em, "%s: obj %s: insert child for tag %p fail!",
                ui_runtime_module_name(module), obj->m_name, tag);
            child->m_obj = (ui_runtime_render_obj_t)module;
            TAILQ_INSERT_TAIL(&module->m_free_obj_childs, child, m_next_for_obj);
            return NULL;
        }
    }

    TAILQ_INSERT_TAIL(&obj->m_childs, child, m_next_for_obj);
    
    return child;
}

void ui_runtime_render_obj_child_free(ui_runtime_render_obj_child_t child) {
    ui_runtime_module_t module = child->m_obj->m_module;

    if (child->m_tag) {
        cpe_hash_table_remove_by_ins(&module->m_obj_childs, child);
    }

    TAILQ_REMOVE(&child->m_obj->m_childs, child, m_next_for_obj);

    child->m_obj = (ui_runtime_render_obj_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_obj_childs, child, m_next_for_obj);
}

void ui_runtime_render_obj_child_real_free(ui_runtime_render_obj_child_t child) {
    ui_runtime_module_t module = (ui_runtime_module_t)child->m_obj;

    TAILQ_REMOVE(&module->m_free_obj_childs, child, m_next_for_obj);
    mem_free(module->m_alloc, child);
}

ui_runtime_render_obj_ref_t
ui_runtime_render_obj_find_child(ui_runtime_render_obj_t obj, void const * tag) {
    struct ui_runtime_render_obj_child key;
    ui_runtime_render_obj_child_t v;

    if (tag == NULL) return NULL;

    key.m_obj = obj;
    key.m_tag = tag;

    v = cpe_hash_table_find(&obj->m_module->m_obj_childs, &key);
    return v ? v->m_child_obj_ref : NULL;
}

void ui_runtime_render_obj_clear_childs(ui_runtime_render_obj_t obj) {
    while(!TAILQ_EMPTY(&obj->m_childs)) {
        ui_runtime_render_obj_child_free(TAILQ_FIRST(&obj->m_childs));
    }
}

uint32_t ui_runtime_render_obj_child_hash(ui_runtime_render_obj_child_t child) {
    return (((uint32_t)((ptr_int_t)child->m_obj)) << 16)
        | (((uint32_t)((ptr_int_t)child->m_tag)) & 0xFFFF);
}

int ui_runtime_render_obj_child_eq(ui_runtime_render_obj_child_t l, ui_runtime_render_obj_child_t r) {
    return (l->m_obj == r->m_obj && l->m_tag == r->m_tag) ? 1 : 0;
}

