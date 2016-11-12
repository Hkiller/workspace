#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "payment_svr_db_data_ops.h"
#include "payment_svr_db_ops.h"

int payment_svr_db_send_query_money(payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require, uint64_t user_id) {
    int pkg_r;
    mongo_pkg_t db_pkg;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "payment_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= payment_svr_mongo_pkg_append_id(db_pkg, user_id, bag_info->bag_id);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    /*需要的列 */
    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= payment_svr_mongo_pkg_append_required_moneies(db_pkg, bag_info->money_type_count);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_meta_data_list, 1, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int payment_svr_db_send_add_money(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff)
{
    mongo_pkg_t db_pkg;
    int pkg_r;
    uint8_t i;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_cmd_init(db_pkg);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_open(db_pkg);

    pkg_r |= mongo_pkg_append_string(db_pkg, "findandmodify", "payment_data");

    pkg_r |= mongo_pkg_append_start_object(db_pkg, "query");
    pkg_r |= payment_svr_mongo_pkg_append_id(db_pkg, user_id, bag_info->bag_id);
    pkg_r |= mongo_pkg_append_finish_object(db_pkg);

    pkg_r |= mongo_pkg_append_int32(db_pkg, "new", 1);

    pkg_r |= mongo_pkg_append_start_object(db_pkg, "update");
    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$inc");
    for(i = 0; i < diff->count; ++i) {
        char buf[64];
        SVR_PAYMENT_MONEY const * money = diff->datas + i;

        if (money->type < PAYMENT_MONEY_TYPE_MIN
            || money->type >= PAYMENT_MONEY_TYPE_MAX
            || (money->type - PAYMENT_MONEY_TYPE_MIN) >= bag_info->money_type_count)
        {
            CPE_ERROR(
                svr->m_em, "%s: %s: money type %d error!",
                payment_svr_name(svr), logic_require_name(require), money->type);
            return -1;
        }

        snprintf(buf, sizeof(buf), "money%d", money->type);
        pkg_r |= mongo_pkg_append_int64(db_pkg, buf, (int64_t)money->count);
    }
    pkg_r |= mongo_pkg_append_finish_object(db_pkg); /*$inc */
    pkg_r |= mongo_pkg_append_finish_object(db_pkg); /*update*/
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: %s: build db pkg fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_meta_data_list, 1, NULL, NULL, NULL) != 0) {
        CPE_ERROR(svr->m_em, "%s: %s: send db request fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int payment_svr_db_send_remove_money(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff)
{
    mongo_pkg_t db_pkg;
    int pkg_r;
    uint8_t i;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_cmd_init(db_pkg);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_open(db_pkg);

    pkg_r |= mongo_pkg_append_string(db_pkg, "findandmodify", "payment_data");

    pkg_r |= mongo_pkg_append_start_object(db_pkg, "query");
    pkg_r |= payment_svr_mongo_pkg_append_id(db_pkg, user_id, bag_info->bag_id);
    for(i = 0; i < diff->count; ++i) {
        char buf[64];
        SVR_PAYMENT_MONEY const * money = diff->datas + i;

        snprintf(buf, sizeof(buf), "money%d", money->type);
        pkg_r |= mongo_pkg_append_start_object(db_pkg, buf);
        pkg_r |= mongo_pkg_append_int64(db_pkg, "$gte", (int64_t)money->count);
        pkg_r |= mongo_pkg_append_finish_object(db_pkg);
    }
    pkg_r |= mongo_pkg_append_finish_object(db_pkg);

    pkg_r |= mongo_pkg_append_int32(db_pkg, "new", 1);

    pkg_r |= mongo_pkg_append_start_object(db_pkg, "update");
    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$inc");
    for(i = 0; i < diff->count; ++i) {
        char buf[64];
        SVR_PAYMENT_MONEY const * money = diff->datas + i;
        snprintf(buf, sizeof(buf), "money%d", money->type);
        pkg_r |= mongo_pkg_append_int64(db_pkg, buf, -(int64_t)money->count);
    }
    pkg_r |= mongo_pkg_append_finish_object(db_pkg); /*$inc */
    pkg_r |= mongo_pkg_append_finish_object(db_pkg); /*update*/
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: %s: build db pkg fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_meta_data_list, 1, NULL, NULL, NULL) != 0) {
        CPE_ERROR(svr->m_em, "%s: %s: send db request fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int payment_svr_db_send_init_money(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff)
{
    mongo_pkg_t db_pkg;
    int pkg_r;
    uint8_t i;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "payment_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= payment_svr_mongo_pkg_append_id(db_pkg, user_id, bag_info->bag_id);

    pkg_r |= mongo_pkg_append_int64(db_pkg, "user_id", (int64_t)user_id);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "bag_id", bag_info->bag_id);

    for(i = 0; i < bag_info->money_type_count && i < PAYMENT_MONEY_TYPE_COUNT; ++i) {
        char buf[64];
        uint8_t money_type = i + PAYMENT_MONEY_TYPE_MIN;

        snprintf(buf, sizeof(buf), "money%d", money_type);
        pkg_r |= mongo_pkg_append_int64(db_pkg, buf, (int64_t)payment_svr_get_count_by_type(diff, money_type, 0));
    }
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: %s: build db pkg fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_meta_data_list, 1, NULL, NULL, NULL) != 0) {
        CPE_ERROR(svr->m_em, "%s: %s: send db request fail!", payment_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int payment_svr_db_build_balance(payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require, SVR_PAYMENT_MONEY_GROUP * balance) {
    logic_data_t query_result_data;
    PAYMENT_DATA_LIST * query_result;
    
    query_result_data = logic_require_data_find(require, dr_meta_name(svr->m_meta_data_list));
    if (query_result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: find query result!",
            payment_svr_name(svr), logic_require_name(require));
        return -1;
    }
    query_result = logic_data_data(query_result_data);

    if (query_result->count > 1) {
        CPE_ERROR(
            svr->m_em, "%s: %s: query result count %d error!",
            payment_svr_name(svr), logic_require_name(require), query_result->count);
        return -1;
    }

    balance->count = 0;

    if (query_result->count == 0) {
        uint8_t i;
        for(i = 0; i < bag_info->money_type_count && i < PAYMENT_MONEY_TYPE_COUNT; ++i) {
            balance->datas[balance->count].type = i + PAYMENT_MONEY_TYPE_MIN;
            balance->datas[balance->count].count = 0;
            balance->count++;
        }
    }
    else {
        if (bag_info->money_type_count >= 1) {
            balance->datas[balance->count].type = PAYMENT_MONEY_TYPE_MIN;
            balance->datas[balance->count].count = query_result->records[0].money1;
            balance->count++;
        }

        if (bag_info->money_type_count >= 2) {
            balance->datas[balance->count].type = PAYMENT_MONEY_TYPE_MIN + 1;
            balance->datas[balance->count].count = query_result->records[0].money2;
            balance->count++;
        }

        if (bag_info->money_type_count >= 3) {
            balance->datas[balance->count].type = PAYMENT_MONEY_TYPE_MIN + 2;
            balance->datas[balance->count].count = query_result->records[0].money3;
            balance->count++;
        }
    }

    return 0;
}
