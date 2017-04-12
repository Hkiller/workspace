#ifndef UI_MODEL_MANIP_COCOS_H
#define UI_MODEL_MANIP_COCOS_H
#include "cpe/utils/memory.h"
#include "model_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_model_id_allocrator * ui_model_id_allocrator_t;

ui_model_id_allocrator_t ui_model_id_allocrator_create(mem_allocrator_t alloc);
void ui_model_id_allocrator_free(ui_model_id_allocrator_t id_alloc);

int ui_model_id_allocrator_remove(ui_model_id_allocrator_t  id_alloc, uint32_t id);
int ui_model_id_allocrator_alloc(ui_model_id_allocrator_t  id_alloc, uint32_t * id);

#ifdef __cplusplus
}
#endif

#endif
