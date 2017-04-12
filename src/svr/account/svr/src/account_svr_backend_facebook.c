#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "account_svr_backend_facebook.h"

int account_svr_backend_facebook_init(account_svr_backend_t backend, cfg_t cfg) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_facebook_t facebook = account_svr_backend_data(backend);
    const char * str_value;

    facebook->m_url = "https://graph.facebook.com";
    facebook->m_exchange_token = 0;
    
    /*app-id*/
    str_value = cfg_get_string(cfg, "app-id", NULL);
    if (str_value == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_facebook_init: app-id not configured!");
        return -1;
    }
    
    facebook->m_app_id = cpe_str_mem_dup(svr->m_alloc, str_value);
    if (facebook->m_app_id == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_facebook_init: dup app-id %s fail!", str_value);
        return -1;
    }

    /*app-secret */
    str_value = cfg_get_string(cfg, "app-secret", NULL);
    if (str_value == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_facebook_init: app-id not configured!");
        return -1;
    }
    
    facebook->m_app_secret = cpe_str_mem_dup(svr->m_alloc, str_value);
    if (facebook->m_app_secret == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_facebook_init: dup app-id %s fail!", str_value);
        return -1;
    }
    
    /*trans group*/
    facebook->m_trans_group = net_trans_group_create(svr->m_trans_mgr, "facebook");
    if (facebook->m_trans_group == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_facebook_init: crate trans group fail!");
        mem_free(svr->m_alloc, facebook->m_app_id);
        facebook->m_app_id = NULL;

        mem_free(svr->m_alloc, facebook->m_app_secret);
        facebook->m_app_secret = NULL;
        return -1;
    }
    
    return 0;
}

void account_svr_backend_facebook_fini(account_svr_backend_t backend) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_facebook_t facebook = account_svr_backend_data(backend);

    mem_free(svr->m_alloc, facebook->m_app_id);
    facebook->m_app_id = NULL;

    mem_free(svr->m_alloc, facebook->m_app_secret);
    facebook->m_app_secret = NULL;
    
    net_trans_group_free(facebook->m_trans_group);
    facebook->m_trans_group = NULL;
}

int account_svr_backend_facebook_check_error(account_svr_backend_t backend, yajl_val data_tree, const char * op) {
    yajl_val val;
    
    if ((val = yajl_tree_get_2(data_tree, "error", yajl_t_object))) {
        yajl_val code = yajl_tree_get_2(val, "code", yajl_t_number);
        yajl_val msg = yajl_tree_get_2(val, "message", yajl_t_string);
        
        if (code == NULL || msg == NULL) {
            CPE_ERROR(
                backend->m_svr->m_em, "%s: facebook: %s: parse result: get error fail",
                account_svr_name(backend->m_svr), op);
        }
        else {
            CPE_ERROR(
                backend->m_svr->m_em, "%s: facebook: %s: parse result: server return error: %d (%s)", 
                account_svr_name(backend->m_svr), op, (int)yajl_get_integer(code), yajl_get_string(msg));
        }

        return -1;
    }

    return 0;
}
