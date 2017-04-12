#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_error.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/evt/evt_manage.h"
#include "gd/evt/evt_read.h"
#include "evt_internal_ops.h"

cpe_hash_string_t s_gd_evt_mgr_default_name;
struct nm_node_type s_nm_node_type_gd_evt_mgr;
static void gd_evt_mgr_dispatch_evt(tl_event_t input, void * context);
static int gd_evt_mgr_apply_evt(dp_req_t req, void * ctx, error_monitor_t em);

gd_evt_mgr_t
gd_evt_mgr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    gd_evt_mgr_t mgr;
    nm_node_t mgr_node;

    if (name == 0) name = cpe_hs_data(s_gd_evt_mgr_default_name);

    if (em == NULL) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct gd_evt_mgr));
    if (mgr_node == NULL) return NULL;

    mgr = (gd_evt_mgr_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_req = NULL;
    mgr->m_oid_max_len = 32;
    mgr->m_carry_size = 0;
    mgr->m_carry_meta = NULL;
    mgr->m_processor_count_in_page = 2048;
    mgr->m_processor_page_count = 0;
    mgr->m_processor_page_capacity = 0;
    mgr->m_processor_buf = NULL;

    if (cpe_range_mgr_init(&mgr->m_ids, alloc) != 0) {
        CPE_ERROR(em, "gd_evt_mgr_create: init range mgr!");
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_req = dp_req_create(gd_app_dp_mgr(app), 0);
    if (mgr->m_req == NULL) {
        CPE_ERROR(em, "gd_evt_mgr_create: create req fail!");
        cpe_range_mgr_fini(&mgr->m_ids);
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_tl = tl_create(gd_app_tl_mgr(app));
    if (mgr->m_tl == NULL) {
        CPE_ERROR(em, "gd_evt_mgr_create: create tl fail!");
        dp_req_free(mgr->m_req);
        cpe_range_mgr_fini(&mgr->m_ids);
        nm_node_free(mgr_node);
        return NULL;
    }

    tl_set_opt(mgr->m_tl, tl_set_event_dispatcher, gd_evt_mgr_dispatch_evt);
    tl_set_opt(mgr->m_tl, tl_set_event_op_context, mgr);

    if (cpe_hash_table_init(
            &mgr->m_responser_to_processor,
            alloc,
            (cpe_hash_fun_t) gd_evt_processor_hash_fun,
            (cpe_hash_eq_t) gd_evt_processor_cmp_fun,
            CPE_HASH_OBJ2ENTRY(gd_evt_processor, m_hh_for_responser_to_processor),
            -1) != 0)
    {
        CPE_ERROR(em, "gd_evt_mgr_create: init responser hash table fail!");
        dp_req_free(mgr->m_req);
        tl_free(mgr->m_tl);
        cpe_range_mgr_fini(&mgr->m_ids);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_evt_defs,
            mgr->m_alloc,
            (cpe_hash_fun_t) gd_evt_def_hash,
            (cpe_hash_eq_t) gd_evt_def_eq,
            CPE_HASH_OBJ2ENTRY(gd_evt_def, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "gd_evt_mgr_create: create: init evt def hash hash table fail!");
        cpe_hash_table_fini(&mgr->m_responser_to_processor);
        dp_req_free(mgr->m_req);
        tl_free(mgr->m_tl);
        cpe_range_mgr_fini(&mgr->m_ids);
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_gd_evt_mgr);

    return mgr;
}

static void gd_evt_mgr_evt_def_free_all(gd_evt_mgr_t mgr) {
    struct cpe_hash_it evt_def_it;
    struct gd_evt_def * evt_def;

    cpe_hash_it_init(&evt_def_it, &mgr->m_evt_defs);

    evt_def = cpe_hash_it_next(&evt_def_it);
    while(evt_def) {
        struct gd_evt_def * next = cpe_hash_it_next(&evt_def_it);

        cpe_hash_table_remove_by_ins(&mgr->m_evt_defs, evt_def);
        mem_free(mgr->m_alloc, evt_def);
        
        evt_def = next;
    }
}

static void gd_evt_mgr_clear(nm_node_t node) {
    gd_evt_mgr_t mgr;
    mgr = (gd_evt_mgr_t)nm_node_data(node);

    gd_evt_mgr_free_processor_buf(mgr);

    cpe_range_mgr_fini(&mgr->m_ids);

    if (mgr->m_req) dp_req_free(mgr->m_req);

    tl_free(mgr->m_tl);

    cpe_hash_table_fini(&mgr->m_responser_to_processor);

    gd_evt_mgr_evt_def_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_evt_defs);
}

void gd_evt_mgr_free(gd_evt_mgr_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_gd_evt_mgr) return;
    nm_node_free(mgr_node);
}

