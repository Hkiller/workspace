#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom/pom_object.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "pom_grp_internal_ops.h"

uint16_t pom_grp_obj_list_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_count: entry %s not exist!", entry);
        return 0;
    }

    return pom_grp_obj_list_count_ex(mgr, obj, entry_meta);
}

uint16_t pom_grp_obj_list_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_capacity: entry %s not exist!", entry);
        return 0;
    }

    return pom_grp_obj_list_capacity_ex(mgr, obj, entry_meta);
}

void * pom_grp_obj_list_at(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_at: entry %s not exist!", entry);
        return NULL;
    }

    return pom_grp_obj_list_at_ex(mgr, obj, entry_meta, pos);
}

int pom_grp_obj_list_append(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_append: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_list_append_ex(mgr, obj, entry_meta, data);
}

int pom_grp_obj_list_insert(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos, void * data) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_insert: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_list_insert_ex(mgr, obj, entry_meta, pos, data);
}

int pom_grp_obj_list_remove(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_remove: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_list_remove_ex(mgr, obj, entry_meta, pos);
}

int pom_grp_obj_list_sort(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, int (*cmp)(void const *, void const *)) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_sort: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_list_sort_ex(mgr, obj, entry_meta, cmp);
}

void * pom_grp_obj_list_bsearch(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void const * key, int (*cmp)(void const *, void const *)) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_bsearch: entry %s not exist!", entry);
        return NULL;
    }

    return pom_grp_obj_list_bsearch_ex(mgr, obj, entry_meta, key, cmp);
}

void * pom_grp_obj_list_lsearch(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void const * key, int (*cmp)(void const *, void const *)) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_list_lsearch: entry %s not exist!", entry);
        return NULL;
    }

    return pom_grp_obj_list_lsearch_ex(mgr, obj, entry_meta, key, cmp);
}

uint16_t * pom_grp_obj_list_count_buf(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    return ((uint16_t *)(((char *)obj) + mgr->m_meta->m_size_buf_start)) + entry->m_data.m_list.m_size_idx;
}

uint16_t pom_grp_obj_list_count_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_list);
    return *pom_grp_obj_list_count_buf(mgr, obj, entry);
}

uint16_t pom_grp_obj_list_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_list);
    return entry->m_data.m_list.m_capacity;
}

void * pom_grp_obj_list_at_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos) {
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t page_pos;
    uint16_t pos_in_page;
    pom_oid_t oid;
    uint16_t * count;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_list);

    if (pos >= entry->m_data.m_list.m_capacity) return NULL;

    count = pom_grp_obj_list_count_buf(mgr, obj, entry);
    assert(*count >= 0);
    assert(*count <= entry->m_data.m_list.m_capacity);
    if (pos >= *count) return NULL;

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_page_size % element_size == 0);

    count_in_page = entry->m_page_size / element_size;
    assert(count_in_page > 0);

    page_pos = pos / count_in_page;
    pos_in_page = pos % count_in_page;
    assert(page_pos < entry->m_page_count);

    oid = ((pom_oid_t *)obj)[entry->m_page_begin + page_pos];
    assert(oid != POM_INVALID_OID);

    page_buf = ((char*)pom_obj_get(mgr->m_omm, oid, mgr->m_em));
    assert(page_buf);

    return page_buf + pos_in_page * element_size;
}

int pom_grp_obj_list_append_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data) {
    uint16_t next_pos;
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t page_pos;
    uint16_t pos_in_page;
    pom_oid_t * oid;
    uint16_t * count;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_list);

    count = pom_grp_obj_list_count_buf(mgr, obj, entry);
    if (*count >= entry->m_data.m_list.m_capacity) {
        CPE_ERROR(
            mgr->m_em, "pom_grp_obj_list_append_ex: entry %s overflow, capacity=%d!",
            entry->m_name, entry->m_data.m_list.m_capacity);
        return -1;
    }


    next_pos = *count;

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_page_size % element_size == 0);

    count_in_page = entry->m_page_size / element_size;
    assert(count_in_page > 0);

    page_pos = next_pos / count_in_page;
    pos_in_page = next_pos % count_in_page;

    assert(page_pos < entry->m_page_count);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin + page_pos;
    if (*oid == POM_INVALID_OID) {
        *oid = pom_obj_alloc(mgr->m_omm, pom_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (*oid == POM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "pom_grp_obj_list_append_ex: alloc %s buf fail!", entry->m_name);
            return -1;
        }
    }

    page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    if (data) memcpy(page_buf + pos_in_page * element_size, data, element_size);
    (*count)++;

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return 0;
}

