#ifndef UI_R_SAVE_UTILS_H
#define UI_R_SAVE_UTILS_H
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "render/model/ui_model_types.h"
#include "protocol/render/model/ui_common.h"

int ui_data_proj_save_gen_meta_file(const char * root, ui_data_src_t src, error_monitor_t em);

void ui_data_proj_save_rect(write_stream_t s, int level, const char * tag, UI_RECT const * rect); 
void ui_data_proj_save_bool(write_stream_t s, int level, const char * tag, uint8_t value);
void ui_data_proj_save_int(write_stream_t s, int level, const char * tag, int value);
void ui_data_proj_save_str(write_stream_t s, int level, const char * tag, const char * value);
void ui_data_proj_save_float(write_stream_t s, int level, const char * tag, float value);
void ui_data_proj_save_vector_2(write_stream_t s, int level, const char * tag, UI_VECTOR_2 const * vec);
void ui_data_proj_save_vector_3(write_stream_t s, int level, const char * tag, UI_VECTOR_3 const * vec);
void ui_data_proj_save_vector_4(write_stream_t s, int level, const char * tag, UI_VECTOR_4 const * vec);
void ui_data_proj_save_color(write_stream_t s, int level, const char * tag, UI_COLOR const * color);
void ui_data_proj_save_unit(write_stream_t s, int level, const char * tag, UI_UNIT const * value);
void ui_data_proj_save_unit_vector_2(write_stream_t s, int level, const char * tag, UI_UNIT_VECTOR_2 const * vec);
void ui_data_proj_save_unit_rect(write_stream_t s, int level, const char * tag, UI_UNIT_RECT const * rect);
void ui_data_proj_save_obj_start(write_stream_t s, int level, const char * tag);
void ui_data_proj_save_obj_end(write_stream_t s, int level, const char * tag);
void ui_data_proj_save_obj_null(write_stream_t s, int level, const char * tag);

void ui_data_proj_save_ol(write_stream_t s, int level, UI_OL const * ol);
void ui_data_proj_save_trans(write_stream_t s, int level, UI_TRANS const * trans);

#endif
