#include "render/utils/ui_rect.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_animation_meta.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_env.h"
#include "RGUIPanel.h"
#include "RGUIPicture.h"
#include "RGUILabel.h"
#include "RGUIButton.h"
#include "RGUIToggle.h"
#include "RGUIProgressBar.h"
#include "RGUIPictureCondition.h"
#include "RGUICheckBox.h"
#include "RGUIRadioBox.h"
#include "RGUIComboBox.h"
#include "RGUIEditBox.h"
#include "RGUIMultiEditBox.h"
#include "RGUILabelCondition.h"
#include "RGUIRichLabel.h"
#include "RGUIRichText.h"
#include "RGUIListBoxCol.h"
#include "RGUIListBoxRow.h"
#include "RGUIListBoxAdvItem.h"
#include "RGUIScrollPanel.h"
#include "RGUISlider.h"
#include "RGUISwitch.h"
#include "RGUISwiper.h"
#include "RGUITab.h"
#include "RGUITabPage.h"
#include "RGUIComboBoxDropList.h"
#include "RGUILib.h"

class RGUIControlRepo {
public:
    template<typename T>
    static int control_init(plugin_ui_control_t control) {
        try {
            assert(plugin_ui_control_product_capacity(control) >= sizeof(T));
            new (plugin_ui_control_product(control)) T();
            return 0;
        }
        catch(...) {
            return -1;
        }
    }

    template<typename T>
    static void control_fini(plugin_ui_control_t control) {
        ((T*)(plugin_ui_control_product(control)))->~T();
    }

    template<typename T>
    static int control_load(plugin_ui_control_t control) {
        try {
            T* c = (T*)plugin_ui_control_product(control);
            ui_data_control_t src = plugin_ui_control_template(control);
            if (src == NULL) src = plugin_ui_control_src(control);
            assert(src);
            c->T::Load(src);
            return 0;
        }
        catch(...) {
            return -1;
        }
    }

    template<typename T>
    static void control_update(plugin_ui_control_t control, float delta) {
        ((T*)(plugin_ui_control_product(control)))->T::Update(delta);
    }

    template<typename T>
    static void control_on_self_loaded(plugin_ui_control_t control) {
        T * c = (T*)plugin_ui_control_product(control);
        c->T::OnLoadProperty();
    }

    template<typename T>
    static void registerControl(plugin_ui_module_t ui_module) {
        plugin_ui_control_meta_t meta =
            plugin_ui_control_meta_create(
                ui_module,
                RGUIControlTraits<T>::TYPE_ID,
                sizeof(T),
                control_init<T>,
                control_fini<T>,
                control_load<T>,
                control_update<T>);
        assert(meta);

        plugin_ui_control_meta_set_on_self_loaded(meta, &control_on_self_loaded<T>);

        T::setup(meta);
    }

    template<typename T>
    static void unregisterControl(plugin_ui_module_t ui_module) {
        plugin_ui_module_unregister_control(ui_module, RGUIControlTraits<T>::TYPE_ID);
    }
};

void RGUILib::Init(plugin_ui_env_t env) {
    plugin_ui_module_t ui_module = plugin_ui_env_module(env);
    RGUIControlRepo::registerControl<RGUIPanel>(ui_module);
    RGUIControlRepo::registerControl<RGUIPicture>(ui_module);
    RGUIControlRepo::registerControl<RGUILabel>(ui_module);
    RGUIControlRepo::registerControl<RGUIButton>(ui_module);
    RGUIControlRepo::registerControl<RGUIToggle>(ui_module);
    RGUIControlRepo::registerControl<RGUIProgressBar>(ui_module);
    RGUIControlRepo::registerControl<RGUIPictureCondition>(ui_module);
    RGUIControlRepo::registerControl<RGUICheckBox>(ui_module);
    RGUIControlRepo::registerControl<RGUIRadioBox>(ui_module);
    RGUIControlRepo::registerControl<RGUIComboBox>(ui_module);
    RGUIControlRepo::registerControl<RGUIEditBox>(ui_module);
    RGUIControlRepo::registerControl<RGUIMultiEditBox>(ui_module);
    RGUIControlRepo::registerControl<RGUILabelCondition>(ui_module);
    RGUIControlRepo::registerControl<RGUIRichLabel>(ui_module);
    RGUIControlRepo::registerControl<RGUIRichText>(ui_module);
    RGUIControlRepo::registerControl<RGUIListBoxCol>(ui_module);
    RGUIControlRepo::registerControl<RGUIListBoxRow>(ui_module);
    RGUIControlRepo::registerControl<RGUIListBoxAdvItem>(ui_module);
    RGUIControlRepo::registerControl<RGUIScrollPanel>(ui_module);
    RGUIControlRepo::registerControl<RGUISlider>(ui_module);
    RGUIControlRepo::registerControl<RGUISwitch>(ui_module);
    RGUIControlRepo::registerControl<RGUITab>(ui_module);
    RGUIControlRepo::registerControl<RGUITabPage>(ui_module);
    RGUIControlRepo::registerControl<RGUISwiper>(ui_module);
    RGUIControlRepo::registerControl<RGUIComboBoxDropList>(ui_module);
}

void RGUILib::ShutDown(plugin_ui_env_t env) {
    plugin_ui_module_t ui_module = plugin_ui_env_module(env);
    
    RGUIControlRepo::unregisterControl<RGUIComboBoxDropList>(ui_module);
    RGUIControlRepo::unregisterControl<RGUISwiper>(ui_module);
    RGUIControlRepo::unregisterControl<RGUITabPage>(ui_module);
    RGUIControlRepo::unregisterControl<RGUITab>(ui_module);
    RGUIControlRepo::unregisterControl<RGUISwitch>(ui_module);
    RGUIControlRepo::unregisterControl<RGUISlider>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIScrollPanel>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIListBoxAdvItem>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIListBoxRow>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIListBoxCol>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIRichText>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIRichLabel>(ui_module);
    RGUIControlRepo::unregisterControl<RGUILabelCondition>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIMultiEditBox>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIEditBox>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIComboBox>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIRadioBox>(ui_module);
    RGUIControlRepo::unregisterControl<RGUICheckBox>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIPictureCondition>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIProgressBar>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIToggle>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIButton>(ui_module);
    RGUIControlRepo::unregisterControl<RGUILabel>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIPicture>(ui_module);
    RGUIControlRepo::unregisterControl<RGUIPanel>(ui_module);
}
