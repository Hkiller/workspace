#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_sp.h"
#include "payment_svr_ops.h"
#include "payment_svr_db_ops.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_iap.h"
#include "protocol/svr/apple_iap/svr_apple_iap_pro.h"

int payment_svr_iap_init(payment_svr_adapter_t adapter, cfg_t cfg) {
    return 0;
}

void payment_svr_iap_fini(payment_svr_adapter_t adapter) {
}

int (*payment_svr_adapter_recv_fun_t)(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req);

/* extern char g_metalib_svr_apple_iap_pro[]; */
/* static logic_op_exec_result_t */
/* payment_svr_iap_charge_on_validate_result( */
/*     logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, */
/*     payment_svr_adapter_t adapter, PAYMENT_RECHARGE_RECORD const * recharge_data, SVR_PAYMENT_RES_RECHARGE_COMMIT * res, BAG_INFO * bag_info); */

/* static logic_op_exec_result_t */
/* payment_svr_iap_charge_on_db_insert( */
/*     logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, */
/*     payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info); */

/* static int payment_svr_iap_charge_insert_validate_res( */
/*     payment_svr_t svr, uint64_t user_id, logic_require_t require,  */
/*     const char * receipt, SVR_APPLE_IAP_RES_VALIDATE const * validate_res); */


int payment_svr_iap_charge_send(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
/*     payment_svr_t svr = adapter->m_svr; */
/*     SVR_APPLE_IAP_REQ_VALIDATE apple_iap_req; */
/*     logic_require_t require; */

/*     if (svr->m_iap_svr_type == 0) { */
/*         set_svr_svr_info_t apple_iap_svr_info = set_svr_svr_info_find_by_name(svr->m_stub, "svr_apple_iap"); */
/*         if (apple_iap_svr_info == NULL) { */
/*             APP_CTX_ERROR(logic_context_app(ctx), "%s: iap: can`t find svr_apple_iap svr info!", payment_svr_name(svr)); */
/*             logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*             return logic_op_exec_result_false; */
/*         } */
/*         svr->m_iap_svr_type = set_svr_svr_info_svr_type_id(apple_iap_svr_info); */
/*     } */

/*     if (svr->m_iap_meta_req_validate == NULL) { */
/*         svr->m_iap_meta_req_validate = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_apple_iap_pro, "svr_apple_iap_req_validate"); */
/*         if (svr->m_iap_meta_req_validate == NULL) { */
/*             APP_CTX_ERROR(logic_context_app(ctx), "%s: iap: can`t find req_validate meta!", payment_svr_name(svr)); */
/*             logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*             return logic_op_exec_result_false; */
/*         } */
/*     } */

/*     cpe_str_dup(apple_iap_req.receipt, sizeof(apple_iap_req.receipt), req->receipt); */

/*     require = logic_require_create(stack, "iap_validate"); */
/*     if (require == NULL) { */
/*         APP_CTX_ERROR(logic_context_app(ctx), "%s: iap:: create logic require for validate fail!", payment_svr_name(svr)); */
/*         logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         return logic_op_exec_result_false; */
/*     } */

/*     if (set_logic_sp_send_req_data( */
/*             svr->m_set_sp, svr->m_iap_svr_type, 0, svr->m_iap_meta_req_validate, &apple_iap_req, sizeof(apple_iap_req), NULL, 0, require) */
/*         != 0) */
/*     { */
/*         APP_CTX_ERROR(logic_context_app(ctx), "%s: iap: send request to apple_iap service fail!", payment_svr_name(svr)); */
/*         logic_require_free(require); */
/*         logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         return logic_op_exec_result_false; */
/*     } */

/*     if (logic_require_timeout_start(require, 35 * 1000) != 0 */
/*         || logic_context_timeout_start(ctx, 35 * 1000) != 0) */
/*     { */
/*         APP_CTX_ERROR(logic_context_app(ctx), "%s: iap: set timeout fail!", payment_svr_name(svr)); */
/*         logic_require_free(require); */
/*         logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         return logic_op_exec_result_false; */
/*     } */

    return 0;
}

int payment_svr_iap_charge_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
/*     payment_svr_t svr = adapter->m_svr; */
    
