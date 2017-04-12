#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/time_utils.h"
#include "cpe/vfs/vfs_builder.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_load_task.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "gd/app/app_module.h"
#include "gd/app/app_module_installer.h"
#include "../EnvExt.hpp"
#include "../UICenterExt.hpp"
#include "../RuningExt.hpp"
#include "AppContainer.hpp"
#include "PkgManager.hpp"
#include "DownloadTask.hpp"

using namespace UI::App;

extern "C" {
    int plugin_app_env_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
    void plugin_app_env_module_app_fini(gd_app_context_t app, gd_app_module_t module);
}

typedef void (*mem_allocrator_free_t)(void * p, struct mem_allocrator * allocrator);

AppContainer::AppContainer()
    : m_app(NULL)
    , m_module_installer(NULL)
    , m_is_stage_ok(false)
    , m_is_download_ok(false)
    , m_pkg_mgr(NULL)
    , m_root_fs(NULL)
    , m_store_fs(NULL)
    , m_task(NULL)
{
    m_alloc.m_alloc = do_alloc;
    m_alloc.m_calloc = do_calloc;
    m_alloc.m_free = do_free;
    
    mem_buffer_init(&m_format_buffer, NULL);
    cpe_error_monitor_init(&m_em, log_to_trace, this);
}

AppContainer::~AppContainer() {
    if (m_task) {
        delete m_task;
        m_task = NULL;
    }

    if (m_pkg_mgr) m_pkg_mgr->uninstall();

    if (m_module_installer) {
        gd_app_module_installer_free(m_module_installer);
        m_module_installer = NULL;
    }
    
    if (m_app) {
        gd_app_context_free(m_app);
        m_app = NULL;
    }
    
    if (m_pkg_mgr) {
        delete m_pkg_mgr;
        m_pkg_mgr = NULL;
    }

    if (m_store_fs) {
        vfs_builder_free(m_store_fs);
        m_store_fs = NULL;
    }
    
    if (m_root_fs) {
        spack_rfs_free(m_root_fs);
        m_root_fs = NULL;
    }
}
    
int AppContainer::createApp(void) {
    assert(m_app == NULL);
    assert(m_pkg_mgr == NULL);

    m_app = gd_app_context_create_main(&m_alloc, 0, 0, 0);
    if (m_app == NULL) {
        CPE_ERROR(&m_em, "FlexApp: create app: create ctx fail!\n");
        return -1;
    }
    gd_app_set_em(m_app, &m_em);
    gd_app_set_debug(m_app, 1);
    gd_app_set_root(m_app, "/");
    gd_app_ins_set(m_app);

    if (gd_app_add_tag(m_app, "flex") != 0) {
        CPE_ERROR(&m_em, "FlexApp: create app: add tag fail!");
        gd_app_context_free(m_app);
        m_app = NULL;
        return -1;
    }

    if (gd_app_module_type_init(
            "plugin_app_env_module",
            plugin_app_env_module_app_init, plugin_app_env_module_app_fini, NULL, NULL, &m_em)
        != 0
        || gd_app_install_module(m_app, "plugin_app_env_module", "plugin_app_env_module", NULL, NULL)
        == NULL)
    {
        CPE_ERROR(&m_em, "FlexApp: create app: create app_env module fail!");
        gd_app_context_free(m_app);
        m_app = NULL;
        return -1;
    }

    m_root_fs = spack_rfs_create(gd_app_vfs_mgr(m_app), "/", gd_app_alloc(m_app), &m_em);
    m_pkg_mgr = PkgManager::create(m_app);
    m_task = new DownloadTask(*this, m_app, "base.spack");

    return 0;
}

bool AppContainer::downloadError(void) const {
    return m_task == NULL || m_task->state() == DownloadTask::Error;
}

uint32_t AppContainer::pkgLoadingCount(void) const {
    return m_pkg_mgr ? m_pkg_mgr->loadingPkgCount() : 0;
}

void AppContainer::resize(uint32_t width, uint32_t height) {
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    env.runing().init();
    
    ui_vector_2_t base_size = plugin_ui_env_origin_sz(env.uiCenter().uiEnv());
    if ((base_size->x > base_size->y && width < height) || (base_size->x < base_size->y && width > height)) {
        int32_t t = width; width = height ; height = t;
    }
    env.runing().setSize(width, height);
}

void AppContainer::reinit(void) {
    ui_runtime_module_t runtime_module = ui_runtime_module_find_nc(m_app, NULL);
    if (runtime_module) {
        ui_runtime_render_init(ui_runtime_module_render(runtime_module));
    }

    plugin_package_module_t package_module = plugin_package_module_find_nc(m_app, NULL);
    if (package_module) {
        plugin_package_module_reinstall(package_module);
    }
}

