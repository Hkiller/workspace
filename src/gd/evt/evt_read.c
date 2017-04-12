#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_json.h"
#include "cpe/tl/tl_action.h"
#include "gd/evt/evt_read.h"
#include "gd/evt/evt_manage.h"
#include "evt_internal_types.h"

gd_evt_t gd_evt_create_ex(gd_evt_mgr_t evm, LPDRMETA data_meta, ssize_t data_capacity, error_monitor_t em) {
    tl_event_t tl_evt;
    gd_evt_t evt;

    assert(evm);
    assert(data_meta);

    if (em == NULL) em = evm->m_em;

    if (data_capacity < 0) {
        data_capacity = dr_meta_size(data_meta);
    }
    else if (data_capacity < (ssize_t)dr_meta_size(data_meta)) {
        CPE_ERROR(
            em, "%s: create event: data_capacity "  FMT_SIZE_T " is to small to contain type %s!",
            gd_evt_mgr_name(evm), data_capacity, dr_meta_name(data_meta));
        return NULL;
    }

    tl_evt = tl_event_create(
        evm->m_tl, sizeof(struct gd_evt) + data_capacity + evm->m_oid_max_len + evm->m_carry_size);
    if (tl_evt == NULL) {
        CPE_ERROR(
            em, "%s: crate event: create tl event fail!",
            gd_evt_mgr_name(evm));
        return NULL;
    }

    evt = gd_evt_cvt(tl_evt);

    evt->m_oid_max_len = evm->m_oid_max_len;
    evt->m_carry_meta = evm->m_carry_meta;
    evt->m_carry_capacity = evm->m_carry_size;
    evt->m_meta = data_meta;
    evt->m_data_capacity = data_capacity;

    bzero(evt + 1, evt->m_oid_max_len);
    dr_meta_set_defaults(gd_evt_data(evt), data_capacity, data_meta, 0);

    return evt;
}

gd_evt_t gd_evt_create(gd_evt_mgr_t evm, const char * typeName, ssize_t data_capacity, error_monitor_t em) {
    struct gd_evt_def key;
    struct gd_evt_def * evt_def;

    assert(evm);

    if (em == NULL) em = evm->m_em;

    key.m_evt_name = typeName;

    evt_def = cpe_hash_table_find(&evm->m_evt_defs, &key);
    if (evt_def == NULL) {
        CPE_ERROR(
            em, "%s: create event: event %s not exist!", 
            gd_evt_mgr_name(evm), typeName);
        return NULL;
    }

    return gd_evt_create_ex(evm, evt_def->m_evt_meta, data_capacity, em);
}

gd_evt_t gd_evt_dyn_create_ex(gd_evt_mgr_t evm, LPDRMETA data_meta, size_t record_capacity, error_monitor_t em) {
    struct dr_meta_dyn_info dyn_info;
    LPDRMETA record_meta;

    if (dr_meta_find_dyn_info(data_meta, &dyn_info) != 0) {
        CPE_ERROR(
            em, "%s: create dyn event: event %s is not dynamic!", 
            gd_evt_mgr_name(evm), dr_meta_name(data_meta));
        return NULL;
    }

    record_meta = dr_entry_self_meta(dyn_info.m_data.m_array.m_array_entry);
    assert(record_meta);

    return gd_evt_create_ex(evm, data_meta, dr_meta_size(data_meta) + record_capacity * dr_meta_size(record_meta), em);
}

gd_evt_t gd_evt_dyn_create(gd_evt_mgr_t evm, const char * typeName, size_t record_capacity, error_monitor_t em) {
    struct gd_evt_def key;
    struct gd_evt_def * evt_def;

    assert(evm);

    if (em == NULL) em = evm->m_em;

    key.m_evt_name = typeName;

    evt_def = cpe_hash_table_find(&evm->m_evt_defs, &key);
    if (evt_def == NULL) {
        CPE_ERROR(
            em, "%s: create event: event %s not exist!", 
            gd_evt_mgr_name(evm), typeName);
        return NULL;
    }

    return gd_evt_dyn_create_ex(evm, evt_def->m_evt_meta, record_capacity, em);
}

int gd_evt_send(
    gd_evt_t evt,
    tl_time_span_t delay,
    tl_time_span_t span,
    int repeatCount)
{
    tl_event_t tl_evt;

    assert(evt);

    tl_evt = tl_event_from_data(evt);

    return tl_event_send_ex(tl_evt, delay, span, repeatCount);
}

