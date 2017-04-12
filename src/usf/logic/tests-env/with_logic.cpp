#include <stdexcept>
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/tests-env/with_app.hpp"
#include "cpe/nm/nm_read.h"
#include "cpe/nm/nm_manage.h"
#include "usf/logic/logic_executor_build.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic/logic_executor_mgr.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/tests-env/with_logic.hpp"

namespace usf { namespace logic { namespace testenv {

with_logic::with_logic() {
}

void with_logic::SetUp() {
}

void with_logic::TearDown() {
    nm_mgr_free_nodes_with_type_name(
        envOf<gd::app::testenv::with_app>().t_nm(),
        "usf_logic_manage");

    nm_mgr_free_nodes_with_type_name(
        envOf<gd::app::testenv::with_app>().t_nm(),
        "usf_logic_executor_type_group");
}

logic_manage_t
with_logic::t_logic_manage(const char * name) {
    logic_manage_t mgr = logic_manage_find_nc(envOf<gd::app::testenv::with_app>().t_app(), name);
    if (mgr == NULL) {
        mgr =
            logic_manage_create(
                envOf<gd::app::testenv::with_app>().t_app(),
                NULL,
                name,
                t_allocrator());
        EXPECT_TRUE(mgr) << "logic_manager create fail!";
    }

    return mgr;
}

logic_executor_mgr_t
with_logic::t_logic_executor_mgr_create(const char * name) {
    error_monitor_t em = NULL;
    if (utils::testenv::with_em * with_em = tryEnvOf<utils::testenv::with_em>()) {
        em = with_em->t_em();
    }

    logic_executor_mgr_t executor_mgr = 
        logic_executor_mgr_create(
            envOf<gd::app::testenv::with_app>().t_app(),
            name, 
            t_allocrator(),
            em);

    EXPECT_TRUE(executor_mgr) << "create executor mgr " << name << " fail!";

    return executor_mgr;
}

logic_executor_mgr_t
with_logic::t_logic_executor_mgr_find(const char * name) {
    return logic_executor_mgr_find_nc(
        envOf<gd::app::testenv::with_app>().t_app(),
        name);
}

logic_context_t
with_logic::t_logic_context_create(size_t capacity, logic_require_id_t id) {
    logic_context_t ctx = logic_context_create(t_logic_manage(), id, capacity);
    EXPECT_TRUE(ctx) << "create logic context fail, capacity=" << capacity << ", id=" << id << "!";
    return ctx;
}

logic_context_t
with_logic::t_logic_context_create(const char * cfg, LPDRMETALIB metalib, size_t capacity, logic_require_id_t id) {
    logic_context_t ctx = t_logic_context_create(capacity, id);
    if (ctx) t_logic_context_install_data(ctx, cfg, metalib);
    return ctx; 
}

logic_context_t
with_logic::t_logic_context_create(cfg_t cfg, LPDRMETALIB metalib, size_t capacity, logic_require_id_t id) {
    logic_context_t ctx = t_logic_context_create(capacity, id);
    if (ctx) t_logic_context_install_data(ctx, cfg, metalib);
    return ctx; 
}

void with_logic::t_logic_context_install_data(logic_context_t context, const char * cfg, LPDRMETALIB metalib) {
    t_logic_context_install_data(context, envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(cfg), metalib);
}

void with_logic::t_logic_context_install_data(logic_context_t context, cfg_t cfg, LPDRMETALIB metalib) {
    cfg_t child;
    struct cfg_it data_it;
    cfg_it_init(&data_it, cfg);

    while((child = cfg_it_next(&data_it))) {
        logic_data_t data;
        LPDRMETA meta;
        meta = dr_lib_find_meta_by_name(metalib, cfg_name(child));
        EXPECT_TRUE(meta) << "t_logic_context_install_data: meta " << cfg_name(child) << " not exist!";
        if (meta == 0) continue;

        data = logic_context_data_get_or_create(context, meta, dr_meta_size(meta) + 1024);
        EXPECT_TRUE(meta)
            << "t_logic_context_install_data: meta "
            << cfg_name(child) << ": data create fail, capacity is"
            << (dr_meta_size(meta) + 1024)
            << "!";
        if (data == 0) continue;

        EXPECT_GT(
            dr_cfg_read(
                logic_data_data(data),
                logic_data_capacity(data),
                child,
                meta,
                0,
                NULL),
            0)
            << "t_logic_context_install_data: meta "
            << cfg_name(child) << ": load data error!"
            ;
    }
}

void with_logic::t_logic_execute(logic_context_t context, logic_executor_t executor) {
    EXPECT_TRUE(context) << "context is null!";
    EXPECT_TRUE(executor) << "executor is null!";
    
    EXPECT_EQ(0, logic_context_bind(context, executor)) << "context bind executor fail!";

    logic_context_execute(context);
}

logic_context_t with_logic::t_logic_context_find(logic_context_id_t id) {
    return logic_context_find(t_logic_manage(), id);
}

logic_context_t with_logic::t_logic_context(logic_context_id_t id) {
    logic_context_t ctx = logic_context_find(t_logic_manage(), id);

    EXPECT_TRUE(ctx) << "logic context " << id << " not exist!";
    if (ctx == 0) throw ::std::runtime_error("logic context not exist!");

    return ctx;
}

logic_executor_type_group_t
with_logic::t_logic_executor_type_group(const char * group_name) {
    logic_executor_type_group_t group = NULL;
    if (group_name == 0) {
        group = logic_executor_type_group_default(envOf<gd::app::testenv::with_app>().t_app());
    }
    else {
        group = logic_executor_type_group_find(
            envOf<gd::app::testenv::with_app>().t_app(),
            cpe_hs_create(t_tmp_allocrator(), group_name));
    }

    if (group == NULL) {
        group = logic_executor_type_group_create(envOf<gd::app::testenv::with_app>().t_app(), NULL, t_tmp_allocrator());
    }

    return group;
}

logic_executor_t
with_logic::t_logic_executor_build(cfg_t cfg, const char * group_name, error_monitor_t em) {
    if (em == 0) {
        if (utils::testenv::with_em * with_em = tryEnvOf<utils::testenv::with_em>()) {
            em = with_em->t_em();
        }
    }

    return logic_executor_build(t_logic_manage(), cfg, t_logic_executor_type_group(group_name), em);
}

logic_executor_t
with_logic::t_logic_executor_build(const char * cfg, const char * group_name, error_monitor_t em) {
    return t_logic_executor_build(
        envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(cfg), group_name, em);
}

logic_executor_t
with_logic::t_logic_executor_action_create(const char * name, cfg_t args, const char * group_name) {
    logic_executor_type_t type = 
        logic_executor_type_find(
            t_logic_executor_type_group(group_name),
            name);
    EXPECT_TRUE(type) << "logic op type " << name << " not exist in " << group_name;

    
    return logic_executor_action_create(
        t_logic_manage(),
        type,
        args);
}

logic_executor_t
with_logic::t_logic_executor_action_create(const char * name, const char * args, const char * group_name) {
    return t_logic_executor_action_create(name, envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(args), group_name);
}

const char *
with_logic::t_logic_executor_dump(logic_executor_t executor) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    logic_executor_dump(executor, (write_stream_t)&stream, 0);
    stream_putc((write_stream_t)&stream, 0);

    const char * r =  t_tmp_strdup((const char *)mem_buffer_make_continuous(&buffer, 0));
    mem_buffer_clear(&buffer);

    return r;
}

}}}
