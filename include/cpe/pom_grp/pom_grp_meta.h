#ifndef CPE_POM_GRP_META_H
#define CPE_POM_GRP_META_H
#include "cpe/dr/dr_types.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*meta operations*/
pom_grp_meta_t pom_grp_meta_create(
    mem_allocrator_t alloc,
    const char * name,
    uint32_t omm_page_size);

void pom_grp_meta_free(pom_grp_meta_t);

const char * pom_grp_meta_name(pom_grp_meta_t meta);
uint32_t pom_grp_omm_page_size(pom_grp_meta_t meta);
uint16_t pom_grp_meta_entry_count(pom_grp_meta_t meta);
pom_grp_entry_meta_t pom_grp_meta_entry_at(pom_grp_meta_t meta, uint16_t pos);
int pom_grp_meta_set_main_entry(pom_grp_meta_t meta, const char * entry_name);
pom_grp_entry_meta_t pom_grp_meta_main_entry(pom_grp_meta_t meta);

void pom_grp_meta_dump(write_stream_t stream, pom_grp_meta_t meta, int ident);

size_t pom_grp_meta_calc_bin_size(
    pom_grp_meta_t meta);

pom_grp_meta_t
pom_grp_meta_build_from_bin(
    mem_allocrator_t alloc, 
    void const * data,
    LPDRMETALIB metalib,
    error_monitor_t em);

void pom_grp_meta_write_to_bin(
    void * data, size_t capacity, pom_grp_meta_t meta);

int pom_grp_meta_compatable(pom_grp_meta_t l, pom_grp_meta_t r);

/*entry operations*/
pom_grp_entry_meta_t
pom_grp_entry_meta_normal_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta,
    error_monitor_t em);

pom_grp_entry_meta_t
pom_grp_entry_meta_list_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta, uint32_t count_per_page, uint32_t capacity,
    int standalone,
    error_monitor_t em);

pom_grp_entry_meta_t
pom_grp_entry_meta_ba_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    uint16_t byte_per_page, uint16_t bit_capacity,
    error_monitor_t em);

pom_grp_entry_meta_t
pom_grp_entry_meta_binary_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    uint32_t capacity,
    error_monitor_t em);

void pom_grp_entry_meta_free(pom_grp_entry_meta_t entry_meta);

pom_grp_entry_meta_t
pom_grp_entry_meta_find(pom_grp_meta_t meta, const char * name);

pom_grp_entry_meta_t
pom_grp_entry_meta_at(pom_grp_meta_t meta, uint16_t pos);

void pom_grp_entry_meta_it_init(pom_grp_meta_t meta, pom_grp_entry_meta_it_t it);

const char * pom_grp_entry_meta_name(pom_grp_entry_meta_t entry_meta);
cpe_hash_string_t pom_grp_entry_meta_name_hs(pom_grp_entry_meta_t entry_meta);
pom_grp_entry_type_t pom_grp_entry_meta_type(pom_grp_entry_meta_t entry_meta);
uint16_t pom_grp_entry_meta_index(pom_grp_entry_meta_t entry_meta);
uint16_t pom_grp_entry_meta_page_count(pom_grp_entry_meta_t entry_meta);
uint16_t pom_grp_entry_meta_page_size(pom_grp_entry_meta_t entry_meta);

LPDRMETA pom_grp_entry_meta_normal_meta(pom_grp_entry_meta_t entry_meta);
uint16_t pom_grp_entry_meta_normal_capacity(pom_grp_entry_meta_t entry_meta);

LPDRMETA pom_grp_entry_meta_list_meta(pom_grp_entry_meta_t entry_meta);
uint16_t pom_grp_entry_meta_list_capacity(pom_grp_entry_meta_t entry_meta);

uint32_t pom_grp_entry_meta_ba_bits(pom_grp_entry_meta_t entry_meta);
uint32_t pom_grp_entry_meta_ba_bytes(pom_grp_entry_meta_t entry_meta);

uint16_t pom_grp_entry_meta_binary_capacity(pom_grp_entry_meta_t entry_meta);

#define pom_grp_entry_meta_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
