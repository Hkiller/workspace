#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_executor_type.h"
#include "logic_internal_ops.h"

static void logic_executor_type_group_clear(nm_node_t node);

CPE_HS_DEF_VAR(logic_executor_type_group_dft_name, "logic_executor_type_group");

struct nm_node_type s_nm_node_type_logic_executor_type_group = {
    "usf_logic_executor_type_group",
    logic_executor_type_group_clear
};

logic_executor_type_group_t
logic_executor_type_group_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc)
{
    logic_executor_type_group_t group;
    nm_node_t group_node;

    if (name == 0) name = cpe_hs_data(logic_executor_type_group_dft_name);

    group_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct logic_executor_type_group));
    if (group_node == NULL) return NULL;

    group = (logic_executor_type_group_t)nm_node_data(group_node);
    group->m_alloc = alloc;
    group->m_app = app;
    nm_node_set_type(group_node, &s_nm_node_type_logic_executor_type_group);

    if (cpe_hash_table_init(
            &group->m_types,
            alloc,
            (cpe_hash_fun_t) logic_executor_type_hash,
            (cpe_hash_eq_t) logic_executor_type_cmp,
            CPE_HASH_OBJ2ENTRY(logic_executor_type, m_hh),
            -1) != 0)
    {
        nm_node_free(group_node);
        return NULL;
    }

    return group;
}

static void logic_executor_type_group_clear(nm_node_t node) {
    logic_executor_type_group_t group;
    group = (logic_executor_type_group_t)nm_node_data(node);

    logic_executor_type_free_all(group);
    cpe_hash_table_fini(&group->m_types);
}

void logic_executor_type_group_free(logic_executor_type_group_t group) {
    nm_node_t group_node;
    assert(group);

    group_node = nm_node_from_data(group);
    if (nm_node_type(group_node) != &s_nm_node_type_logic_executor_type_group) return;
    nm_node_free(group_node);
}

logic_executor_type_group_t
logic_executor_type_group_find(
    gd_app_context_t app,
    cpe_hash_string_t name)
{
    nm_node_t node;

    if (name == NULL) name = logic_executor_type_group_dft_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_executor_type_group) return NULL;
    return (logic_executor_type_group_t)nm_node_data(node);
}

logic_executor_type_group_t
logic_executor_type_group_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return logic_executor_type_group_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_executor_type_group) return NULL;
    return (logic_executor_type_group_t)nm_node_data(node);
}

logic_executor_type_group_t
logic_executor_type_group_default(gd_app_context_t app) {
    return logic_executor_type_group_find(app, logic_executor_type_group_dft_name);
}

gd_app_context_t logic_executor_type_group_app(logic_executor_type_group_t group) {
    return group->m_app;
}

const char * logic_executor_type_group_name(logic_executor_type_group_t group) {
    return nm_node_name(nm_node_from_data(group));
}

static
logic_executor_type_t
logic_executor_type_group_type_next(struct logic_executor_type_it * it) {
    struct cpe_hash_it * typeIt;

    assert(sizeof(it->m_data) >= sizeof(struct cpe_hash_it));

    typeIt = (struct cpe_hash_it *)(&it->m_data);

    return (logic_executor_type_t)cpe_hash_it_next(typeIt);
}

void logic_executor_type_group_types(logic_executor_type_it_t it, logic_executor_type_group_t group) {
    struct cpe_hash_it * typeIt;

    assert(sizeof(it->m_data) >= sizeof(struct cpe_hash_it));

    typeIt = (struct cpe_hash_it *)(&it->m_data);

    cpe_hash_it_init(typeIt, &group->m_types);
    it->next = logic_executor_type_group_type_next;
}

cpe_hash_string_t
logic_executor_type_group_name_hs(logic_executor_type_group_t group) {
    return nm_node_name_hs(nm_node_from_data(group));
}

EXPORT_DIRECTIVE
int logic_executor_type_group_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    logic_executor_type_group_t logic_executor_type_group;

    logic_executor_type_group = logic_executor_type_group_create(app, gd_app_module_name(module), gd_app_alloc(app));
    if (logic_executor_type_group == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void logic_executor_type_group_app_fini(gd_app_context_t app, gd_app_module_t module) {
    logic_executor_type_group_t logic_executor_type_group;

    logic_executor_type_group = logic_executor_type_group_find_nc(app, gd_app_module_name(module));
    if (logic_executor_type_group) {
        logic_executor_type_group_free(logic_executor_type_group);
    }
}

