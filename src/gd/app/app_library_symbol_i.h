#ifndef GD_APP_LIBRARY_SYMBOL_I_H
#define GD_APP_LIBRARY_SYMBOL_I_H
#include "app_library_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gd_app_symbol {
    struct cpe_hash_entry m_hh;
    const char * m_lib;
    const char * m_name;
    void * m_symbol;
};

gd_app_symbol_t gd_app_symbol_create(const char * lib, const char * name, void * p);
void gd_app_symbol_free(gd_app_symbol_t sym);

gd_app_symbol_t gd_app_symbol_find(const char * lib, const char * name);
    
uint32_t gd_app_symbol_hash(gd_app_symbol_t symbol);
int gd_app_symbol_eq(gd_app_symbol_t l, gd_app_symbol_t r);
    
#ifdef __cplusplus
}
#endif

#endif
