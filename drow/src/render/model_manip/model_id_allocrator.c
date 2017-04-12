#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "model_id_allocrator_i.h"

ui_model_id_allocrator_t
ui_model_id_allocrator_create(mem_allocrator_t alloc) {
    ui_model_id_allocrator_t id_alloc;

    id_alloc = mem_calloc(alloc, sizeof(struct ui_model_id_allocrator));
    if (id_alloc == NULL) return NULL;

    id_alloc->m_alloc = alloc;
    return id_alloc;
}

void ui_model_id_allocrator_free(ui_model_id_allocrator_t id_alloc) {
    if (id_alloc->m_allocked_ids) {
        mem_free(id_alloc->m_alloc, id_alloc->m_allocked_ids);
    }

    mem_free(id_alloc->m_alloc, id_alloc);    
}

static int id_cmp(void const *l, void const * r) {
    return (int)(*(uint32_t*)l) - (int)(*(uint32_t*)r);
}

int ui_model_id_allocrator_remove(ui_model_id_allocrator_t  id_alloc, uint32_t id) {
    if (id_alloc->m_allocked_count) {
        if (bsearch(&id, id_alloc->m_allocked_ids, id_alloc->m_allocked_count, sizeof(id), id_cmp)) return -1;
    }

    if (id_alloc->m_allocked_count + 1 > id_alloc->m_allocked_capacity) {
        uint32_t new_capacity = id_alloc->m_allocked_capacity < 64 ? 64 : (id_alloc->m_allocked_capacity * 2);
        void * new_buf = mem_alloc(id_alloc->m_alloc, sizeof(uint32_t) * new_capacity);
        if (new_buf == NULL) return -1;

        if (id_alloc->m_allocked_count) {
            memcpy(new_buf, id_alloc->m_allocked_ids, sizeof(uint32_t) * id_alloc->m_allocked_count);
        }

        if (id_alloc->m_allocked_ids) {
            mem_free(id_alloc->m_alloc, id_alloc->m_allocked_ids);
        }

        id_alloc->m_allocked_ids = new_buf;
        id_alloc->m_allocked_capacity = new_capacity;
    }

    assert(id_alloc->m_allocked_count + 1 <= id_alloc->m_allocked_capacity);

    id_alloc->m_allocked_ids[id_alloc->m_allocked_count++] = id;

    qsort(id_alloc->m_allocked_ids, id_alloc->m_allocked_count, sizeof(id_alloc->m_allocked_ids[0]), id_cmp);

    return 0;
}

int ui_model_id_allocrator_alloc(ui_model_id_allocrator_t  id_alloc, uint32_t * id) {
    uint32_t i;
    for(i = 0; i < 2048; ++i) {
        if (bsearch(&id_alloc->m_next_id, id_alloc->m_allocked_ids, id_alloc->m_allocked_count, sizeof(*id), id_cmp)) {
            id_alloc->m_next_id++;
            continue;
        }
        else {
            *id = id_alloc->m_next_id;
            id_alloc->m_next_id++;
            return 0;
        }
    }

    return -1;
}

