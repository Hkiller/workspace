#ifndef UI_R_SAVE_UTILS_H
#define UI_R_SAVE_UTILS_H
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "render/model/ui_model_types.h"
#include "protocol/render/model/ui_common.h"
#include "plugin/particle/plugin_particle_types.h"

void plugin_particle_manip_proj_save_bool(write_stream_t s, int level, const char * tag, uint8_t value);
void plugin_particle_manip_proj_save_int(write_stream_t s, int level, const char * tag, int value);
void plugin_particle_manip_proj_save_str(write_stream_t s, int level, const char * tag, const char * value);
void plugin_particle_manip_proj_save_float(write_stream_t s, int level, const char * tag, float value);
void plugin_particle_manip_proj_save_obj_start(write_stream_t s, int level, const char * tag);
void plugin_particle_manip_proj_save_obj_end(write_stream_t s, int level, const char * tag);
void plugin_particle_manip_proj_save_curve_chanel(write_stream_t s, int level, const char * tag, plugin_particle_data_t particle, uint16_t chanel_id);

#endif
