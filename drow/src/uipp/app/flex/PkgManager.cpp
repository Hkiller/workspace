#include <assert.h>
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/spack/spack_rfs.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_installer.h"
#include "plugin/package/plugin_package_package.h"
#include "PkgManager.hpp"
#include "DownloadTask.hpp"
#include "DownloadMonitor.hpp"

struct PkgManagerImpl : public PkgManager {
    struct Pkg : public DownloadMonitor {
        enum State { Loading, Error, Loaded };
        
        Pkg(PkgManagerImpl & mgr, spack_rfs_t fs, const char * name, plugin_package_package_t package)
            : m_mgr(mgr)
            , m_name_buf(name)
            , m_name(m_name_buf.c_str())
            , m_package(package)
            , m_fs(fs)
            , m_total_size(0)
            , m_loaded_size(0)
        {
            cpe_hash_entry_init(&m_hh);
            if (cpe_hash_table_insert(&mgr.m_pkgs, this) != 0) {
                APP_CTX_ERROR(mgr.m_app, "pkg: %s insert fail!", name);
                assert(0);
            }

            char url_buf[128];
            snprintf(url_buf, sizeof(url_buf), "%s.spack", name);
            m_task = new DownloadTask(*this, m_mgr.m_app, url_buf);
            m_state = Loading;
            enqueue();
        }

        ~Pkg() {
            cpe_hash_table_remove_by_ins(&m_mgr.m_pkgs, this);
            dequeue();
            
            if (m_task) {
                delete m_task;
                m_task = NULL;
            }

            if (m_package) {
                if (plugin_package_package_is_installed(m_package)) {
                    plugin_package_package_uninstall(m_package);
                }
            }
            
            if (m_fs) {
                spack_rfs_free(m_fs);
                m_fs = NULL;
            }
        }
        
        virtual void onRestart(DownloadTask & task) {
            spack_rfs_clear(m_fs);
        }
        
        virtual void onComplete(DownloadTask & task) {
            if (task.state() == DownloadTask::Complete) {
                if (spack_rfs_append_complete(m_fs) != 0) {
                    APP_CTX_ERROR(m_mgr.m_app, "Pkg: %s: complete: build fs fail!", m_name);
                    setState(Error);
                    task.setError();
                    return;
                }
        
                delete m_task;
                m_task = NULL;

                setState(Loaded);
                APP_CTX_INFO(m_mgr.m_app, "Pkg: %s: complete: success!", m_name);

                if (m_package && !plugin_package_package_is_installed(m_package)) {
                    if (plugin_package_package_install(m_package)) {
                        APP_CTX_ERROR(m_mgr.m_app, "Pkg: %s: install fail!", m_name);
                    }
                }
            }
            else {
                setState(Error);
                task.setError();
            }
        }
        
        virtual void onProgress(DownloadTask & task, int32_t bytesTotal, int32_t bytesLoaded) {
            m_total_size = bytesTotal;
            m_loaded_size = bytesTotal;

            if (m_package && m_total_size > 0) {
                plugin_package_package_set_total_size(m_package, m_total_size);
                plugin_package_package_set_progress(m_package, (float)m_loaded_size / (float)m_total_size);
            }
            
            mem_buffer_t buffer = gd_app_tmp_buffer(m_mgr.m_app);
            void * data = task.readData(buffer);
            if (data == NULL) {
                APP_CTX_ERROR(m_mgr.m_app, "Pkg: %s: onProgress: read data fail, size=%d!", m_name, (int)mem_buffer_size(buffer));
                task.setError();
                return;
            }

            if (spack_rfs_append_data(m_fs, data, mem_buffer_size(buffer)) != 0) {
                APP_CTX_ERROR(m_mgr.m_app, "Pkg: %s: onProgress: fs append data fail", m_name);
                task.setError();
                return;
            }

            //APP_CTX_INFO(m_mgr.m_app, "Pkg: %s: onProgress: fs append %d data", m_name, (int)mem_buffer_size(buffer));
        }

        void setState(State s) {
            if (m_state == s) return;
            
            dequeue();
            m_state = s;
            enqueue();

            if (m_package
                && m_state != Loading
                && plugin_package_package_state(m_package) == plugin_package_package_downloading)
            {
                if (m_state == Error) {
                    plugin_package_package_set_progress(m_package, 0.0f);
                }
                else {
                    plugin_package_package_set_progress(m_package, 1.0f);
                    plugin_package_package_set_path(m_package, spack_rfs_path(m_fs));
                }
            }
        }
        
        void dequeue(void) {
            switch(m_state) {
            case Loading:
                assert(m_mgr.m_loading_pkg_count > 0);
                m_mgr.m_loading_pkg_count--;
                TAILQ_REMOVE(&m_mgr.m_loading_pkgs, this, m_next);
                break;
            case Error:
                assert(m_mgr.m_error_pkg_count > 0);
                m_mgr.m_error_pkg_count--;
                TAILQ_REMOVE(&m_mgr.m_error_pkgs, this, m_next);
                break;
            case Loaded:
                assert(m_mgr.m_loaded_pkg_count > 0);
                m_mgr.m_loaded_pkg_count--;
                TAILQ_REMOVE(&m_mgr.m_loaded_pkgs, this, m_next);
                break;
            }
        }

