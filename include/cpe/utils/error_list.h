#ifndef CPE_UTILS_ERRORLIST_H
#define CPE_UTILS_ERRORLIST_H
#include "error.h"
#include "memory.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct error_list_node;
struct error_list;
typedef struct error_list * error_list_t;

/*error list operations*/
error_list_t cpe_error_list_create(mem_allocrator_t alloc);
void cpe_error_list_free(error_list_t el);

void cpe_error_list_clear(error_list_t el);
char * cpe_error_list_dump(error_list_t el, mem_buffer_t buffer, int ident);

void cpe_error_list_visit(error_list_t el, void(*p)(void * ctx, struct error_info * info, const char * msg), void * ctx);
int cpe_error_list_have_errno(error_list_t el, int e);
int cpe_error_list_have_msg(error_list_t el, const char * partMsg);
int cpe_error_list_error_count(error_list_t el);
void cpe_error_list_collect(struct error_info * info, void * context, const char * fmt, va_list args);

#ifdef __cplusplus
}
#endif

#endif
