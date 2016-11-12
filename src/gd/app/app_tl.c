#include "cpe/pal/pal_string.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_tl.h"
#include "gd/app/app_context.h"
#include "app_internal_ops.h"

tl_manage_t
app_tl_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc)
{
    struct gd_app_tl * app_tl;
    size_t name_len;

    if (app_tl_manage_find(app, name)) return NULL;

    name_len = strlen(name) + 1;

    app_tl =  (struct gd_app_tl *)mem_alloc(app->m_alloc, sizeof(struct gd_app_tl) + name_len);

    app_tl->m_tl_mgr = tl_manage_create(alloc);
    if (app_tl->m_tl_mgr == NULL) {
        mem_free(app->m_alloc, app_tl);
        return NULL;
    }

    memcpy(app_tl + 1, name, name_len);

    app_tl->m_app = app;
    app_tl->m_name = (const char *)(app_tl + 1);
    
    TAILQ_INSERT_TAIL(&app->m_tls, app_tl, m_next);

    return app_tl->m_tl_mgr;
}

void gd_app_tl_free(struct gd_app_tl * app_tl) {
    TAILQ_REMOVE(&app_tl->m_app->m_tls, app_tl, m_next);

    tl_manage_free(app_tl->m_tl_mgr);

    mem_free(app_tl->m_app->m_alloc, app_tl);
}

void app_tl_manage_free(gd_app_context_t app, tl_manage_t tl_mgr) {
    struct gd_app_tl * app_tl;

    TAILQ_FOREACH(app_tl, &app->m_tls, m_next) {
        if (app_tl->m_tl_mgr == tl_mgr) {
            gd_app_tl_free(app_tl);
            return;
        }
    }
}

tl_manage_t
app_tl_manage_find(gd_app_context_t app, const char * name) {
    struct gd_app_tl * app_tl;

    if (name == NULL) return gd_app_tl_mgr(app);

    TAILQ_FOREACH(app_tl, &app->m_tls, m_next) {
        if (strcmp(app_tl->m_name, name) == 0) {
            return app_tl->m_tl_mgr;
        }
    }

    return NULL;
}


