#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "app_library_symbol_i.h"

uint8_t g_symbols_init = 0;
static struct cpe_hash_table g_symbols;

int gd_app_symbols_init(void) {
    if (g_symbols_init) return 0;

    if (cpe_hash_table_init(
            &g_symbols,
            NULL,
            (cpe_hash_fun_t) gd_app_symbol_hash,
            (cpe_hash_eq_t) gd_app_symbol_eq,
            CPE_HASH_OBJ2ENTRY(gd_app_symbol, m_hh),
            -1) != 0)
    {
        printf("gd_app_symbols_init fail!\n");
        return -1;
    }

    g_symbols_init = 1;
    
    return 0;
}

void gd_app_symbols_fini(void) {
    struct cpe_hash_it symbol_it;
    gd_app_symbol_t symbol;

    assert(g_symbols_init);
    
    cpe_hash_it_init(&symbol_it, &g_symbols);

    symbol = cpe_hash_it_next(&symbol_it);
    while (symbol) {
        gd_app_symbol_t next = cpe_hash_it_next(&symbol_it);
        gd_app_symbol_free(symbol);
        symbol = next;
    }

    g_symbols_init = 0;
}

gd_app_symbol_t
gd_app_symbol_create(const char * lib, const char * name, void * p) {
    gd_app_symbol_t symbol;
    size_t lib_len = strlen(lib) + 1;
    size_t name_len = strlen(name) + 1;

    assert(g_symbols_init);

    symbol = mem_alloc(NULL, sizeof(struct gd_app_symbol) + lib_len + name_len);
    if (symbol == NULL) return NULL;

    symbol->m_lib = (void*)(symbol + 1);
    symbol->m_name = symbol->m_lib + lib_len;
    symbol->m_symbol = p;
    
    memcpy((void*)symbol->m_lib, lib, lib_len);
    memcpy((void*)symbol->m_name, name, name_len);

    cpe_hash_entry_init(&symbol->m_hh);
    if (cpe_hash_table_insert_unique(&g_symbols, symbol) != 0) {
        mem_free(NULL, symbol);
        return NULL;
    }

    return symbol;
}

void gd_app_symbol_free(gd_app_symbol_t symbol) {
    cpe_hash_table_remove_by_ins(&g_symbols, symbol);
    mem_free(NULL, symbol);
}

gd_app_symbol_t gd_app_symbol_find(const char * lib, const char * name) {
    struct gd_app_symbol key;

    if (!g_symbols_init) return NULL;

    key.m_lib = lib;
    key.m_name = name;
    return cpe_hash_table_find(&g_symbols, &key);
}

int gd_app_lib_register_symbol(const char * libName, const char * symbolName, void * symbol) {
    assert(g_symbols_init);
    return gd_app_symbol_create(libName, symbolName, symbol) ? 0 : -1;
}

uint32_t gd_app_symbol_hash(gd_app_symbol_t symbol) {
    return cpe_hash_str(symbol->m_name, strlen(symbol->m_name));
}

int gd_app_symbol_eq(gd_app_symbol_t l, gd_app_symbol_t r) {
    return (strcmp(l->m_name, r->m_name) == 0 && strcmp(l->m_lib, r->m_lib) == 0) ? 1 : 0;
}
