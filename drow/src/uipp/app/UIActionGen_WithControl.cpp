#include "gdpp/app/Log.hpp"
#include "ui/sprite_ui/ui_sprite_ui_module.h"
#include "ui/sprite_ui/ui_sprite_ui_env.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "UIActionGen_WithControl.hpp"
#include "UICenterExt.hpp"
#include "EnvExt.hpp"

namespace UI { namespace App {

void UIActionGen_WithControl_Base::setControlName(const char * control_name) { 
    m_control_name = control_name;
    * cpe_str_trim_tail(&m_control_name[0] + m_control_name.size(), &m_control_name[0]) = 0;
}

plugin_ui_page_t
UIActionGen_WithControl_Base::findPage(const char * control_name, const char * * name_start, mem_buffer_t input_buffer) {
    struct mem_buffer local_buffer;
    mem_buffer_t buffer = NULL;
    
    if (control_name == NULL) control_name = m_control_name.c_str();

    if (control_name[0] == ':') {
        if (input_buffer) {
            buffer = input_buffer;
        }
        else {
            mem_buffer_init(&local_buffer, NULL);
            buffer = &local_buffer;
        }
        
        const char * r = (char*)ui_sprite_fsm_action_calc_str_with_dft(buffer, control_name + 1, action(), NULL, NULL);
        if (r == NULL) {
            APP_CTX_ERROR(
                action().world().app(), "entity %d(%s): %s: calc '%s' error!",
                action().entity().id(), action().entity().name(), action().name(), control_name + 1);
            if (buffer == &local_buffer) mem_buffer_clear(buffer);
            return NULL;
        }

        control_name = r;
    }

    char page_name[64];
    const char * p = strchr(control_name, '.');
    if (p == NULL) {
        APP_CTX_ERROR(
            action().world().app(), "entity %d(%s): %s: control '%s' format error!",
            action().entity().id(), action().entity().name(), action().name(), control_name);
        if (buffer == &local_buffer) mem_buffer_clear(buffer);
        return NULL;
    }

    if (name_start) *name_start = p + 1;
    
    size_t len = p - control_name;
    memcpy(page_name, control_name, len);
    page_name[len] = 0;

    ui_sprite_ui_module_t ui_module = ui_sprite_ui_module_find_nc(action().world().app(), NULL);
    if (ui_module == NULL) {
        APP_CTX_ERROR(action().world().app(), "entity %d(%s): %s: no ui module!", action().entity().id(), action().entity().name(), action().name());
        return NULL;
    }

    plugin_ui_page_t page = ui_sprite_ui_find_page_from_action(ui_module, action(), page_name);
    if (page == NULL) {
        APP_CTX_ERROR(
            action().world().app(), "entity %d(%s): %s: page %s not exist!",
            action().entity().id(), action().entity().name(), action().name(), page_name);
        if (buffer == &local_buffer) mem_buffer_clear(buffer);
        return NULL;
    }

    if (buffer == &local_buffer) mem_buffer_clear(buffer);
    
    return page;
}

RGUIControl *
UIActionGen_WithControl_Base::findTargetControl(const char * control_name) {
    struct mem_buffer name_buffer;
    mem_buffer_init(&name_buffer, NULL);
    
    const char * control_part_start = NULL;
    plugin_ui_page_t page = findPage(control_name, &control_part_start, &name_buffer);

    if (page == NULL || control_part_start == NULL) {
        mem_buffer_clear(&name_buffer);
        return NULL;
    }

    struct mem_buffer control_buffer;
    uint8_t is_calc = 0;

    if (control_part_start[0] == ':') {
        mem_buffer_init(&control_buffer, NULL);
        is_calc = 1;

        dr_data_source_t data_source = NULL;
        dr_data_source data_source_buf;
        if (plugin_ui_page_data(page)) {
            data_source_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
            data_source_buf.m_data.m_data = plugin_ui_page_data(page);
            data_source_buf.m_data.m_size = plugin_ui_page_data_size(page);
            data_source_buf.m_next = data_source;
            data_source = &data_source_buf;
        }
        
        const char * r = (char*)ui_sprite_fsm_action_calc_str_with_dft(&control_buffer, control_part_start + 1, action(), data_source, NULL);
        if (r == NULL) {
            APP_CTX_ERROR(
                action().world().app(), "entity %d(%s): %s: calc '%s' error!",
                action().entity().id(), action().entity().name(), action().name(), control_part_start + 1);
            mem_buffer_clear(&name_buffer);
            mem_buffer_clear(&control_buffer);
            return NULL;
        }

        control_part_start = r;
    }
    
    RGUIControl * control = RGUIControl::cast(plugin_ui_control_find_child_by_path(plugin_ui_page_root_control(page), control_part_start));
    if (control == NULL) {
        APP_CTX_ERROR(
            action().world().app(), "entity %d(%s): %s: page %s: control %s not exist!",
            action().entity().id(), action().entity().name(), action().name(), plugin_ui_page_name(page), control_part_start);
        mem_buffer_clear(&name_buffer);
        if (is_calc) mem_buffer_clear(&control_buffer);
        return NULL;
    }

    mem_buffer_clear(&name_buffer);
    if (is_calc) mem_buffer_clear(&control_buffer);
    return control;
}

}}
