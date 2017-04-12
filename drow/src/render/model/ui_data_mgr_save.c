#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/file.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "ui_data_mgr_i.h"

#define ui_data_mgr_add_processing_src(src) \
    if (processing_src_count + 1 >= processing_src_capacity) { \
        uint32_t new_capacity = processing_src_capacity ? processing_src_capacity * 2 : 128; \
        ui_data_src_t * new_buf = mem_alloc(NULL, sizeof(processing_srcs[0]) * new_capacity); \
        if (new_buf == NULL) {                                          \
            CPE_ERROR(mgr->m_em, "ui_data_mgr_save: alloc buf fail, capacity = %d", new_capacity); \
            goto SAVE_COMPLETE;                                         \
        }                                                               \
        if (processing_srcs) {                                          \
            memcpy(new_buf, processing_srcs, sizeof(processing_srcs[0]) * processing_src_count); \
            mem_free(NULL, processing_srcs);                            \
        }                                                               \
        processing_srcs = new_buf;                                      \
        processing_src_capacity = new_capacity;                         \
    }                                                                   \
    processing_srcs[processing_src_count++] = src

int ui_data_mgr_save(ui_data_mgr_t mgr, const char * dir) {
    ui_data_src_t * processing_srcs = NULL;
    uint32_t processing_src_capacity = 0;
    uint32_t processing_src_count = 0;
    ui_data_src_t root = ui_data_mgr_src_root(mgr);
    struct mem_buffer path_buff;

    mem_buffer_init(&path_buff, NULL);

    if (dir == NULL) dir = ui_data_src_data(root);

    ui_data_mgr_add_processing_src(root);

    while(processing_src_count > 0) {
        ui_data_src_t src = processing_srcs[--processing_src_count];

        if(ui_data_src_type(src) == ui_data_src_type_dir) {
            struct ui_data_src_it src_it;
            ui_data_src_t child;

            ui_data_src_childs(&src_it, src);
            while((child = ui_data_src_it_next(&src_it))) {
                ui_data_mgr_add_processing_src(child);
            }
            break;
        }
        else {
            if (ui_data_src_load_state(src) == ui_data_src_state_loaded) {
                ui_data_src_save(src, dir, mgr->m_em);
            }
        }
    }

SAVE_COMPLETE:
    if (processing_srcs) {
        mem_free(NULL, processing_srcs);
    }

    mem_buffer_clear(&path_buff);

    return 0;
}

int ui_data_mgr_save_by_type(ui_data_mgr_t mgr, const char * dir, ui_data_src_type_t type) {
    ui_data_src_t * processing_srcs = NULL;
    uint32_t processing_src_capacity = 0;
    uint32_t processing_src_count = 0;
    ui_data_src_t root = ui_data_mgr_src_root(mgr);
    struct mem_buffer path_buff;

    mem_buffer_init(&path_buff, NULL);

    if (dir == NULL) dir = ui_data_src_data(root);

    ui_data_mgr_add_processing_src(root);

    while(processing_src_count > 0) {
        ui_data_src_t src = processing_srcs[--processing_src_count];

        if (ui_data_src_type(src) == ui_data_src_type_dir) {
            struct ui_data_src_it src_it;
            ui_data_src_t child;

            ui_data_src_childs(&src_it, src);
            while((child = ui_data_src_it_next(&src_it))) {
                ui_data_mgr_add_processing_src(child);
            }
        }
        else if (ui_data_src_type(src) == type) {
            if (ui_data_src_load_state(src) == ui_data_src_state_loaded) {
                ui_data_src_save(src, dir, mgr->m_em);
            }
        }
    }

SAVE_COMPLETE:
    if (processing_srcs) {
        mem_free(NULL, processing_srcs);
    }

    mem_buffer_clear(&path_buff);

    return 0;
}
