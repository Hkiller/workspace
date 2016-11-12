#include <assert.h>
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "gd/dr_cvt/dr_cvt_manage.h" 
#include "dr_cvt_internal_ops.h"

int dr_cvt_type_create_ex(dr_cvt_manage_t mgr, const char * name, dr_cvt_fun_t encode, dr_cvt_fun_t decode, void * ctx) {
    struct dr_cvt_type * cvt_type;
    size_t name_len;

    name_len = strlen(name) + 1;

    cvt_type = (struct dr_cvt_type *)mem_alloc(mgr->m_alloc, sizeof(struct dr_cvt_type) + name_len);
    if (cvt_type == NULL) {
        CPE_ERROR(mgr->m_em, "%s: dr_cvt_type_create_ex: alloc fail!", dr_cvt_manage_name(mgr));
        return -1;
    }

    memcpy(cvt_type + 1, name, name_len);

    cvt_type->m_mgr = mgr;
    cvt_type->m_name = (char *)(cvt_type + 1);
    cvt_type->m_encode = encode;
    cvt_type->m_decode = decode;
    cvt_type->m_ctx = ctx;
    cvt_type->m_ref_count = 0;

    cpe_hash_entry_init(&cvt_type->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_cvt_types, cvt_type) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: dr_cvt_type_create_ex: name %s duplicate!",
            dr_cvt_manage_name(mgr), name);
        mem_free(mgr->m_alloc, cvt_type);
        return -1;
    }
    
    return 0;
}

void dr_cvt_type_free_ex(dr_cvt_manage_t mgr, const char * name) {
    struct dr_cvt_type * cvt_type = dr_cvt_type_find(mgr, name);
    if (cvt_type == NULL) {
        CPE_ERROR(mgr->m_em, "%s: dr_cvt_type_free_ex: type %s not exist!", dr_cvt_manage_name(mgr), name);
    }
    else {
        dr_cvt_type_free_i(cvt_type);
    }
}

int dr_cvt_type_create(gd_app_context_t app, const char * name, dr_cvt_fun_t encode, dr_cvt_fun_t decode, void * ctx) {
    dr_cvt_manage_t cvt_mgr = dr_cvt_manage_default(app);
    if (cvt_mgr == NULL) {
        APP_CTX_ERROR(app, "dr_cvt_type_crate: default dr_cvt_manage not exist!");
        return -1;
    }

    return dr_cvt_type_create_ex(cvt_mgr, name, encode, decode, ctx);
}

void dr_cvt_type_free(gd_app_context_t app, const char * name) {
    dr_cvt_manage_t cvt_mgr = dr_cvt_manage_default(app);
    if (cvt_mgr == NULL) {
        APP_CTX_ERROR(app, "dr_cvt_type_free: type %s: default dr_cvt_manage not exist!", name);
        return;
    }

    dr_cvt_type_free_ex(cvt_mgr, name);
}

struct dr_cvt_type * dr_cvt_type_find(dr_cvt_manage_t mgr, const char * name) {
    struct dr_cvt_type key;
    key.m_name = name;

    return (struct dr_cvt_type *)cpe_hash_table_find(&mgr->m_cvt_types, &key);
}

void dr_cvt_type_free_i(struct dr_cvt_type * cvt_type) {
    cpe_hash_table_remove_by_ins(&cvt_type->m_mgr->m_cvt_types, cvt_type);
    mem_free(cvt_type->m_mgr->m_alloc, cvt_type);
}

uint32_t dr_cvt_type_hash(const struct dr_cvt_type * cvt) {
    return cpe_hash_str(cvt->m_name, strlen(cvt->m_name));
}

int dr_cvt_type_cmp(const struct dr_cvt_type * l, const struct dr_cvt_type * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void dr_cvt_type_free_all(dr_cvt_manage_t mgr) {
    struct cpe_hash_it cvt_type_it;
    struct dr_cvt_type * cvt_type;

    cpe_hash_it_init(&cvt_type_it, &mgr->m_cvt_types);

    cvt_type = cpe_hash_it_next(&cvt_type_it);
    while (cvt_type) {
        struct dr_cvt_type * next = cpe_hash_it_next(&cvt_type_it);
        dr_cvt_type_free_i(cvt_type);
        cvt_type = next;
    }
}