/*     if (strcmp(logic_require_name(require), "iap_validate") == 0) { */
/*         return payment_svr_iap_charge_on_validate_result(ctx, stack, require, adapter, req, res, bag_info); */
/*     } */
/*     else if (strcmp(logic_require_name(require), "iap_db_insert") == 0) { */
/*         return payment_svr_iap_charge_on_db_insert(ctx, stack, require, svr, req, res, bag_info); */
/*     } */
/*     else { */
/*         APP_CTX_ERROR(logic_context_app(ctx), "%s: iap: recv: unknown require %s!", payment_svr_name(svr), logic_require_name(require)); */
/*         logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         return logic_op_exec_result_false; */
/*     } */

/*     return logic_op_exec_result_true; */
/* } */

/* logic_op_exec_result_t */
/* payment_svr_iap_charge_on_validate_result( */
/*     logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, */
/*     payment_svr_adapter_t adapter, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info) */
/* { */
/*     payment_svr_t svr = adapter->m_svr; */
/*     logic_data_t validate_res_data; */
/*     SVR_APPLE_IAP_RES_VALIDATE * validate_res; */
/*     logic_require_t insert_require; */
/*     logic_data_t bill_data; */
/*     PAYMENT_BILL_DATA * bill; */
/*     uint16_t i; */

/*     if (logic_require_state(require) != logic_require_state_done) { */
/*         if (logic_require_state(require) == logic_require_state_error) { */
/*             CPE_ERROR( */
/*                 svr->m_em, "%s: iap_on_validate_result: require state error, errno=%d!", */
/*                 payment_svr_name(svr), logic_require_error(require)); */

/*             switch(logic_require_error(require)) { */
/*             case SVR_APPLE_IAP_ERRNO_VALIDATE_TIMEOUT: */
/*                 logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_TIMEOUT); */
/*                 break; */
/*             default: */
/*                 logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*                 break; */
/*             } */
/*         } */
/*         else if (logic_require_state(require) == logic_require_state_timeout) { */
/*             CPE_ERROR(svr->m_em, "%s: iap_on_validate_result: require timeout!", payment_svr_name(svr)); */
/*             res->result = SVR_PAYMENT_ERRNO_RECHARGE_TIMEOUT; */
/*             res->service_result = 0; */
/*         } */
/*         else { */
/*             CPE_ERROR( */
/*                 svr->m_em, "%s: iap_on_validate_result: require state error, state = %s!", */
/*                 payment_svr_name(svr), logic_require_state_name(logic_require_state(require))); */
/*             logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         } */

/*         return logic_op_exec_result_false; */
/*     } */

/*     validate_res_data = logic_require_data_find(require, "svr_apple_iap_res_validate"); */
/*     if (validate_res_data == NULL) { */
/*         CPE_ERROR(svr->m_em, "%s: iap_on_validate_result: can`t find validate result data!", payment_svr_name(svr)); */
/*         logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         return logic_op_exec_result_false; */
/*     } */
/*     validate_res = logic_data_data(validate_res_data); */
/*     assert(validate_res); */

/*     if (validate_res->status != 0) { */
/*         CPE_ERROR( */
/*             svr->m_em, "%s: iap_on_validate_result: ipa validate return status %d, not success!", */
/*             payment_svr_name(svr), validate_res->status); */
/*         res->result = SVR_PAYMENT_ERRNO_RECHARGE_IAP_ERROR; */
/*         res->service_result = validate_res->status; */
/*         return logic_op_exec_result_false; */
/*     } */

/*     /\*将支付信息插入数据库，插入结果判断是否已经支付 *\/ */
/*     insert_require = logic_require_create(stack, "iap_db_insert"); */
/*     if (insert_require == NULL) { */
/*         APP_CTX_ERROR(logic_context_app(ctx), "%s: iap_on_validate_result: create logic require for insert fail!", payment_svr_name(svr)); */
/*         logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         return logic_op_exec_result_false; */
/*     } */

