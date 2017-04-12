#include <assert.h>
#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_error.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

int dr_lib_meta_num(LPDRMETALIB metaLib) {
    return metaLib->m_meta_count;
}

LPDRMETA dr_lib_meta_at(LPDRMETALIB metaLib, int idx) {
    assert(metaLib);

    if (idx >= 0 && idx < metaLib->m_meta_count) {
        char * base = (char *)(metaLib + 1);
        struct idx_meta_by_orig * idx_start = (struct idx_meta_by_orig *)(base + metaLib->m_startpos_meta_by_orig);

        return (LPDRMETA)(base + (idx_start + idx)->m_diff_to_base);
    }
    else {
        return NULL;
    }
}

LPDRMETA dr_lib_find_meta_by_name(LPDRMETALIB metaLib, const char* name) {
    char * base;
    struct idx_meta_by_name * searchStart;
    int beginPos, endPos, curPos;

    assert(metaLib);
    assert(name);

    base = (char *)(metaLib + 1);

    searchStart = (struct idx_meta_by_name * )
        (base + metaLib->m_startpos_meta_by_name);

    for(beginPos = 0, endPos = metaLib->m_meta_count, curPos = (endPos - beginPos - 1) / 2;
        beginPos < endPos;
        curPos = beginPos + (endPos - beginPos - 1) / 2)
    {
        struct idx_meta_by_name * curItem;
        int cmp_result;

        curItem = searchStart + curPos;
        cmp_result = strcmp(name, base + curItem->m_name_pos);
        if (cmp_result == 0) {
            return (LPDRMETA)(base + curItem->m_diff_to_base);
        }
        else if (cmp_result < 0) {
            endPos = curPos;
        }
        else { /*cmp_result > 0*/
            beginPos = curPos + 1;
        }
    }
    
    return NULL;
}

LPDRMETA dr_lib_find_meta_by_id(LPDRMETALIB metaLib, int id) {
    char * base;
    struct idx_meta_by_id * searchStart;
    int beginPos, endPos, curPos;

    base = (char *)(metaLib + 1);

    searchStart = (struct idx_meta_by_id * )
        (base + metaLib->m_startpos_meta_by_id);

    for(beginPos = 0, endPos = metaLib->m_meta_count, curPos = (endPos - beginPos - 1) / 2;
        beginPos < endPos;
        curPos = beginPos + (endPos - beginPos - 1) / 2)
    {
        struct idx_meta_by_id * curItem = searchStart + curPos;

        if ((int32_t)id == (int32_t)curItem->m_id) {
            return (LPDRMETA)(base + curItem->m_diff_to_base);
        }
        else if ((int32_t)id < (int32_t)curItem->m_id) {
            endPos = curPos;
        }
        else { /*id > curItem->m_id*/
            beginPos = curPos + 1;
        }
    }
    
    return NULL;
}

int dr_meta_based_version(LPDRMETA meta) {
    return meta->m_based_version;
}

int dr_meta_current_version(LPDRMETA meta) {
    return meta->m_current_version;
}

int dr_meta_type(LPDRMETA meta) {
    return meta->m_type;
}

const char *dr_meta_name(LPDRMETA meta) {
    return (char *)(meta) - meta->m_self_pos + meta->m_name_pos;
}

const char *dr_meta_desc(LPDRMETA meta) {
    if (meta->m_desc_pos < 0) {
        return "";
    }
    else {
        return (char *)(meta) - meta->m_self_pos + meta->m_desc_pos;
    }
}

int dr_meta_id(LPDRMETA meta) {
    return meta->m_id;
}

size_t dr_meta_size(LPDRMETA meta) {
    return meta->m_real_data_size;
}

int dr_meta_align(LPDRMETA meta) {
    return meta->m_align;
}

int dr_meta_require_align(LPDRMETA meta) {
    return meta->m_require_align;
}

int dr_meta_entry_num(LPDRMETA meta) {
    return meta->m_entry_count;
}

