#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "ui_ed_rel_i.h"
#include "ui_ed_obj_i.h"

ui_ed_rel_t ui_ed_rel_create_i(ui_ed_rel_type_t rel_type, ui_ed_obj_t a_side, ui_ed_obj_t b_side) {
    ui_ed_mgr_t ed_mgr = a_side->m_src->m_ed_mgr;
    ui_ed_rel_t rel;

    rel = mem_alloc(ed_mgr->m_alloc, sizeof(struct ui_ed_rel));
    if (rel == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed obj rel alloc fail!");
        return NULL;
    }

    rel->m_type = rel_type;
    rel->m_a_side = a_side;
    TAILQ_INSERT_TAIL(&a_side->m_use_objs, rel, m_next_for_a_side);

    rel->m_b_side = b_side;
    if (b_side) {
        TAILQ_INSERT_TAIL(&b_side->m_used_by_objs, rel, m_next_for_b_side);
    }

    return rel;
}

void ui_ed_rel_free_i(ui_ed_rel_t rel) {
    ui_ed_mgr_t ed_mgr = rel->m_a_side->m_src->m_ed_mgr;

    if (rel->m_a_side) {
        TAILQ_REMOVE(&rel->m_a_side->m_use_objs, rel, m_next_for_a_side);
    }

    if (rel->m_b_side) {
        TAILQ_REMOVE(&rel->m_b_side->m_used_by_objs, rel, m_next_for_b_side);
    }

    mem_free(ed_mgr->m_alloc, rel);
}

void ui_ed_rel_connect(ui_ed_rel_t rel) {
    assert(rel->m_b_side == NULL);

    rel->m_b_side = rel->m_a_side->m_meta->m_rels[rel->m_type - UI_ED_REL_TYPE_MIN].m_load(rel->m_a_side);
    if (rel->m_b_side) {
        TAILQ_INSERT_TAIL(&rel->m_b_side->m_used_by_objs, rel, m_next_for_b_side);
    }
}

void ui_ed_rel_disconnect(ui_ed_rel_t rel) {
    if (rel->m_b_side) {
        TAILQ_REMOVE(&rel->m_b_side->m_used_by_objs, rel, m_next_for_b_side);
        rel->m_b_side = NULL;
    }
}

const char * ui_ed_rel_type_name(ui_ed_rel_type_t rel_type) {
    switch(rel_type) {
    case ui_ed_rel_type_use_img:
        return "use_img";
    case ui_ed_rel_type_use_actor:
        return "use_actor";
    default:
        return "unknown-rel-type";
    }
}

ui_ed_rel_type_t ui_ed_rel_type_from_name(const char * rel_type_name) {
    if (strcmp(rel_type_name, "use_img") == 0) {
        return ui_ed_rel_type_use_img;
    }
    else if (strcmp(rel_type_name, "use_actor") == 0) {
        return ui_ed_rel_type_use_actor;
    }
    else {
        return 0;
    }
}
