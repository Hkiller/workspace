#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "RGUIRichText.h"
#include "plugin/layout/plugin_layout_font_info.h"
#include "plugin/layout/plugin_layout_layout_rich_block.h"
#include "plugin/ui/plugin_ui_control_attr_meta.h"

RGUIRichText::RGUIRichText(const char * layout)
    : RGUILabel(layout)
{
}

void RGUIRichText::Load( ui_data_control_t control ) {
    RGUILabel::Load(control);

    if (type() == ui_control_type_rich_text) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.label.text);

        plugin_layout_layout_rich_t rich = layout_rich(m_text_frame);
        if (rich) {
            ui_data_control_addition_it addition_it;
            ui_data_control_additions(&addition_it, control);
            while(ui_data_control_addition_t addition = ui_data_control_addition_it_next(&addition_it)) {
                UI_CONTROL_ADDITION const * addition_data = ui_data_control_addition_data(addition);
                if (addition_data->type != ui_control_addition_type_rich_element) continue;

                plugin_layout_layout_rich_block_t block = plugin_layout_layout_rich_block_create(rich);
                if (block == NULL) break;

                plugin_layout_layout_rich_block_set_context(block, ui_data_control_msg(control, addition_data->data.rich_element.text_id), 1);

                plugin_layout_font_draw d;
                cvt_font_draw(d, addition_data->data.rich_element.text_drow);
                plugin_layout_layout_rich_block_set_font_draw(block, &d);
            }
        }
    }
}

RGUIRichText::~RGUIRichText( void ) {
}
