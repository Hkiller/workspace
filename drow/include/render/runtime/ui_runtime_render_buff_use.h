#ifndef UI_RUNTIME_RENDER_BUFF_USE_H
#define UI_RUNTIME_RENDER_BUFF_USE_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_vertex_v3f_t2f_c4ub {
    ui_vector_3 m_pos;
    ui_vector_2 m_uv;
    uint32_t m_c;
};
    
typedef enum ui_runtime_render_buff_source {
    ui_runtime_render_buff_source_device,
    ui_runtime_render_buff_source_inline,
} ui_runtime_render_buff_source_t;
    
struct ui_runtime_render_buff_use {
    ui_runtime_render_buff_source_t m_data_source;
    union {
        ui_runtime_render_vertex_buff_t m_vertex_buf;
        struct {
            ui_runtime_render_buff_type_t m_e_type;
            void const * m_buf;
            uint32_t m_count;
        } m_inline;
    };
};

struct ui_runtime_render_buff_use ui_runtime_render_buff_inline(void const * buf, ui_runtime_render_buff_type_t e_type, uint32_t count);
struct ui_runtime_render_buff_use ui_runtime_render_buff_device(ui_runtime_render_vertex_buff_t vertex_buf);

int ui_runtime_render_copy_buf(ui_runtime_render_t render, ui_runtime_render_buff_use_t o_buff_use, ui_runtime_render_buff_use_t i_buff_use);
void * ui_runtime_render_alloc_buf(ui_runtime_render_t render, ui_runtime_render_buff_type_t e_type, uint32_t count);
void * ui_runtime_render_append_buf(ui_runtime_render_t render, ui_runtime_render_buff_use_t buff_use, uint32_t append_count);
    
uint32_t ui_runtime_render_buff_use_count(ui_runtime_render_buff_use_t buff_use);
ui_runtime_render_buff_type_t ui_runtime_render_buff_use_type(ui_runtime_render_buff_use_t buff_use);

uint32_t ui_runtime_render_buff_type_stride(ui_runtime_render_buff_type_t e_type);
    
#ifdef __cplusplus
}
#endif

#endif
