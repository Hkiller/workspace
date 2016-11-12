#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "rank_f_svr_ops.h"

rank_f_svr_index_info_t
rank_f_svr_index_info_create(rank_f_svr_t svr, uint16_t id) {
    if (id >= (sizeof(svr->m_index_infos) / sizeof(svr->m_index_infos[0]))) {
        CPE_ERROR(svr->m_em, "%s: create index %d: id overflow!", rank_f_svr_name(svr), id);
        return NULL;
    }

    if (svr->m_index_infos[id].m_svr != NULL) {
        CPE_ERROR(svr->m_em, "%s: create index %d: id duplicate!", rank_f_svr_name(svr), id);
        return NULL;
    }

    svr->m_index_infos[id].m_svr = svr;
    svr->m_index_infos[id].m_id = id;
    svr->m_index_infos[id].m_sorter_count = 0;
    return &svr->m_index_infos[id];
}

static int cmp_int8_asc(void const * l, void const * r) {
    return *(int8_t const *)l < *(int8_t const *)r
        ? -1
        :  (*(int8_t const *)l == *(int8_t const *)r
            ? 0
            : 1);
}

static int cmp_int8_dsc(void const * l, void const * r) {
    return *(int8_t const *)l < *(int8_t const *)r
        ? 1
        :  (*(int8_t const *)l == *(int8_t const *)r
            ? 0
            : -1);
}

static int cmp_uint8_asc(void const * l, void const * r) {
    return *(uint8_t const *)l < *(uint8_t const *)r
        ? -1
        :  (*(uint8_t const *)l == *(uint8_t const *)r
            ? 0
            : 1);
}

static int cmp_uint8_dsc(void const * l, void const * r) {
    return *(uint8_t const *)l < *(uint8_t const *)r
        ? 1
        :  (*(uint8_t const *)l == *(uint8_t const *)r
            ? 0
            : -1);
}

static int cmp_int16_asc(void const * l, void const * r) {
    return *(int16_t const *)l < *(int16_t const *)r
        ? -1
        :  (*(int16_t const *)l == *(int16_t const *)r
            ? 0
            : 1);
}

static int cmp_int16_dsc(void const * l, void const * r) {
    return *(int16_t const *)l < *(int16_t const *)r
        ? 1
        :  (*(int16_t const *)l == *(int16_t const *)r
            ? 0
            : -1);
}

static int cmp_uint16_asc(void const * l, void const * r) {
    return *(uint16_t const *)l < *(uint16_t const *)r
        ? -1
        :  (*(uint16_t const *)l == *(uint16_t const *)r
            ? 0
            : 1);
}

static int cmp_uint16_dsc(void const * l, void const * r) {
    return *(uint16_t const *)l < *(uint16_t const *)r
        ? 1
        :  (*(uint16_t const *)l == *(uint16_t const *)r
            ? 0
            : -1);
}

static int cmp_int32_asc(void const * l, void const * r) {
    return *(int32_t const *)l < *(int32_t const *)r
        ? -1
        :  (*(int32_t const *)l == *(int32_t const *)r
            ? 0
            : 1);
}

static int cmp_int32_dsc(void const * l, void const * r) {
    return *(int32_t const *)l < *(int32_t const *)r
        ? 1
        :  (*(int32_t const *)l == *(int32_t const *)r
            ? 0
            : -1);
}

static int cmp_uint32_asc(void const * l, void const * r) {
    return *(uint32_t const *)l < *(uint32_t const *)r
        ? -1
        :  (*(uint32_t const *)l == *(uint32_t const *)r
            ? 0
            : 1);
}

static int cmp_uint32_dsc(void const * l, void const * r) {
    return *(uint32_t const *)l < *(uint32_t const *)r
        ? 1
        :  (*(uint32_t const *)l == *(uint32_t const *)r
            ? 0
            : -1);
}

static int cmp_int64_asc(void const * l, void const * r) {
    return *(int64_t const *)l < *(int64_t const *)r
        ? -1
        :  (*(int64_t const *)l == *(int64_t const *)r
            ? 0
            : 1);
}

static int cmp_int64_dsc(void const * l, void const * r) {
    return *(int64_t const *)l < *(int64_t const *)r
        ? 1
        :  (*(int64_t const *)l == *(int64_t const *)r
            ? 0
            : -1);
}

static int cmp_uint64_asc(void const * l, void const * r) {
    return *(uint64_t const *)l < *(uint64_t const *)r
        ? -1
        :  (*(uint64_t const *)l == *(uint64_t const *)r
            ? 0
            : 1);
}

static int cmp_uint64_dsc(void const * l, void const * r) {
    return *(uint64_t const *)l < *(uint64_t const *)r
        ? 1
        :  (*(uint64_t const *)l == *(uint64_t const *)r
            ? 0
            : -1);
}

