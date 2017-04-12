#include <assert.h>
#include "usf/logic/logic_data.h"
#include "protocol/mongo_cli/mongo_cli.h"
#include "usf/mongo_cli/mongo_cli_result.h"

mongo_cli_result_t
mongo_cli_result_find(logic_require_t require) {
    logic_data_t data = logic_require_data_find(require, "mongo_lasterror");
    if (data == NULL) return NULL;
    return logic_data_data(data);
}

int32_t mongo_cli_result_n(mongo_cli_result_t result) {
    assert(result);
    return ((MONGO_LASTERROR *)result)->n;
}

int32_t mongo_cli_result_code(mongo_cli_result_t result) {
    assert(result);
    return ((MONGO_LASTERROR *)result)->code;
}


