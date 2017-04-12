#include "cpe/pal/pal_string.h"
#include "ui_proj_utils.h"

const char * ui_data_proj_postfix(ui_data_src_type_t type) {
    switch(type) {
    case ui_data_src_type_dir:
        return "";
    case ui_data_src_type_module:
        return "ibk";
    case ui_data_src_type_sprite:
        return "frm";
    case ui_data_src_type_action:
        return "act";
    case ui_data_src_type_layout:
        return "lay";
    default:
        return NULL;
    }
}

const char * ui_data_proj_control_tag_name(uint8_t control_type) {
    switch (control_type) {
    case ui_control_type_window:
        return "Window";
    case ui_control_type_panel:
        return "RGUIPanel";
    case ui_control_type_picture:
        return "RGUIPicture";
    case ui_control_type_label:
        return "RGUILabel";
    case ui_control_type_button:
        return "RGUIButton";
    case ui_control_type_toggle:
        return "RGUIToggle";
    case ui_control_type_progress:
        return "RGUIProgressBar";
    case ui_control_type_picture_cond:
        return "RGUIPictureCondition";
    case ui_control_type_check_box:
        return "RGUICheckBox";
    case ui_control_type_radio_box:
        return "RGUIRadioBox";
    case ui_control_type_combo_box:
        return "RGUIComboBox";
    case ui_control_type_edit_box:
        return "RGUIEditBox";
    case ui_control_type_list_box_adv_item:
        return "RGUIListBoxAdvItem";
    case ui_control_type_multi_edit_box:
        return "RGUIMultiEditBox";
    case ui_control_type_label_condition:
        return "RGUILabelCondition";
    case ui_control_type_rich_label:
        return "RGUIRichLabel";
    case ui_control_type_rich_text:
        return "RGUIRichText";
    case ui_control_type_list_box_col:
        return "RGUIListBoxCol";
    case ui_control_type_list_box_row:
        return "RGUIListBoxRow";
    case ui_control_type_scroll_panel:
        return "RGUIScrollPanel";
    case ui_control_type_slider:
        return "RGUISlider";
    case ui_control_type_switcher:
        return "RGUISwitch";
    case ui_control_type_tab:
        return "RGUITab";
    case ui_control_type_tab_page:
        return "RGUITabPage";
    case ui_control_type_swiper:
        return "RGUISwiper";
    case ui_control_type_comb_box_drop_list:
        return "RGUIComboBoxDropList";
    default:
        return NULL;
    }
}
