#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_svr.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_type.h"
#include "payment_svr_waiting.h"

int payment_svr_chuangku_offline_init(payment_svr_adapter_t adapter, cfg_t cfg) {
    return 0;
}

void payment_svr_chuangku_offline_fini(payment_svr_adapter_t adapter) {
}

int payment_svr_chuangku_offline_charge_send(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_chuangku_offline * chuangku_offline = (struct payment_svr_adapter_chuangku_offline *)adapter->m_private;

    record->state = PAYMENT_RECHARGE_SUCCESS;
    record->error = 0;
    record->error_msg[0] = 0;

    return 0;
}

int payment_svr_chuangku_offline_charge_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    return 0;
}
