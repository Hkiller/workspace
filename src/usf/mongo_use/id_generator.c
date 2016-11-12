#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "usf/mongo_use/id_generator.h"
#include "mongo_use_internal_ops.h"

static void mongo_id_generator_clear(nm_node_t node);
static int mongo_id_generator_gen(gd_id_t * r, gd_id_generator_t gen, const char * id_name);

struct nm_node_type s_nm_node_type_mongo_id_generator = {
    "usf_mongo_id_generator",
    mongo_id_generator_clear
};

mongo_id_generator_t
mongo_id_generator_create(
    gd_app_context_t app,
    const char * name,
    mongo_cli_proxy_t mongo_cli,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    mongo_id_generator_t generator;
    nm_node_t generator_node;

    generator_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct mongo_id_generator));
    if (generator_node == NULL) return NULL;

    generator = (mongo_id_generator_t)nm_node_data(generator_node);
    bzero(generator, sizeof(struct mongo_id_generator));

    generator->m_gen.magic = (uint32_t)GD_ID_GENERATOR_MAGIC;
    generator->m_gen.gen_fun = mongo_id_generator_gen;
    generator->m_gen.app = app;
    generator->m_alloc = alloc;
    generator->m_em = em;
    generator->m_debug = 0;
    generator->m_mongo_cli = mongo_cli;

    if (cpe_hash_table_init(
            &generator->m_id_infos,
            alloc,
            (cpe_hash_fun_t) mongo_id_info_hash,
            (cpe_hash_eq_t) mongo_id_info_eq,
            CPE_HASH_OBJ2ENTRY(mongo_id_info, m_hh),
            -1) != 0)
    {
        nm_node_free(generator_node);
        return NULL;
    }

    nm_node_set_type(generator_node, &s_nm_node_type_mongo_id_generator);

    return generator;
} 

static void mongo_id_generator_clear(nm_node_t node) {
    mongo_id_generator_t generator;

    generator = (mongo_id_generator_t)nm_node_data(node);

    mongo_id_info_free_all(generator);
    cpe_hash_table_fini(&generator->m_id_infos);
}

void mongo_id_generator_free(mongo_id_generator_t generator) {
    nm_node_t generator_node;
    assert(generator);

    generator_node = nm_node_from_data(generator);
    if (nm_node_type(generator_node) != &s_nm_node_type_mongo_id_generator) return;
    nm_node_free(generator_node);
}

mongo_id_generator_t
mongo_id_generator_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_id_generator) return NULL;
    return (mongo_id_generator_t)nm_node_data(node);
}

mongo_id_generator_t
mongo_id_generator_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_id_generator) return NULL;
    return (mongo_id_generator_t)nm_node_data(node);
}

gd_app_context_t mongo_id_generator_app(mongo_id_generator_t generator) {
    return generator->m_gen.app;
}

const char * mongo_id_generator_name(mongo_id_generator_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
mongo_id_generator_name_hs(mongo_id_generator_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

static int mongo_id_generator_gen(gd_id_t * r, gd_id_generator_t gen, const char * id_name) {
    mongo_id_generator_t generator = (mongo_id_generator_t)gen;
    struct mongo_id_info key;
    struct mongo_id_info * id_info;

    key.m_name = id_name;

    id_info = (struct mongo_id_info *)cpe_hash_table_find(&generator->m_id_infos, &key);
    if (id_info == NULL || id_info->m_left_id_count <= 0) return -1;

    *r = id_info->m_next_id++;
    --id_info->m_left_id_count;
    return 0;
}

EXPORT_DIRECTIVE
int mongo_id_generator_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    const char * mongo_cli_name;
    mongo_cli_proxy_t mongo_cli;
    mongo_id_generator_t generator;
    struct cfg_it id_name_it;
    cfg_t id_name_cfg;

    mongo_cli_name = cfg_get_string(cfg, "mongo-cli", NULL);
    if (mongo_cli_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s create: mongo-cli not configured!",
            gd_app_module_name(module));
        return -1;
    }

    mongo_cli = mongo_cli_proxy_find_nc(app, mongo_cli_name);
    if (mongo_cli == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s create: get mongo_cli %s fail!",
            gd_app_module_name(module), mongo_cli_name);
        return -1;
    }

    generator = mongo_id_generator_create(app, gd_app_module_name(module), mongo_cli, gd_app_alloc(app), gd_app_em(app));
    if (generator == NULL) return -1;

    cfg_it_init(&id_name_it, cfg_find_cfg(cfg, "ids"));
    while((id_name_cfg = cfg_it_next(&id_name_it))) {
        const char * id_name = cfg_get_string(id_name_cfg, "name", NULL);
        if (id_name == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s create: regist id, config type error!",
                gd_app_module_name(module));
            return -1;
        }

        if (mongo_id_generator_regist_id(
                generator,
                id_name,
                cfg_get_uint32(id_name_cfg, "start", 1024),
                cfg_get_uint32(id_name_cfg, "inc", 1024))
            != 0)
        {
            CPE_ERROR(
                gd_app_em(app), "%s create: regist id %s error!",
                gd_app_module_name(module), id_name);
            return -1;
        }
    }

    generator->m_debug = cfg_get_int32(cfg, "debug", generator->m_debug);

    return 0;
}

EXPORT_DIRECTIVE
void mongo_id_generator_app_fini(gd_app_context_t app, gd_app_module_t module) {
    mongo_id_generator_t id_generator;

    id_generator = mongo_id_generator_find_nc(app, gd_app_module_name(module));
    if (id_generator) {
        mongo_id_generator_free(id_generator);
    }
}