        void enqueue(void) {
            switch(m_state) {
            case Loading:
                m_mgr.m_loading_pkg_count++;
                TAILQ_INSERT_TAIL(&m_mgr.m_loading_pkgs, this, m_next);
                break;
            case Error:
                m_mgr.m_error_pkg_count++;
                TAILQ_INSERT_TAIL(&m_mgr.m_error_pkgs, this, m_next);
                break;
            case Loaded:
                m_mgr.m_loaded_pkg_count++;
                TAILQ_INSERT_TAIL(&m_mgr.m_loaded_pkgs, this, m_next);
                break;
            }
        }

        int attach(plugin_package_package_t package) {
            if(m_package != NULL) {
                APP_CTX_ERROR(m_mgr.m_app, "pkg: %s attach already have package!", m_name);
                return -1;
            }

            switch(m_state) {
            case Pkg::Loading:
                m_package = package;
                if (m_total_size > 0) {
                    plugin_package_package_set_total_size(m_package, m_total_size);
                    plugin_package_package_set_progress(m_package, (float)m_loaded_size / (float)m_total_size);
                }
                return 0;
            case Error:
                APP_CTX_ERROR(m_mgr.m_app, "pkg: %s attach already have error!", m_name);
                if (plugin_package_package_state(m_package) == plugin_package_package_downloading) {
                    plugin_package_package_set_progress(package, 0.0f);
                }
                return 0;
            case Loaded:
                m_package = package;
                if (m_total_size > 0) {
                    plugin_package_package_set_total_size(m_package, m_total_size);
                }
                if (plugin_package_package_state(m_package) == plugin_package_package_downloading) {
                    plugin_package_package_set_progress(package, 1.0f);
                }
                if (!plugin_package_package_is_installed(package)) {
                    plugin_package_package_set_path(package, spack_rfs_path(m_fs));
                }
                return 0;
            default:
                APP_CTX_ERROR(m_mgr.m_app, "pkg: %s attach state %d unknown!", m_name, m_state);
                return -1;
            }
        }
        
        PkgManagerImpl & m_mgr;
        ::std::string m_name_buf;
        const char * m_name;
        plugin_package_package_t m_package;
        cpe_hash_entry m_hh;
        State m_state;
        TAILQ_ENTRY(Pkg) m_next;
        spack_rfs_t m_fs;
        DownloadTask * m_task;
        int32_t m_total_size;
        int32_t m_loaded_size;
        
        static uint32_t hash(Pkg * pkg) {
            return cpe_hash_str(pkg->m_name, strlen(pkg->m_name));
        }

        static int eq(Pkg * l, Pkg * r) {
            return strcmp(l->m_name, r->m_name) == 0 ? 1 : 0;
        }
    };

    PkgManagerImpl(gd_app_context_t app)
        : m_app(app)
        , m_installer(NULL)
        , m_loading_pkg_count(0)
        , m_error_pkg_count(0)
        , m_loaded_pkg_count(0)
    {
        TAILQ_INIT(&m_loading_pkgs);
        TAILQ_INIT(&m_error_pkgs);
        TAILQ_INIT(&m_loaded_pkgs);

        if (cpe_hash_table_init(
                &m_pkgs,
                gd_app_alloc(app),
                (cpe_hash_fun_t) Pkg::hash,
                (cpe_hash_eq_t) Pkg::eq,
                CPE_HASH_OBJ2ENTRY(Pkg, m_hh),
                -1) != 0)
        {
            APP_CTX_ERROR(m_app, "PkgManagerImpl:: init pkgs fail!");
            return;
        }
    }

    ~PkgManagerImpl() {
        uninstall();
        clearPkgs();
    }

    virtual int addPkg(const char * name) {
        Pkg * pkg = findPkg(name);
        if (pkg) return 0;

        char path_buf[128];
        snprintf(path_buf, sizeof(path_buf), "/packages/%s", name);
        spack_rfs_t fs = spack_rfs_create(gd_app_vfs_mgr(m_app), path_buf, gd_app_alloc(m_app), gd_app_em(m_app));
        if (fs == NULL) {
            APP_CTX_ERROR(m_app, "PkgManager: addPkg %s: create fs fail!", name);
            return -1;
        }

        new Pkg(*this, fs, name, NULL);
        
        return 0;
    }
    
    virtual uint32_t loadingPkgCount(void) const {
        return m_loading_pkg_count;
    }

