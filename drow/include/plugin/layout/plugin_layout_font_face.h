#ifndef PLUGIN_LAYOUT_FONT_SYS_FACE_H
#define PLUGIN_LAYOUT_FONT_SYS_FACE_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_font_face_t
plugin_layout_font_create(plugin_layout_module_t module, plugin_layout_font_id_t font_id);
void plugin_layout_font_face_free(plugin_layout_font_face_t face);

plugin_layout_font_face_t
plugin_layout_font_face_find(plugin_layout_module_t module, plugin_layout_font_id_t font_id);

plugin_layout_font_face_t
plugin_layout_font_face_check_create(plugin_layout_module_t module, plugin_layout_font_id_t font_id);

void * plugin_layout_font_face_data(plugin_layout_font_face_t face);
plugin_layout_font_face_t plugin_layout_font_face_from_data(void * data);

#ifdef __cplusplus
}
#endif

#endif
