#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_error.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_validate.h"
#include "gd/app/app_log.h"
#include "gd/app/app_library.h"
#include "gd/app/app_module.h"
#include "gd/app/app_library.h"
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "dr_store_internal_types.h"

static int dr_store_loader_load_from_symbol(
    gd_app_context_t app, gd_app_module_t module, dr_store_manage_t mgr, const char * arg)
{
    int rv;
    LPDRMETALIB metalib;
    
    metalib = gd_app_lib_sym(NULL, arg, gd_app_em(app));
    if (metalib == NULL) {
        APP_CTX_ERROR(app, "%s: read load-from-symbol %s: error!", gd_app_module_name(module), arg);
        return -1;
    }

    rv = dr_store_add_lib(mgr, gd_app_module_name(module), metalib, NULL, NULL);

    if (rv == 0) {
        if (mgr->m_debug) {
            APP_CTX_INFO(app, "%s: metalib load from symbol %s: success!", gd_app_module_name(module), arg);
        }
    }

    return rv;
}

static int dr_store_loader_add_file_to_builder(
    gd_app_context_t app, gd_app_module_t module, dr_store_manage_t mgr, dr_metalib_builder_t builder,
    const char * name, const char * file)
{
    if (file == NULL) {
        APP_CTX_ERROR(
            app, "%s: load from file: file is NULL!",
            gd_app_module_name(module));
        return -1;
    }

    if (dr_metalib_builder_add_file(builder, name, file) == NULL) {
        if (name) {
            APP_CTX_ERROR(
                app, "%s: load from file: add source %s[%s] fail!",
                gd_app_module_name(module), file, name);
        }
        else {
            APP_CTX_ERROR(
                app, "%s: load from file: add source %s fail!",
                gd_app_module_name(module), file);
        }
        return -1;
    }

    if (mgr->m_debug) {
        if (name) {
            APP_CTX_INFO(
                app, "%s: load from file: add source %s[%s] success!",
                gd_app_module_name(module), file, name);
        }
        else {
            APP_CTX_INFO(
                app, "%s: load from file: add source %s success!",
                gd_app_module_name(module), file);
        }
    }

    return 0;    
}

static void dr_store_loader_lib_destory(LPDRMETALIB lib, void * ctx) {
    mem_free((mem_allocrator_t)ctx, lib);
}

