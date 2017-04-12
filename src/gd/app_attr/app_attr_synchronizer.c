#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "app_attr_synchronizer_i.h"
#include "app_attr_attr_binding_i.h"

struct app_attr_synchronizer_add_attr_ctx {
    app_attr_synchronizer_t m_synchronizer;
    int m_rv;
};

static void app_attr_synchronizer_add_attr_binding(void * ctx, const char * value);

app_attr_synchronizer_t
app_attr_synchronizer_create(
    app_attr_provider_t provider, const char * name,
    void * attrs, app_attr_synchronizer_start_fun_t sync_start_fun)
{
    app_attr_module_t module = provider->m_module;
    app_attr_synchronizer_t synchronizer;
    struct app_attr_synchronizer_add_attr_ctx ctx;
    
    synchronizer = mem_calloc(provider->m_module->m_alloc, sizeof(struct app_attr_synchronizer));
    if (synchronizer == NULL) {
        CPE_ERROR(module->m_em, "ad: create synchronizer %s: alloc fail!", name);
        return NULL;
    }

    synchronizer->m_provider = provider;
    cpe_str_dup(synchronizer->m_name, sizeof(synchronizer->m_name), name);
    synchronizer->m_sync_start_fun = sync_start_fun;
    synchronizer->m_state = app_attr_synchronizer_idle;
    
    TAILQ_INIT(&synchronizer->m_attrs);

    TAILQ_INSERT_TAIL(&provider->m_synchronizers, synchronizer, m_next_for_provider);

    ctx.m_synchronizer = synchronizer;
    ctx.m_rv = 0;

    if (attrs) {
        cpe_str_list_for_each(attrs, ':', app_attr_synchronizer_add_attr_binding, &ctx);
        if (ctx.m_rv) {
            app_attr_synchronizer_free(synchronizer);
            return NULL;
        }
    }
    else {
        app_attr_attr_t attr;

        TAILQ_FOREACH(attr, &provider->m_attrs, m_next_for_provider) {
            if (app_attr_attr_binding_create(app_attr_attr_binding_synchronizer, attr, synchronizer) == NULL) {
                CPE_ERROR(module->m_em, "app_attr_synchronizer_create: binding attr %s create synchronizer fail!", attr->m_name);
                app_attr_synchronizer_free(synchronizer);
                return NULL;
            }
        }
    }

    return synchronizer;
}

void app_attr_synchronizer_free(app_attr_synchronizer_t synchronizer) {
    switch(synchronizer->m_state) {
    case app_attr_synchronizer_trigger:
        app_attr_synchronizer_set_state(synchronizer, app_attr_synchronizer_idle);
        break;
    case app_attr_synchronizer_runing:
        app_attr_synchronizer_set_done(synchronizer, 0);
        break;
    case app_attr_synchronizer_idle:
        break;
    }
    assert(synchronizer->m_state == app_attr_synchronizer_idle);
    
    while(!TAILQ_EMPTY(&synchronizer->m_attrs)) {
        app_attr_attr_binding_free(TAILQ_FIRST(&synchronizer->m_attrs));
    }

    TAILQ_REMOVE(&synchronizer->m_provider->m_synchronizers, synchronizer, m_next_for_provider);
    mem_free(synchronizer->m_provider->m_module->m_alloc, synchronizer);
}

int app_attr_synchronizer_set_done(app_attr_synchronizer_t synchronizer, uint8_t success) {
    app_attr_attr_binding_t attr_binding;
    
    if (synchronizer->m_state != app_attr_synchronizer_runing) {
        CPE_ERROR(synchronizer->m_provider->m_module->m_em, "app_attr_synchronizer: %s.%s: set done: not runing!", synchronizer->m_provider->m_name, synchronizer->m_name);
        return -1;
    }

    TAILQ_FOREACH(attr_binding, &synchronizer->m_attrs, m_next_for_product) {
        app_attr_attr_set_readable(attr_binding->m_attr);
    }

    app_attr_synchronizer_set_state(synchronizer, app_attr_synchronizer_idle);
    return 0;
}

void app_attr_synchronizer_set_state(app_attr_synchronizer_t synchronizer, app_attr_synchronizer_state_t state) {
    app_attr_module_t module = synchronizer->m_provider->m_module;
    
    if (synchronizer->m_state == state) return;

    if (synchronizer->m_state == app_attr_synchronizer_trigger) {
        TAILQ_REMOVE(&module->m_synchronizer_to_process, synchronizer, m_next_for_state);
    }
    else if (state == app_attr_synchronizer_trigger) {
        TAILQ_INSERT_TAIL(&module->m_synchronizer_to_process, synchronizer, m_next_for_state);
    }

    if (module->m_debug) {
        CPE_INFO(
            module->m_em, "app_attr_synchronizer: %s.%s: state %s ==> %s",
            synchronizer->m_provider->m_name, synchronizer->m_name,
            app_attr_synchronizer_state_str(synchronizer->m_state),
            app_attr_synchronizer_state_str(state));
    }

    synchronizer->m_state = state;
}

static app_attr_synchronizer_t app_attr_provider_synchronizer_next(struct app_attr_synchronizer_it * it) {
    app_attr_synchronizer_t * data = (app_attr_synchronizer_t *)(it->m_data);
    app_attr_synchronizer_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_provider);
    return r;
}

void app_attr_provider_synchronizers(app_attr_provider_t provider, app_attr_synchronizer_it_t it) {
    *(app_attr_synchronizer_t *)(it->m_data) = TAILQ_FIRST(&provider->m_synchronizers);
    it->next = app_attr_provider_synchronizer_next;
}

void app_attr_synchronizer_tick(app_attr_module_t module) {
    app_attr_synchronizer_t synchronizer, next_synchronizer;
    
    for(synchronizer = TAILQ_FIRST(&module->m_synchronizer_to_process); synchronizer; synchronizer = next_synchronizer) {
        next_synchronizer = TAILQ_NEXT(synchronizer, m_next_for_state);

        app_attr_synchronizer_set_state(synchronizer, app_attr_synchronizer_runing);
        if (synchronizer->m_sync_start_fun(synchronizer->m_provider->m_ctx, synchronizer) != 0) {
            CPE_ERROR(module->m_em, "app_attr_synchronizer_tick: %s.%s start fail!", synchronizer->m_provider->m_name, synchronizer->m_name);
            app_attr_synchronizer_set_state(synchronizer, app_attr_synchronizer_idle);
        }
    }
}

const char * app_attr_synchronizer_state_str(app_attr_synchronizer_state_t state) {
    switch(state) {
    case app_attr_synchronizer_idle:
        return "idle";
    case app_attr_synchronizer_trigger:
        return "trigger";
    case app_attr_synchronizer_runing:
        return "runing";
    default:
        return "unknown";
    }
}

static void app_attr_synchronizer_add_attr_binding(void * i_ctx, const char * value) {
    struct app_attr_synchronizer_add_attr_ctx * ctx = i_ctx;
    app_attr_module_t module = ctx->m_synchronizer->m_provider->m_module;
    app_attr_attr_t attr;

    attr = app_attr_attr_find(module, value);
    if (attr == NULL) {
        CPE_ERROR(module->m_em, "app_attr_synchronizer_create: binding attr %s not exist!", value);
        ctx->m_rv = -1;
        return;
    }

    if (app_attr_attr_binding_create(app_attr_attr_binding_synchronizer, attr, ctx->m_synchronizer) == NULL) {
        CPE_ERROR(module->m_em, "app_attr_synchronizer_create: binding attr %s create synchronizer fail!", value);
        ctx->m_rv = -1;
        return;
    }
}