gd_evt_mgr_t gd_evt_mgr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) name = s_gd_evt_mgr_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gd_evt_mgr) return NULL;
    return (gd_evt_mgr_t)nm_node_data(node);
}

gd_evt_mgr_t gd_evt_mgr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;
    if (name == NULL) return gd_evt_mgr_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gd_evt_mgr) return NULL;
    return (gd_evt_mgr_t)nm_node_data(node);
}

gd_evt_mgr_t gd_evt_mgr_default(gd_app_context_t app) {
    return gd_evt_mgr_find(app, s_gd_evt_mgr_default_name);
}

gd_app_context_t gd_evt_mgr_app(gd_evt_mgr_t mgr) {
    return mgr->m_app;
}

const char * gd_evt_mgr_name(gd_evt_mgr_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
gd_evt_mgr_name_hs(gd_evt_mgr_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int gd_evt_mgr_register_evt(gd_evt_mgr_t mgr, LPDRMETA cmd_meta) {
    struct gd_evt_def * evt_def;


    evt_def = mem_alloc(mgr->m_alloc, sizeof(struct gd_evt_def));
    if (evt_def == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: register evt %s, alloc fail!", 
            gd_evt_mgr_name(mgr), dr_meta_name(cmd_meta));
        return -1;
    }

    evt_def->m_evt_name = dr_meta_name(cmd_meta);
    evt_def->m_evt_meta = cmd_meta;
    cpe_hash_entry_init(&evt_def->m_hh);

    if (cpe_hash_table_insert_unique(&mgr->m_evt_defs, evt_def) != 0) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: register evt %s, event already exist!",
            gd_evt_mgr_name(mgr), dr_meta_name(cmd_meta));
        mem_free(mgr->m_alloc, evt_def);
        return -1;
    }

    return 0;
}

int gd_evt_mgr_register_evt_in_metalib(gd_evt_mgr_t mgr, LPDRMETALIB metalib) {
    int rv;
    int i, count;

    rv = 0;
    count = dr_lib_meta_num(metalib);
    for(i = 0; i < count; ++i) {
        if (gd_evt_mgr_register_evt(mgr, dr_lib_meta_at(metalib, i)) != 0) {
            rv = -1;
        }
    }

    return rv;
}

tl_t gd_evt_mgr_tl(gd_evt_mgr_t mgr) {
    return mgr->m_tl;
}

void gd_evg_mgr_set_carry_info(gd_evt_mgr_t mgr, LPDRMETA carry_meta, size_t carry_size) {
    mgr->m_carry_size = carry_size;
    mgr->m_carry_meta = carry_meta;
}

int gd_evt_mgr_regist_responser(
    gd_evt_mgr_t mgr, 
    evt_processor_id_t * id,
    const char * oid, gd_evt_process_fun_t fun, void * ctx,
    void * arg, void (*arg_fini)(void *))
{
    evt_processor_id_t newProcessorId;
    struct gd_evt_processor * newProcessorData;
    char rspNameBuf[128];
    dp_rsp_t rsp;

    if (gd_evt_processor_alloc(mgr, &newProcessorId) != 0) {
        return -1;
    }

    newProcessorData = gd_evt_processor_get(mgr, newProcessorId);
    assert(newProcessorData);
    assert(newProcessorData->m_process_ctx == NULL);
    assert(newProcessorData->m_state == evt_processor_state_NotInResponserHash);

    newProcessorData->m_process_ctx = ctx;
    newProcessorData->m_process_arg = arg;
    newProcessorData->m_process_arg_free = arg_fini;
    newProcessorData->m_process_fun = fun;

    if (cpe_hash_table_insert(&mgr->m_responser_to_processor, newProcessorData) != 0) {
        gd_evt_processor_free_basic(mgr, newProcessorData);
        gd_evt_processor_free_id(mgr, newProcessorId);
        CPE_ERROR(
            mgr->m_em, "%s: regist processor: insert to responser processor list fail!",
            gd_evt_mgr_name(mgr));
        return -1;
    }
    newProcessorData->m_state = evt_processor_state_InResponserHash;

    snprintf(rspNameBuf, sizeof(rspNameBuf), "%s.rsp.%d", gd_evt_mgr_name(mgr), newProcessorId);
    rsp = dp_rsp_create(gd_app_dp_mgr(mgr->m_app), rspNameBuf);
    if (rsp == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: regist processor: create rsp fail!",
            gd_evt_mgr_name(mgr));
        gd_evt_processor_free_basic(mgr, newProcessorData);
        gd_evt_processor_free_id(mgr, newProcessorId);
        return -1;
    }

    dp_rsp_set_processor(rsp, gd_evt_mgr_apply_evt, newProcessorData);

    if (dp_rsp_bind_string(rsp, oid, mgr->m_em) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: regist processor: bind rsp to target %s!",
            gd_evt_mgr_name(mgr), oid);
        dp_rsp_free(rsp);
        gd_evt_processor_free_basic(mgr, newProcessorData);
        gd_evt_processor_free_id(mgr, newProcessorId);
        return -1;
    }

    if (id) *id = newProcessorId;

    return 0;
}