char * dr_meta_off_to_path(LPDRMETA meta, int a_iOff, char * a_pBuf, size_t a_iBufSize) {
    int writePos = 0;
    LPDRMETA pstCurMeta = meta;
    LPDRMETAENTRY pstEntry;
    char * base = (char *)(meta) - meta->m_self_pos;

    if (a_iBufSize <= 0) {
        return NULL;
    }

    while(a_iOff >= 0 && (size_t)writePos < a_iBufSize) {
        int beginPos, endPos, curPos;
        LPDRMETAENTRY curEntry = NULL;

        for(beginPos = 0, endPos = pstCurMeta->m_entry_count, curPos = (endPos - beginPos - 1) / 2;
            beginPos < endPos && curEntry == NULL;
            curPos = beginPos + (endPos - beginPos - 1) / 2)
        {
            pstEntry = dr_meta_entry_at(pstCurMeta, curPos);

            if (a_iOff < pstEntry->m_data_start_pos) {
                endPos = curPos;
            }
            else { /* a_iOff >= pstEntry->m_data_start_pos */
                if (a_iOff < (pstEntry->m_data_start_pos + pstEntry->m_unitsize)) {
                    curEntry = pstEntry;
                }
                else {
                    beginPos = curPos + 1;
                }
            }
        }

        if (curEntry == NULL) {
            return NULL;
        }

        writePos += snprintf(a_pBuf + writePos,
                             a_iBufSize - writePos,
                             pstCurMeta == meta ? "%s" : ".%s",
                             dr_entry_name(curEntry));

        if (curEntry->m_type == CPE_DR_TYPE_STRUCT) {
            pstCurMeta = (LPDRMETA)(base + curEntry->m_ref_type_pos);
            a_iOff -= curEntry->m_data_start_pos;
        }
        else {
            break;
        }
    }

    return writePos == 0 ? NULL : a_pBuf;
}

int dr_meta_path_to_off(LPDRMETA meta, const char * path, LPDRMETAENTRY * entry) {
    int off;
    LPDRMETAENTRY e = dr_meta_find_entry_by_path_ex(meta, path, &off);
    if (e) {
        if (entry) *entry = e;
        return off;
    }
    else {
        if (entry) *entry = NULL;
        return -1;
    }
}

LPDRMETALIB dr_meta_owner_lib(LPDRMETA meta) {
    char * base = (char *)(meta) - meta->m_self_pos;
    return ((LPDRMETALIB)base) - 1;
}

LPDRMETAENTRY dr_meta_find_entry_by_name(LPDRMETA meta, const char* name) {
    uint32_t i;
    char * base = (char *)(meta) - meta->m_self_pos;
    LPDRMETAENTRY pstEntryBegin = (LPDRMETAENTRY)(meta + 1);

    for(i = 0; i < meta->m_entry_count; ++i) {
        LPDRMETAENTRY pstCurEntry = pstEntryBegin + i;

        if (strcmp(base + pstCurEntry->m_name_pos, name) == 0) {
            return pstCurEntry;
        }
    }

    return NULL;
}

LPDRMETAENTRY dr_meta_find_entry_by_id(LPDRMETA meta, int a_iId) {
    uint32_t i;
    LPDRMETAENTRY pstEntryBegin = (LPDRMETAENTRY)(meta + 1);

    if (a_iId < 0) {
        return NULL;
    }

    for(i = 0; i < meta->m_entry_count; ++i) {
        LPDRMETAENTRY pstCurEntry = pstEntryBegin + i;

        if (pstCurEntry->m_id == a_iId) {
            return pstCurEntry;
        }
    }

    return NULL;
}

int dr_meta_find_entry_idx_by_name(LPDRMETA meta, const char* name) {
    uint32_t i;
    char * base = (char *)(meta) - meta->m_self_pos;
    LPDRMETAENTRY pstEntryBegin = (LPDRMETAENTRY)(meta + 1);

    for(i = 0; i < meta->m_entry_count; ++i) {
        LPDRMETAENTRY pstCurEntry = pstEntryBegin + i;

        if (strcmp(base + pstCurEntry->m_name_pos, name) == 0) {
            return i;
        }
    }

    return -1;
}

int dr_meta_find_entry_idx_by_id(LPDRMETA meta, int a_iId) {
    uint32_t i;
    LPDRMETAENTRY pstEntryBegin = (LPDRMETAENTRY)(meta + 1);

    if (a_iId < 0) {
        return -1;
    }

    for(i = 0; i < meta->m_entry_count; ++i) {
        LPDRMETAENTRY pstCurEntry = pstEntryBegin + i;

        if (pstCurEntry->m_id == a_iId) {
            return i;
        }
    }

    return -1;
}

LPDRMETAENTRY dr_meta_entry_at(LPDRMETA meta, int a_idxEntry) {
    if (a_idxEntry < 0 || (uint32_t)a_idxEntry >= meta->m_entry_count) {
        return NULL;
    }

    return (struct tagDRMetaEntry *)(meta + 1) + a_idxEntry;
}

int dr_meta_key_entry_num(LPDRMETA meta) {
    return meta->m_key_num;
}

dr_idx_entry_info_t dr_meta_key_info_at(LPDRMETA meta, int idx) {
    if (idx < 0 || idx >= meta->m_key_num) return NULL;
    return ((dr_idx_entry_info_t)(((char*)meta) + meta->m_key_start_from_meta)) + idx;
}

LPDRMETAENTRY dr_meta_key_entry_at(LPDRMETA meta, int idx) {
    dr_idx_entry_info_t info;
    char * base = (char *)(meta) - meta->m_self_pos;

    info = dr_meta_key_info_at(meta, idx);
    if (info == NULL) return NULL;
    return (LPDRMETAENTRY)(base + info->m_entry_diff_to_base);
}

int dr_meta_index_num(LPDRMETA meta) {
    return meta->m_index_count;
}

dr_index_info_t dr_meta_index_at(LPDRMETA meta, int idx) {
    if (idx < 0 || (uint32_t)idx >= meta->m_index_count) return NULL;

    return ((dr_index_info_t)(((char*)meta) + meta->m_index_pos_from_meta)) + idx;
}

LPDRMETA dr_index_meta(dr_index_info_t index) {
    return (LPDRMETA)(((char *)index) - index->m_diff_to_meta);
}

const char * dr_index_name(dr_index_info_t index) {
    if (index->m_name_pos < 0) {
        return "";
    }
    else {
        LPDRMETA meta = (LPDRMETA)(((char *)index) - index->m_diff_to_meta);
        char * base = (char *)(meta) - meta->m_self_pos;
        return base + index->m_name_pos;
    }
}

int dr_index_entry_num(dr_index_info_t index) {
    return index->m_entry_num;
}

dr_index_entry_info_t dr_index_entry_info_at(dr_index_info_t index, int idx) {
    LPDRMETA meta;

    if (idx < 0 || idx >= index->m_entry_num) return NULL;

    meta = (LPDRMETA)(((char *)index) - index->m_diff_to_meta);

    return ((dr_index_entry_info_t)(((char*)meta) + index->m_entry_start_pos_to_meta)) + idx;
}

LPDRMETAENTRY dr_index_entry_at(dr_index_info_t index, int idx) {
    LPDRMETA meta;
    char * base;
    dr_index_entry_info_t info = dr_index_entry_info_at(index, idx);
    if (info == NULL) return NULL;

    meta = (LPDRMETA)(((char *)index) - index->m_diff_to_meta);
    base = (char *)(meta) - meta->m_self_pos;
    return (LPDRMETAENTRY)(base + info->m_entry_diff_to_base);
}

LPDRMETAENTRY dr_meta_find_entry_by_path(LPDRMETA meta, const char* entryPath) {
    return dr_meta_find_entry_by_path_ex(meta, entryPath, NULL);
}

LPDRMETAENTRY dr_meta_lsearch_entry_by_type_name(LPDRMETA meta, const char * type_name) {
    int i;
    size_t count;
    for(i = 0, count = dr_meta_entry_num(meta); i < count; ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(meta, i);
        if (strcmp(dr_entry_type_name(entry), type_name) == 0) return entry;
    }

    return NULL;
}

LPDRMETAENTRY dr_meta_find_entry_by_name_len(LPDRMETA meta, const char * entry_name, size_t entry_name_len) {
    int i;
    LPDRMETAENTRY entry_base = (LPDRMETAENTRY)(meta + 1);
    char * base = (char *)(meta) - meta->m_self_pos;

    for(i = 0; i < meta->m_entry_count; ++i) {
        LPDRMETAENTRY entry = entry_base + i;
        const char * cur_entry_name = base + entry->m_name_pos;
        if (strncmp(cur_entry_name, entry_name, entry_name_len) == 0
            && strlen(cur_entry_name) == entry_name_len)
        {
            return entry;
        }
    }

    return NULL;
}

