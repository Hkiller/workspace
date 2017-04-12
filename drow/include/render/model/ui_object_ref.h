#ifndef UI_MODEL_OBJECT_REF_H
#define UI_MODEL_OBJECT_REF_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "ui_model_types.h"
#include "protocol/render/model/ui_object_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

UI_OBJECT_URL * ui_object_ref_parse(const char * url, UI_OBJECT_URL * buf, error_monitor_t em);

//void ui_object_ref_dump(UI_OBJECT_URL const * rul, struct 

#ifdef __cplusplus
}
#endif

#endif
