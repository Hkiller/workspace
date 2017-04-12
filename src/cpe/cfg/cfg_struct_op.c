#include <assert.h>
#include <string.h>
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_types.h"
#include "cfg_internal_ops.h"

int cfg_struct_item_cmp(struct cfg_struct_item * l, struct cfg_struct_item * r) {
    return strcmp(l->m_name, r->m_name);
}

RB_GENERATE(cfg_struct_item_tree, cfg_struct_item, m_linkage, cfg_struct_item_cmp);

void cfg_struct_init(struct cfg_struct * s) {
    s->m_count = 0;
    RB_INIT(&s->m_items);
}

void cfg_struct_fini(struct cfg_struct * cfg) {
    while(!RB_EMPTY(&cfg->m_items)) {
        cfg_struct_item_delete(cfg, &(RB_ROOT(&cfg->m_items)->m_data));
    }
}

struct cfg_struct_item * cfg_struct_find_item(struct cfg_struct * s, const char * name) {
    struct cfg_struct_item tmp;

    if (!s || s->m_type != CPE_CFG_TYPE_STRUCT) return NULL;

    tmp.m_name = name;
    return RB_FIND(cfg_struct_item_tree, &s->m_items, &tmp);
}

int cfg_struct_count(cfg_t cfg) {
    assert(cfg);

    return ((struct cfg_struct *)cfg)->m_count;
}

cfg_t cfg_struct_find_cfg(cfg_t cfg, const char * name) {
    struct cfg_struct_item * found = cfg_struct_find_item((struct cfg_struct *)cfg, name);
    return found ? &found->m_data : NULL;
}

void cfg_struct_item_delete(struct cfg_struct * s, cfg_t cfg) {
    struct cfg_struct_item * item;

    assert(s || s->m_manage);

    if (s == NULL || cfg == NULL) return;

    item = cfg_to_struct_item(cfg);

    RB_REMOVE(cfg_struct_item_tree, &s->m_items, item);

    cfg_fini(&item->m_data);

    --s->m_count;
    /*name is the alloc start adress, see cfg_struct_add_item*/
    mem_free(item->m_data.m_manage->m_alloc, (void*)item->m_name);
}

/*0: return old
  1: delete old, create new
  2: old-add-to-new
  3: new-as-sub-of-old
*/
static int cfg_struct_decide_do_old(int oldType, int newType, cfg_policy_t policy) {
    if (policy == cfg_replace) return 1;

    if (cfg_type_is_value(oldType) || cfg_type_is_value(newType))
        return policy == cfg_merge_use_exist ? 4 : 1;

    if (oldType == newType) return 0;

    if (oldType == CPE_CFG_TYPE_STRUCT) {
        assert(newType == CPE_CFG_TYPE_SEQUENCE);
        return 3;
    }
    else {
        assert(oldType == CPE_CFG_TYPE_SEQUENCE);
        assert(newType == CPE_CFG_TYPE_STRUCT);
        return 2;
    }
}

cfg_t cfg_struct_item_do_create(struct cfg_struct * s, const char * name, int type, size_t capacity) {
    int nameLen;
    int allocSize;
    int paddingLen;
    void * data;
    struct cfg_struct_item * item;

    nameLen = (int)strlen(name) + 1;
    paddingLen = nameLen % sizeof(struct cfg_struct_item);
    if (paddingLen)
    {
        paddingLen = sizeof(struct cfg_struct_item) - paddingLen;
    }
    allocSize = (int)(nameLen + paddingLen + sizeof(struct cfg_struct_item) + capacity);

    data = mem_alloc(s->m_manage->m_alloc, allocSize);
    if (data == NULL) return NULL;

    memcpy(data, name, nameLen);

    item = (struct cfg_struct_item *)(((char*)data) + nameLen + paddingLen);

    item->m_data.m_manage = s->m_manage;
    item->m_data.m_parent = (cfg_t)s;
    item->m_data.m_type = type;
    item->m_name = (const char*)data;

    if (RB_INSERT(cfg_struct_item_tree, &s->m_items, item) != NULL) {
        mem_free(item->m_data.m_manage->m_alloc, data);
        return NULL;
    }
    else {
        cfg_t newCfg = &item->m_data;
        if (type == CPE_CFG_TYPE_STRUCT) {
            cfg_struct_init((struct cfg_struct *)newCfg);
        }
        else if (type == CPE_CFG_TYPE_SEQUENCE) {
            cfg_seq_init((struct cfg_seq *)newCfg);
        }

        ++s->m_count;
        return newCfg;
    }
}

cfg_t cfg_struct_item_create(struct cfg_struct * s, const char * name, int type, size_t capacity, cfg_policy_t policy) {
    struct cfg_struct_item * old_item;
    cfg_t newCfg;

    assert(s || s->m_manage);
    assert(name);

    if (!s || s->m_type != CPE_DR_TYPE_STRUCT) return NULL;

    old_item = cfg_struct_find_item(s, name);
    if (old_item) {
        int do_old_op = cfg_struct_decide_do_old(old_item->m_data.m_type, type, policy);
        switch(do_old_op) {
        case 0: /*return old*/
            return &old_item->m_data;
        case 1: /*delete old, create new*/
            cfg_struct_item_delete(s, &old_item->m_data);
            return cfg_struct_item_do_create(s, name, type, capacity);
        case 2: {/*old(seq)-add-to-new(struct)*/
            assert(old_item->m_data.m_type == CPE_CFG_TYPE_SEQUENCE);
            RB_REMOVE(cfg_struct_item_tree, &s->m_items, old_item);
            newCfg = cfg_struct_item_do_create(s, name, type, capacity);
            if (newCfg) {
                struct cfg_seq * subSeq = (struct cfg_seq *)cfg_struct_add_seq(newCfg, "", cfg_replace);
                assert(subSeq);

                cfg_seq_fini(subSeq);
                memcpy(
                    ((char*)subSeq) + sizeof(struct cfg),
                    ((char*)&old_item->m_data) + sizeof(struct cfg),
                    sizeof(struct cfg_seq) - sizeof(struct cfg));

                cfg_seq_init((struct cfg_seq *)&old_item->m_data);
            }

            if (old_item) {
                cfg_fini(&old_item->m_data);
                /*name is the alloc start adress, see cfg_struct_add_item*/
                mem_free(old_item->m_data.m_manage->m_alloc, (void*)old_item->m_name);
            }

            return newCfg;
        }
        case 3: /*new-as-sub-of-old(struct)*/
            return cfg_struct_item_create(
                (struct cfg_struct *)&old_item->m_data, "", type, capacity, policy);
        default: /*4*/
            return NULL;
        }
    }
    else {
        return cfg_struct_item_do_create(s, name, type, capacity);
    }
}
