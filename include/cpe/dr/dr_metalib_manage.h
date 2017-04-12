#ifndef CPE_DR_METALIB_MANAGE_H
#define CPE_DR_METALIB_MANAGE_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_external.h"

#ifdef __cplusplus
extern "C" {
#endif

/*lib operations*/	
LPDRMETA dr_lib_find_meta_by_name(LPDRMETALIB metaLib, const char* name);
LPDRMETA dr_lib_find_meta_by_id(LPDRMETALIB metaLib, int id);
int dr_lib_meta_num(LPDRMETALIB metaLib);
LPDRMETA dr_lib_meta_at(LPDRMETALIB metaLib, int idx);

/*meta operations*/	
int dr_meta_based_version(LPDRMETA meta);
int dr_meta_current_version(LPDRMETA meta);
int dr_meta_type(LPDRMETA meta);
const char *dr_meta_name(LPDRMETA meta);
const char *dr_meta_desc(LPDRMETA meta);
int dr_meta_id(LPDRMETA meta);
size_t dr_meta_size(LPDRMETA meta);
int dr_meta_align(LPDRMETA meta);
int dr_meta_require_align(LPDRMETA meta);
int dr_meta_entry_num(LPDRMETA meta);
LPDRMETAENTRY dr_meta_entry_at(LPDRMETA meta, int idx);
int dr_meta_find_entry_idx_by_name(LPDRMETA meta, const char* name);
int dr_meta_find_entry_idx_by_id(LPDRMETA meta, int id);
char *dr_meta_off_to_path(LPDRMETA meta, int off, char * buf, size_t bufSize);
int dr_meta_path_to_off(LPDRMETA meta, const char * path, LPDRMETAENTRY * entry);
LPDRMETAENTRY dr_meta_find_entry_by_name(LPDRMETA meta, const char* name);
LPDRMETAENTRY dr_meta_find_entry_by_name_len(LPDRMETA meta, const char * entry_name, size_t entry_name_len);
LPDRMETAENTRY dr_meta_find_entry_by_id(LPDRMETA meta, int id);
LPDRMETAENTRY dr_meta_find_entry_by_path(LPDRMETA meta, const char* entryPath);
LPDRMETAENTRY dr_meta_find_entry_by_path_ex(LPDRMETA meta, const char* entryPath, int * off);
LPDRMETAENTRY dr_meta_lsearch_entry_by_type_name(LPDRMETA meta, const char * type_name);
LPDRMETALIB dr_meta_owner_lib(LPDRMETA meta);

int dr_meta_find_dyn_info(LPDRMETA meta, dr_meta_dyn_info_t array_info);
ssize_t dr_meta_calc_dyn_size(LPDRMETA meta, size_t record_count);

/*meta.key operations*/	
int dr_meta_key_entry_num(LPDRMETA meta);
dr_idx_entry_info_t dr_meta_key_info_at(LPDRMETA meta, int idx);
LPDRMETAENTRY dr_meta_key_entry_at(LPDRMETA meta, int idx);

/*meta.index operations*/	
int dr_meta_index_num(LPDRMETA meta);
dr_index_info_t dr_meta_index_at(LPDRMETA meta, int idx);
LPDRMETA dr_index_meta(dr_index_info_t index);
const char * dr_index_name(dr_index_info_t index);
int dr_index_entry_num(dr_index_info_t index);
dr_index_entry_info_t dr_index_entry_info_at(dr_index_info_t index, int idx);

LPDRMETAENTRY dr_index_entry_at(dr_index_info_t index, int idx);

/*entry operations*/	
int dr_entry_version(LPDRMETAENTRY entry);
const char * dr_entry_name(LPDRMETAENTRY entry);
const char * dr_entry_cname(LPDRMETAENTRY entry);
const char * dr_entry_desc(LPDRMETAENTRY entry);
const void * dr_entry_dft_value(LPDRMETAENTRY entry);
int dr_entry_is_key(LPDRMETAENTRY entry);
LPDRMETA dr_entry_ref_meta(LPDRMETAENTRY entry);
LPDRMETA dr_entry_self_meta(LPDRMETAENTRY entry);
LPDRMACROSGROUP dr_entry_macrosgroup(LPDRMETAENTRY entry);
int dr_entry_id(LPDRMETAENTRY entry);
size_t dr_entry_size(LPDRMETAENTRY entry);
size_t dr_entry_element_size(LPDRMETAENTRY entry);
size_t dr_entry_require_align(LPDRMETAENTRY entry);
int dr_entry_type(LPDRMETAENTRY entry);
const char * dr_entry_type_name(LPDRMETAENTRY entry);
int dr_entry_array_count(LPDRMETAENTRY entry);
LPDRMETAENTRY dr_entry_array_refer_entry(LPDRMETAENTRY entry);
LPDRMETAENTRY dr_entry_select_entry(LPDRMETAENTRY entry);
const char *dr_entry_customattr(LPDRMETALIB metaLib, LPDRMETAENTRY entry);
size_t dr_entry_data_start_pos(LPDRMETAENTRY entry, int index);
size_t dr_entry_array_calc_ele_num(LPDRMETAENTRY entry, size_t buf_capacity);
size_t dr_entry_array_calc_buf_capacity(LPDRMETAENTRY entry, size_t ele_num);

/*macro operations*/	
int dr_lib_find_macro_value(int *a_piID, LPDRMETALIB metaLib, const  char *name);
LPDRMACRO dr_lib_macro_at(LPDRMETALIB metaLib, int a_iIdx);
LPDRMACRO dr_lib_macro_find(LPDRMETALIB metaLib, const char * name);

int dr_lib_macro_num(LPDRMETALIB metaLib);
const char* dr_macro_name(LPDRMETALIB metaLib, LPDRMACRO macro);
int dr_macro_value(LPDRMACRO macro);
const char* dr_macro_desc(LPDRMETALIB metaLib, LPDRMACRO macro);

LPDRMACROSGROUP dr_macro_macrosgroup(LPDRMETALIB metaLib, LPDRMACRO macro);

/*macro-group operations*/	
int dr_lib_macrosgroup_num(LPDRMETALIB metaLib);
LPDRMACROSGROUP dr_lib_macrosgroup_at(LPDRMETALIB metaLib, int a_iIdx);

const char* dr_macrosgroup_name(LPDRMACROSGROUP macroGroup);
int dr_macrosgroup_macro_num(LPDRMACROSGROUP macroGroup);
LPDRMACRO dr_macrosgroup_macro_at(LPDRMETALIB metaLib, LPDRMACROSGROUP macroGroup, int idx);
LPDRMACRO dr_macrosgroup_find_macro_by_name(LPDRMETALIB metaLib, LPDRMACROSGROUP macroGroup, const char * name);

void dr_lib_print(write_stream_t stream, LPDRMETALIB metaLib, int ident);
const char * dr_lib_dump(mem_buffer_t buffer, LPDRMETALIB metaLib, int ident);

#ifdef __cplusplus
}
#endif


#endif
