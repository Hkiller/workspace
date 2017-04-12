#ifndef DROW_RGUI_WINDOW_H_INCLEDED
#define DROW_RGUI_WINDOW_H_INCLEDED
#include <memory>
#include "cpepp/dr/Meta.hpp"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_page_meta.h"
#include "plugin/ui/plugin_ui_page_slot.h"
#include "RGUIPanel.h"

class RGUIWindow : public RGUIPanel {
public:
	RGUIWindow(void);

	bool WasModal(void) const { return plugin_ui_page_modal(page()) ? true : false; }

    virtual void onPageUpdate(float delta);
    virtual void onPageChanged(void);
    virtual void onPageHide(void);
    virtual void onPageLoaded(void);
    
    plugin_ui_page_t page(void) const { return plugin_ui_page_from_product((void*)this); }
    plugin_ui_page_meta_t page_meta(void) const { return plugin_ui_page_meta(page()); }
    const char * name(void) const { return plugin_ui_page_name(page()); }

    void setChanged(bool changed) { plugin_ui_page_set_changed(page(), changed ? 1 : 0); }
    void destory(void);

    /*state */
    plugin_ui_phase_node_t cur_phase(void);
    plugin_ui_state_node_t cur_state(void);
    plugin_ui_state_node_t page_top_state_node(void);
    plugin_ui_aspect_t cur_runtime_aspect(void);

    const char * page_top_state_name(void);
    const char * cur_state_name(void);
    bool is_state_runing(const char * state_name);

    /*package */
    plugin_package_module_t package_mgr(void) const;
    void package_gc(void);
    void package_add(const char * package_name);
    void package_clear_all(void) { plugin_ui_page_pacakge_remove_all(page()); }
    void package_load_all_async(plugin_package_load_task_t task = NULL);
    void package_load_all_sync(void);
    void package_load_in_queue_async(const char * package_name, const char * queue_name, plugin_package_load_task_t task = NULL);
    void package_load_in_scope_async(const char * package_name, plugin_ui_package_scope_t scope, plugin_package_load_task_t task = NULL);
    uint16_t package_total_download_complete_count(void) const;
    uint16_t package_total_download_count(void) const;
    uint16_t package_total_load_complete_count(void) const;
    uint16_t package_total_load_count(void) const;
    Drow::PkgLoadTask & package_create_load_task(void);
    
    /*popup*/
    Drow::Popup & create_popup(const char * def);
    void close_popups(void);
    void close_popups(LPDRMETA data_meta);
    void close_popups(const char * popup_name);
    bool have_popup(void) const { return plugin_ui_page_have_popup(page()) ? true : false; }
    bool have_popup(const char * popup_name) const { return plugin_ui_page_find_popup_by_name(page(), popup_name) ? true : false; }
    Drow::Popup & owner_popup(void);

    template<typename T>
    void close_popups(void) { close_popups(Cpe::Dr::MetaTraits<T>::META); }

    /*Slot*/
    Drow::Slot & slot(const char * name);
    Drow::Slot * findSlot(const char * name) { return (Drow::Slot*)plugin_ui_page_slot_find(page(), name); }
    
    /*Page data*/
    LPDRMETA pageDataMeta(void) const { return plugin_ui_page_data_meta(page()); }
    void * pageData(void) { return plugin_ui_page_data(page()); }
    void const * pageData(void) const { return plugin_ui_page_data(page()); }
    uint32_t pageDataSize(void) const { return plugin_ui_page_data_size(page()); }

    template<typename T>
    void setPageData(T & data) {
        setPageData(Cpe::Dr::MetaTraits<T>::META, &data, (uint32_t)Cpe::Dr::MetaTraits<T>::data_size(data));
    }
    void setPageData(LPDRMETA meta, void * data, uint32_t data_size) {
        plugin_ui_page_set_data(page(), meta, data, data_size);
    }

    /*event */
    void sendEvent(LPDRMETA meta, void * data, size_t data_size) {
        plugin_ui_page_send_event(page(), meta, data, data_size);
    }
    