/*     if (payment_svr_iap_charge_insert_validate_res( */
/*             svr, req->user_id, insert_require, req->receipt, validate_res) */
/*         != 0) */
/*     { */
/*         logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         logic_require_free(insert_require); */
/*         return logic_op_exec_result_false; */
/*     } */

    return 0;
}

/* static logic_op_exec_result_t */
/* payment_svr_iap_charge_on_db_insert( */
/*     logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, */
/*     payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE const * req, SVR_PAYMENT_RES_RECHARGE * res, BAG_INFO * bag_info) */
/* { */
/*     if (logic_require_state(require) != logic_require_state_done) { */
/*         if (logic_require_state(require) == logic_require_state_error) { */
/*             if (logic_require_error(require) == mongo_data_error_duplicate_key) { */
/*                 CPE_ERROR( */
/*                     svr->m_em, "%s: %s: already recharged!", */
/*                     payment_svr_name(svr), logic_require_name(require)); */
/*                 logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_PROCESSED); */
/*             } */
/*             else { */
/*                 CPE_ERROR( */
/*                     svr->m_em, "%s: %s: require state error, errno=%d, db error!", */
/*                     payment_svr_name(svr), logic_require_name(require), logic_require_error(require)); */
/*                 logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*             } */
/*         } */
/*         else { */
/*             CPE_ERROR( */
/*                 svr->m_em, "%s: %s: db error %d!", */
/*                 payment_svr_name(svr), logic_require_name(require), logic_require_error(require)); */
/*             logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL); */
/*         } */

/*         return logic_op_exec_result_false; */
/*     } */

/*     return payment_svr_op_recharge_db_update(ctx, stack, svr, req, bag_info); */
/* } */

/* static int payment_svr_iap_charge_insert_validate_res( */
/*     payment_svr_t svr, uint64_t user_id, logic_require_t require, */
/*     const char * receipt, SVR_APPLE_IAP_RES_VALIDATE const * validate_res) */
/* { */
/*     mongo_pkg_t db_pkg; */
/*     int pkg_r; */

/*     db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db); */
/*     if (db_pkg == NULL) { */
/*         CPE_ERROR(svr->m_em, "%s: iap_insert_validate_res: get db pkg fail!", payment_svr_name(svr)); */
/*         return -1; */
/*     } */

/*     mongo_pkg_init(db_pkg); */
/*     mongo_pkg_set_collection(db_pkg, "payment_iap_record"); */
/*     mongo_pkg_set_op(db_pkg, mongo_db_op_insert); */

/*     pkg_r = 0; */
/*     pkg_r |= mongo_pkg_doc_open(db_pkg); */

/*     pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)validate_res->receipt.original_transaction_id); */
/*     pkg_r |= mongo_pkg_append_int64(db_pkg, "purchase_date", (int64_t)validate_res->receipt.original_purchase_date_ms); */
/*     pkg_r |= mongo_pkg_append_int32(db_pkg, "status", validate_res->status); */
/*     pkg_r |= mongo_pkg_append_int32(db_pkg, "quantity", validate_res->receipt.quantity); */
/*     pkg_r |= mongo_pkg_append_string(db_pkg, "product_id", validate_res->receipt.product_id); */
/*     pkg_r |= mongo_pkg_append_int64(db_pkg, "item_id", validate_res->receipt.item_id); */
/*     pkg_r |= mongo_pkg_append_int32(db_pkg, "version_external_identifier", (int32_t)validate_res->receipt.version_external_identifier); */
/*     pkg_r |= mongo_pkg_append_string(db_pkg, "bid", validate_res->receipt.bid); */
/*     pkg_r |= mongo_pkg_append_string(db_pkg, "bvrs", validate_res->receipt.bvrs); */

/*     pkg_r |= mongo_pkg_doc_close(db_pkg); */

/*     if (pkg_r) { */
/*         CPE_ERROR(svr->m_em, "%s: iap_insert_validate_res: build db pkg fail!", payment_svr_name(svr)); */
/*         return -1; */
/*     } */

/*     if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, NULL) != 0) { */
/*         CPE_ERROR(svr->m_em, "%s: iap_insert_validate_res: send db request fail!", payment_svr_name(svr)); */
/*         return -1; */
/*     } */

/*     return 0; */
/* } */
