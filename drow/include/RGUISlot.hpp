#ifndef DROW_RGUI_SLOT_H_INCLEDED
#define DROW_RGUI_SLOT_H_INCLEDED
#include "cpepp/utils/ClassCategory.hpp"
#include "plugin/ui/plugin_ui_types.h"
#include "plugin/ui/plugin_ui_page_slot.h"
#include "RGUI.h"

namespace Drow {

class Slot : public Cpe::Utils::SimulateObject {
public:
    operator plugin_ui_page_slot_t () const { return (plugin_ui_page_slot_t)this; }

    const char * name(void) const { return plugin_ui_page_slot_name(*this); }
    Slot & operator=(const char * value);
};

}

#endif
