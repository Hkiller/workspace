#ifndef PLUGIN_UI_ASPECT_REF_I_H
#define PLUGIN_UI_ASPECT_REF_I_H
#include "plugin_ui_aspect_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_aspect_ref {
    plugin_ui_aspect_t m_aspect;
    TAILQ_ENTRY(plugin_ui_aspect_ref) m_next_for_aspect;
    void * m_obj;
    TAILQ_ENTRY(plugin_ui_aspect_ref) m_next_for_obj;
    uint8_t m_is_tie;
};

plugin_ui_aspect_ref_t
plugin_ui_aspect_ref_create(
    plugin_ui_aspect_t aspect, plugin_ui_aspect_ref_list_t * list_on_aspect,
    void * obj, plugin_ui_aspect_ref_list_t * list_on_obj,
    uint8_t is_tie);

void plugin_ui_aspect_ref_free(
    plugin_ui_aspect_ref_t ref,
    plugin_ui_aspect_ref_list_t * list_on_aspect,
    plugin_ui_aspect_ref_list_t * list_on_obj);
    
void plugin_ui_aspect_ref_real_free(plugin_ui_aspect_ref_t aspect_ref);

#ifdef __cplusplus
}
#endif

#endif
