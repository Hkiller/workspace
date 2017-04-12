#ifndef DROW_UI_CONTROL_BUTTON_H
#define DROW_UI_CONTROL_BUTTON_H
#include "RGUILabel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIButton, ui_control_type_button);

class RGUIButton : public RGUILabel {
public:
	RGUIButton(const char * layout = "basic");

	plugin_ui_control_frame_t SetDownFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL)
    {
        return SetFrame(render_obj_ref, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, pos_policy, aspect);
    }
	plugin_ui_control_frame_t SetDownFrame(const char * res, plugin_ui_aspect_t aspect = NULL) {
        return SetFrame(res, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, aspect);
    }
	plugin_ui_control_frame_t AddDownFrame(
        ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL)
    {
        return AddFrame(render_obj_ref, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, pos_policy, aspect);
    }
	plugin_ui_control_frame_t AddDownFrame(const char * res, plugin_ui_aspect_t aspect = NULL) {
        return AddFrame(res, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, aspect);
    }
	void ClearDownFrame(plugin_ui_aspect_t aspect = NULL) {
        return ClearFrame(plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, aspect);
    }

	bool SetBackAndDownFrame(ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL);
	bool SetBackAndDownFrame(const char * res, plugin_ui_aspect_t aspect = NULL);
	bool AddBackAndDownFrame(ui_runtime_render_obj_ref_t render_obj_ref, uint8_t pos_policy = ui_pos_policy_center, plugin_ui_aspect_t aspect = NULL);
	bool AddBackAndDownFrame(const char * res, plugin_ui_aspect_t aspect = NULL);

    struct plugin_ui_cfg_button const & button_cfg(void) const;

    void SetDownPosAdj(const ui_vector_2 &  pos_adj);
    void SetDownScaleAdj(const ui_vector_2 &  scale_adj);

    virtual void Load(ui_data_control_t control);
            void Load(UI_CONTROL_DOWN const & data);

protected:
    struct plugin_ui_cfg_button & check_create_button_cfg(void);
    void startRiseAnim(void);
    void startDownAnim(void);
    void createDefaultDownFrames(void);
    
    static int down_res_setter(plugin_ui_control_t control, dr_value_t value);
    static int add_down_res_setter(plugin_ui_control_t control, dr_value_t value);
    static int back_down_res_setter(plugin_ui_control_t control, dr_value_t value);
    static int add_back_down_res_setter(plugin_ui_control_t control, dr_value_t value);
    static void on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

    plugin_ui_control_frame_t m_down_text;
    
protected:
    plugin_ui_cfg_button_t m_cfg;
    ~RGUIButton();

    friend class RGUIControlRepo; 
};

#endif
