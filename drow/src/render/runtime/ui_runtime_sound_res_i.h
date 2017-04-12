#ifndef UI_RUNTIME_SOUND_RES_I_H
#define UI_RUNTIME_SOUND_RES_I_H
#include "render/runtime/ui_runtime_sound_res.h"
#include "ui_runtime_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_sound_res {
    ui_runtime_sound_backend_t m_backend;
    char m_data[64];
};

#ifdef __cplusplus
}
#endif

#endif 
