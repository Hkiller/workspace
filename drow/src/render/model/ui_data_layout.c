#include <assert.h>
#include "render/utils/ui_string_table.h"
#include "ui_data_layout_i.h"
#include "ui_data_src_i.h"

ui_data_layout_t ui_data_layout_create(ui_data_mgr_t mgr, ui_data_src_t src) {
    ui_data_layout_t layout;

    if (src->m_type != ui_data_src_type_layout) {
        CPE_ERROR(
            mgr->m_em, "create layout at %s: src not layout!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    if (src->m_product) {
        CPE_ERROR(
            mgr->m_em, "create layout at %s: product already loaded!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    layout = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_layout));
    if (layout == NULL) {
        CPE_ERROR(
            mgr->m_em, "create layout at %s: alloc fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    layout->m_mgr = mgr;
    layout->m_src = src;
    layout->m_root = NULL;
    layout->m_control_count = 0;
    layout->m_string_table = ui_string_table_create(mgr->m_alloc, mgr->m_em);
    if (layout->m_string_table == NULL) {
        CPE_ERROR(
            mgr->m_em, "create layout at %s: create string table fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        mem_free(mgr->m_alloc, layout);
        return NULL;
    }

    src->m_product = layout;
    return layout;
}

void ui_data_layout_free(ui_data_layout_t layout) {
    ui_data_mgr_t mgr = layout->m_mgr;

    assert(layout->m_src->m_product == layout);
    layout->m_src->m_product = NULL;

    if (layout->m_root) {
        ui_data_control_free(layout->m_root);
        assert(layout->m_root == NULL);
    }

    assert(layout->m_control_count == 0);

    if (layout->m_string_table) {
        ui_string_table_free(layout->m_string_table);
        layout->m_string_table = NULL;
    }
    
    mem_free(mgr->m_alloc, layout);
}

ui_data_mgr_t ui_data_layout_mgr(ui_data_layout_t layout) {
    return layout->m_mgr;
}

ui_data_src_t ui_data_layout_src(ui_data_layout_t layout) {
    return layout->m_src;
}

ui_data_control_t ui_data_layout_root(ui_data_layout_t layout) {
    return layout->m_root;
}

uint16_t ui_data_layout_control_count(ui_data_layout_t layout) {
    return layout->m_control_count;
}

ui_string_table_t ui_data_layout_strings(ui_data_layout_t layout) {
    return layout->m_string_table;
}

int ui_data_control_ref_to_object_ref(UI_OBJECT_URL * obj_url, UI_CONTROL_OBJECT_URL const * ref) {
    obj_url->type = ref->type;
    switch(obj_url->type) {
    case UI_OBJECT_TYPE_IMG_BLOCK:
        obj_url->data.img_block.id = ref->res_id;
        obj_url->data.img_block.name[0] = 0;
        obj_url->data.img_block.src.type = UI_OBJECT_SRC_REF_TYPE_BY_ID;
        obj_url->data.img_block.src.data.by_id.src_id = ref->src_id;
        break;
    case UI_OBJECT_TYPE_FRAME:
        obj_url->data.frame.id = ref->res_id;
        obj_url->data.frame.name[0] = 0;
        obj_url->data.frame.src.type = UI_OBJECT_SRC_REF_TYPE_BY_ID;
        obj_url->data.frame.src.data.by_id.src_id = ref->src_id;
        break;
    case UI_OBJECT_TYPE_ACTOR:
        obj_url->data.actor.id = ref->res_id;
        obj_url->data.actor.name[0] = 0;
        obj_url->data.actor.src.type = UI_OBJECT_SRC_REF_TYPE_BY_ID;
        obj_url->data.actor.src.data.by_id.src_id = ref->src_id;
        break;
    default:
        return -1;
    }

    return 0;
}

