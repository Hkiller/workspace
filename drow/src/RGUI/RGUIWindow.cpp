#include "render/model/ui_data_src.h"
#include "plugin/package/plugin_package_queue.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_load_task.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_page_eh.h"
#include "plugin/ui/plugin_ui_page_meta.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "gdpp/app/Log.hpp"
#include "RGUIWindow.h"
#include "RGUIPopup.hpp"
#include "RGUIPkgLoadTask.hpp"

RGUIWindow::RGUIWindow(void) {
}

RGUIWindow::~RGUIWindow( void ) {
}

void RGUIWindow::destory(void) {
    plugin_ui_page_free(page());
}

void RGUIWindow::onPageUpdate(float delta) {
}

void RGUIWindow::onPageChanged(void) {
}

void RGUIWindow::onPageHide(void) {
}

void RGUIWindow::onPageLoaded(void) {
}

void RGUIWindow::call_on_update(plugin_ui_page_t page, float delta) {
    try {
        cast(page)->onPageUpdate(delta);
    }
    catch(...) {
    }
}

void RGUIWindow::call_on_changed(plugin_ui_page_t page) {
    try {
        cast(page)->onPageChanged();
    }
    catch(...) {
    }
}

void RGUIWindow::call_on_hide(plugin_ui_page_t page) {
    try {
        RGUIWindow & window = *cast(page);
        plugin_package_load_task_free_by_ctx(window.package_mgr(), page);
        window.onPageHide();
    }
    catch(...) {
    }
}

void RGUIWindow::call_on_loaded(plugin_ui_page_t page) {
    try {
        cast(page)->onPageLoaded();
    }
    catch(...) {
    }
}

RGUIWindow * RGUIWindow::create_page(plugin_ui_env_t env, const char * page_name, const char * module_name) {
    plugin_ui_page_meta_t page_meta = NULL;

    if (module_name) {
        page_meta = plugin_ui_page_meta_find(plugin_ui_env_module(env), module_name);
        if (page_meta == NULL) {
            APP_CTX_THROW_EXCEPTION(
                plugin_ui_env_app(env), ::std::runtime_error,
                "RGUIWindow: create_page: page type %s meta not exist!", module_name);
        }
    }
    
    plugin_ui_page_t page = plugin_ui_page_create(env, page_name, page_meta);
    if (page == NULL) return NULL;

    return cast(page);
}

void RGUIWindow::addEventHandler(const char * event, plugin_ui_page_eh_scope_t scope, plugin_ui_page_eh_fun_t fun, void * ctx) {
    if (plugin_ui_page_eh_create(page(), event, scope, fun, ctx) == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIWindow %s: addEventHandler: add event handler fail!", name());
    }
}

Drow::Popup & RGUIWindow::create_popup(const char * def) {
    Drow::Popup & popup = Drow::Popup::create(ui_env(), def);

    plugin_ui_popup_set_create_from_page(popup, page());
    
    return popup;
}

void RGUIWindow::close_popups(void) {
    plugin_ui_popup_close_from_page(page());
}

void RGUIWindow::close_popups(LPDRMETA data_meta) {
    plugin_ui_popup_close_from_page_by_data_meta(page(), data_meta);
}

void RGUIWindow::close_popups(const char * popup_name) {
    plugin_ui_popup_close_from_page_by_name(page(), popup_name);
}

Drow::Popup & RGUIWindow::owner_popup(void) {
    plugin_ui_popup_t popup = plugin_ui_page_visible_in_popup(page());
    if (popup == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIWindow %s: is not in popup", name());
    }

    return Drow::Popup::cast(popup);
}

Drow::Slot & RGUIWindow::slot(const char * slot_name) {
    Drow::Slot * slot = findSlot(slot_name);
    if (slot == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "RGUIWindow %s: slot %s not exist", name(), slot_name);
    }
    return *slot;
}

plugin_ui_phase_node_t RGUIWindow::cur_phase(void) {
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(ui_env());
    assert(cur_phase);

    return cur_phase;
}

plugin_ui_state_node_t RGUIWindow::cur_state(void) {
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(ui_env());
    assert(cur_phase);
        
    plugin_ui_state_node_t cur_state = plugin_ui_state_node_current(cur_phase);
    assert(cur_state);

    return cur_state;
}

plugin_ui_state_node_t RGUIWindow::page_top_state_node(void) {
    return plugin_ui_page_top_state_node(page());
}

const char * RGUIWindow::page_top_state_name(void) {
    plugin_ui_state_node_t state_node = page_top_state_node();
    return state_node ? plugin_ui_state_node_name(state_node) : "";
}

