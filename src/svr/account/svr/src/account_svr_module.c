#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/net_trans/net_trans_manage.h"
#include "usf/mongo_use/id_generator.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_sp.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "account_svr_module.h"
#include "account_svr_conn_info.h"
#include "account_svr_backend.h"

extern char g_metalib_svr_account_pro[];

static int account_svr_app_load_user_infos(account_svr_t account_svr, cfg_t cfg, uint8_t user_state);

EXPORT_DIRECTIVE
int account_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    set_logic_sp_t set_sp;
    account_svr_t account_svr;
    set_logic_rsp_manage_t rsp_manage;
    mongo_cli_proxy_t db;
    mongo_id_generator_t id_generator;
    net_trans_manage_t trans_mgr;
    
    trans_mgr = net_trans_manage_find_nc(app, cfg_get_string(cfg, "trans-mgr", NULL));
    if (trans_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: trans-mgr %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "trans-mgr", "default"));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    set_sp = set_logic_sp_find_nc(app, cfg_get_string(cfg, "set-sp", NULL));
    if (set_sp == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-sp %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-sp", "default"));
        return -1;
    }

    rsp_manage = set_logic_rsp_manage_find_nc(app, cfg_get_string(cfg, "rsp-manage", NULL));
    if (rsp_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: rsp-manage %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "rsp-manage", "default"));
        return -1;
    }

    db = mongo_cli_proxy_find_nc(app, cfg_get_string(cfg, "db", NULL));
    if (db == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: db %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "db", "default"));
        return -1;
    }

    id_generator = mongo_id_generator_find_nc(app, "id-generator");
    if (id_generator == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: add: get id-generate fail!", gd_app_module_name(module));
        return -1;
    }

    account_svr =
        account_svr_create(
            app, gd_app_module_name(module),
            stub, set_sp, rsp_manage, db, id_generator,
            trans_mgr,
            gd_app_alloc(app), gd_app_em(app));
    if (account_svr == NULL) return -1;

    if (set_logic_rsp_build(
            account_svr->m_rsp_manage,
            cfg_find_cfg(gd_app_cfg(app), "rsps"), (LPDRMETALIB)g_metalib_svr_account_pro, gd_app_em(app)) != 0)
    {
        CPE_ERROR(gd_app_em(app), "%s: create: load rsps fail!", gd_app_module_name(module));
        account_svr_free(account_svr);
        return -1;
    }

    if (account_svr_app_load_user_infos(account_svr, cfg_find_cfg(gd_app_cfg(app), "accounts.internal"), SVR_ACCOUNT_STATE_INTERNAL) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load accounts.internal fail!", gd_app_module_name(module));
        account_svr_free(account_svr);
        return -1;
    }

    if (account_svr_app_load_user_infos(account_svr, cfg_find_cfg(gd_app_cfg(app), "accounts.testing"), SVR_ACCOUNT_STATE_TEST) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load accounts.testing fail!", gd_app_module_name(module));
        account_svr_free(account_svr);
        return -1;
    }

    if (account_svr_app_load_backends(account_svr, cfg_find_cfg(gd_app_cfg(app), "backends")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load backends fail!", gd_app_module_name(module));
        account_svr_free(account_svr);
        return -1;
    }
    
    account_svr->m_debug = cfg_get_int8(cfg, "debug", account_svr->m_debug);

    if (account_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void account_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    account_svr_t account_svr;

    account_svr = account_svr_find_nc(app, gd_app_module_name(module));
    if (account_svr) {
        account_svr_free(account_svr);
    }
}

static int account_svr_app_load_user_infos(account_svr_t account_svr, cfg_t cfg, uint8_t user_state) {
    struct cfg_it child_it;
    cfg_t child;

    cfg_it_init(&child_it, cfg);

    while((child = cfg_it_next(&child_it))) {
        const char * account_type;
        const char * account;
        SVR_ACCOUNT_LOGIC_ID logic_id;
        
        child = cfg_child_only(child);

        account_type = cfg_name(child);
        logic_id.account_type = account_svr_account_type_from_str(account_type);
        if (logic_id.account_type == 0) {
            CPE_ERROR(account_svr->m_em, "account_svr_app_load_user_infos: account %s not exist", account_type);
            return -1;
        }

        account = cfg_as_string(child, NULL);
        if (account == NULL) {
            CPE_ERROR(account_svr->m_em, "account_svr_app_load_user_infos: account not configured");
            return -1;
        }
        cpe_str_dup(logic_id.account, sizeof(logic_id.account), account);
        
        if (account_svr_account_info_create(account_svr, &logic_id, user_state) == NULL) {
            return -1;
        }

        CPE_INFO(account_svr->m_em, "account_svr_app_load_user_infos: %s: %s", account_type, account);
    }

    return 0;
}

uint8_t account_svr_account_type_from_str(const char * account_type) {
    if (strcmp(account_type, "email") == 0) {
        return SVR_ACCOUNT_EMAIL;
    }
    else if (strcmp(account_type, "device") == 0) {
        return SVR_ACCOUNT_DEVICE;
    }
    else if (strcmp(account_type, "qq") == 0) {
        return SVR_ACCOUNT_QQ;
    }
    else if (strcmp(account_type, "weixin") == 0) {
        return SVR_ACCOUNT_WEIXIN;
    }
    else if (strcmp(account_type, "qihoo") == 0) {
        return SVR_ACCOUNT_QIHOO;
    }
    else if (strcmp(account_type, "damai") == 0) {
        return SVR_ACCOUNT_DAMAI;
    }
    else {
        return 0;
    }
}