static int cmp_float_asc(void const * l, void const * r) {
    return *(float const *)l < *(float const *)r
        ? -1
        :  (*(float const *)l == *(float const *)r
            ? 0
            : 1);
}

static int cmp_float_dsc(void const * l, void const * r) {
    return *(float const *)l < *(float const *)r
        ? 1
        :  (*(float const *)l == *(float const *)r
            ? 0
            : -1);
}

static int cmp_double_asc(void const * l, void const * r) {
    return *(double const *)l < *(double const *)r
        ? -1
        :  (*(double const *)l == *(double const *)r
            ? 0
            : 1);
}

static int cmp_double_dsc(void const * l, void const * r) {
    return *(double const *)l < *(double const *)r
        ? 1
        :  (*(double const *)l == *(double const *)r
            ? 0
            : -1);
}

static int cmp_string_asc(void const * l, void const * r) {
    return strcmp(l, r);
}

static int cmp_string_dsc(void const * l, void const * r) {
    return strcmp(r, l);
}

int rank_f_svr_index_info_add_sorter(rank_f_svr_index_info_t index_info, const char * entry_path, const char * str_order) {
    rank_f_svr_t svr = index_info->m_svr;
    struct rank_f_svr_index_sorter * sorter;
    int order = 0;
    int start_pos;

    if (strcmp(str_order, "asc") == 0) {
        order = 1;
    }
    else if (strcmp(str_order, "dsc") == 0) {
        order = 0;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: create index %d(%s, %s): order is unknown!",
            rank_f_svr_name(svr), index_info->m_id, entry_path, str_order);
        return -1;
    }

    if (index_info->m_sorter_count + 1 >= ((sizeof(index_info->m_sorters) / sizeof(index_info->m_sorters[0])))) {
        CPE_ERROR(
            svr->m_em, "%s: create index %d(%s, %s): sorter count %d full!",
            rank_f_svr_name(svr), index_info->m_id, entry_path, str_order, index_info->m_sorter_count);
        return -1;
    }

    sorter = &index_info->m_sorters[index_info->m_sorter_count];

    sorter->m_sort_entry = dr_meta_find_entry_by_path_ex(svr->m_data_meta, entry_path, &start_pos);
    if (sorter->m_sort_entry == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: create index %d(%s, %s): entry not exist!",
            rank_f_svr_name(svr), index_info->m_id, entry_path, str_order);
        return -1;
    }
    sorter->m_data_start_pos = start_pos;

    switch(dr_entry_type(sorter->m_sort_entry)) {
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_INT8:
        sorter->m_sort_fun = order ? cmp_int8_asc : cmp_int8_dsc;
        break;
    case CPE_DR_TYPE_UCHAR:
    case CPE_DR_TYPE_UINT8:
        sorter->m_sort_fun = order ? cmp_uint8_asc : cmp_uint8_dsc;
        break;
    case CPE_DR_TYPE_INT16:
        sorter->m_sort_fun = order ? cmp_int16_asc : cmp_int16_dsc;
        break;
    case CPE_DR_TYPE_UINT16:
        sorter->m_sort_fun = order ? cmp_uint16_asc : cmp_uint16_dsc;
        break;
    case CPE_DR_TYPE_INT32:
        sorter->m_sort_fun = order ? cmp_int32_asc : cmp_int32_dsc;
        break;
    case CPE_DR_TYPE_UINT32:
        sorter->m_sort_fun = order ? cmp_uint32_asc : cmp_uint32_dsc;
        break;
    case CPE_DR_TYPE_INT64:
        sorter->m_sort_fun = order ? cmp_int64_asc : cmp_int64_dsc;
        break;
    case CPE_DR_TYPE_UINT64:
        sorter->m_sort_fun = order ? cmp_uint64_asc : cmp_uint64_dsc;
        break;
    case CPE_DR_TYPE_FLOAT:
        sorter->m_sort_fun = order ? cmp_float_asc : cmp_float_dsc;
        break;
    case CPE_DR_TYPE_DOUBLE:
        sorter->m_sort_fun = order ? cmp_double_asc : cmp_double_dsc;
        break;
    case CPE_DR_TYPE_STRING:
        sorter->m_sort_fun = order ? cmp_string_asc : cmp_string_dsc;
        break;
    default:
        CPE_ERROR(
            svr->m_em, "%s: create index %d(%s, %s): entry type %s not support cmp!",
            rank_f_svr_name(svr), index_info->m_id, entry_path, str_order, dr_type_name(dr_entry_type(sorter->m_sort_entry)));
        return -1;
    }

    ++index_info->m_sorter_count;
    return 0;
}
