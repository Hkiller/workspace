#ifndef UI_R_UTILS_H
#define UI_R_UTILS_H
#include "render/model/ui_model_types.h"

const char * plugin_particle_manip_proj_particle_mod_type_name(uint8_t mod_type);
uint32_t plugin_particle_manip_proj_particle_mod_type_hash(uint8_t mod_type);
uint8_t plugin_particle_manip_proj_particle_mod_type(const char * mod_type_name);

#endif
