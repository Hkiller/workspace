#ifndef CPE_POM_GRP_OBJ_H
#define CPE_POM_GRP_OBJ_H
#include "cpe/utils/bitarry.h"
#include "cpe/pom/pom_object.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_grp_obj_t
pom_grp_obj_alloc(pom_grp_obj_mgr_t mgr);

void pom_grp_obj_free(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);
void pom_grp_obj_clear(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);
int pom_grp_obj_is_empty(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);

pom_oid_t pom_grp_obj_oid(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);
uint16_t pom_grp_obj_page_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);
uint16_t pom_grp_obj_page_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);
pom_oid_t pom_grp_obj_page_oid(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, uint16_t pos);

int pom_grp_obj_validate(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em);

void pom_grp_objs(pom_grp_obj_mgr_t mgr, pom_grp_obj_it_t it);

#define pom_grp_obj_it_next(it) ((pom_grp_obj_t)pom_obj_it_next(&((it)->m_data)))

/*normal data ops*/
uint16_t pom_grp_obj_nromal_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_normal(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_normal_check_or_create(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
int pom_grp_obj_normal_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data);

uint16_t pom_grp_obj_normal_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_normal_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_normal_check_or_create_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
int pom_grp_obj_normal_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data);

/*list data ops*/
uint16_t pom_grp_obj_list_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
uint16_t pom_grp_obj_list_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_list_at(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos);
int pom_grp_obj_list_append(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data);
int pom_grp_obj_list_insert(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos, void * data);
int pom_grp_obj_list_remove(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos);
int pom_grp_obj_list_clear(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
int pom_grp_obj_list_sort(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, int (*cmp)(void const *, void const *));
void * pom_grp_obj_list_bsearch(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, const void * key, int (*cmp)(void const *, void const *));
void * pom_grp_obj_list_lsearch(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, const void * key, int (*equal)(void const *, void const *));

uint16_t pom_grp_obj_list_count_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
uint16_t pom_grp_obj_list_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_list_at_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos);
int pom_grp_obj_list_append_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data);
int pom_grp_obj_list_insert_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos, void * data);
int pom_grp_obj_list_remove_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos);
int pom_grp_obj_list_clear_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
int pom_grp_obj_list_sort_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, int (*cmp)(void const *, void const *));
void * pom_grp_obj_list_bsearch_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, const void * key, int (*cmp)(void const *, void const *));
void * pom_grp_obj_list_lsearch_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, const void * key, int (*equal)(void const *, void const *));

/*ba data ops*/
uint16_t pom_grp_obj_ba_bit_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
uint16_t pom_grp_obj_ba_byte_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
uint16_t pom_grp_obj_ba_bit_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
int pom_grp_obj_ba_set_all(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, cpe_ba_value_t value);
int pom_grp_obj_ba_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint32_t pos, cpe_ba_value_t value);
cpe_ba_value_t pom_grp_obj_ba_get(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint32_t pos);
int pom_grp_obj_ba_get_binary(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data, uint32_t size);
int pom_grp_obj_ba_set_binary(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void const * data, uint32_t size);

uint16_t pom_grp_obj_ba_bit_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
uint16_t pom_grp_obj_ba_byte_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
uint16_t pom_grp_obj_ba_bit_count_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
int pom_grp_obj_ba_set_all_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, cpe_ba_value_t value);
int pom_grp_obj_ba_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint32_t pos, cpe_ba_value_t value);
cpe_ba_value_t pom_grp_obj_ba_get_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint32_t pos);
int pom_grp_obj_ba_get_binary_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data, uint32_t size);
int pom_grp_obj_ba_set_binary_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void const * data, uint32_t size);

/*binary data ops*/
uint16_t pom_grp_obj_binary_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_binary(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_binary_check_or_create(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
int pom_grp_obj_binary_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void const * data, uint16_t capacity);

uint16_t pom_grp_obj_binary_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_binary_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_binary_check_or_create_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
int pom_grp_obj_binary_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void const * data, uint16_t capacity);
int pom_grp_obj_binary_get_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data, uint16_t capacity);

#ifdef __cplusplus
}
#endif

#endif
