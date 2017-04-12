#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "cpe/pom/pom_object.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "pom_grp_internal_ops.h"

uint16_t pom_grp_obj_ba_bit_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_bit_capacity: entry %s not exist!", entry);
        return 0;
    }

    return pom_grp_obj_ba_bit_capacity_ex(mgr, obj, entry_meta);
}

uint16_t pom_grp_obj_ba_byte_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_byte_capacity: entry %s not exist!", entry);
        return 0;
    }

    return pom_grp_obj_ba_byte_capacity_ex(mgr, obj, entry_meta);
}

uint16_t pom_grp_obj_ba_bit_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_count: entry %s not exist!", entry);
        return 0;
    }

    return pom_grp_obj_ba_bit_count_ex(mgr, obj, entry_meta);
}

int pom_grp_obj_ba_set_all(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, cpe_ba_value_t value) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_count: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_ba_set_all_ex(mgr, obj, entry_meta, value);
}

int pom_grp_obj_ba_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint32_t pos, cpe_ba_value_t value) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_count: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_ba_set_ex(mgr, obj, entry_meta, pos, value);
}

cpe_ba_value_t pom_grp_obj_ba_get(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint32_t pos) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_count: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_ba_get_ex(mgr, obj, entry_meta, pos);
}

int pom_grp_obj_ba_get_binary(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data, uint32_t capacity) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_get_all: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_ba_get_binary_ex(mgr, obj, entry_meta, data, capacity);
}

int pom_grp_obj_ba_set_binary(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void const * data, uint32_t capacity) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_set_all: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_ba_set_binary_ex(mgr, obj, entry_meta, data, capacity);
}

uint16_t pom_grp_obj_ba_bit_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);
    
    return entry->m_data.m_ba.m_bit_capacity;
}

uint16_t pom_grp_obj_ba_byte_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);
    
    return entry->m_page_size * entry->m_page_count;
}

uint16_t pom_grp_obj_ba_bit_count_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    uint16_t count;
    uint16_t i;
    pom_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin;

    count = 0;
    for(i = 0; i < entry->m_page_count; ++i, ++oid) {
        if (*oid == POM_INVALID_OID) continue;

        page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
        count += cpe_ba_count((cpe_ba_t)page_buf, cpe_ba_bits_from_bytes(entry->m_page_size));
    }

    return (uint16_t)count;
}

int pom_grp_obj_ba_set_all_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, cpe_ba_value_t value) {
    uint16_t i;
    pom_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin;

    if (value) {
        if (*oid == POM_INVALID_OID) {
            *oid = pom_obj_alloc(mgr->m_omm, pom_grp_entry_meta_name_hs(entry), mgr->m_em);
            if (*oid == POM_INVALID_OID) {
                CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_set_all_ex: alloc %s buf fail!", entry->m_name);
                return -1;
            }
        }

        page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
        memset(page_buf, entry->m_page_size, 0xff);
    }
    else {
        for(i = 0; i < entry->m_page_count; ++i, ++oid) {
            if (*oid == POM_INVALID_OID) continue;

            pom_obj_free(mgr->m_omm, *oid, mgr->m_em);
            *oid = POM_INVALID_OID;
        }
    }

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return 0;
}

int pom_grp_obj_ba_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint32_t pos, cpe_ba_value_t value) {
    uint16_t set_page_pos;
    uint32_t set_pos_in_page;
    pom_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);

    if (pos >= entry->m_data.m_ba.m_bit_capacity) return -1;

    set_page_pos = pos / cpe_ba_bits_from_bytes(entry->m_page_size);
    assert(set_page_pos < entry->m_page_count);
    assert(cpe_ba_bits_from_bytes(set_page_pos * entry->m_page_size) <= pos);
    set_pos_in_page = (uint32_t)(pos - cpe_ba_bits_from_bytes(set_page_pos * entry->m_page_size));

    oid = ((pom_oid_t *)obj) + entry->m_page_begin + set_page_pos;

    if (*oid == POM_INVALID_OID) {
        if (value == cpe_ba_false) return 0;

        *oid = pom_obj_alloc(mgr->m_omm, pom_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (*oid == POM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_set_ex: alloc %s buf fail!", entry->m_name);
            return -1;
        }

        page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
        bzero(page_buf, entry->m_page_size);
    }
    else {
        page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
    }

    assert(page_buf);

    cpe_ba_set((cpe_ba_t)page_buf, set_pos_in_page, value);

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return 0;
}

cpe_ba_value_t pom_grp_obj_ba_get_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint32_t pos) {
    uint16_t get_page_pos;
    uint32_t get_pos_in_page;
    pom_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);

    if (pos >= entry->m_data.m_ba.m_bit_capacity) return cpe_ba_false;

    get_page_pos = pos / cpe_ba_bits_from_bytes(entry->m_page_size);
    assert(get_page_pos < entry->m_page_count);
    assert(cpe_ba_bits_from_bytes(get_page_pos * entry->m_page_size) <= pos);
    get_pos_in_page = (uint32_t)(pos - cpe_ba_bits_from_bytes(get_page_pos * entry->m_page_size));

    oid = ((pom_oid_t *)obj) + entry->m_page_begin + get_page_pos;
    if (*oid == POM_INVALID_OID) return cpe_ba_false;

    page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    return cpe_ba_get((cpe_ba_t)page_buf, get_pos_in_page);
}

int pom_grp_obj_ba_get_binary_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data, uint32_t capacity) {
    pom_oid_t * oid;
    
    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);

    if (capacity != cpe_ba_bytes_from_bits(entry->m_data.m_ba.m_bit_capacity)) return -1;

    oid = ((pom_oid_t *)obj) + entry->m_page_begin;
    while(capacity > 0) {
        uint32_t copy_size = entry->m_page_size;
        if (copy_size > capacity) copy_size = capacity;

        if (*oid == POM_INVALID_OID) {
            bzero(data, copy_size);
        }
        else {
            char * page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
            memcpy(data, page_buf, copy_size);
        }

        ++oid;
        capacity -= copy_size;
        data = ((char *)data) + copy_size;
    }

    return 0;
}

int pom_grp_obj_ba_set_binary_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void const * data, uint32_t capacity) {
    pom_oid_t * oid;
    
    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_ba);

    if (capacity != cpe_ba_bytes_from_bits(entry->m_data.m_ba.m_bit_capacity)) return -1;

    oid = ((pom_oid_t *)obj) + entry->m_page_begin;
    while(capacity > 0) {
        char * page_buf;
        uint32_t copy_size = entry->m_page_size;
        if (copy_size > capacity) copy_size = capacity;

        if (*oid == POM_INVALID_OID) {
            *oid = pom_obj_alloc(mgr->m_omm, pom_grp_entry_meta_name_hs(entry), mgr->m_em);
            if (*oid == POM_INVALID_OID) {
                CPE_ERROR(mgr->m_em, "pom_grp_obj_ba_set_binary_ex: alloc %s buf fail!", entry->m_name);
                return -1;
            }
        }

        page_buf = ((char*)pom_obj_get(mgr->m_omm, *oid, mgr->m_em));
        memcpy(page_buf, data, copy_size);

        ++oid;
        capacity -= copy_size;
        data = ((const char *)data) + copy_size;
    }

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return 0;
}
