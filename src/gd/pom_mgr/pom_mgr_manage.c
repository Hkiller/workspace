#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_types.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/pom_mgr/pom_mgr_manage.h"
#include "pom_mgr_internal_ops.h"

struct nm_node_type s_nm_node_type_pom_manage;

pom_manage_t
pom_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    pom_manage_t mgr;
    nm_node_t mgr_node;

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct pom_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (pom_manage_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_obj_mgr = NULL;
    mgr->m_fini = NULL;

    nm_node_set_type(mgr_node, &s_nm_node_type_pom_manage);

    return mgr;
}

static void pom_manage_clear(nm_node_t node) {
    pom_manage_t mgr;
    mgr = (pom_manage_t)nm_node_data(node);

    pom_manage_obj_mgr_clear(mgr);
}

void pom_manage_free(pom_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_pom_manage) return;
    nm_node_free(mgr_node);
}

pom_manage_t
pom_manage_find(
    gd_app_context_t app,
    cpe_hash_string_t name)
{
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_pom_manage) return NULL;
    return (pom_manage_t)nm_node_data(node);
}

pom_manage_t
pom_manage_find_nc(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_pom_manage) return NULL;
    return (pom_manage_t)nm_node_data(node);
}

gd_app_context_t pom_manage_app(pom_manage_t mgr) {
    return mgr->m_app;
}

const char * pom_manage_name(pom_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
pom_manage_name_hs(pom_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

void pom_manage_obj_mgr_clear(pom_manage_t mgr) {
    if (mgr->m_obj_mgr) {
        if (mgr->m_fini) mgr->m_fini(mgr->m_obj_mgr);
        pom_grp_obj_mgr_free(mgr->m_obj_mgr);
        mgr->m_obj_mgr = NULL;
        mgr->m_fini = NULL;
    }
}

struct nm_node_type s_nm_node_type_pom_manage = {
    "gd_pom_manage",
    pom_manage_clear
};

