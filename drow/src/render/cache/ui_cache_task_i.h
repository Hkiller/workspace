#ifndef UI_CACHE_TASK_I_H
#define UI_CACHE_TASK_I_H
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_cache_task_type {
    ui_cache_task_type_load_res = 1,
};

struct ui_cache_task {
    enum ui_cache_task_type m_task_type;
    union {
        struct {
            uint16_t m_op_id;
            ui_cache_res_t m_res;
            char m_root[256];
        } m_load_res;
    };
};

struct ui_cache_back_task {
    enum ui_cache_task_type m_task_type;
    union {
        struct {
            ui_cache_res_t m_res;
        } m_load_res;
    };
};
    
int ui_cache_add_load_res_task(ui_cache_manager_t cache_mgr, ui_cache_res_t res, const char * root);

ptr_int_t ui_cache_task_main_thread_process(void * ctx, ptr_int_t arg, float delta_s);
    
#ifdef __cplusplus
}
#endif

#endif
