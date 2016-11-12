#ifndef GD_APP_CONTEXT_CHILD_H
#define GD_APP_CONTEXT_CHILD_H
#include "app_basic.h"

#ifdef __cplusplus
extern "C" {
#endif

gd_app_child_context_t 
gd_app_child_context_create_follow(
    gd_app_context_t from, const char * lib_name,
    mem_allocrator_t alloc, size_t capacity,
    int argc, char ** argv);

gd_app_child_context_t 
gd_app_child_context_create_inline(
    gd_app_context_t from, const char * lib_name,
    mem_allocrator_t alloc, size_t capacity,
    int argc, char ** argv);

void * gd_app_context_context_child(gd_app_child_context_t child_context);

int gd_app_child_context_set_root(gd_app_child_context_t child_context, const char * root);
int gd_app_child_context_start(gd_app_child_context_t child_context);

#ifdef __cplusplus
}
#endif

#endif