    virtual int install(void) {
        if (m_installer == NULL) {
            plugin_package_module_t pkg_module = plugin_package_module_find_nc(m_app, NULL);
            if (pkg_module == NULL) {
                APP_CTX_ERROR(m_app, "PkgManager: install: no pkg module!");
                return -1;
            }

            m_installer = plugin_package_installer_create(
                pkg_module, (void*)this, do_install_start, do_installer_cancel);
            if (m_installer == NULL) {
                APP_CTX_ERROR(m_app, "PkgManager: install: create installer fail!");
                return -1;
            }

            cpe_hash_it pkg_it;
            cpe_hash_it_init(&pkg_it, &m_pkgs);
            while(Pkg * pkg = (Pkg *)cpe_hash_it_next(&pkg_it)) {

                plugin_package_package_t package = plugin_package_package_find(pkg_module, pkg->m_name);
                if (package == NULL) {
                    APP_CTX_ERROR(m_app, "PkgManager: install: exist pkg %s not exist!", pkg->m_name);
                    continue;
                }

                if (pkg->attach(package) != 0) {
                    APP_CTX_ERROR(m_app, "PkgManager: install: exist pkg %s attach fail!", pkg->m_name);
                    continue;
                }
                
                if (pkg->m_state != Pkg::Loaded) {
                    APP_CTX_INFO(m_app, "PkgManager: install: exist pkg %s not loaded!", pkg->m_name);
                    continue;
                }

                if (plugin_package_package_is_installed(package)) {
                    APP_CTX_INFO(m_app, "PkgManager: install: exist pkg %s already installed, skip!", pkg->m_name);
                    continue;
                }
                
                if (plugin_package_package_install(package) != 0) {
                    APP_CTX_ERROR(m_app, "PkgManager: install: exist pkg %s install fail!", pkg->m_name);
                    continue;
                }
                else {
                    APP_CTX_INFO(m_app, "PkgManager: install: exist pkg %s install success!", pkg->m_name);
                }
            }
        }

        return 0;
    }
    
    virtual void uninstall(void) {
        if (m_installer) {
            plugin_package_installer_free(m_installer);
            m_installer = NULL;
        }
    }

    void clearPkgs(void) {
        cpe_hash_it pkg_it;
        Pkg * pkg;

        cpe_hash_it_init(&pkg_it, &m_pkgs);

        pkg = (Pkg *)cpe_hash_it_next(&pkg_it);
        while (pkg) {
            Pkg * next = (Pkg *)cpe_hash_it_next(&pkg_it);
            delete pkg;
            pkg = next;
        }
    }

    Pkg * findPkg(const char * name) {
        char key_buf[sizeof(Pkg)];
        Pkg * key = (Pkg *)key_buf;
        key->m_name = name;
        return (Pkg *)cpe_hash_table_find(&m_pkgs, key);
    }

    int installStart(plugin_package_package_t package) {
        const char * pkg_name = plugin_package_package_name(package);

        Pkg * pkg = findPkg(pkg_name);
        if (pkg) {
            return pkg->attach(package);
        }

        char path_buf[128];
        snprintf(path_buf, sizeof(path_buf), "/packages/%s", pkg_name);
        spack_rfs_t fs = spack_rfs_create(gd_app_vfs_mgr(m_app), path_buf, gd_app_alloc(m_app), gd_app_em(m_app));
        if (fs == NULL) {
            APP_CTX_ERROR(m_app, "pkg: %s: install start: create fs fail!", pkg_name);
            return -1;
        }

        new Pkg(*this, fs, pkg_name, package);

        return 0;
    }

    void installCancel(plugin_package_package_t package) {
        const char * pkg_name = plugin_package_package_name(package);
        Pkg * pkg = findPkg(pkg_name);
        if (pkg) delete pkg;
    }

    static int do_install_start(void * ctx, plugin_package_package_t package) {
        PkgManagerImpl* self = ((PkgManagerImpl*)ctx);
        try {
            return self->installStart(package);
        }
        catch(::std::runtime_error const & e) {
            APP_CTX_ERROR(self->m_app, "pkg: %s: start install: %s", e.what(), plugin_package_package_name(package));
            return -1;
        }
        catch(...) {
            APP_CTX_ERROR(self->m_app, "pkg: %s: start install: unknown exception", plugin_package_package_name(package));
            return -1;
        }
    }
    
    static void do_installer_cancel(void * ctx, plugin_package_package_t package) {
        PkgManagerImpl* self = ((PkgManagerImpl*)ctx);
        try {
            self->installCancel(package);
        }
        catch(::std::runtime_error const & e) {
            APP_CTX_ERROR(self->m_app, "pkg: %s: cancel install: %s", e.what(), plugin_package_package_name(package));
        }
        catch(...) {
            APP_CTX_ERROR(self->m_app, "pkg: %s: cancel install: unknown exception", plugin_package_package_name(package));
        }
    }
    
    typedef TAILQ_HEAD(pkg_list, Pkg) pkg_list_t;

    gd_app_context_t m_app;
    struct cpe_hash_table m_pkgs;
    plugin_package_installer_t m_installer;
    
    /*loading*/
    uint32_t m_loading_pkg_count;
    pkg_list_t m_loading_pkgs;
    /*error*/
    uint32_t m_error_pkg_count;
    pkg_list_t m_error_pkgs;
    /*loaded*/
    uint32_t m_loaded_pkg_count;
    pkg_list_t m_loaded_pkgs;
};

PkgManager * PkgManager::create(gd_app_context_t app) {
    return new PkgManagerImpl(app);
}

PkgManager::~PkgManager() {
}