plugin_ui_aspect_t RGUIWindow::cur_runtime_aspect(void) {
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(ui_env());
    assert(cur_phase);

    plugin_ui_aspect_t aspect = plugin_ui_phase_node_runtime_aspect(cur_phase);
    assert(aspect);

    return aspect;
}

const char * RGUIWindow::cur_state_name(void) {
    return plugin_ui_state_node_name(cur_state());
}

bool RGUIWindow::is_state_runing(const char * state_name) {
    plugin_ui_phase_node_t cur_phase = plugin_ui_phase_node_current(ui_env());
    assert(cur_phase);
        
    return plugin_ui_state_node_find_by_process(cur_phase, state_name) ? true : false;
}

plugin_package_module_t RGUIWindow::package_mgr(void) const {
    plugin_package_module_t package_module = plugin_ui_env_package_mgr(ui_env());
    assert(package_module);
    return package_module;
}

void RGUIWindow::package_gc(void) {
    plugin_package_module_gc(package_mgr());
}

void RGUIWindow::package_add(const char * package_name) {
    if (plugin_ui_page_pacakge_add(page(), package_name) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: add package %s fail", name(), package_name);
    }
}

void RGUIWindow::package_load_all_async(plugin_package_load_task_t task) {
    if (plugin_ui_page_pacakge_load_all_async(page(), task) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: load all package async", name());
    }
}

void RGUIWindow::package_load_all_sync(void) {
    if (plugin_ui_page_pacakge_load_all_sync(page()) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: load all package sync", name());
    }
}

void RGUIWindow::package_load_in_queue_async(const char * package_name, const char * queue_name, plugin_package_load_task_t task) {
    if (plugin_ui_env_load_package_to_queue_async(ui_env(), package_name, queue_name, task) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: load package %s in queue %s async fail", name(), package_name, queue_name);
    }
}

void RGUIWindow::package_load_in_scope_async(const char * package_name, plugin_ui_package_scope_t scope, plugin_package_load_task_t task) {
    if (plugin_ui_env_load_package_async(ui_env(), package_name, scope, task) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: load package %s to scope %d async fail", name(), package_name, scope);
    }   
}

uint16_t RGUIWindow::package_total_download_complete_count(void) const {
    return plugin_package_module_total_download_complete_count(package_mgr());
}

uint16_t RGUIWindow::package_total_download_count(void) const {
    return plugin_package_module_total_download_count(package_mgr());
}

uint16_t RGUIWindow::package_total_load_complete_count(void) const {
    return plugin_package_module_total_load_complete_count(package_mgr());
}

uint16_t RGUIWindow::package_total_load_count(void) const {
    return plugin_package_module_total_load_count(package_mgr());
}

struct RGUIWindowPkgLoadTaskCtx {
    Drow::PkgLoadTask::on_progress_fun_t m_on_progress;
    void * ctx;
};

static void process_package_load(void * ctx, plugin_package_load_task_t task, plugin_package_package_t package, float progress) {
    RGUIWindow & window = *RGUIWindow::cast((plugin_ui_page_t)ctx);
    RGUIWindowPkgLoadTaskCtx * task_ctx = (RGUIWindowPkgLoadTaskCtx*)plugin_package_load_task_carry_data(task, sizeof(RGUIWindowPkgLoadTaskCtx));
    assert(task_ctx);

    if (package == NULL) { /*进度上报 */
        if (task_ctx->m_on_progress) {
            try {
                (window.*task_ctx->m_on_progress)(*(Drow::PkgLoadTask*)task, progress);
            }
            APP_CTX_CATCH_EXCEPTION(window.app(), "%s: package load: ", window.name());
        }
    }
}

Drow::PkgLoadTask & RGUIWindow::package_create_load_task(void) {
    plugin_package_load_task_t load_task = plugin_package_load_task_create(package_mgr(), page(), process_package_load, sizeof(RGUIWindowPkgLoadTaskCtx));
    if (load_task == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: create package load task fail", name());
    }
    return *(Drow::PkgLoadTask*)load_task;
}

Drow::PkgLoadTask & Drow::PkgLoadTask::onProgress(on_progress_fun_t fun) {
    RGUIWindowPkgLoadTaskCtx * task_ctx = (RGUIWindowPkgLoadTaskCtx*)plugin_package_load_task_carry_data(*this, sizeof(RGUIWindowPkgLoadTaskCtx));
    task_ctx->m_on_progress = fun;
    return *this;
}
