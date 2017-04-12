#ifndef UIPP_APP_ENV_FLEX_APP_CONTAINER_H
#define UIPP_APP_ENV_FLEX_APP_CONTAINER_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/memory.h"
#include "cpe/spack/spack_rfs.h"
#include "DownloadMonitor.hpp"

class AppContainer : public DownloadMonitor {
public:
    AppContainer();
    virtual ~AppContainer();

    error_monitor_t em(void) { return &m_em; }
    
    int createApp(void);
    int startApp(void);
    int tickStartApp(void);

    void resize(uint32_t width, uint32_t height);
    void reinit(void);
    
    bool isStageOk(void) const { return m_is_stage_ok; }
    bool isDownloadOk(void) const { return m_is_download_ok; }
    bool isStarting(void) const { return m_module_installer != NULL; }
    bool downloadError(void) const;
    void setStageOk(void) { m_is_stage_ok = true; }
    uint32_t pkgLoadingCount(void) const;
    
private:
    mem_buffer m_format_buffer;
    mem_allocrator m_alloc;
    error_monitor m_em;
    gd_app_context_t m_app;
    gd_app_module_installer_t m_module_installer;
    bool m_is_stage_ok;
    bool m_is_download_ok;
    PkgManager * m_pkg_mgr;
    spack_rfs_t m_root_fs;
    vfs_builder_t m_store_fs;
    DownloadTask * m_task;

    virtual void onRestart(DownloadTask & task);
    virtual void onComplete(DownloadTask & task);
    virtual void onProgress(DownloadTask & task, int32_t bytesTotal, int32_t bytesLoaded);
    
    static void log_to_trace(struct error_info * info, void * context, const char * fmt, va_list args);
    static void * do_alloc(size_t size, struct mem_allocrator * allocrator);
    static void * do_calloc(size_t size, struct mem_allocrator * allocrator);
    static void do_free(void * p, struct mem_allocrator * allocrator);

    static void on_minimal_download(void * ctx, plugin_package_load_task_t task, plugin_package_package_t package, float progress);
};

extern AppContainer * g_app_container;

#endif
