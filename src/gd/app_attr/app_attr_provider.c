#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "app_attr_provider_i.h"
#include "app_attr_synchronizer_i.h"
#include "app_attr_attr_i.h"
#include "app_attr_attr_binding_i.h"
#include "app_attr_request_i.h"

static int app_attr_provider_build_attrs(app_attr_provider_t provider, uint32_t start_pos, LPDRMETA meta, mem_buffer_t path_buffer);

app_attr_provider_t
app_attr_provider_create(
    app_attr_module_t module, const char * name,
    void * ctx,
    void * data, size_t data_size, LPDRMETA data_meta)
{
    app_attr_provider_t provider;

    provider = mem_calloc(module->m_alloc, sizeof(struct app_attr_provider));
    if (provider == NULL) {
        CPE_ERROR(module->m_em, "ad: create provider %s: alloc fail!", name);
        return NULL;
    }

    provider->m_module = module;
    cpe_str_dup(provider->m_name, sizeof(provider->m_name), name);
    provider->m_ctx = ctx;
    provider->m_data = data;
    provider->m_data_size = data_size;
    provider->m_data_meta = data_meta;

    TAILQ_INIT(&provider->m_synchronizers);
    TAILQ_INIT(&provider->m_attrs);

    TAILQ_INSERT_TAIL(&module->m_providers, provider, m_next);

    mem_buffer_clear_data(gd_app_tmp_buffer(module->m_app));
    if (app_attr_provider_build_attrs(provider, 0, data_meta, gd_app_tmp_buffer(module->m_app)) != 0) {
        app_attr_provider_free(provider);
        return NULL;
    }

    return provider;
}

void app_attr_provider_free(app_attr_provider_t provider) {
    app_attr_module_t module;

    module = provider->m_module;

    while(!TAILQ_EMPTY(&provider->m_synchronizers)) {
        app_attr_synchronizer_free(TAILQ_FIRST(&provider->m_synchronizers));
    }

    while(!TAILQ_EMPTY(&provider->m_attrs)) {
        app_attr_attr_free(TAILQ_FIRST(&provider->m_attrs));
    }
    
    TAILQ_REMOVE(&module->m_providers, provider, m_next);

    mem_free(module->m_alloc, provider);
}

struct app_attr_provider_set_attrs_ctx {
    app_attr_provider_t m_provider;
    int m_rv;
};

static void app_attr_provider_set_attrs_wait_process(void * i_ctx, const char * value) {
    struct app_attr_provider_set_attrs_ctx * ctx = i_ctx;
    app_attr_module_t module = ctx->m_provider->m_module;
    app_attr_attr_t attr;

    attr = app_attr_attr_find_in_provider(ctx->m_provider, value);
    if (attr == NULL) {
        CPE_ERROR(module->m_em, "app_attr_provider_set_attrs_wait: provider %s attr %s not exist!", ctx->m_provider->m_name, value);
        ctx->m_rv = -1;
        return;
    }

    attr->m_state = app_attr_attr_waiting;
}

int app_attr_provider_set_attrs_wait(app_attr_provider_t provider, const char * attrs) {
    struct app_attr_provider_set_attrs_ctx ctx;
    ctx.m_provider = provider;
    ctx.m_rv = 0;
    cpe_str_list_for_each(attrs, ':', app_attr_provider_set_attrs_wait_process, &ctx);
    return ctx.m_rv;
}

void app_attr_provider_set_attrs_wait_all(app_attr_provider_t provider) {
    app_attr_attr_t attr;

    TAILQ_FOREACH(attr, &provider->m_attrs, m_next_for_provider) {
        attr->m_state = app_attr_attr_waiting;
    }
}

static void app_attr_provider_set_attrs_changed_process(void * i_ctx, const char * value) {
    struct app_attr_provider_set_attrs_ctx * ctx = i_ctx;
    app_attr_module_t module = ctx->m_provider->m_module;
    app_attr_attr_t attr;
    
    attr = app_attr_attr_find(module, value);
    if (attr == NULL) {
        CPE_ERROR(module->m_em, "app_attr_provider_set_attrs_changed: attr %s not exist!", value);
        ctx->m_rv = -1;
        return;
    }

    app_attr_attr_set_readable(attr);
}

int app_attr_provider_set_attrs_changed(app_attr_provider_t provider, const char * attrs) {
    struct app_attr_provider_set_attrs_ctx ctx;
    ctx.m_provider = provider;
    ctx.m_rv = 0;
    cpe_str_list_for_each(attrs, ':', app_attr_provider_set_attrs_changed_process, &ctx);
    return ctx.m_rv;
}

static app_attr_provider_t app_attr_module_provider_next(struct app_attr_provider_it * it) {
    app_attr_provider_t * data = (app_attr_provider_t *)(it->m_data);
    app_attr_provider_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next);
    return r;
}

void app_attr_module_providers(app_attr_module_t module, app_attr_provider_it_t it) {
    *(app_attr_provider_t *)(it->m_data) = TAILQ_FIRST(&module->m_providers);
    it->next = app_attr_module_provider_next;
}

static int app_attr_provider_build_attrs(app_attr_provider_t provider, uint32_t start_pos, LPDRMETA meta, mem_buffer_t path_buffer) {
    size_t path_size = mem_buffer_size(path_buffer);
    int i;

    for (i = 0; i < dr_meta_entry_num(meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(meta, i);

        if (path_size > 0) mem_buffer_strcat(path_buffer, ".");
        mem_buffer_strcat(path_buffer, dr_entry_name(entry));
        
        if (dr_entry_type(entry) <= CPE_DR_TYPE_COMPOSITE) {
            if (app_attr_provider_build_attrs(provider, (uint32_t)dr_entry_data_start_pos(entry, 0), dr_entry_ref_meta(entry), path_buffer) != 0) return -1;
        }
        else {
            app_attr_attr_t attr;

            attr = app_attr_attr_create(provider, mem_buffer_make_continuous(path_buffer, 0), entry, (uint32_t)dr_entry_data_start_pos(entry, 0) + start_pos);
            if (attr == NULL) return -1;
        }

        mem_buffer_set_size(path_buffer, path_size);
        if (path_size > 0) {
            *((char*)mem_buffer_make_continuous(path_buffer, 0) + path_size - 1) = 0;
        }
    }

    return 0;
}

