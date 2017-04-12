#ifndef DROW_PLUGIN_PACK_TYPES_H
#define DROW_PLUGIN_PACK_TYPES_H
#include "cpe/pal/pal_types.h"
#include "render/utils/ui_utils_types.h"
#include "render/model/ui_model_types.h"
#include "render/cache/ui_cache_types.h"
#include "render/model_ed/ui_ed_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_pack_module * plugin_pack_module_t;
typedef struct plugin_pack_packer * plugin_pack_packer_t;
typedef struct plugin_pack_language * plugin_pack_language_t;
typedef struct plugin_pack_language_it * plugin_pack_language_it_t;    
typedef struct plugin_pack_texture * plugin_pack_texture_t;
typedef struct plugin_pack_texture_it * plugin_pack_texture_it_t;

#ifdef __cplusplus
}
#endif

#endif
