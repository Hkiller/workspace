#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_data_entry.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

dr_data_entry_t dr_data_entry_find(dr_data_entry_t buff, dr_data_t data, const char * attr_name) {
    int from_entry_start;

    buff->m_entry = dr_meta_find_entry_by_path_ex(data->m_meta, attr_name, &from_entry_start);
    if (buff->m_entry == NULL) return NULL;

    buff->m_data = ((char *)data->m_data) + from_entry_start;
    buff->m_size = dr_entry_size(buff->m_entry);

    return buff;
}

dr_data_entry_t dr_data_entry_search_in_source(dr_data_entry_t buff, dr_data_source_t data_source, const char * attr_name) {
    while(data_source) {
        dr_data_entry_t r = dr_data_entry_find(buff, &data_source->m_data, attr_name);
        if (r) return r;

        data_source = data_source->m_next;
    }

    return NULL;
}

int dr_data_entry_set_from_entry(dr_data_entry_t to, dr_data_entry_t from, error_monitor_t em) {
    if (dr_entry_type(to->m_entry) == CPE_DR_TYPE_STRUCT && dr_entry_type(from->m_entry) == CPE_DR_TYPE_STRUCT) {
        if (dr_meta_copy_same_entry(
                to->m_data, to->m_size, dr_entry_ref_meta(to->m_entry),
                from->m_data, from->m_size, dr_entry_ref_meta(from->m_entry),
                0, em)
            <= 0)
        {
            CPE_ERROR(
                em, "set entry %s(%s) from %s(%s): copy same entry error",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry),
                dr_entry_name(from->m_entry), dr_entry_type_name(from->m_entry));
            return -1;
        }
        else {
            return 0;
        }
    }
    else if (dr_entry_type(to->m_entry) > CPE_DR_TYPE_COMPOSITE && dr_entry_type(from->m_entry) > CPE_DR_TYPE_COMPOSITE) {
        if (dr_entry_set_from_ctype(to->m_data, from->m_data, dr_entry_type(from->m_entry), to->m_entry, em) != 0) {
            CPE_ERROR(
                em, "set entry %s(%s) from %s(%s): set with ctype error",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry),
                dr_entry_name(from->m_entry), dr_entry_type_name(from->m_entry));
            return -1;
        }
        else {
            return 0;
        }
    }
    else {
        CPE_ERROR(
            em, "set entry %s(%s) from %s(%s): type convert error",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry),
            dr_entry_name(from->m_entry), dr_entry_type_name(from->m_entry));
        return -1;
    }
}

int dr_data_entry_set_from_string(dr_data_entry_t to, const char * value, error_monitor_t em) {
    if (dr_entry_set_from_string(to->m_data, value, to->m_entry, em) != 0) {
        CPE_ERROR(
            em, "set entry %s(%s) from string: set fail, str=%s!",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry),
            value);
        return -1;
    }

    return 0;
}

int dr_data_entry_set_from_value(dr_data_entry_t to, dr_value_t from, error_monitor_t em) {
    if (dr_entry_type(to->m_entry) == CPE_DR_TYPE_STRUCT && from->m_type == CPE_DR_TYPE_STRUCT) {
        if (dr_meta_copy_same_entry(
                to->m_data, to->m_size, dr_entry_ref_meta(to->m_entry),
                from->m_data, from->m_size, from->m_meta,
                0, em)
            <= 0)
        {
            CPE_ERROR(
                em, "set entry %s(%s) from %s: copy same entry error",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), dr_meta_name(from->m_meta));
            return -1;
        }
        else {
            return 0;
        }
    }
    else if (dr_entry_type(to->m_entry) > CPE_DR_TYPE_COMPOSITE && from->m_type > CPE_DR_TYPE_COMPOSITE) {
        if (dr_entry_set_from_ctype(to->m_data, from->m_data, from->m_type, to->m_entry, em) != 0) {
            CPE_ERROR(
                em, "set entry %s(%s) from %s: set with ctype error",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), dr_type_name(from->m_type));
            return -1;
        }
        else {
            return 0;
        }
    }
    else {
        CPE_ERROR(
            em, "set entry %s(%s) from %s: type convert error",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), dr_type_name(from->m_type));

        return -1;
    }
}