int AppContainer::startApp(void) {
    assert(m_app);
    assert(m_is_stage_ok);
    assert(m_is_download_ok);

    /*启动 */
    int64_t start_time = cur_time_ms();
    if (gd_app_cfg_reload(m_app) != 0) {
        APP_CTX_ERROR(m_app, "FlexApp: startApp: load cfg fail!");
        return -1;
    }
    APP_CTX_ERROR(m_app, "FlexApp: startApp: load cfg in %d ms!", (int)(cur_time_ms() - start_time));

    m_module_installer = gd_app_module_installer_create(m_app, cfg_find_cfg(gd_app_cfg(m_app), "modules.load"));
    if (m_module_installer == NULL) {
        APP_CTX_ERROR(m_app, "FlexApp: startApp: create installer fail!");
        return -1;
    }
    
    return 0;
}

int AppContainer::tickStartApp(void)  {
    if(m_module_installer == NULL) {
        APP_CTX_ERROR(m_app, "FlexApp: tickStartApp: is already done!");
        return -1;
    }

    int64_t start_time = cur_time_ms();
    uint16_t load_count = 0;
    uint16_t max_use_time = (1000 / 30) / 2; //默认保持帧率30，每次使用帧率的一半
    
    while(!gd_app_module_installer_is_done(m_module_installer)) {
        int rv = gd_app_module_installer_install_one(m_module_installer);
        if (rv != 0) return rv;
        ++load_count;
        
        if (cur_time_ms() - start_time >= max_use_time) {
            CPE_INFO(&m_em, "FlexApp: tickStartApp: load %d modules in %d ms", load_count, (int)(cur_time_ms() - start_time));
            return 0;
        }
    }

    gd_app_module_installer_free(m_module_installer);
    m_module_installer = NULL;
    
    /*安装包管理 */
    if (m_pkg_mgr->install() != 0) {
        CPE_ERROR(&m_em, "FlexApp: tickStartApp: pkg installer: install fail");
        return -1;
    }

    /*初始化Env */
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    env.runing().init();

    return 0;
}

void AppContainer::onRestart(DownloadTask & task) {
    spack_rfs_clear(m_root_fs);
}

void AppContainer::onComplete(DownloadTask & task) {
    if (task.state() == DownloadTask::Complete) {
        if (spack_rfs_append_complete(m_root_fs) != 0) {
            APP_CTX_ERROR(m_app, "FlexApp: on base pkg complete: build root fs fail!");
            task.setError();
            return;
        }
        
        m_is_download_ok = true;
        delete m_task;
        m_task = NULL;
    }
    else {
        task.setError();
    }
}

void AppContainer::onProgress(DownloadTask & task, int32_t bytesTotal, int32_t bytesLoaded) {
    mem_buffer_t buffer = gd_app_tmp_buffer(m_app);
    void * data = task.readData(buffer);
    if (data == NULL) {
        APP_CTX_ERROR(m_app, "FlexApp: onProgress: read data fail, size=%d!", (int)mem_buffer_size(buffer));
        task.setError();
        return;
    }

    if (spack_rfs_append_data(m_root_fs, data, mem_buffer_size(buffer)) != 0) {
        APP_CTX_ERROR(m_app, "FlexApp: onProgress: root fs append data fail");
        task.setError();
        return;
    }

    APP_CTX_INFO(
        m_app, "FlexApp: onProgress: root fs append %d data (%.2f/%.2f)",
        (int)mem_buffer_size(buffer), (float)bytesLoaded / 1024.0f / 1024.0f, (float)bytesTotal / 1024.0f / 1024.0f);
}

void AppContainer::log_to_trace(struct error_info * info, void * context, const char * fmt, va_list args) {
    AppContainer * self = (AppContainer *)context;
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&self->m_format_buffer);
    size_t n = 0;
        
    mem_buffer_clear_data(&self->m_format_buffer);

    if (info->m_file) {
        n += stream_printf((write_stream_t)&s, "%s(%d): ", info->m_file, info->m_line > 0 ? info->m_line : 0);
    }
    else if (info->m_line >= 0) {
        n += stream_printf((write_stream_t)&s, "%d: ", info->m_line);
    }
        
    n += stream_vprintf((write_stream_t)&s, fmt, args);
    stream_putc((write_stream_t)&s, 0);

    const char * c_msg = (const char *)mem_buffer_make_continuous(&self->m_format_buffer, 0);
        
    AS3_DeclareVar(msg, String);
    AS3_CopyCStringToVar(msg, c_msg, n);
    AS3_Trace(msg);
}

void * AppContainer::do_alloc(size_t size, struct mem_allocrator * allocrator) {
    return malloc(size);
}

void * AppContainer::do_calloc(size_t size, struct mem_allocrator * allocrator) {
    void * p = do_alloc(size, allocrator);
    if (p) bzero(p, size);
    return p;
}

void AppContainer::do_free(void * p, struct mem_allocrator * allocrator) {
    free(p);
}

AppContainer * g_app_container = NULL;