void gd_evt_mgr_unregist_responser(gd_evt_mgr_t mgr, void * ctx) {
    struct gd_evt_processor key;
    struct gd_evt_processor * node;
    char rspNameBuf[128];
    dp_rsp_t rsp;

    key.m_process_ctx = ctx;

    node = (struct gd_evt_processor *)cpe_hash_table_find(&mgr->m_responser_to_processor, &key);
    while(node) {
        struct gd_evt_processor * next = 
            cpe_hash_table_find_next(&mgr->m_responser_to_processor, node);
        assert(node->m_process_ctx);


        snprintf(rspNameBuf, sizeof(rspNameBuf), "%s.rsp.%d", gd_evt_mgr_name(mgr), node->m_id);
        rsp = dp_rsp_find_by_name(gd_app_dp_mgr(mgr->m_app), rspNameBuf);
        if (rsp) dp_rsp_free(rsp);

        gd_evt_processor_free_id(mgr, node->m_id);
        gd_evt_processor_free_basic(mgr, node);

        node = next;
    }
}

static int gd_evt_mgr_apply_evt(dp_req_t req, void * ctx, error_monitor_t em) {
    struct gd_evt_processor * pn = (struct gd_evt_processor *)ctx;
    tl_event_t input = (tl_event_t)dp_req_data(req);
    
    pn->m_process_fun(gd_evt_cvt(input), pn->m_process_ctx, pn->m_process_arg);

    return 0;
}

static void gd_evt_mgr_dispatch_evt(tl_event_t input, void * context) {
    gd_evt_mgr_t mgr;
    gd_evt_t evt = NULL;

    mgr = (gd_evt_mgr_t)context;
    assert(mgr);

    evt = gd_evt_cvt(input);
    if (evt == NULL) {
        CPE_ERROR(mgr->m_em, "%s: dispatch evt: cast to gd_evt fail!", gd_evt_mgr_name(mgr));
        return;
    }

    dp_req_set_buf(mgr->m_req, (void*)input, (size_t)0);

    if (dp_dispatch_by_string(gd_evt_target_hs(evt), dp_req_mgr(mgr->m_req), mgr->m_req, mgr->m_em) != 0) {
        struct write_stream_error stream = CPE_WRITE_STREAM_ERROR_INITIALIZER(mgr->m_em);
        stream_printf((write_stream_t)&stream, "%s: dispatch success, event=", gd_evt_mgr_name(mgr));
        gd_evt_dump((write_stream_t)&stream, evt);
    }
    else {
        if (mgr->m_debug) {
            struct write_stream_error stream = CPE_WRITE_STREAM_ERROR_INITIALIZER(mgr->m_em);
            stream_printf((write_stream_t)&stream, "%s: dispatch success, event=", gd_evt_mgr_name(mgr));
            gd_evt_dump((write_stream_t)&stream, evt);
        }
    }
}


uint32_t gd_evt_def_hash(const struct gd_evt_def * o) {
    return cpe_hash_str(o->m_evt_name, strlen(o->m_evt_name));
}

int gd_evt_def_eq(const struct gd_evt_def * l, const struct gd_evt_def * r) {
    return strcmp(l->m_evt_name, r->m_evt_name) == 0;
}

CPE_HS_DEF_VAR(s_gd_evt_mgr_default_name, "gd_evt_mgr");

struct nm_node_type s_nm_node_type_gd_evt_mgr = {
    "gd_evt_mgr",
    gd_evt_mgr_clear
};
