#include <assert.h>
#include <string.h>
#include "cpe/cfg/cfg_manage.h"
#include "cfg_internal_ops.h"

int cfg_seq_count(cfg_t cfg) {
    return cfg ? ((struct cfg_seq *)cfg)->m_count : 0;
}

cfg_t cfg_seq_at(cfg_t cfg, int pos) {
    struct cfg_seq_block * block;
    struct cfg_seq * s;

    assert(cfg);
    if (cfg->m_type != CPE_CFG_TYPE_SEQUENCE) return NULL;

    s = (struct cfg_seq *)cfg;

    if (pos == -1) pos = ((int)s->m_count) - 1;
    if (pos < 0 || pos >= s->m_count) return NULL;

    block = &s->m_block_head;
    while(pos >= CPE_CFG_SEQ_BLOCK_ITEM_COUNT) {
        pos -= CPE_CFG_SEQ_BLOCK_ITEM_COUNT;
        block = block->m_next;
        if (block == NULL) return NULL;
    }

    assert(block);
    assert(pos < CPE_CFG_SEQ_BLOCK_ITEM_COUNT);

    return block->m_items[pos];
}

cfg_t cfg_seq_item_create(struct cfg_seq * s, int type, size_t capacity) {
    cfg_t rv;
    int insertIdx;
    struct cfg_seq_block * head;
    struct cfg_seq_block ** insertBlocl;

    if (s->m_type != CPE_CFG_TYPE_SEQUENCE) return NULL;

    assert(s);
    assert(s->m_manage);

    insertIdx = s->m_count;
    head = &s->m_block_head;
    insertBlocl = &head;

    while(insertIdx >= CPE_CFG_SEQ_BLOCK_ITEM_COUNT) {
        insertIdx -= CPE_CFG_SEQ_BLOCK_ITEM_COUNT;

        assert(*insertBlocl);
        insertBlocl = &(*insertBlocl)->m_next;
        if (*insertBlocl == NULL) {
            *insertBlocl = (struct cfg_seq_block *)
                mem_alloc(s->m_manage->m_alloc, sizeof(struct cfg_seq_block));
            if (*insertBlocl == NULL) return NULL;
            (*insertBlocl)->m_next = NULL;
        }
    }

    assert(*insertBlocl);
    assert(insertIdx < CPE_CFG_SEQ_BLOCK_ITEM_COUNT);

    rv = (cfg_t)mem_alloc(s->m_manage->m_alloc, sizeof(struct cfg) + capacity);
    if (rv == NULL) return NULL;

    rv->m_manage = s->m_manage;
    rv->m_type = type;
    rv->m_parent = (cfg_t)s;

    (*insertBlocl)->m_items[insertIdx] = rv;

    ++s->m_count;

    if (type == CPE_CFG_TYPE_STRUCT) {
        cfg_struct_init((struct cfg_struct *)rv);
    }
    else if (type == CPE_CFG_TYPE_SEQUENCE) {
        cfg_seq_init((struct cfg_seq *)rv);
    }

    return rv;
}

int cfg_seq_find_in_block(struct cfg_seq_block * b, int count, cfg_t cfg) {
    int i;

    for(i = 0; i < count; ++i) {
        if (b->m_items[i] == cfg) {
            return i;
        }
    }

    return -1;
}

void cfg_seq_item_delete(struct cfg_seq * s, cfg_t cfg) {
    struct cfg_seq_block * b;
    int leftCount;
    int posInB;

    /*find position*/
    b = &s->m_block_head;
    leftCount = s->m_count;
    posInB = -1;

    while(leftCount && b) {
        int countInB = 
            leftCount > CPE_CFG_SEQ_BLOCK_ITEM_COUNT
            ? CPE_CFG_SEQ_BLOCK_ITEM_COUNT
            : leftCount;

        posInB = cfg_seq_find_in_block(b, countInB, cfg);
        if (posInB < 0) {
            leftCount -= countInB;
            b = b->m_next;
        }
        else {
            leftCount -= (posInB + 1);
            break;
        }
    }

    if (posInB < 0) return; //not found

    /*free cfg*/
    cfg_fini(cfg);
    mem_free(s->m_manage->m_alloc, cfg);
    cfg = NULL;

    /*move next here*/
    while(leftCount && b) {
        int leftCountInB = 
            (leftCount + posInB + 1) > CPE_CFG_SEQ_BLOCK_ITEM_COUNT
            ? CPE_CFG_SEQ_BLOCK_ITEM_COUNT - (posInB + 1)
            : leftCount;

        if (leftCountInB > 0) {
            memmove(
                &b->m_items[posInB],
                &b->m_items[posInB + 1],
                sizeof(cfg_t) * leftCountInB);

            leftCount -= leftCountInB;
        }

        if (leftCount && b->m_next) {
            b->m_items[CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 1] = b->m_next->m_items[0];
            leftCount -= 1;
            posInB = 0;
        }
        else {
            posInB = -1;
        }

        b = b->m_next;
    }

    --s->m_count;
}

void cfg_seq_init(struct cfg_seq * s) {
    s->m_count = 0;
    s->m_block_head.m_next = NULL;
}

static void cfg_seq_fini_block(mem_allocrator_t alloc, struct cfg_seq_block * b, int count) {
    int i;

    for(i = 0; i < count; ++i) {
        cfg_fini(b->m_items[i]);
        mem_free(alloc, b->m_items[i]);
    }
}

void cfg_seq_fini(struct cfg_seq * s) {
    struct cfg_seq_block * b;
    int blockFreeSize;
    
    b = &s->m_block_head;
    while(b) {
        struct cfg_seq_block * n = b->m_next;

        blockFreeSize = 
            s->m_count > CPE_CFG_SEQ_BLOCK_ITEM_COUNT
            ? CPE_CFG_SEQ_BLOCK_ITEM_COUNT
            : s->m_count;

        cfg_seq_fini_block(s->m_manage->m_alloc, b, blockFreeSize);

        s->m_count -= blockFreeSize;

        if (b != &s->m_block_head) {
            mem_free(s->m_manage->m_alloc, b);
        }

        b = n;
    }

    s->m_count = 0;
    s->m_block_head.m_next = NULL;
}
