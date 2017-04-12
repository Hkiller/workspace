#ifndef UI_MODEL_ED_SEARCH_I_H
#define UI_MODEL_ED_SEARCH_I_H
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_search.h"
#include "ui_ed_obj_i.h"

typedef struct ui_ed_search_root * ui_ed_search_root_t;

typedef TAILQ_HEAD(ui_ed_search_root_list, ui_ed_search_root) ui_ed_search_root_list_t;

typedef enum ui_ed_search_state {
    ui_ed_search_init
    , ui_ed_search_processing
    , ui_ed_search_completed
} ui_ed_search_state_t;

struct ui_ed_search {
    ui_ed_mgr_t m_ed_mgr;
    ui_ed_search_state_t m_search_state;
    ui_ed_search_root_list_t m_search_roots;
    uint64_t m_search_types;
    struct ui_data_src_it m_src_it;
    ui_ed_search_root_t m_cur_root;
    ui_ed_src_t m_cur_src;
    ui_ed_obj_t m_cur_obj;
};

struct ui_ed_search_root {
    ui_data_src_t m_root;
    TAILQ_ENTRY(ui_ed_search_root) m_next;
};

#endif
