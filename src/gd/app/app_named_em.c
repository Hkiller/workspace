#include <assert.h>
#include "app_internal_ops.h"

uint32_t gd_app_em_hash(const struct gd_app_em * app_em) {
    return cpe_hash_str(app_em->m_name, strlen(app_em->m_name));
}

int gd_app_em_cmp(const struct gd_app_em * l, const struct gd_app_em * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

error_monitor_t gd_app_named_em(gd_app_context_t context, const char * name) {
    struct gd_app_em key;
    struct gd_app_em * app_em;

    if (name == NULL) return context->m_em;

    key.m_name = name;

    app_em = (struct gd_app_em *)cpe_hash_table_find(&context->m_named_ems, &key);

    return app_em ? app_em->m_em : context->m_em;
}

int gd_app_set_named_em(gd_app_context_t context, const char * name, error_monitor_t em) {
    struct gd_app_em key;
    struct gd_app_em * app_em;

    key.m_name = name;

    app_em = (struct gd_app_em *)cpe_hash_table_find(&context->m_named_ems, &key);
    if (app_em == NULL) {
        size_t name_len = strlen(name) + 1;

        app_em = mem_alloc(context->m_alloc, sizeof(struct gd_app_em) + name_len);
        if (app_em == NULL) return -1;

        memcpy(app_em + 1, name, name_len);

        app_em->m_name = (char *)(app_em + 1);
        cpe_hash_entry_init(&app_em->m_hh);

        if (cpe_hash_table_insert_unique(&context->m_named_ems, app_em) != 0) {
            mem_free(context->m_alloc, app_em);
            return -1;
        }
    }

    assert(app_em);
    app_em->m_em = em;

    return 0;
}

static void gd_app_em_free(gd_app_context_t context, struct gd_app_em * app_em) {
    assert(app_em);
    cpe_hash_table_remove_by_ins(&context->m_named_ems, app_em);
    mem_free(context->m_alloc, app_em);
}

void gd_app_remove_named_em(gd_app_context_t context, const char * name) {
    struct gd_app_em key;
    struct gd_app_em * app_em;

    key.m_name = name;

    app_em = (struct gd_app_em *)cpe_hash_table_find(&context->m_named_ems, &key);
    if (app_em) gd_app_em_free(context, app_em);
}

void gd_app_em_free_all(gd_app_context_t context) {
    struct cpe_hash_it app_em_it;
    struct gd_app_em * app_em;

    cpe_hash_it_init(&app_em_it, &context->m_named_ems);

    app_em = cpe_hash_it_next(&app_em_it);
    while(app_em) {
        struct gd_app_em * next = cpe_hash_it_next(&app_em_it);
        gd_app_em_free(context, app_em);
        app_em = next;
    }
}
