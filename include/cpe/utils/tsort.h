#ifndef CPE_UTILS_TSORT_H
#define CPE_UTILS_TSORT_H
#include "cpe/pal/pal_types.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsorter_str * tsorter_str_t;
typedef struct tsorter_str_element * tsorter_str_element_t;
typedef struct tsorter_str_depend * tsorter_str_depend_t;

typedef struct tsorter_str_it {
    const char * (*next)(struct tsorter_str_it * it);
    tsorter_str_element_t ctx;
} * tsorter_str_it_t;


tsorter_str_t tsorter_str_create(mem_allocrator_t alloc, int dup_str);
void tsorter_str_free(tsorter_str_t sorter);
int tsorter_str_add_dep(tsorter_str_t sorter, const char * dep_from, const char * dep_to); 
int tsorter_str_add_element(tsorter_str_t sorter, const char * name);

tsorter_str_element_t tsorter_str_element_check_create(tsorter_str_t sorter, const char * value);
tsorter_str_element_t tsorter_str_element_find(tsorter_str_t sorter, const char * value);
void tsorter_str_element_free(tsorter_str_element_t element);

const char * tsorter_str_element_value(tsorter_str_element_t element);
int tsorter_str_element_add_dep(tsorter_str_element_t element, const char * depend_from);

tsorter_str_depend_t tsorter_str_depend_create(tsorter_str_element_t denend_to, tsorter_str_element_t depent_from);
void tsorter_str_depend_free(tsorter_str_depend_t dep);

int tsorter_str_sort(tsorter_str_it_t it, tsorter_str_t sorter);

#define tsorter_str_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
