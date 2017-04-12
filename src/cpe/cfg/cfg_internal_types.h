#ifndef CPE_CFG_IMPL_INTERNAL_TYPES_H
#define CPE_CFG_IMPL_INTERNAL_TYPES_H
#include "cpe/pal/pal_tree.h"
#include "cpe/utils/memory.h"
#include "cpe/cfg/cfg_types.h"

typedef struct cfg_manage * cfg_manage_t;

#define CPE_CFG_HEAD_DATA           \
    cfg_manage_t m_manage;      \
    int m_type;                    \
    cfg_t m_parent;

struct cfg {
    CPE_CFG_HEAD_DATA
};

struct cfg_struct_item {
    RB_ENTRY(cfg_struct_item) m_linkage;
    const char * m_name;
    struct cfg m_data;
};

RB_HEAD(cfg_struct_item_tree, cfg_struct_item);

struct cfg_struct {
    CPE_CFG_HEAD_DATA
    int m_count;
    struct cfg_struct_item_tree m_items;
};

#define CPE_CFG_SEQ_BLOCK_ITEM_COUNT 64

struct cfg_seq_block {
    struct cfg_seq_block * m_next;
    cfg_t m_items[CPE_CFG_SEQ_BLOCK_ITEM_COUNT];
};

struct cfg_seq {
    CPE_CFG_HEAD_DATA
    int m_count;
    struct cfg_seq_block m_block_head; 
};

struct cfg_manage {
    mem_allocrator_t m_alloc;
    struct cfg_struct m_root;
};

struct cfg_format_bin_head {
    uint32_t m_magic;
    uint32_t m_data_start;
    uint32_t m_data_capacity;
    uint32_t m_strpool_start;
    uint32_t m_strpool_capacity;
};

extern const char CFG_FORMAT_BIN_MATIC[4];

#endif
