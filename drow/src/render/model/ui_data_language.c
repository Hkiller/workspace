#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui_data_language_i.h"
#include "ui_data_src_i.h"

ui_data_language_t
ui_data_language_create(ui_data_mgr_t data_mgr, const char * name) {
    ui_data_language_t language;

    language = mem_alloc(data_mgr->m_alloc, sizeof(struct ui_data_language));
    if (language == NULL) {
        CPE_ERROR(data_mgr->m_em, "ui_data_language_create: alloc fail!");
        return NULL;
    }

    language->m_data_mgr = data_mgr;
    cpe_str_dup(language->m_name, sizeof(language->m_name), name);
    TAILQ_INIT(&language->m_srcs);
    
    TAILQ_INSERT_TAIL(&data_mgr->m_languages, language, m_next);
    return language;
}

void ui_data_language_free(ui_data_language_t language) {
    ui_data_mgr_t data_mgr = language->m_data_mgr;

    while (!TAILQ_EMPTY(&language->m_srcs)) {
        ui_data_src_free(TAILQ_FIRST(&language->m_srcs));
    }

    TAILQ_REMOVE(&language->m_data_mgr->m_languages, language, m_next);

    mem_free(data_mgr->m_alloc, language);
}

ui_data_language_t
ui_data_language_find(ui_data_mgr_t data_mgr, const char * name) {
    ui_data_language_t language;

    TAILQ_FOREACH(language, &data_mgr->m_languages, m_next) {
        if (strcmp(language->m_name, name) == 0) return language;
    }

    return NULL;
}

const char * ui_data_language_name(ui_data_language_t language) {
    return language->m_name;
}

ui_data_language_t ui_data_active_language(ui_data_mgr_t data_mgr) {
    return data_mgr->m_active_language;
}

void ui_data_language_active(ui_data_language_t language) {
    ui_data_mgr_t data_mgr = language->m_data_mgr;
    ui_data_src_t src;

    if (data_mgr->m_active_language) {
        ui_data_language_deactive(data_mgr->m_active_language);
    }

    assert(data_mgr->m_active_language == NULL);

    TAILQ_FOREACH(src, &language->m_srcs, m_next_for_language) {
        assert(src->m_base_src == NULL);
        ui_data_language_connect_src(language, src, NULL);
    }
    
    data_mgr->m_active_language = language;
}

void ui_data_language_connect_src(ui_data_language_t language, ui_data_src_t language_src, ui_data_src_t base_src) {
    if (base_src == NULL) {
        assert(language_src);
        assert(language_src->m_parent);
        base_src = ui_data_src_child_find_with_language(language_src->m_parent, language_src->m_data, language_src->m_type, NULL);
        if (base_src == NULL) return;
    }

    if (language_src == NULL) {
        assert(base_src);
        language_src = ui_data_language_find_src(language, base_src);
        if (language_src == NULL) return;
    }
    
    assert(base_src->m_language == NULL);
    assert(base_src->m_language_src == NULL);
    assert(language_src->m_base_src == NULL);
    assert(strcmp(language_src->m_data, base_src->m_data) == 0);
    
    base_src->m_language_src = language_src;
    language_src->m_base_src = base_src;
}

void ui_data_language_disconnect_src(ui_data_src_t language_src, ui_data_src_t base_src) {
    if (language_src) {
        assert(base_src == NULL || base_src == language_src->m_base_src);

        if (language_src->m_base_src) {
            language_src->m_base_src->m_language_src = NULL;
            language_src->m_base_src = NULL;
        }
    }
    else {
        assert(base_src);
        if (base_src->m_language_src) {
            base_src->m_language_src->m_base_src = NULL;
            base_src->m_language_src = NULL;
        }
    }
}

void ui_data_language_deactive(ui_data_language_t language) {
    ui_data_mgr_t data_mgr = language->m_data_mgr;

    if (data_mgr->m_active_language == language) {
        ui_data_src_t src;

        TAILQ_FOREACH(src, &language->m_srcs, m_next_for_language) {
            ui_data_language_disconnect_src(src, NULL);
        }

        data_mgr->m_active_language = NULL;
    }
}

static ui_data_language_t ui_data_language_in_mgr_next(struct ui_data_language_it * it) {
    ui_data_language_t * data = (ui_data_language_t *)(it->m_data);
    ui_data_language_t r = *data;

    if (r) {
        *data = TAILQ_NEXT(r, m_next);
    }

    return r;
}

void ui_data_languages(ui_data_language_it_t it, ui_data_mgr_t mgr) {
    *(ui_data_language_t *)(it->m_data) = TAILQ_FIRST(&mgr->m_languages);
    it->next = ui_data_language_in_mgr_next;
}

ui_data_src_t
ui_data_language_find_src(ui_data_language_t language, ui_data_src_t base_src) {
    ui_data_src_t r;

    for(r = cpe_hash_table_find(&language->m_data_mgr->m_srcs_by_name, base_src);
        r;
        r = cpe_hash_table_find_next(&language->m_data_mgr->m_srcs_by_name, r))
    {
        ui_data_src_t check_l, check_r;
        
        if (r->m_language != language || r->m_type != base_src->m_type) continue;

        for(check_l = r->m_parent, check_r = base_src->m_parent;
            check_l != NULL && check_r != NULL;
            check_l = check_l->m_parent, check_r = check_r->m_parent)
        {
            if (check_l == check_r) return r;
            if (strcmp(check_l->m_data, check_r->m_data) != 0) break;
        }
    }

    return NULL;
}

static ui_data_src_t ui_data_language_src_next(struct ui_data_src_it * it) {
    ui_data_src_t * data = (ui_data_src_t *)(it->m_data);
    ui_data_src_t r = *data;

    if (r) {
        *data = TAILQ_NEXT(r, m_next_for_language);
    }

    return r;
}

void ui_data_language_srcs(ui_data_src_it_t it, ui_data_language_t language) {
    *(ui_data_src_t *)(it->m_data) = TAILQ_FIRST(&language->m_srcs);
    it->next = ui_data_language_src_next;
}
