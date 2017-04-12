#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_bson.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "protocol/mongo_cli/mongo_cli.h"
#include "mongo_cli_internal_ops.h"

int mongo_cli_proxy_send(
    mongo_cli_proxy_t agent, mongo_pkg_t pkg, logic_require_t require,
    LPDRMETA result_meta, int result_count_init, const char * result_prefix,
    mongo_cli_pkg_parser parser, void * parser_ctx)
{
    dp_req_t dp_req;

    if (agent->m_outgoing_send_to == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: send: no outgoing_send_to configured",
            mongo_cli_proxy_name(agent));
        goto SEND_ERROR;
    }

    if (strcmp(mongo_pkg_db(pkg), "") == 0) {
        if (agent->m_dft_db[0] == 0) {
            CPE_ERROR(agent->m_em, "%s: send: sending pkg no db name!", mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }

        if (mongo_pkg_set_db(pkg, agent->m_dft_db) != 0) {
            CPE_ERROR(
                agent->m_em, "%s: send: sending pkg set dft db %s fail!",
                mongo_cli_proxy_name(agent), agent->m_dft_db);
            goto SEND_ERROR;
        }
    }

    /*查询请求需要设置好接受返回数据的结构 */
    if (mongo_pkg_op(pkg) == mongo_db_op_query || mongo_pkg_op(pkg) == mongo_db_op_get_more) {
        if (require == NULL) {
            CPE_ERROR(agent->m_em, "%s: send_request: query operation no require!", mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }
 
        if (result_meta == NULL && parser == NULL) {
            CPE_ERROR(agent->m_em, "%s: send_request: query operation no result meta or parser!", mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }

        mongo_pkg_set_id(pkg, logic_require_id(require));
    }

    dp_req = mongo_pkg_to_dp_req(pkg);
    if (dp_dispatch_by_string(agent->m_outgoing_send_to, dp_req_mgr(dp_req), dp_req, agent->m_em) != 0) {
        CPE_INFO(agent->m_em, "%s: send_request: dispatch return fail!", mongo_cli_proxy_name(agent));
        goto SEND_ERROR;
    }

    if (require) {
        struct mongo_cli_require_keep keep_data;
        keep_data.m_parser = parser;
        keep_data.m_parse_ctx = parser_ctx;
        keep_data.m_result_meta = result_meta;
        keep_data.m_result_count_init = result_count_init;
        if (result_prefix) {
            cpe_str_dup(keep_data.m_prefix, sizeof(keep_data.m_prefix), result_prefix);
        }
        else {
            keep_data.m_prefix[0] = 0;
        }

        keep_data.m_cmd = 0;
        if (strcmp(mongo_pkg_collection(pkg), "$cmd") == 0) {
            bson_iter_t bson_it;
            if (mongo_pkg_find(&bson_it, pkg, 0, "findandmodify") == 0) {
                keep_data.m_cmd = MONGO_CMD_FINDANDMODIFY;
            }
        }

        /*查询请求有响应，其他请求需要单独获取响应 */
        if (mongo_pkg_op(pkg) != mongo_db_op_query && mongo_pkg_op(pkg) != mongo_db_op_get_more) {
            mongo_pkg_t cmd_pkg = mongo_cli_proxy_cmd_buf(agent);
            dp_req_t dp_req;

            mongo_pkg_cmd_init(cmd_pkg);
            mongo_pkg_set_db(cmd_pkg, mongo_pkg_db(pkg));

            mongo_pkg_set_id(cmd_pkg, logic_require_id(require));

            mongo_pkg_doc_open(cmd_pkg);
            mongo_pkg_append_int32(cmd_pkg, "getlasterror", 1);
            mongo_pkg_doc_close(cmd_pkg);

            keep_data.m_result_meta = agent->m_meta_lasterror;
            keep_data.m_result_count_init = 1;

            dp_req = mongo_pkg_to_dp_req(cmd_pkg);
            if (dp_dispatch_by_string(agent->m_outgoing_send_to, dp_req_mgr(dp_req), dp_req, agent->m_em) != 0) {
                CPE_INFO(agent->m_em, "%s: send_request: dispatch getLastError return fail!", mongo_cli_proxy_name(agent));
                goto SEND_ERROR;
            }
        }

        if (logic_require_queue_add(agent->m_require_queue, logic_require_id(require), &keep_data, sizeof(keep_data)) != 0) {
            CPE_ERROR(agent->m_em, "%s: send_request: save require id fail!", mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }
    }

    return 0;

SEND_ERROR:
    if (require) logic_require_error(require);
    return -1;
}
