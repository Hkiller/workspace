#ifndef PLUGIN_UI_ANIM_FRAME_MOVE_I_H
#define PLUGIN_UI_ANIM_FRAME_MOVE_I_H
#include "render/utils/ui_percent_decorator.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_frame_move.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum plugin_ui_anim_frame_move_complete_op {
    plugin_ui_anim_frame_move_complete_noop,
    plugin_ui_anim_frame_move_complete_remove,
};
    
struct plugin_ui_anim_frame_move {
    /*config*/
	struct ui_percent_decorator m_percent_decorator;
    plugin_ui_move_algorithm_t m_algorithm;
    float m_duration;
    enum plugin_ui_anim_frame_move_complete_op m_complete_op;
    struct ui_vector_2 m_origin;
    uint8_t m_update_target;
    char * m_cfg_target;

    /*runtime*/
    ui_vector_2 m_target;
    float m_runing_time;
};

int plugin_ui_anim_frame_move_regist(plugin_ui_module_t module);
void plugin_ui_anim_frame_move_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
