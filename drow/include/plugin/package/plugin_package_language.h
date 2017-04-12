#ifndef UI_PLUGIN_PACKAGE_LANGUAGE_H
#define UI_PLUGIN_PACKAGE_LANGUAGE_H
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_package_language_t plugin_package_language_create(plugin_package_package_t package, ui_data_language_t data_language);
void plugin_package_language_free(plugin_package_language_t language);

plugin_package_language_t
plugin_package_language_find(plugin_package_package_t package, ui_data_language_t data_language);
    
void plugin_package_package_languages(plugin_package_language_it_t language_it, plugin_package_package_t package);

/*it*/
struct plugin_package_language_it {
    plugin_package_language_t (*next)(plugin_package_language_it_t it);
    char m_data[64];
};

#define plugin_package_language_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