    void sendEvent(const char * def, dr_data_source_t data_source = NULL) {
        plugin_ui_page_build_and_send_event(page(), def, data_source);
    }

    template<typename T>
    void sendEvent(T const & data) {
		T d = data;
        this->sendEvent(Cpe::Dr::MetaTraits<T>::META, &d, Cpe::Dr::MetaTraits<T>::data_size(d));
    }

	template<typename T>
	void sendEvent(T & data) {
		this->sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
	}

    template<typename T>
    void sendEvent(void) {
        T data;
        this->sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    void addEventHandler(const char * event, plugin_ui_page_eh_scope_t scope, plugin_ui_page_eh_fun_t fun, void * ctx);
    
    using RGUIPanel::cast;
    static RGUIWindow * cast(plugin_ui_page_t page) { return (RGUIWindow*)plugin_ui_page_product(page); }

    
    /**/
    static RGUIWindow * create_page(plugin_ui_env_t env, const char * page_name, const char * module_name);
    
    template<typename T>
    static T * create_page(plugin_ui_env_t env, const char * page_name = "", const char * module_name = T::NAME) {
        return dynamic_cast<T*>(create_page(env, page_name, module_name));
    }
    
    template<typename T>
    static int page_init(plugin_ui_page_t page) {
        try {
            assert(plugin_ui_page_product_capacity(page) >= sizeof(T));
            new (plugin_ui_page_product(page)) T();
            return 0;
        } catch(...) {
            return -1;
        }
    }

    template<typename T>
    static void page_fini(plugin_ui_page_t page) {
        ((T *)(plugin_ui_page_product(page)))->~T();
    }

    template<typename T>
    static int page_install(plugin_ui_module_t ui_module, const char * module_name = T::NAME) {
        plugin_ui_page_meta_t meta =plugin_ui_page_meta_find(ui_module, module_name);
        if (meta == NULL) {
            meta = plugin_ui_page_meta_create(ui_module, module_name, sizeof(T), page_init<T>, page_fini<T>);
            if (meta == NULL) return -1;
        }
        
        plugin_ui_page_meta_set_on_changed(meta, call_on_changed);
        plugin_ui_page_meta_set_on_hide(meta, call_on_hide);
        plugin_ui_page_meta_set_on_load(meta, call_on_loaded);
        plugin_ui_page_meta_set_on_update(meta, call_on_update);
        return 0;
    }

    template<typename T>
    static void page_uninstall(plugin_ui_module_t ui_module, const char * module_name = T::NAME) {
        plugin_ui_page_meta_t meta = plugin_ui_page_meta_find(ui_module, module_name);
        assert(meta);
        plugin_ui_page_meta_free(meta);
    }

protected:
	virtual ~RGUIWindow( void );

private:
    static void call_on_update(plugin_ui_page_t page, float delta);
    static void call_on_changed(plugin_ui_page_t page);
    static void call_on_hide(plugin_ui_page_t page);
    static void call_on_loaded(plugin_ui_page_t page);
};


#define UI_PAGE_DEF(__module_name, __module_impl)                       \
extern "C"                                                              \
EXPORT_DIRECTIVE                                                        \
int __module_name ## _app_init(                                         \
    gd_app_context_t app, gd_app_module_t module,                       \
    cfg_t moduleCfg)                                                    \
{                                                                       \
    plugin_ui_module_t ui_module = plugin_ui_module_find_nc(app, NULL); \
    return RGUIWindow::page_install<__module_impl>(ui_module, #__module_name); \
}                                                                       \
                                                                        \
extern "C"                                                              \
EXPORT_DIRECTIVE                                                        \
void __module_name ## _app_fini(                                        \
    gd_app_context_t app, gd_app_module_t module)                       \
{                                                                       \
    plugin_ui_module_t ui_module = plugin_ui_module_find_nc(app, NULL); \
    assert(ui_module);                                                  \
    RGUIWindow::page_uninstall<__module_impl> (ui_module, #__module_name); \
}                                                                       \


#endif
