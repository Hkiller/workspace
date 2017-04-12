#define _GNU_SOURCE
#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_dlfcn.h"
#include "gd/app/app_library.h"
#include "app_library_symbol_i.h"
#include "app_internal_ops.h"

void * gd_app_default_lib_handler = NULL;
static int gd_app_default_lib_handler_loaded = 0;
static
gd_app_lib_list_t g_app_libs = TAILQ_HEAD_INITIALIZER(g_app_libs);

void gd_set_default_library(void * handler) {
    gd_app_default_lib_handler = handler;
}

static
struct gd_app_lib *
gd_app_lib_create(const char * libName, error_monitor_t em) {
    size_t nameLen;
    struct gd_app_lib * lib = NULL;
    char * buf;

    nameLen = strlen(libName);

    buf = (char*)mem_alloc(NULL, nameLen + 1 + sizeof(struct gd_app_lib));
    if (buf == NULL) {
        CPE_ERROR(em, "create lib %s: alloc buf fail!", libName);
        return NULL;
    }
    
    memcpy(buf, libName, nameLen + 1);
    lib = (struct gd_app_lib *)(buf + nameLen + 1);

    lib->m_name = buf;
    lib->m_handler = NULL;
    TAILQ_INIT(&lib->m_modules);

    lib->m_handler = dlopen(libName, RTLD_NOW | RTLD_LOCAL);
    if (lib->m_handler == NULL) {
        CPE_ERROR(em, "create lib %s: %s!", libName, dlerror());
        mem_free(NULL, buf);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&g_app_libs, lib, m_next);

    return lib;
}

static
void gd_app_lib_free(struct gd_app_lib * lib) {
    if (lib->m_handler) {
        dlclose(lib->m_handler);
        lib->m_handler = NULL;
    }

    TAILQ_REMOVE(&g_app_libs, lib, m_next);

    mem_free(NULL, lib->m_name);
}

static
struct gd_app_lib *
gd_app_lib_find(const char * libName) {
    struct gd_app_lib * lib;

    TAILQ_FOREACH(lib, &g_app_libs, m_next) {
        if (strcmp(lib->m_name, libName) == 0) {
            return lib;
        }
    }

    return NULL;
}

struct gd_app_lib *
gd_app_lib_open_for_module(
    const char * libName,
    struct gd_app_module_type * module,
    error_monitor_t em)
{
    struct gd_app_lib * lib;

    assert(module);
    assert(libName);

    lib = gd_app_lib_find(libName);

    if (lib == NULL) {
        lib = gd_app_lib_create(libName, em);
        if (lib == NULL) return NULL;
    }

    TAILQ_INSERT_TAIL(&lib->m_modules, module, m_qh_for_lib);

    return lib;
}

void gd_app_lib_close_for_module(
    struct gd_app_lib * lib,
    struct gd_app_module_type * module,
    error_monitor_t em)
{
    assert(lib);
    assert(module);

    if (lib == NULL) return;

    TAILQ_INSERT_TAIL(&lib->m_modules, module, m_qh_for_lib);

    if (TAILQ_EMPTY(&lib->m_modules)) {
        gd_app_lib_free(lib);
    }
}

void * gd_app_lib_sym(struct gd_app_lib * lib, const char * symName, error_monitor_t em) {
    gd_app_symbol_t symbol;
    void * sym;

    symbol = gd_app_symbol_find(lib ? lib->m_name : "", symName);
    if (symbol) return symbol->m_symbol;
                            
    dlerror();

    if (gd_app_default_lib_handler_loaded == 0 && lib == NULL && gd_app_default_lib_handler == NULL) {
#ifdef __CYGWIN32__
        gd_app_default_lib_handler = dlopen(NULL, RTLD_NOW);
#else
        Dl_info dl_info;
        bzero(&dl_info, sizeof (dl_info));
        gd_app_default_lib_handler_loaded = 1;
        if (dladdr(&gd_app_lib_sym, &dl_info) == 0) {
            CPE_ERROR(em, "locate default lib handler fail: %s", dlerror());
        }
        else {
            CPE_ERROR(em, "locate default lib handler success: %s at 0x%p", dl_info.dli_fname, dl_info.dli_fbase);
            gd_app_default_lib_handler = dlopen(dl_info.dli_fname, RTLD_NOW);
            if (gd_app_default_lib_handler) {
                dlclose(gd_app_default_lib_handler);
            }
            else {
                gd_app_default_lib_handler = dlopen(NULL, RTLD_NOW);
            }
        }
#endif
    }

    sym = lib
        ? dlsym(lib->m_handler, symName)
        : gd_app_default_lib_handler
		  ? dlsym(gd_app_default_lib_handler, symName)
		  : NULL;

    if (sym == NULL && em) {
        const char * e = dlerror();
        if (e) {
            CPE_ERROR(em, "find symbol %s error: %s\n", symName, e);
        }
    }

    return sym;
}
