#ifndef CPE_DP_BINDING_H
#define CPE_DP_BINDING_H
#include "dp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define dp_binding_next(it) ((it)->next ? (it)->next(it) : NULL)

int dp_binding_numeric(uint32_t * value,  dp_binding_t binding);
int dp_binding_string(char const * * cmd,  dp_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif
