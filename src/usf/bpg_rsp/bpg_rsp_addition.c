#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "usf/bpg_rsp/bpg_rsp_addition.h"
#include "protocol/bpg_rsp/bpg_rsp_addition.h"

int16_t bpg_rsp_addition_data_count(logic_context_t ctx) {
    logic_data_t data;
    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    return data == NULL
        ? 0
        : ((BPG_RSP_ADDITION_DATA *)logic_data_data(data))->count;
}

uint32_t bpg_rsp_addition_data_at(logic_context_t ctx, int16_t pos) {
    logic_data_t data;
    BPG_RSP_ADDITION_DATA * addition_data;

    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data == NULL || addition_data->count <= pos) return 0;

    return addition_data->pieces[pos];
}

#define BPG_RSP_ADDITION_DATA_ONCE_SIZE (16)
extern char g_metalib_carry_package[];

static BPG_RSP_ADDITION_DATA *
bpg_rsp_addition_data_create(logic_context_t ctx, size_t capacity) {
    LPDRMETA meta;
    BPG_RSP_ADDITION_DATA * addition_data;
    logic_data_t data;

    meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_carry_package, "bpg_rsp_addition_data");
    if (meta == NULL) return NULL;

    data = logic_context_data_get_or_create(ctx, meta, sizeof(BPG_RSP_ADDITION_DATA) + (capacity - 1) * sizeof(uint32_t));

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data) addition_data->capacity = capacity;

    return addition_data;
}

int bpg_rsp_meta_id_cmp(const void *m1, const void *m2) {
	// sort meta_id in ascending sort.
    //return *((const int32_t *)m2) - *((const int32_t *)m1);
	return *((const int32_t *)m1) - *((const int32_t *)m2);
}

int bpg_rsp_addition_data_remove(logic_context_t ctx, uint32_t meta_id) {
    logic_data_t data;
    BPG_RSP_ADDITION_DATA * addition_data;
    int32_t * element;
    int element_pos;

    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data == NULL) {
        return -1;
    }

    element =
        (int32_t *)bsearch(
            &meta_id, addition_data->pieces,
            addition_data->count, sizeof(meta_id),
            bpg_rsp_meta_id_cmp);
    if (element == NULL) {
        return -1;
    }

    element_pos = element - addition_data->pieces;
    if (element_pos + 1 < addition_data->count) {
        memmove(element, element + 1, sizeof(*element) * (addition_data->count  - 1 - element_pos));
    }

    --addition_data->count;

    return 0;
}

int bpg_rsp_addition_data_add(logic_context_t ctx, uint32_t meta_id) {
    logic_data_t data;
    BPG_RSP_ADDITION_DATA * addition_data;

    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data == NULL) {
        addition_data = 
            bpg_rsp_addition_data_create(ctx, BPG_RSP_ADDITION_DATA_ONCE_SIZE);
        if (addition_data == NULL) return -1;
        assert(addition_data->count == 0);
    }

    if (bsearch(
            &meta_id, addition_data->pieces,
            addition_data->count, sizeof(meta_id),
            bpg_rsp_meta_id_cmp))
    {
        return 0;
    }

    if (addition_data->count >= addition_data->capacity) {
        addition_data = 
            bpg_rsp_addition_data_create(ctx, addition_data->capacity + BPG_RSP_ADDITION_DATA_ONCE_SIZE);
        if (addition_data == NULL) return -1;
    }

    assert(addition_data->count < addition_data->capacity);

    addition_data->pieces[addition_data->count] = meta_id;
    addition_data->count += 1;

    qsort(addition_data->pieces, addition_data->count, sizeof(meta_id), bpg_rsp_meta_id_cmp);

    return 0;
}

int bpg_rsp_addition_data_add_all(logic_context_t ctx) {
    struct logic_data_it data_it;
    logic_data_t data;

    int rv = 0;

    logic_context_datas(ctx, &data_it);
    while((data = logic_data_next(&data_it))) {
        LPDRMETA meta = logic_data_meta(data);
        if (dr_meta_id(meta) != -1) {
            if (bpg_rsp_addition_data_add(ctx, dr_meta_id(meta)) != 0) rv = -1;
        }
    }

    return rv;
}

static int bpg_rsp_addition_data_cmp(const void * l, const void * r) {
    const uint32_t * l_v = (const uint32_t *)l;
    const uint32_t * r_v = (const uint32_t *)r;

    return *l_v == *r_v ? 0
        : *l_v < *r_v ? -1
        : 1;
}

int bpg_rsp_addition_data_exist(logic_context_t ctx, uint32_t meta_id) {
    logic_data_t data;
    BPG_RSP_ADDITION_DATA * addition_data;

    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data == 0) return 0;

    return bsearch(
        &meta_id,
        addition_data->pieces,
        addition_data->count,
        sizeof(addition_data->pieces[0]),
        bpg_rsp_addition_data_cmp)
        ? 1
        : 0;
}
