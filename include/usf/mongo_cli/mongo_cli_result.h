#ifndef USF_MONGO_CLI_RESULT_H
#define USF_MONGO_CLI_RESULT_H
#include "mongo_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_cli_result_t
mongo_cli_result_find(logic_require_t require);

int32_t mongo_cli_result_n(mongo_cli_result_t result);
int32_t mongo_cli_result_code(mongo_cli_result_t result);

#ifdef __cplusplus
}
#endif

#endif