LPDRMETAENTRY dr_meta_find_entry_by_path_ex(LPDRMETA meta, const char* entryPath, int * off) {
    char * base;
    LPDRMETA cur_meta;
    LPDRMETAENTRY entry = NULL;
    const char * checkBegin;
    const char * pointPos;
    const char * arrayPos;

    assert(meta);
    assert(entryPath);

    base = (char *)(meta) - meta->m_self_pos;

    cur_meta = meta;
    if (off) *off = 0;

    checkBegin = entryPath;
    pointPos = strchr(checkBegin, '.');
    arrayPos = strchr(checkBegin, '[');

    while(arrayPos) {
        if (*checkBegin == '[') {
            char * array_end;
            int array_pos;
            
            assert(checkBegin == arrayPos);

            if (entry == NULL) return NULL;
            if (entry->m_array_count == 1) return NULL;

            array_pos = (int)strtol(checkBegin + 1, &array_end, 10);
            if (array_end == NULL || *array_end != ']') return NULL;

            if (array_pos < 0 || (entry->m_array_count > 0 && array_pos >= entry->m_array_count)) return NULL;

            if (entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
                cur_meta = (LPDRMETA)(base + entry->m_ref_type_pos);
            }
            else {
                cur_meta = NULL;
            }

            if (off) *off += dr_entry_element_size(entry) * array_pos;

            checkBegin = array_end + 1;

            if (*checkBegin == '.') {
                checkBegin++;
                pointPos = *checkBegin ? strchr(checkBegin, '.') : NULL;
            }

            arrayPos = *checkBegin ? strchr(checkBegin, '[') : NULL;
        }
        else {
            if (cur_meta == NULL) return NULL;

            if (pointPos && pointPos < arrayPos) {
                entry = dr_meta_find_entry_by_name_len(cur_meta, checkBegin, pointPos - checkBegin);
                if (entry == NULL) return NULL;

                if (entry->m_type > CPE_DR_TYPE_COMPOSITE) {
                    cur_meta = NULL;
                }
                else {
                    if (entry->m_array_count == 1) {
                        cur_meta = (LPDRMETA)(base + entry->m_ref_type_pos);
                    }
                    else {
                        cur_meta = NULL;
                    }
                }

                if (off) *off += entry->m_data_start_pos;

                checkBegin = pointPos + 1;
                pointPos = *checkBegin ? strchr(checkBegin, '.') : NULL;
            }
            else {
                entry = dr_meta_find_entry_by_name_len(cur_meta, checkBegin, arrayPos - checkBegin);
                if (entry == NULL) return NULL;

                if (off) *off += entry->m_data_start_pos;

                checkBegin = arrayPos;
            }
        }
    }

    while(pointPos) {
        if (cur_meta == NULL) return NULL;

        entry = dr_meta_find_entry_by_name_len(cur_meta, checkBegin, pointPos - checkBegin);
        if (entry == NULL) return NULL;

        if (entry->m_type > CPE_DR_TYPE_COMPOSITE) {
            cur_meta = NULL;
        }
        else {
            if (entry->m_array_count == 1) {
                cur_meta = (LPDRMETA)(base + entry->m_ref_type_pos);
            }
            else {
                cur_meta = NULL;
            }
        }

        if (off) *off += entry->m_data_start_pos;

        checkBegin = pointPos + 1;
        pointPos = *checkBegin ? strchr(checkBegin, '.') : NULL;
    }

    if (*checkBegin) {
        if (cur_meta == NULL) return NULL;

        entry = dr_meta_find_entry_by_name(cur_meta, checkBegin);
        if (entry && off) *off += entry->m_data_start_pos;
    }

    return entry;
}

int dr_entry_version(LPDRMETAENTRY entry) {
    assert(entry);
    return entry->m_version;
}

