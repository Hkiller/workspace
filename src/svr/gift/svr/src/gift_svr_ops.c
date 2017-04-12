#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "gift_svr_ops.h"

uint8_t gift_svr_op_check_db_result(gift_svr_t svr, logic_context_t ctx, logic_require_t require) {
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            CPE_ERROR(
                svr->m_em, "%s: %s: db request error, errno=%d!",
                gift_svr_name(svr), logic_require_name(require), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_GIFT_ERRNO_DB);
            return -1;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: %s: db request state error, state=%s!",
                gift_svr_name(svr), logic_require_name(require), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
            return -1;
        }
    }

    return 0;
}



