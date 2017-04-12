#ifndef CPE_DR_METALIB_BUILD_H
#define CPE_DR_METALIB_BUILD_H
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_external.h"
#include "cpe/vfs/vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DRInBuildMetaLib * dr_inbuild_create_lib(void);
void dr_inbuild_free_lib(struct DRInBuildMetaLib * ctx);

void dr_inbuild_set_dft_align(struct DRInBuildMetaLib * inBuildMetaLib, uint8_t align);

int dr_inbuild_tsort(
    struct DRInBuildMetaLib * inBuildMetaLib,
    error_monitor_t er);

int dr_inbuild_build_lib(
    mem_buffer_t buffer,
    struct DRInBuildMetaLib * inBuildMetaLib,
    error_monitor_t er);

/*detail operations*/
struct DRInBuildMacro * dr_inbuild_metalib_add_macro(struct DRInBuildMetaLib * inBuildMetaLib);
void dr_inbuild_metalib_remove_macro(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro);
int dr_inbuild_metalib_add_macro_to_index(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro);
struct DRInBuildMacro * dr_inbuild_metalib_find_macro(struct DRInBuildMetaLib * inBuildMetaLib, const char * macro_name);

struct DRInBuildMeta * dr_inbuild_metalib_add_meta(struct DRInBuildMetaLib * inBuildMetaLib);
void dr_inbuild_metalib_remove_meta(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMeta * meta);
struct DRInBuildMeta * dr_inbuild_metalib_find_meta(struct DRInBuildMetaLib * inBuildMetaLib, const char * meta_name);

struct DRInBuildMeta *
dr_inbuild_metalib_copy_meta(struct DRInBuildMetaLib * inBuildMetaLib, LPDRMETA meta);

struct DRInBuildMeta *
dr_inbuild_metalib_copy_meta_r(struct DRInBuildMetaLib * inBuildMetaLib, LPDRMETA meta);

int dr_inbuild_metalib_copy_meta_ref_metas(struct DRInBuildMetaLib * inBuildMetaLib, LPDRMETA src_meta);
int dr_inbuild_metalib_version(struct DRInBuildMetaLib * inBuildMetaLib);
void dr_inbuild_metalib_set_version(struct DRInBuildMetaLib * inBuildMetaLib, int version);
void dr_inbuild_metalib_set_tagsetversion(struct DRInBuildMetaLib * inBuildMetaLib, int version);
void dr_inbuild_metalib_set_name(struct DRInBuildMetaLib * inBuildMetaLib, const char * name);

int dr_inbuild_meta_current_version(struct DRInBuildMeta * meta);
void dr_inbuild_meta_set_type(struct DRInBuildMeta * meta, int type);
void dr_inbuild_meta_set_align(struct DRInBuildMeta * meta, int align);
void dr_inbuild_meta_set_id(struct DRInBuildMeta * meta, int id);
void dr_inbuild_meta_set_base_version(struct DRInBuildMeta * meta, int version);
void dr_inbuild_meta_set_current_version(struct DRInBuildMeta * meta, int version);
int dr_inbuild_meta_set_name(struct DRInBuildMeta * meta, const char * name);
void dr_inbuild_meta_set_desc(struct DRInBuildMeta * meta, const char * desc);

int dr_inbuild_meta_add_key_entries(struct DRInBuildMeta * meta, const char * names);

struct dr_inbuild_index * dr_inbuild_meta_add_index(struct DRInBuildMeta * meta, const char * name);
int dr_inbuild_index_add_entries(struct dr_inbuild_index * index, const char * names);

struct DRInBuildMetaEntry * dr_inbuild_meta_add_entry(struct DRInBuildMeta * meta);
void dr_inbuild_meta_remove_entry(struct DRInBuildMeta * meta, struct DRInBuildMetaEntry * entry);
struct DRInBuildMetaEntry *
dr_inbuild_meta_copy_entry(struct DRInBuildMeta * meta, LPDRMETAENTRY entry);
struct DRInBuildMetaEntry *
dr_inbuild_meta_find_entry(struct DRInBuildMeta * meta, const char * name);

int dr_inbuild_meta_init(struct DRInBuildMeta * new_meta, LPDRMETA src_meta);
int dr_inbuild_meta_copy_entrys(struct DRInBuildMeta * new_meta, LPDRMETA src_meta);
int dr_inbuild_meta_copy_key_entrys(struct DRInBuildMeta * new_meta, LPDRMETA src_meta);
int dr_inbuild_meta_copy_keys(struct DRInBuildMeta * new_meta, LPDRMETA src_meta);
int dr_inbuild_meta_copy_index(struct DRInBuildMeta * new_meta, dr_index_info_t src_index);
int dr_inbuild_meta_copy_indexes(struct DRInBuildMeta * new_meta, LPDRMETA src_meta);

int dr_inbuild_entry_version(struct DRInBuildMetaEntry * entry);
void dr_inbuild_entry_set_type(struct DRInBuildMetaEntry * entry, const char * type_name);
void dr_inbuild_entry_set_id(struct DRInBuildMetaEntry * entry, int id);
void dr_inbuild_entry_set_array_count(struct DRInBuildMetaEntry * entry, int array_count);
void dr_inbuild_entry_set_array_refer(struct DRInBuildMetaEntry * entry, const char * refer);
void dr_inbuild_entry_set_selector(struct DRInBuildMetaEntry * entry, const char * selector);
void dr_inbuild_entry_set_version(struct DRInBuildMetaEntry * entry, int version);
void dr_inbuild_entry_set_name(struct DRInBuildMetaEntry * entry, const char * name);
void dr_inbuild_entry_set_desc(struct DRInBuildMetaEntry * entry, const char * desc);
void dr_inbuild_entry_set_size(struct DRInBuildMetaEntry * entry, int size);

LPDRMETALIB dr_metalib_build_from_group(
    mem_allocrator_t alloc, error_monitor_t em, vfs_mgr_t vfs, const char * group_root, const char * group);

LPDRMETALIB dr_metalib_build_from_dir(
    mem_allocrator_t alloc, error_monitor_t em, vfs_mgr_t vfs, const char * group_root);
    
#ifdef __cplusplus
}
#endif


#endif