static const char * dr_store_loader_make_full_path(mem_buffer_t  buffer, gd_app_context_t app, cfg_t cfg) {
    mem_buffer_clear_data(buffer);
    mem_buffer_strcat(buffer, gd_app_root(app));
    mem_buffer_strcat(buffer, "/");
    mem_buffer_strcat(buffer, cfg_as_string(cfg, NULL));

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

static int dr_store_loader_load_from_file(
    gd_app_context_t app, gd_app_module_t module, dr_store_manage_t mgr, cfg_t cfg)
{
    struct cfg_it child_it;
    cfg_t child;
    dr_metalib_builder_t builder = NULL;
    void * buf = NULL;
    int rv;
    struct mem_buffer buffer;
    struct mem_buffer filename_buffer;

    builder = dr_metalib_builder_create(gd_app_alloc(app), gd_app_em(app));
    if (builder == NULL) {
        APP_CTX_ERROR(app, "%s: load from file: create dr_metalib_builder fail!", gd_app_module_name(module));
        return -1;
    }

    mem_buffer_init(&buffer, 0);
    mem_buffer_init(&filename_buffer, 0);

    rv = 0;
    if (cfg_type(cfg) == CPE_CFG_TYPE_STRING) {
        rv = dr_store_loader_add_file_to_builder(
            app, module, mgr, builder, NULL,
            dr_store_loader_make_full_path(&filename_buffer, app, cfg));
    }
    else if (cfg_type(cfg) == CPE_CFG_TYPE_SEQUENCE) {
        cfg_it_init(&child_it, cfg);
        while((child = cfg_it_next(&child_it))) {
            if (dr_store_loader_add_file_to_builder(
                    app, module, mgr, builder, NULL, 
                    dr_store_loader_make_full_path(&filename_buffer, app, child)) != 0)
            {
                rv = -1;
            }
        }
    }
    else if (cfg_type(cfg) == CPE_CFG_TYPE_STRUCT) {
        cfg_it_init(&child_it, cfg);
        while((child = cfg_it_next(&child_it))) {
            if (dr_store_loader_add_file_to_builder(
                    app, module, mgr, builder, cfg_name(child),
                    dr_store_loader_make_full_path(&filename_buffer, app, child)) != 0)
            {
                rv = -1;
            }
        }
    }
    else {
        APP_CTX_ERROR(
            app, "%s: load from file: not support cfg type %d!",
            gd_app_module_name(module), cfg_type(cfg));
        rv = -1;
    }

    if (rv != 0) goto FROM_FILE_COMPLETE;

    dr_metalib_builder_analize(builder);
    if (dr_inbuild_build_lib(
            &buffer,
            dr_metalib_bilder_lib(builder),
            gd_app_em(app)) != 0)
    {
        APP_CTX_ERROR(
            app, "%s: load from file: build metalib fail!",
            gd_app_module_name(module));
        rv = -1;
        goto FROM_FILE_COMPLETE;
    }

    buf = mem_alloc(gd_app_alloc(app), mem_buffer_size(&buffer));
    if (buf == NULL) {
        APP_CTX_ERROR(
            app, "%s: load from file: alloc buf to store metalib fail, size is %d!",
            gd_app_module_name(module), (int)mem_buffer_size(&buffer));
        rv = -1;
        goto FROM_FILE_COMPLETE;
    }

    if (mem_buffer_read(buf, mem_buffer_size(&buffer), &buffer) != mem_buffer_size(&buffer)) {
        APP_CTX_ERROR(
            app, "%s: load from file: copy to save buffer fail!",
            gd_app_module_name(module));
        rv = -1;
        goto FROM_FILE_COMPLETE;
    }

    if (dr_store_add_lib(
            mgr, gd_app_module_name(module),
            (LPDRMETALIB)buf, dr_store_loader_lib_destory, gd_app_alloc(app)) != 0)
    {
        rv = -1;
        goto FROM_FILE_COMPLETE;
    }

    buf = NULL;

FROM_FILE_COMPLETE:
    if (rv == 0) {
        if (mgr->m_debug) {
            APP_CTX_INFO(app, "%s: metalib load from files: success!", gd_app_module_name(module));
        }
    }

    if (buf) mem_free(gd_app_alloc(app), buf);

    mem_buffer_clear(&buffer);
    mem_buffer_clear(&filename_buffer);
    dr_metalib_builder_free(builder);

    return rv;
}

static int dr_store_loader_do_validate(gd_app_context_t app, gd_app_module_t module, LPDRMETALIB metalib, cfg_t cfg) {
    int rv;

    struct cfg_it child_it;
    cfg_t child;

    rv = 0;

    cfg_it_init(&child_it, cfg);

    while((child = cfg_it_next(&child_it))) {
        const char * validate_name = cfg_as_string(child, NULL);
        if (validate_name == NULL) {
            APP_CTX_ERROR(app, "%s: validate: NULL valudate name!", gd_app_module_name(module));
            rv = -1;
            continue;
        }

        if (strcmp(validate_name, "align")== 0){
            if (dr_metalib_validate_align(gd_app_em(app), metalib) != 0) {
                rv = -1;
            }
        }
        else {
            APP_CTX_ERROR(app, "%s: validate: unknown valudate name %s!", gd_app_module_name(module), validate_name);
            rv = -1;
        }
    }

    return rv;
}

EXPORT_DIRECTIVE
int dr_store_loader_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    const char * arg;
    cfg_t child_cfg;
    dr_store_manage_t mgr = NULL;
    int rv;

    mgr = dr_store_manage_default(app);
    if (mgr == NULL) {
        APP_CTX_ERROR(app, "%s: dr_store_manage_default not exist!", gd_app_module_name(module));
        return -1;
    }

    if ((arg = cfg_get_string(cfg, "load-from-symbol", NULL))) {
        rv = dr_store_loader_load_from_symbol(app, module, mgr, arg);
    }
    else if ((arg = cfg_get_string(cfg, "load-from-bin", NULL))) {
        rv = dr_store_manage_load_from_bin(mgr, gd_app_module_name(module), arg);
    }
    else if ((child_cfg = cfg_find_cfg(cfg, "load-from-file"))) {
        rv = dr_store_loader_load_from_file(app, module, mgr, child_cfg);
    }
    else {
        APP_CTX_ERROR(app, "%s: no any load way!", gd_app_module_name(module));
        rv = -1;
    }

    if (rv == 0 && (child_cfg = cfg_find_cfg(cfg, "validate"))) {
        dr_store_t store;
        LPDRMETALIB metalib;

        store = dr_store_find(mgr, gd_app_module_name(module));
        assert(store);

        metalib = dr_store_lib(store);
        if (metalib == NULL) {
            APP_CTX_ERROR(app, "%s: meta lib not exist!", gd_app_module_name(module));
            dr_store_free(store);
            rv = -1;
        }
        else if (dr_store_loader_do_validate(app, module, metalib, child_cfg) != 0) {
            dr_store_free(store);
            rv = -1;
        }
    }

    if (rv == 0 && cfg_get_int32(cfg, "dump", 0)) {
        struct write_stream_error stream = CPE_WRITE_STREAM_ERROR_INITIALIZER(gd_app_em(app));
        dr_store_t store = dr_store_find(mgr, gd_app_module_name(module));
        assert(store);

        stream_printf((write_stream_t)&stream, "*** dump meta lib of %s ***\n", gd_app_module_name(module));

        dr_lib_print((write_stream_t)&stream, dr_store_lib(store), 4);

        stream_do_flush_to_error((write_stream_t)&stream);
    }

    return rv;
}

EXPORT_DIRECTIVE
void dr_store_loader_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dr_store_manage_t mgr;

    mgr = dr_store_manage_default(app);
    if (mgr) {

        dr_store_t store;
        store = dr_store_find(mgr, gd_app_module_name(module));
        if (store) {
            dr_store_free(store);
        }
    }
}

