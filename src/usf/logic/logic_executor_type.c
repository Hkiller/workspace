#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic/logic_executor.h"
#include "logic_internal_ops.h"

logic_executor_type_t
logic_executor_type_create(logic_executor_type_group_t group, const char * name) {
    logic_executor_type_t type;
    size_t name_len;
    char * buf;

    name_len = strlen(name) + 1;
    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(group->m_alloc, sizeof(struct logic_executor_type) + name_len);
    if (buf == NULL) return NULL;

    cpe_str_dup(buf, name_len, name);

    type = (logic_executor_type_t)(buf + name_len);
    type->m_group = group;
    type->m_name = (char *)buf;
    type->m_op = NULL;
    type->m_ctx_fini = NULL;

    cpe_hash_entry_init(&type->m_hh);
    if (cpe_hash_table_insert_unique(&group->m_types, type) != 0) {
        mem_free(group->m_alloc, buf);
        return NULL;
    }

    return type;
}

void logic_executor_type_free(logic_executor_type_t t) {
    cpe_hash_table_remove_by_ins(&t->m_group->m_types, t);
    if (t->m_ctx_fini) t->m_ctx_fini(t->m_ctx);
    mem_free(t->m_group->m_alloc, (void*)t->m_name);
}

void logic_executor_type_free_all(logic_executor_type_group_t group) {
    struct cpe_hash_it type_it;
    logic_executor_type_t type;

    cpe_hash_it_init(&type_it, &group->m_types);

    type = cpe_hash_it_next(&type_it);
    while(type) {
        logic_executor_type_t next = cpe_hash_it_next(&type_it);
        logic_executor_type_free(type);
        type = next;
    }
}

logic_executor_type_t
logic_executor_type_find(
    logic_executor_type_group_t group,
    const char * name)
{
    struct logic_executor_type key;
    key.m_name = (char*)name;

    return (logic_executor_type_t)cpe_hash_table_find(&group->m_types, &key);
}

const char * logic_executor_type_name(logic_executor_type_t type) {
    return type->m_name;
}

void * logic_executor_type_ctx(logic_executor_type_t type) {
    return type->m_ctx;
}

int logic_executor_type_bind(logic_executor_type_t type, logic_op_fun_t fun, void * ctx, logic_op_ctx_fini_fun_t ctx_fini) {
    assert(type);

    if (type->m_ctx_fini) type->m_ctx_fini(type->m_ctx);

    type->m_op = fun;
    type->m_ctx = ctx;
    type->m_ctx_fini = ctx_fini;

    return 0;
}

uint32_t logic_executor_type_hash(const struct logic_executor_type * type) {
    return cpe_hash_str(type->m_name, strlen(type->m_name));
}

int logic_executor_type_cmp(const struct logic_executor_type * l, const struct logic_executor_type * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

logic_executor_type_t
logic_executor_type_create_global(
    gd_app_context_t app,
    const char * group_name,
    const char * name,
    logic_op_fun_t op_fun,
    void * op_ctx,
    logic_op_ctx_fini_fun_t ctx_fini,
    error_monitor_t em)
{
    logic_executor_type_group_t type_group;
    logic_executor_type_t type;

    if (group_name) {
        type_group = logic_executor_type_group_find_nc(app, group_name);
    }
    else {
        type_group = logic_executor_type_group_default(app);
    }

    if (type_group == NULL) {
        CPE_ERROR(
            em, "logic_executor_type_create_in_group: group %s not exist!",
            group_name ? group_name : "default");
        return NULL;
    }

    type = logic_executor_type_create(type_group, name);
    if (type == NULL) {
        CPE_ERROR(
            em, "logic_executor_type_create_in_group: create %s in group %s fail!",
            name, group_name ? group_name : "default");
        return NULL;
    }

    if (logic_executor_type_bind(type, op_fun, op_ctx, ctx_fini) != 0) {
        CPE_ERROR(
            em, "logic_executor_type_create_in_group: bind for %s in group %s fail!",
            name, group_name ? group_name : "default");
        logic_executor_type_free(type);
        return NULL;
    }        
    else {
        return type;
    }
}

void logic_executor_type_remove_global(gd_app_context_t app, const char * name, logic_op_fun_t op_fun) {
    
}
