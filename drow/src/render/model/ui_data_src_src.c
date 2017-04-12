#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "ui_data_src_src_i.h"

ui_data_src_src_t
ui_data_src_src_create(ui_data_src_t user, ui_data_src_t be_using) {
    ui_data_mgr_t mgr = user->m_mgr;
    ui_data_src_src_t src_src;

    TAILQ_FOREACH(src_src, &user->m_using_srcs, m_next_for_user) {
        if (src_src->m_using_src == be_using) return src_src;
    }
    
    src_src = TAILQ_FIRST(&mgr->m_free_src_srcs);
    if (src_src) {
        TAILQ_REMOVE(&mgr->m_free_src_srcs, src_src, m_next_for_user);
    }
    else {
        src_src = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_src_src));
        if (src_src == NULL) {
            CPE_ERROR(mgr->m_em, "ui_data_src_src_create: alloc fail!");
            return NULL;
        }
    }

    src_src->m_user_src = user;
    src_src->m_using_src = be_using;

    TAILQ_INSERT_TAIL(&user->m_using_srcs, src_src, m_next_for_user);
    TAILQ_INSERT_TAIL(&be_using->m_user_srcs, src_src, m_next_for_using);

    return src_src;
}

ui_data_src_src_t ui_data_src_src_create_by_id(ui_data_src_t user, uint32_t src_id, uint8_t type) {
    ui_data_mgr_t mgr = user->m_mgr;
    ui_data_src_t be_using;

    be_using = ui_data_src_find_by_id(mgr, src_id);
    if (be_using == NULL) {
        CPE_ERROR(mgr->m_em, "ui_data_src_src_create: using src " FMT_UINT32_T "(type=%s) not exist!", src_id, ui_data_src_type_name(type));
        return NULL;
    }

    if (ui_data_src_type(be_using) != type) {
        CPE_ERROR(
            mgr->m_em, "ui_data_src_src_create: using src " FMT_UINT32_T " type maismatch expect %s, but %s!",
            src_id, ui_data_src_type_name(type), ui_data_src_type_name(ui_data_src_type(be_using)));
        return NULL;
    }
    
    return ui_data_src_src_create(user, be_using);
}

ui_data_src_src_t ui_data_src_src_create_by_path(ui_data_src_t user, const char * path, uint8_t type) {
    ui_data_mgr_t mgr = user->m_mgr;
    ui_data_src_t be_using;

    be_using = ui_data_src_find_by_path(mgr, path, type);
    if (be_using == NULL) {
        CPE_ERROR(mgr->m_em, "ui_data_src_src_create: using src %s(type=%s) not exist!", path, ui_data_src_type_name(type));
        return NULL;
    }

    return ui_data_src_src_create(user, be_using);
}

ui_data_src_src_t ui_data_src_src_create_by_src_ref(ui_data_src_t user, UI_OBJECT_SRC_REF const * obj_ref, uint8_t type) {
    switch(obj_ref->type) {
    case UI_OBJECT_SRC_REF_TYPE_BY_ID:
        return ui_data_src_src_create_by_id(user, obj_ref->data.by_id.src_id, type);
    case UI_OBJECT_SRC_REF_TYPE_BY_PATH:
        return ui_data_src_src_create_by_path(user, obj_ref->data.by_path.path, type);
    default:
        CPE_ERROR(user->m_mgr->m_em, "src-src: create by src ref: obl ref type %d unknown!", obj_ref->type);
        return NULL;
    }
}

int ui_data_src_src_create_by_url(ui_data_src_t user, UI_OBJECT_URL const * url) {
    switch(url->type) {
    case UI_OBJECT_TYPE_NONE:
        return 0;
    case UI_OBJECT_TYPE_IMG_BLOCK:
        return ui_data_src_src_create_by_src_ref(user, &url->data.img_block.src, ui_data_src_type_module) ? 0 : -1;
    case UI_OBJECT_TYPE_FRAME:
        return ui_data_src_src_create_by_src_ref(user, &url->data.frame.src, ui_data_src_type_sprite) ? 0 : -1;
    case UI_OBJECT_TYPE_ACTOR:
        return ui_data_src_src_create_by_src_ref(user, &url->data.actor.src, ui_data_src_type_action) ? 0 : -1;
    case UI_OBJECT_TYPE_SKELETON:
        return ui_data_src_src_create_by_src_ref(user, &url->data.skeleton.src, ui_data_src_type_spine_skeleton) ? 0 : -1;
    case UI_OBJECT_TYPE_LAYOUT:
        return ui_data_src_src_create_by_src_ref(user, &url->data.layout.src, ui_data_src_type_layout) ? 0 : -1;
    default:
        CPE_ERROR(user->m_mgr->m_em, "add use: obl ref type %d unknown!", url->type);
        return -1;
    }
}

uint16_t ui_data_src_src_remove(ui_data_src_t user, ui_data_src_t be_used) {
    ui_data_src_src_t src_src, next_src_src;
    uint8_t removed_count = 0;
    
    for(src_src = TAILQ_FIRST(&user->m_using_srcs); src_src; src_src = next_src_src) {
        next_src_src = TAILQ_NEXT(src_src, m_next_for_user);

        if (src_src->m_using_src == be_used) {
            ui_data_src_src_free(src_src);
            removed_count++;
        }
    }

    return removed_count;
}

void ui_data_src_src_free(ui_data_src_src_t src_src) {
    ui_data_mgr_t mgr = src_src->m_user_src->m_mgr;
    
    TAILQ_REMOVE(&src_src->m_user_src->m_using_srcs, src_src, m_next_for_user);
    TAILQ_REMOVE(&src_src->m_using_src->m_user_srcs, src_src, m_next_for_using);

    src_src->m_user_src = (void*)mgr;
    TAILQ_INSERT_TAIL(&mgr->m_free_src_srcs, src_src, m_next_for_user);
}

void ui_data_src_src_real_free(ui_data_src_src_t src_src) {
    ui_data_mgr_t mgr = (void*)src_src->m_user_src;
    TAILQ_REMOVE(&mgr->m_free_src_srcs, src_src, m_next_for_user);
    mem_free(mgr->m_alloc, src_src);
}