const char * gd_evt_target(gd_evt_t evt) {
    return cpe_hs_data((cpe_hash_string_t)(evt + 1));
}

cpe_hash_string_t gd_evt_target_hs(gd_evt_t evt) {
    return (cpe_hash_string_t)(evt + 1);
}

int gd_evt_set_target(gd_evt_t evt, const char * target) {
    size_t name_len = cpe_hs_len_to_binary_len(strlen(target));
    if (name_len > evt->m_oid_max_len) return -1;

    cpe_hs_init(
        (cpe_hash_string_t)(evt + 1),
        evt->m_oid_max_len,
        target);

    return 0;
}

void * gd_evt_data(gd_evt_t evt) {
    return ((char *)(evt + 1)) + evt->m_oid_max_len + evt->m_carry_capacity;
}

size_t gd_evt_data_capacity(gd_evt_t evt) {
    return evt->m_data_capacity;
}

void * gd_evt_carry_data(gd_evt_t evt) {
    return ((char*)(evt + 1)) + evt->m_oid_max_len;
}

LPDRMETA gd_evt_carry_meta(gd_evt_t evt) {
    return evt->m_carry_meta;
}

size_t gd_evt_carry_data_capacity(gd_evt_t evt) {
    return evt->m_carry_capacity;
}

gd_evt_t gd_evt_cvt(tl_event_t tl_evt) {
    return (gd_evt_t)tl_event_data(tl_evt);
}

LPDRMETA gd_evt_meta(gd_evt_t evt) {
    return evt->m_meta;
}

const char * gd_evt_type(gd_evt_t evt) {
    return dr_meta_name(evt->m_meta);
}

void gd_evt_dump(write_stream_t stream, gd_evt_t evt) {
    dr_json_print(stream, gd_evt_data(evt), gd_evt_data_capacity(evt), gd_evt_meta(evt), DR_JSON_PRINT_MINIMIZE, NULL);
}

int gd_evt_set_from_string(gd_evt_t evt, const char * arg, const char * data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_set_from_string(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_int8(gd_evt_t evt, const char * arg, int8_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_set_from_int8(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_uint8(gd_evt_t evt, const char * arg, uint8_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        return -1;
    }

    return dr_entry_set_from_uint8(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_int16(gd_evt_t evt, const char * arg, int16_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_set_from_int16(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_uint16(gd_evt_t evt, const char * arg, uint16_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        return -1;
    }

    return dr_entry_set_from_uint16(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_int32(gd_evt_t evt, const char * arg, int32_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_set_from_int32(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_uint32(gd_evt_t evt, const char * arg, uint32_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        return -1;
    }

    return dr_entry_set_from_uint32(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_int64(gd_evt_t evt, const char * arg, int64_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_set_from_int64(gd_evt_data(evt), data, entry, em);
}

int gd_evt_set_from_uint64(gd_evt_t evt, const char * arg, uint64_t data, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        return -1;
    }

    return dr_entry_set_from_uint64(gd_evt_data(evt), data, entry, em);
}


int gd_evt_try_get_int8(int8_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_int8(result, gd_evt_data(evt), entry, em);
}

int gd_evt_try_get_uint8(uint8_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_uint8(result, gd_evt_data(evt), entry, em);
}

int gd_evt_try_get_int16(int16_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_int16(result, gd_evt_data(evt), entry, em);
}

int gd_evt_try_get_uint16(uint16_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_uint16(result, gd_evt_data(evt), entry, em);
}

int gd_evt_try_get_int32(int32_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_int32(result, gd_evt_data(evt), entry, em);
}

int gd_evt_try_get_uint32(uint32_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_uint32(result, gd_evt_data(evt), entry, em);
}

int gd_evt_try_get_int64(int64_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_int64(result, gd_evt_data(evt), entry, em);
}

int gd_evt_try_get_uint64(uint64_t * result, gd_evt_t evt, const char * arg, error_monitor_t em) {
    LPDRMETAENTRY entry = dr_meta_find_entry_by_name(evt->m_meta, arg);
    if (entry == NULL) {
        CPE_ERROR(em, "event %s have no arg %s!", gd_evt_type(evt), arg);
        return -1;
    }

    return dr_entry_try_read_uint64(result, gd_evt_data(evt), entry, em);
}
