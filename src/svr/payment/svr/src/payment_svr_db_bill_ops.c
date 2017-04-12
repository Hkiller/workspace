#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "payment_svr_db_ops.h"

void payment_svr_db_add_bill(
    payment_svr_t svr, BAG_INFO const * bag_info, uint64_t user_id, 
    PAYMENT_BILL_DATA const * bill_data, SVR_PAYMENT_MONEY_GROUP const * balance)
{
    mongo_pkg_t db_pkg;
    int pkg_r;
    int i;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: add_bill: get db pkg fail!", payment_svr_name(svr));
        return;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "payment_bill");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_open(db_pkg);

    pkg_r |= mongo_pkg_append_int64(db_pkg, "user_id", (int64_t)user_id);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "bag_id", bag_info->bag_id);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "op_time", tl_manage_time_sec(gd_app_tl_mgr(svr->m_app)));

    for(i = 0; i < bag_info->money_type_count && i < PAYMENT_MONEY_TYPE_COUNT; ++i) {
        char buf[64];
        uint8_t money_type = i + PAYMENT_MONEY_TYPE_MIN;
        int64_t diff_count = (int64_t)payment_svr_get_count_by_type(&bill_data->money, money_type, 0);

        if (bill_data->way == payment_bill_way_out) {
            diff_count = - diff_count;
        }

        snprintf(buf, sizeof(buf), "diff%d", money_type);
        pkg_r |= mongo_pkg_append_int64(db_pkg, buf, diff_count);

        snprintf(buf, sizeof(buf), "money%d", money_type);
        pkg_r |= mongo_pkg_append_int64(db_pkg, buf, (int64_t)payment_svr_get_count_by_type(balance, money_type, 0));
    }

    if (bill_data->product_id) {
        pkg_r |= mongo_pkg_append_int64(db_pkg, "product_id", (int64_t)bill_data->product_id);
    }

    if (bill_data->acitvity_id) {
        pkg_r |= mongo_pkg_append_int64(db_pkg, "acitvity_id", (int64_t)bill_data->acitvity_id);
    }

    if (bill_data->gift_id) {
        pkg_r |= mongo_pkg_append_int64(db_pkg, "gift_id", (int64_t)bill_data->gift_id);
    }

    if (bill_data->recharge_way_info[0]) {
        pkg_r |= mongo_pkg_append_string(db_pkg, "recharge_way_info", bill_data->recharge_way_info);
    }

    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: add_bill: build db pkg fail!", payment_svr_name(svr));
        return;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, NULL, NULL, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(svr->m_em, "%s: add_bill: send db request fail!", payment_svr_name(svr));
        return;
    }
}
