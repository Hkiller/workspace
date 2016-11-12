#ifndef CPE_POM_GRP_STORE_H
#define CPE_POM_GRP_STORE_H
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "cpe/cfg/cfg_types.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_grp_store_t
pom_grp_store_create(
    mem_allocrator_t alloc,
    pom_grp_meta_t meta,
    LPDRMETA dr_meta,
    error_monitor_t em);

void pom_grp_store_free(pom_grp_store_t store);

uint32_t pom_grp_store_table_count(pom_grp_store_t store);
pom_grp_meta_t pom_grp_store_meta(pom_grp_store_t store);

void pom_grp_store_tables(pom_grp_store_t store, pom_grp_store_table_it_t it);
pom_grp_store_table_t pom_grp_store_table_find(pom_grp_store_t store, const char * name);

const char * pom_grp_store_table_name(pom_grp_store_table_t table);
LPDRMETA pom_grp_store_table_meta(pom_grp_store_table_t table);

void pom_grp_store_entries(pom_grp_store_t store, pom_grp_store_entry_it_t it);
pom_grp_store_entry_t pom_grp_store_entry_find(pom_grp_store_t store, const char * name);

void pom_grp_table_entries(pom_grp_store_table_t table, pom_grp_store_entry_it_t it);
pom_grp_store_entry_t pom_grp_table_entry_find(pom_grp_store_table_t table, const char * name);

const char * pom_grp_store_entry_name(pom_grp_store_entry_t entry);
pom_grp_entry_meta_t pom_grp_store_entry_meta(pom_grp_store_entry_t entry);

pom_grp_meta_t pom_grp_store_meta(pom_grp_store_t store);

int pom_grp_meta_build_store_meta(
    mem_buffer_t buffer,
    pom_grp_meta_t meta,
    error_monitor_t em);

int pom_grp_store_build_obj(
    pom_grp_store_t store, pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj,
    void const * data_buf, size_t data_size, LPDRMETA data_meta);

int pom_grp_store_write_obj(
    pom_grp_store_t store, pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj,
    void * data_buf, size_t data_capacity, LPDRMETA data_meta);

#define pom_grp_store_table_it_next(it) ((it)->next ? (it)->next(it) : NULL)
#define pom_grp_store_entry_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