int pom_grp_obj_list_insert_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos, void * data) {
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t insert_page_pos;
    uint16_t insert_pos_in_page;
    uint16_t last_page_pos;
    uint16_t last_pos_in_page;
    pom_oid_t * oid;
    uint16_t * count;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_list);

    count = pom_grp_obj_list_count_buf(mgr, obj, entry);
    if (*count == pos) return pom_grp_obj_list_append_ex(mgr, obj, entry, data);

    if (pos > *count) {
        CPE_ERROR(
            mgr->m_em, "pom_grp_obj_list_append_ex: entry %s: insert pos invalid, count=%d!",
            entry->m_name, *count);
        return -1;
    }

    if (*count >= entry->m_data.m_list.m_capacity) {
        CPE_ERROR(
            mgr->m_em, "pom_grp_obj_list_insert_ex: entry %s: insert overflow, capacity=%d!",
            entry->m_name, entry->m_data.m_list.m_capacity);
        return -1;
    }

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_page_size % element_size == 0);

    count_in_page = entry->m_page_size / element_size;
    assert(count_in_page > 0);

    insert_page_pos = pos / count_in_page;
    insert_pos_in_page = pos % count_in_page;
    assert(insert_page_pos < entry->m_page_count);

    last_page_pos = (*count) / count_in_page;
    last_pos_in_page = (*count) % count_in_page;
    assert(last_page_pos < entry->m_page_count);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin + last_page_pos;
    if (*oid == POM_INVALID_OID) {
        *oid = pom_obj_alloc(mgr->m_omm, pom_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (*oid == POM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "pom_grp_obj_list_insert_ex: alloc %s buf fail!", entry->m_name);
            return -1;
        }
    }

    page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    while(last_page_pos > insert_page_pos) {
        char * pre_page_buf;

        if (last_pos_in_page > 0) {
            memmove(page_buf + element_size, page_buf, element_size * (last_pos_in_page - 1));
        }

        --oid;
        assert(*oid != POM_INVALID_OID);

        pre_page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
        assert(pre_page_buf);

        memmove(page_buf, pre_page_buf + element_size * (count_in_page - 1), element_size);

        page_buf = pre_page_buf;
        --last_page_pos;
        last_pos_in_page = count_in_page - 1;
    }

    
    assert(*oid != POM_INVALID_OID);
    assert(page_buf);

    assert(*oid == *(((pom_oid_t *)obj) + entry->m_page_begin + insert_page_pos));
    assert(page_buf == pom_obj_get(mgr->m_omm, *oid, mgr->m_em));

    if (insert_pos_in_page + 1 < count_in_page) {
        memmove(
            page_buf + element_size * (insert_pos_in_page + 1),
            page_buf + element_size * insert_pos_in_page,
            element_size * (count_in_page - (insert_pos_in_page + 1)));
    }

    if (data) memcpy(page_buf + element_size * insert_pos_in_page, data, element_size);
    ++(*count);

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return 0;
}

int pom_grp_obj_list_remove_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos) {
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t remove_page_pos;
    uint16_t remove_pos_in_page;
    uint16_t last_page_pos;
    pom_oid_t * oid;
    uint16_t * count;
    char * page_buf;
    uint16_t last_page_left_count;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_list);

    count = pom_grp_obj_list_count_buf(mgr, obj, entry);
    if (pos >= *count) return -1;

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_page_size % element_size == 0);

    count_in_page = entry->m_page_size / element_size;
    assert(count_in_page > 0);

    remove_page_pos = pos / count_in_page;
    remove_pos_in_page = pos % count_in_page;
    assert(remove_page_pos < entry->m_page_count);

    assert(*count > 0);

    last_page_pos = ((*count) - 1) / count_in_page;
    assert(last_page_pos < entry->m_page_count);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin + remove_page_pos;
    assert(*oid != POM_INVALID_OID);

    page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    while(remove_page_pos < last_page_pos) {
        char * next_page_buf;

        if (remove_pos_in_page + 1 < count_in_page) {
            memmove(
                page_buf + element_size * remove_pos_in_page,
                page_buf + element_size * (remove_pos_in_page + 1),
                element_size * (count_in_page - remove_pos_in_page - 1));
        }

        ++oid;
        assert(*oid != POM_INVALID_OID);

        next_page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
        assert(next_page_buf);

        memmove(page_buf + element_size * (count_in_page - 1), next_page_buf, element_size);

        page_buf = next_page_buf;
        ++remove_page_pos;
        remove_pos_in_page = 0;
    };

    last_page_left_count =
        (*count)
        - (count_in_page * remove_page_pos)
        - remove_pos_in_page
        - 1;
    if (last_page_left_count > 0) {
        memmove(
            page_buf + element_size * remove_pos_in_page,
            page_buf + element_size * (remove_pos_in_page + 1),
            element_size * last_page_left_count);
    }
    else {
        if (remove_pos_in_page == 0) {
            pom_obj_free(mgr->m_omm, *oid, mgr->m_em);
            *oid = POM_INVALID_OID;
        }
    }
    
    --(*count);
    assert(*count >= 0);
    assert(*count <= entry->m_data.m_list.m_capacity);

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return 0;
}

int pom_grp_obj_list_clear_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    uint16_t * count;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_list);

    count = pom_grp_obj_list_count_buf(mgr, obj, entry);
    *count = 0;

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return 0;
}

int pom_grp_obj_list_sort_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, int (*cmp)(void const *, void const *)) {
    return 0;
}

void * pom_grp_obj_list_bsearch_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, const void * key, int (*cmp)(void const *, void const *)) {
    return NULL;
}

void * pom_grp_obj_list_lsearch_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, const void * key, int (*cmp)(void const *, void const *)) {
    return NULL;
}
