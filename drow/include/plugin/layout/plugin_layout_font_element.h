#ifndef PLUGIN_LAYOUT_FONT_ELEMENT_H
#define PLUGIN_LAYOUT_FONT_ELEMENT_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_font_element_t
plugin_layout_font_element_create(plugin_layout_font_face_t face, uint32_t codeindex);
void plugin_layout_font_element_free(plugin_layout_font_element_t element);

plugin_layout_font_element_t
plugin_layout_font_element_find(plugin_layout_font_face_t face, uint32_t charter);

plugin_layout_font_element_t
plugin_layout_font_element_check_create(plugin_layout_font_face_t face, uint32_t charter);

void * plugin_layout_font_element_data(plugin_layout_font_element_t face);

void plugin_layout_font_element_clear_not_used(plugin_layout_module_t module);

#ifdef __cplusplus
}
#endif

#endif