const char *dr_entry_name(LPDRMETAENTRY entry) {
    assert(entry);
    if (entry->m_name_pos < 0) {
        return "";
    }
    else {
        LPDRMETA pstMeta = (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
        char * base = (char *)(pstMeta) - pstMeta->m_self_pos;
        return base + entry->m_name_pos;
    }
}

const char *dr_entry_cname(LPDRMETAENTRY entry) {
    assert(entry);
    if (entry->m_cname_pos < 0) {
        return "";
    }
    else {
        LPDRMETA pstMeta = (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
        char * base = (char *)(pstMeta) - pstMeta->m_self_pos;
        return base + entry->m_cname_pos;
    }
}

const char *dr_entry_desc(LPDRMETAENTRY entry) {
    assert(entry);
    if (entry->m_desc_pos < 0) {
        return "";
    }
    else {
        LPDRMETA pstMeta = (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
        char * base = (char *)(pstMeta) - pstMeta->m_self_pos;
        return base + entry->m_desc_pos;
    }
}

const void * dr_entry_dft_value(LPDRMETAENTRY entry) {
    assert(entry);
    if (entry->m_dft_value_pos < 0) {
        return NULL;
    }
    else {
        LPDRMETA pstMeta = (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
        char * base = (char *)(pstMeta) - pstMeta->m_self_pos;
        return (const void *)(base + entry->m_dft_value_pos);
    }
}

int dr_entry_is_key(LPDRMETAENTRY entry) {
    int i;
    LPDRMETA meta = dr_entry_self_meta(entry);

    for(i = 0; i < meta->m_key_num; ++i) {
        dr_idx_entry_info_t key_info = dr_meta_key_info_at(meta, i);
        assert(key_info);

        if (key_info->m_data_start_pos == entry->m_data_start_pos) return 1;
    }

    return 0;

}

LPDRMETAENTRY
dr_entry_select_entry(LPDRMETAENTRY entry) {
    assert(entry);
    if (entry->m_select_entry_pos < 0) {
        return NULL;
    }
    else {
        LPDRMETA pstMeta = (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
        char * base = (char *)(pstMeta) - pstMeta->m_self_pos;
        return (LPDRMETAENTRY)(base + entry->m_select_entry_pos);
    }
}

LPDRMACROSGROUP dr_entry_macrosgroup(LPDRMETAENTRY entry) {
    //TODO
    return NULL;
}

int dr_entry_id(LPDRMETAENTRY entry) {
    return entry->m_id;
}

size_t dr_entry_size(LPDRMETAENTRY entry) {
    return entry->m_unitsize;
}

int dr_entry_type(LPDRMETAENTRY entry) {
    return entry->m_type;
}

const char * dr_entry_type_name(LPDRMETAENTRY entry) {
    if (entry->m_type > CPE_DR_TYPE_COMPOSITE) {
        const struct tagDRCTypeInfo * typeInfo;

        typeInfo = dr_find_ctype_info_by_type(entry->m_type);

        return typeInfo == NULL ? "unknown-type" : typeInfo->m_name;
    }
    else {
        return dr_meta_name(dr_entry_ref_meta(entry));
    }
}

int dr_entry_array_count(LPDRMETAENTRY entry) {
    return entry->m_array_count;
}

size_t dr_entry_data_start_pos(LPDRMETAENTRY entry, int index) {
    if (entry->m_array_count == 1 || index == 0) return entry->m_data_start_pos;
    
    return entry->m_data_start_pos
        + dr_entry_element_size(entry) * index;
}

size_t dr_entry_array_calc_ele_num(LPDRMETAENTRY entry, size_t capacity) {
    return capacity / dr_entry_element_size(entry);
}

size_t dr_entry_array_calc_buf_capacity(LPDRMETAENTRY entry, size_t count) {
    return count * dr_entry_element_size(entry);
}

size_t dr_entry_require_align(LPDRMETAENTRY entry) {
    if (entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
        LPDRMETA self_meta = dr_entry_self_meta(entry);
        LPDRMETA ref_meta = dr_entry_ref_meta(entry);
        return ref_meta->m_require_align > self_meta->m_align ? self_meta->m_align : ref_meta->m_require_align;
    }
    else if (entry->m_type == CPE_DR_TYPE_STRING) {
        return 1;
    }
    else {
        const struct tagDRCTypeInfo * typeInfo;
        typeInfo = dr_find_ctype_info_by_type(entry->m_type);
        if (typeInfo == NULL) return 0;
        return typeInfo->m_size;
    }
}

size_t dr_entry_element_size(LPDRMETAENTRY entry) {
    if (entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
        LPDRMETA refMeta = dr_entry_ref_meta(entry);
        if (refMeta == NULL) return 0;
        return dr_meta_size(refMeta);
    }
    else if (entry->m_type == CPE_DR_TYPE_STRING) {
        return entry->m_size;
    }
    else {
        const struct tagDRCTypeInfo * typeInfo;
        typeInfo = dr_find_ctype_info_by_type(entry->m_type);
        if (typeInfo == NULL) return 0;
        return typeInfo->m_size;
    }
}

LPDRMETAENTRY
dr_entry_array_refer_entry(LPDRMETAENTRY entry) {
    assert(entry);
    if (entry->m_array_count == 1 || entry->m_array_refer_entry_pos < 0) {
        return NULL;
    }
    else {
        LPDRMETA pstMeta = (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
        char * base = (char *)(pstMeta) - pstMeta->m_self_pos;
        return (LPDRMETAENTRY)(base + entry->m_array_refer_entry_pos);
    }
}

LPDRMETA dr_entry_self_meta(LPDRMETAENTRY entry) {
    return (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
}

LPDRMETA dr_entry_ref_meta(LPDRMETAENTRY entry) {
    if (entry->m_type > CPE_DR_TYPE_COMPOSITE) {
        return NULL;
    }
    else {
        LPDRMETA pstMeta = (LPDRMETA)((char * )entry - entry->m_self_to_meta_pos);
        char * base = (char *)(pstMeta) - pstMeta->m_self_pos;
        return (LPDRMETA)(base + entry->m_ref_type_pos);
    }
}

const char *dr_entry_customattr(LPDRMETALIB metaLib, LPDRMETAENTRY entry) {
    //TODO
    return NULL;
}

int dr_lib_find_macro_value(int *result, LPDRMETALIB metaLib, const  char *name) {
    LPDRMACRO macro;

    assert(result);

    macro = dr_lib_macro_find(metaLib, name);
    if (macro) {
        *result = macro->m_value;
        return 0;
    }
    else {
        return -1;
    }
}

int dr_lib_macro_num(LPDRMETALIB metaLib) {
    assert(metaLib);

    return metaLib->m_macro_count;
}

LPDRMACRO dr_lib_macro_at(LPDRMETALIB metaLib, int a_iIdx) {
    assert(metaLib);

    if (a_iIdx < 0 || a_iIdx >= metaLib->m_macro_count) {
        return 0;
    }
    else {
        char * base = (char *)(metaLib + 1);
        return (struct tagDRMacro*)(base + metaLib->m_startpos_macro) + a_iIdx;
    }
}

LPDRMACRO dr_lib_macro_find(LPDRMETALIB metaLib, const char * name) {
    char * base;
    int i;
    struct tagDRMacro* macros = 0;
    struct tagDRMacro* curMacro = 0;

    assert(metaLib);
    assert(name);

    base = (char *)(metaLib + 1);
    macros = (struct tagDRMacro*)(base + metaLib->m_startpos_macro);

    for(i = 0; i < metaLib->m_macro_count; ++i) {
        curMacro = macros + i;
        if (strcmp(name, (char *)(metaLib + 1) + curMacro->m_name_pos) == 0) {
            return curMacro;
        }
    }

    return NULL;
}

const char* dr_macro_name(LPDRMETALIB metaLib, LPDRMACRO a_pstMacro) {
    char * base;

    assert(metaLib);
    base = (char *)(metaLib + 1);

    return (char *)base + a_pstMacro->m_name_pos;
}

int dr_macro_value(LPDRMACRO a_pstMacro) {
    return a_pstMacro->m_value;
}

const char* dr_macro_desc(LPDRMETALIB metaLib, LPDRMACRO a_pstMacro) {
    assert(metaLib);
    if (a_pstMacro->m_desc_pos < 0) {
        return "";
    }
    else {
        char * base = (char *)(metaLib + 1);
        return (char *)(base) + a_pstMacro->m_desc_pos;
    }
}

LPDRMACRO dr_macrosgroup_find_macro_by_name(LPDRMETALIB metaLib, LPDRMACROSGROUP macroGroup, const char * name) {
    return dr_lib_macro_find(metaLib, name);
}

LPDRMACROSGROUP dr_macro_macrosgroup(LPDRMETALIB metaLib, LPDRMACRO a_pstMacro) {
    //TODO
    return NULL;
}

int dr_lib_macrosgroup_num(LPDRMETALIB metaLib) {
    //TODO
    return -1;
}

LPDRMACROSGROUP dr_lib_macrosgroup_at(LPDRMETALIB metaLib, int a_iIdx) {
    //TODO
    return NULL;
}

const char* dr_macrosgroup_name(LPDRMACROSGROUP a_pstGroup) {
    //TODO
    return NULL;
}

int dr_macrosgroup_macro_num(LPDRMACROSGROUP a_pstGroup) {
    //TODO
    return -1;
}

LPDRMACRO dr_macrosgroup_macro_at(LPDRMETALIB metaLib, LPDRMACROSGROUP a_pstGroup, int a_iIdx) {
    //TODO
    return NULL;
}

int dr_meta_find_dyn_info(LPDRMETA meta, dr_meta_dyn_info_t dyn_info) {
    LPDRMETAENTRY last_entry;

    if (dr_meta_type(meta) == CPE_DR_TYPE_UNION) {
        int i;
        for(i = 0; i < dr_meta_entry_num(meta); ++i) {
            LPDRMETA ref_meta = dr_entry_ref_meta(dr_meta_entry_at(meta, i));
            if (ref_meta == NULL) continue;

            if (dr_meta_find_dyn_info(ref_meta, dyn_info) == 0) {
                dyn_info->m_type = dr_meta_dyn_info_type_union;
                dyn_info->m_data.m_union.m_union_entry = NULL;
                dyn_info->m_data.m_union.m_union_start = 0;
                dyn_info->m_data.m_union.m_union_select_entry = NULL;
                dyn_info->m_data.m_union.m_union_select_start = 0;
                return 0;
            }
        }

        return -1;
    }
    else {
        if (dr_meta_entry_num(meta) == 0) return -1;

        last_entry = dr_meta_entry_at(meta, dr_meta_entry_num(meta) - 1);
        assert(last_entry);

        if (dr_entry_array_count(last_entry) == 0) {
            dyn_info->m_type = dr_meta_dyn_info_type_array;

            dyn_info->m_data.m_array.m_array_entry = last_entry;
            dyn_info->m_data.m_array.m_array_start = (uint32_t)dr_entry_data_start_pos(last_entry, 0);

            dyn_info->m_data.m_array.m_refer_entry = dr_entry_array_refer_entry(last_entry);
            if (dyn_info->m_data.m_array.m_refer_entry) {
                dyn_info->m_data.m_array.m_refer_start = (uint32_t)dr_entry_data_start_pos(dyn_info->m_data.m_array.m_refer_entry, 0);
            }

            return 0;
        }
        else if (dr_entry_type(last_entry) <= CPE_DR_TYPE_COMPOSITE) {
            if (dr_meta_find_dyn_info(dr_entry_ref_meta(last_entry), dyn_info) == 0) {
                switch(dyn_info->m_type) {
                case dr_meta_dyn_info_type_union:
                    if (dyn_info->m_data.m_union.m_union_entry == NULL) {
                        dyn_info->m_data.m_union.m_union_entry = last_entry;
                        dyn_info->m_data.m_union.m_union_start = (uint32_t)dr_entry_data_start_pos(last_entry, 0);

                        dyn_info->m_data.m_union.m_union_select_entry = dr_entry_select_entry(last_entry);
                        if (dyn_info->m_data.m_union.m_union_select_entry) {
                            dyn_info->m_data.m_union.m_union_select_start = (uint32_t)dr_entry_data_start_pos(dyn_info->m_data.m_union.m_union_select_entry, 0);
                        }
                    }
                    else {
                        dyn_info->m_data.m_union.m_union_start += dr_entry_data_start_pos(last_entry, 0);
                        if (dyn_info->m_data.m_union.m_union_select_entry) {
                            dyn_info->m_data.m_union.m_union_select_start += dr_entry_data_start_pos(last_entry, 0);
                        }
                    }

                    break;
                case dr_meta_dyn_info_type_array:
                    dyn_info->m_data.m_array.m_array_start += dr_entry_data_start_pos(last_entry, 0);
                    if (dyn_info->m_data.m_array.m_refer_entry) dyn_info->m_data.m_array.m_refer_start += dr_entry_data_start_pos(last_entry, 0);
                }

                return 0;
            }
            else {
                return -1;
            }
        }
        else {
            return -1;
        }
    }
}

ssize_t dr_meta_calc_dyn_size(LPDRMETA meta, size_t record_count) {
    struct dr_meta_dyn_info dyn_info;

    if (dr_meta_find_dyn_info(meta, &dyn_info) != 0) return -1;

    if (record_count > 1) {
        size_t element_size;
        element_size = dr_entry_element_size(dyn_info.m_data.m_array.m_array_entry);
        assert(element_size <= dr_meta_size(meta));
        return dr_meta_size(meta) + (record_count - 1) * element_size;
    }
    else {
        return dr_meta_size(meta);
    }
}

