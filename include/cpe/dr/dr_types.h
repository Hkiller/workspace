#ifndef CPE_DR_TYPES_H
#define CPE_DR_TYPES_H
#include "cpe/utils/utils_types.h"
#include "cpe/dr/dr_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t dr_longlong;
typedef uint64_t dr_ulonglong;
typedef uint16_t dr_wchar_t;
typedef uint32_t dr_date_t;
typedef uint32_t dr_time_t;
typedef uint64_t dr_datetime_t;
typedef uint32_t dr_ip_t;

typedef struct tagDRMetaLib	*LPDRMETALIB;
typedef struct tagDRLibParam	DRLIBPARAM;
typedef struct tagDRLibParam	*LPDRLIBPARAM;
typedef struct tagDRMeta	*LPDRMETA;
typedef struct tagDRMetaEntry	*LPDRMETAENTRY;
typedef struct tagDRData	DRDATA;
typedef struct tagDRData	*LPDRDATA;
typedef struct tagDRMacro		*LPDRMACRO;
typedef struct tagDRMacrosGroup		*LPDRMACROSGROUP;
typedef struct dr_idx_entry_info * dr_idx_entry_info_t;
typedef struct dr_index_info * dr_index_info_t;
typedef struct dr_index_entry_info * dr_index_entry_info_t;

typedef struct dr_value {
    uint8_t m_type;
    LPDRMETA m_meta;
    void * m_data;
    size_t m_size;
} * dr_value_t;

typedef struct dr_data {
    LPDRMETA m_meta;
    void * m_data;
    size_t m_size;
} * dr_data_t;

typedef struct dr_data_it {
    dr_data_t (*next)(struct dr_data_it * it);
    char m_data[64];
} * dr_data_it_t;

typedef struct dr_data_entry {
    LPDRMETAENTRY m_entry;
    void * m_data;
    size_t m_size;
} * dr_data_entry_t;

typedef struct dr_data_source {
    struct dr_data m_data;
    struct dr_data_source * m_next;
} * dr_data_source_t;

enum dr_code_error {
    dr_code_error_format_error = -1
    , dr_code_error_not_enough_input = -2
    , dr_code_error_not_enough_output = -3
    , dr_code_error_internal = -4
};

typedef struct dr_meta_dyn_info {
    enum {
        dr_meta_dyn_info_type_union
        , dr_meta_dyn_info_type_array
    } m_type;
    union {
        struct {
            LPDRMETAENTRY m_union_entry;
            uint32_t m_union_start;
            LPDRMETAENTRY m_union_select_entry;
            uint32_t m_union_select_start;
        } m_union;
        struct {
            LPDRMETAENTRY m_array_entry;
            uint32_t m_array_start;
            LPDRMETAENTRY m_refer_entry;
            uint32_t m_refer_start;
        } m_array;
    } m_data;
} * dr_meta_dyn_info_t;

struct tagDRLibParam
{
	int iID;

	int iTagSetVersion;

	size_t iSize;
	size_t iMacrosGroupSize;
	size_t iMetaSize;
	size_t iStrBufSize;

	int iMaxMacros;
	int iMaxMetas;

	int iMaxMacrosGroupNum;

	int iRes;
	int  iVersion;

	char szName[CPE_DR_NAME_LEN];
};

typedef enum dr_metalib_source_type {
    dr_metalib_source_type_file
    , dr_metalib_source_type_memory
} dr_metalib_source_type_t;

typedef enum dr_metalib_source_format {
    dr_metalib_source_format_xml
} dr_metalib_source_format_t;

typedef enum dr_metalib_source_from {
    dr_metalib_source_from_user
    , dr_metalib_source_from_depend
} dr_metalib_source_from_t;

typedef enum dr_metalib_source_state {
    dr_metalib_source_state_analized
    , dr_metalib_source_state_not_analize
    , dr_metalib_source_state_analizing
} dr_metalib_source_state_t;

typedef struct dr_metalib_source_element * dr_metalib_source_element_t;
typedef struct dr_metalib_source * dr_metalib_source_t;
typedef struct dr_metalib_builder * dr_metalib_builder_t;

typedef enum dr_metalib_source_element_type {
    dr_metalib_source_element_type_lib
    , dr_metalib_source_element_type_macro
    , dr_metalib_source_element_type_meta
} dr_metalib_source_element_type_t;

typedef struct dr_metalib_source_element_it {
    dr_metalib_source_element_t (*next)(struct dr_metalib_source_element_it * it);
    dr_metalib_source_element_t m_data;
} * dr_metalib_source_element_it_t;

typedef struct dr_metalib_source_it {
    dr_metalib_source_t (*next)(struct dr_metalib_source_it * it);
    char m_data[16];
} * dr_metalib_source_it_t;

struct DRInBuildMetaLib;
struct DRInBuildMacro;
struct DRInBuildMeta;
struct DRInBuildMetaEntry;

#define dr_data_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
