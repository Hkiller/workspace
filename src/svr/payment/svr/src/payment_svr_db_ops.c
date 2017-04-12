#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "payment_svr_db_ops.h"

int payment_svr_mongo_pkg_append_id(mongo_pkg_t db_pkg, uint64_t uid, uint16_t bag_id) {
    if (bag_id == 0) {
        return mongo_pkg_append_int64(db_pkg, "_id", (int64_t)uid);
    }
    else {
        char buf[64];
        snprintf(buf, sizeof(buf), FMT_UINT64_T"-%d", uid, bag_id);
        return mongo_pkg_append_string(db_pkg, "_id", buf);
    }
}

int payment_svr_mongo_pkg_append_required_moneies(mongo_pkg_t db_pkg, uint8_t money_type_count) {
    uint8_t i;
    for(i = 0; i < money_type_count && i < PAYMENT_MONEY_TYPE_COUNT; ++i) {
        char col_name[64];
        snprintf(col_name, sizeof(col_name), "money%d", i + 1);
        if (mongo_pkg_append_int32(db_pkg, col_name, 1) != 0) return -1;
    }

    return 0;
}

int payment_svr_db_validate_result(payment_svr_t svr, logic_require_t require) {
    if (logic_require_state(require) == logic_require_state_done) return 0;

    if (logic_require_state(require) == logic_require_state_error) {
        CPE_ERROR(
            svr->m_em, "%s: %s: require state error, errno=%d, db error!",
            payment_svr_name(svr), logic_require_name(require), logic_require_error(require));
        return SVR_PAYMENT_ERRNO_DB;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: %s: require state error, state = %s!",
            payment_svr_name(svr), logic_require_name(require), logic_require_state_name(logic_require_state(require)));
        return SVR_PAYMENT_ERRNO_INTERNAL;
    }
}

int payment_svr_find_count_by_type(uint64_t * result, SVR_PAYMENT_MONEY_GROUP const * monies, uint8_t money_type) {
    uint8_t i;
    for(i = 0; i < monies->count; ++i) {
        if (monies->datas[i].type == money_type) {
            *result = monies->datas[i].count;
            return 0;
        }
    }

    return -1;
}

uint64_t payment_svr_get_count_by_type(SVR_PAYMENT_MONEY_GROUP const * monies, uint8_t money_type, uint64_t dft) {
    uint64_t result;
    if (payment_svr_find_count_by_type(&result, monies, money_type) == 0) return result;
    return dft;
}
