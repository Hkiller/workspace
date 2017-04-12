#ifndef UI_PLUGIN_PARTICLE_OBJ_SUB_I_H
#define UI_PLUGIN_PARTICLE_OBJ_SUB_I_H
#include "plugin_particle_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj_sub {
    struct plugin_particle_obj m_obj;
    struct ui_matrix_4x4 m_dominated_xform; /** dominated tranformation */
    struct ui_color m_dominated_color; /** dominated color */
};

#ifdef __cplusplus
}
#endif

#endif
