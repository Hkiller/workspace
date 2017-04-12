#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_src.h"
#include "ui_data_layout_i.h"
#include "ui_data_src_i.h"

ui_data_control_t ui_data_control_create(ui_data_layout_t layout, ui_data_control_t parent) {
    ui_data_mgr_t mgr = layout->m_mgr;
    ui_data_control_t control;

    if (parent ==  NULL) {
        if (layout->m_root != NULL) {
            CPE_ERROR(
                mgr->m_em, "create control in layout %s: no parent!",
                ui_data_src_path_dump(&mgr->m_dump_buffer, layout->m_src));
            return NULL;
        }
    }

    control = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_control));
    if (control == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in layout %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, layout->m_src));
        return NULL;
    }

    bzero(control, sizeof(*control));
    control->m_layout = layout;
    control->m_parent = parent;
    TAILQ_INIT(&control->m_childs);
    TAILQ_INIT(&control->m_anims);
    TAILQ_INIT(&control->m_additions);

    if (parent) {
        parent->m_child_count++;
        TAILQ_INSERT_TAIL(&parent->m_childs, control, m_next_for_parent);
    }
    else {
        assert(layout->m_root == NULL);
        layout->m_root = control;
    }

    dr_meta_set_defaults(&control->m_data, sizeof(control->m_data), mgr->m_meta_control, DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);
    control->m_data.id = (uint32_t)-1;
    
    layout->m_control_count++;
    
    return control;
}

void ui_data_control_free(ui_data_control_t control) {
    ui_data_layout_t layout = control->m_layout;
    ui_data_mgr_t mgr = layout->m_mgr;

    while(!TAILQ_EMPTY(&control->m_childs)) {
        ui_data_control_free(TAILQ_FIRST(&control->m_childs));
    }
    assert(control->m_child_count == 0);

    while(!TAILQ_EMPTY(&control->m_anims)) {
        ui_data_control_anim_free(TAILQ_FIRST(&control->m_anims));
    }
    assert(control->m_anim_count == 0);

    while(!TAILQ_EMPTY(&control->m_additions)) {
        ui_data_control_addition_free(TAILQ_FIRST(&control->m_additions));
    }
    assert(control->m_addition_count == 0);

    if (control->m_parent) {
        control->m_parent->m_child_count--;
        TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
    }

    if (control == layout->m_root) {
        layout->m_root = NULL;
    }

    if (control->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&layout->m_mgr->m_controls_by_id, control);
    }

    layout->m_control_count--;
    
    mem_free(mgr->m_alloc, control);
}

ui_data_layout_t ui_data_control_layout(ui_data_control_t control) {
    return control->m_layout;
}

uint8_t ui_data_control_type(ui_data_control_t control) {
    return control->m_data.type;
}

LPDRMETA ui_data_control_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_control;
}

UI_CONTROL *
ui_data_control_data(ui_data_control_t control) {
    return &control->m_data;
}

const char * ui_data_control_msg(ui_data_control_t control, uint32_t msg_id) {
    return msg_id ? ui_string_table_message(control->m_layout->m_string_table, msg_id) : "";
}

ui_data_control_t ui_data_control_parent(ui_data_control_t control) {
    return control->m_parent;
}

uint16_t ui_data_control_child_count(ui_data_control_t control) {
    return control->m_child_count;
}

static ui_data_control_t ui_data_control_child_next(struct ui_data_control_it * it) {
    ui_data_control_t * data = (ui_data_control_t *)(it->m_data);
    ui_data_control_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_parent);

    return r;
}

void ui_data_control_childs(ui_data_control_it_t it, ui_data_control_t control) {
    *(ui_data_control_t *)(it->m_data) = TAILQ_FIRST(&control->m_childs);
    it->next = ui_data_control_child_next;
}

ui_data_control_t ui_data_control_child_find_by_name(ui_data_control_t control, const char * name) {
    ui_data_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        if (strcmp(ui_data_control_name(child), name) == 0) return child;
    }

    return NULL;
}

ui_data_control_t ui_data_control_find_by_id(ui_data_layout_t layout, uint32_t id) {
    char key_buf[sizeof(struct ui_data_control) + sizeof(UI_CONTROL_BASIC)];
    ui_data_control_t key = (ui_data_control_t)&key_buf;

    key->m_layout = layout;
    key->m_data.id = id;

    return cpe_hash_table_find(&layout->m_mgr->m_controls_by_id, key);
}

int ui_data_control_set_id(ui_data_control_t control, uint32_t id) {
    ui_data_layout_t layout = control->m_layout;
    uint32_t old_id = control->m_data.id;
    assert(layout);

    if (control->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&layout->m_mgr->m_controls_by_id, control);
    }

    control->m_data.id = id;
    if (control->m_data.id != (uint32_t)-1) {
        cpe_hash_entry_init(&control->m_hh_for_mgr);
        if (cpe_hash_table_insert(&layout->m_mgr->m_controls_by_id, control) != 0) {
            control->m_data.id = old_id;
            if (old_id != (uint32_t)-1) {
                cpe_hash_table_insert_unique(&layout->m_mgr->m_controls_by_id, control);
            }
            return -1;
        }
    }

    return 0;
}

uint32_t ui_data_control_id(ui_data_control_t control) {
    return control->m_data.id;
}

const char * ui_data_control_name(ui_data_control_t control) {
    return ui_data_control_msg(control, control->m_data.name_id);
}

uint32_t ui_data_control_hash(const ui_data_control_t control) {
    return control->m_layout->m_src->m_id & control->m_data.id;
}

int ui_data_control_eq(const ui_data_control_t l, const ui_data_control_t r) {
    return l->m_data.id == r->m_data.id && l->m_layout == r->m_layout;
}

static const char * s_control_type_names[] = {
    "unknown",
    "window",
    "panel",
    "picture",
    "label",
    "button",
    "toggle",
    "progress",
    "picture_cond",
    "check_box",
    "radio_box",
    "combo_box",
    "edit_box",
    "multi_edit_box",
    "label_condition",
    "rich_label",
    "rich_text",
    "list_box_col",
    "list_box_row",
    "list_box_adv_item",
    "scroll_panel",
    "slider",
    "switcher",
    "tab",
    "tab_page",
    "swiper",
    "comb_box_drop_list",
};

const char * ui_data_control_type_name(uint8_t control_type) {
    if (control_type >= CPE_ARRAY_SIZE(s_control_type_names)) return s_control_type_names[0];
    return s_control_type_names[control_type];
}
