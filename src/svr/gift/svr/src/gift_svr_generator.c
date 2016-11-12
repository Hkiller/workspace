#include "cpe/utils/random.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gift_svr_generator.h"
#include "cpe/aom/aom_obj_mgr.h"

static uint8_t gift_svr_generator_char_scope_element_count(gift_svr_generator_char_scope_t scope);
static uint8_t gift_svr_generator_char_scope_element_min(gift_svr_generator_char_scope_t scope);
static int gift_svr_generator_char_scope_from_str(gift_svr_t svr, cfg_t cfg, const char * attr, gift_svr_generator_char_scope_t * scope);

gift_svr_generator_t
gift_svr_generator_create(gift_svr_t svr, const char * name, cfg_t cfg) {
    gift_svr_generator_t generator;
    struct cfg_it block_it;
    cfg_t block_cfg;
    float total_count;
    uint8_t i;

    if (gift_svr_generator_find(svr, name) != NULL) {
        CPE_ERROR(svr->m_em, "%s: create generator: generator %s already exist!", gift_svr_name(svr), name);
        return NULL;
    }

    generator = mem_alloc(svr->m_alloc, sizeof(struct gift_svr_generator));
    if (generator == NULL) {
        CPE_ERROR(svr->m_em, "%s: create generator: alloc fail!", gift_svr_name(svr));
        return NULL;
    }

    generator->m_svr = svr;
    cpe_str_dup(generator->m_name, sizeof(generator->m_name), name);

    generator->m_prefix.m_len = cfg_get_uint8(cfg, "prefix-len", 0);
    if (generator->m_prefix.m_len == 0) {
        CPE_ERROR(svr->m_em, "%s: generator %s: prefix-len not configured!", gift_svr_name(svr), name);
        mem_free(generator->m_svr->m_alloc, generator);
        return NULL;
    }

    if (generator->m_prefix.m_len + 1 >= CPE_TYPE_ARRAY_SIZE(SVR_GIFT_GENERATE_RECORD, prefix)) {
        CPE_ERROR(
            svr->m_em, "%s: generator %s: prefix-len %d overflow, max=%d!",
            gift_svr_name(svr), name, generator->m_prefix.m_len,
            (int)(CPE_TYPE_ARRAY_SIZE(SVR_GIFT_GENERATE_RECORD, prefix) - 1));
        mem_free(generator->m_svr->m_alloc, generator);
        return NULL;
    }
    
    if (gift_svr_generator_char_scope_from_str(svr, cfg, "prefix-scope", &generator->m_prefix.m_scope) != 0) {
        mem_free(generator->m_svr->m_alloc, generator);
        return NULL;
    }

    generator->m_block_count = 0;

    cfg_it_init(&block_it, cfg_find_cfg(cfg, "blocks"));
    while((block_cfg = cfg_it_next(&block_it))) {
        struct gift_svr_generator_block * block;
        
        if (generator->m_block_count >= CPE_ARRAY_SIZE(generator->m_blocks)) {
            CPE_ERROR(svr->m_em, "%s: generator %s: block count overflow!", gift_svr_name(svr), name);
            mem_free(generator->m_svr->m_alloc, generator);
            return NULL;
        }

        block = &generator->m_blocks[generator->m_block_count++];
        block->m_len = cfg_get_uint8(block_cfg, "len", 0);
        
        if (block->m_len == 0) {
            CPE_ERROR(svr->m_em, "%s: generator %s: block len not configured!", gift_svr_name(svr), name);
            mem_free(generator->m_svr->m_alloc, generator);
            return NULL;
        }

        if (gift_svr_generator_char_scope_from_str(svr, block_cfg, "scope", &block->m_scope) != 0) {
            mem_free(generator->m_svr->m_alloc, generator);
            return NULL;
        }
    }

    if (generator->m_block_count == 0) {
        CPE_ERROR(svr->m_em, "%s: generator %s: no blocks configured!", gift_svr_name(svr), name);
        mem_free(generator->m_svr->m_alloc, generator);
        return NULL;
    }

    /*计算可能性 */
    
    generator->m_cdkey_len = generator->m_prefix.m_len;
    total_count = 1.0f;
    
    for(i = 0; i < generator->m_block_count; ++i) {
        generator->m_cdkey_len += generator->m_blocks[i].m_len;
        total_count *= pow(gift_svr_generator_char_scope_element_count(generator->m_blocks[i].m_scope), (float)generator->m_blocks[i].m_len);
    }

    total_count /= 5.0f;
    
    generator->m_max_generate_count = total_count > UINT16_MAX ? UINT16_MAX : (uint16_t)total_count;

    if (generator->m_cdkey_len > GIFT_SVR_GENERATOR_MAX_DATA_LEN) {
        CPE_ERROR(
            svr->m_em, "%s: generator %s: total len %d overflow, max-total-len=%d!",
            gift_svr_name(svr), name, generator->m_cdkey_len, GIFT_SVR_GENERATOR_MAX_DATA_LEN);
        mem_free(generator->m_svr->m_alloc, generator);
        return NULL;
    }
    
    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: generator %s: total-len=%d, block=%d, max-generate-count=%d",
            gift_svr_name(svr), name, generator->m_cdkey_len, generator->m_block_count, generator->m_max_generate_count);
    }

    TAILQ_INSERT_TAIL(&svr->m_generators, generator, m_next);
    
    return generator;
}

void gift_svr_generator_free(gift_svr_generator_t generator) {
    TAILQ_REMOVE(&generator->m_svr->m_generators, generator, m_next);
    mem_free(generator->m_svr->m_alloc, generator);
}

gift_svr_generator_t
gift_svr_generator_find(gift_svr_t svr, const char * name) {
    gift_svr_generator_t generator;

    TAILQ_FOREACH(generator, &svr->m_generators, m_next) {
        if (strcmp(generator->m_name, name) == 0) return generator;
    }

    return NULL;
}

struct gift_svr_generator_node {
    uint8_t m_element_count;
    uint8_t m_element_min;
    uint8_t * m_source_records;
    uint8_t m_source_record_count;
};

struct gift_svr_generator_ctx {
    uint8_t m_prefix_len;
    char m_prefix[GIFT_SVR_GENERATOR_MAX_DATA_LEN];
    SVR_GIFT_USE_BASIC * m_records;
    uint16_t * m_record_count;
    struct cpe_rand_ctx * m_rand_ctx;
};

static void gift_svr_generator_node_reset(struct gift_svr_generator_node * node) {
    uint8_t i;
    for(i = 0; i < node->m_element_count; ++i) {
        node->m_source_records[i] = i;
    }
    node->m_source_record_count = node->m_element_count;
}

static uint8_t gift_svr_generator_node_select(struct gift_svr_generator_node * node, struct gift_svr_generator_ctx * ctx) {
    uint8_t c;
    uint8_t idx;
    
    assert(node->m_source_record_count > 0);

    idx = cpe_rand_ctx_generate(ctx->m_rand_ctx, node->m_source_record_count);

    c = node->m_source_records[idx];

    if (idx != node->m_source_record_count - 1) {
        node->m_source_records[idx] = node->m_source_records[node->m_source_record_count - 1];
    }

    node->m_source_record_count--;

    return node->m_element_min + c;
}

static uint16_t gift_svr_generator_gen_node(
    struct gift_svr_generator_ctx * ctx, uint16_t require_count,
    struct gift_svr_generator_node * nodes, uint8_t node_count)
{
    struct gift_svr_generator_node * cur_node;
    uint16_t generated_count = 0;
    uint8_t i;

    assert(node_count > 0);
    assert(require_count > 0);

    cur_node = &nodes[0];
    
    if (cur_node->m_source_record_count == 0) gift_svr_generator_node_reset(cur_node);

    if (node_count > 1) {
        /*不是最后的节点， 需要递归处理下去 */
        uint16_t local_require_count;
        uint16_t next_require_count;

        local_require_count = require_count;
        if (local_require_count > cur_node->m_source_record_count) {
            local_require_count = cur_node->m_source_record_count;
        }

        next_require_count = ceil((float)require_count / (float)local_require_count);
        assert(next_require_count > 0);

        if (local_require_count > cur_node->m_source_record_count) gift_svr_generator_node_reset(cur_node);

        ctx->m_prefix_len++;
        for(i = 0; i < local_require_count; ++i) {
            uint16_t sub_generated_count;
            
            ctx->m_prefix[ctx->m_prefix_len - 1] = gift_svr_generator_node_select(cur_node, ctx);

            sub_generated_count = gift_svr_generator_gen_node(ctx, cpe_min(require_count, next_require_count), nodes + 1, node_count - 1);
            assert(sub_generated_count > 0);
            assert(sub_generated_count <= require_count);

            require_count -= sub_generated_count;
            generated_count += sub_generated_count;
            if (require_count == 0) break;
        }
        ctx->m_prefix_len--;
    }
    else {
        if (require_count > cur_node->m_element_count) {
            require_count = cur_node->m_element_count;
        }

        if (require_count > cur_node->m_source_record_count) gift_svr_generator_node_reset(cur_node);

        for(i = 0; i < require_count; ++i) {
            SVR_GIFT_USE_BASIC * record = &ctx->m_records[(*ctx->m_record_count)++];

            memcpy(record->cdkey, ctx->m_prefix, ctx->m_prefix_len);
            record->cdkey[ctx->m_prefix_len] = gift_svr_generator_node_select(cur_node, ctx);
            record->cdkey[ctx->m_prefix_len + 1] = 0;
            
            generated_count++;
        }
    }

    return generated_count;
}

int gift_svr_generator_gen(gift_svr_generator_t generator, const char * prefix, SVR_GIFT_USE_BASIC * records, uint16_t * record_count) {
    gift_svr_t svr = generator->m_svr;
    struct gift_svr_generator_ctx ctx;
    uint16_t require_count;
    mem_buffer_t tmp_buffer = gd_app_tmp_buffer(svr->m_app);
    uint16_t i, j;
    uint8_t node_count = 0;
    struct gift_svr_generator_node nodes[GIFT_SVR_GENERATOR_MAX_DATA_LEN];

    require_count = *record_count;
    *record_count = 0;
    
    ctx.m_records = records;
    ctx.m_record_count = record_count;
    ctx.m_prefix_len = strlen(prefix);
    cpe_str_dup(ctx.m_prefix, sizeof(ctx.m_prefix), prefix);
    ctx.m_rand_ctx = cpe_rand_ctx_dft();

    /*构造每一位的随机范围 */
    mem_buffer_clear_data(tmp_buffer);
    for(i = 0; i < generator->m_block_count; ++i) {
        gift_svr_generator_block_t block = &generator->m_blocks[i];
        for(j = 0; j < block->m_len; ++j) {
            struct gift_svr_generator_node * node;
            
            if (node_count >= CPE_ARRAY_SIZE(nodes)) {
                CPE_ERROR(svr->m_em, "%s: generate: node count %d overflow!", gift_svr_name(svr), node_count);
                return -1;
            }

            node = &nodes[node_count++];
            node->m_element_count = gift_svr_generator_char_scope_element_count(block->m_scope);
            node->m_element_min = gift_svr_generator_char_scope_element_min(block->m_scope);
            node->m_source_records = mem_buffer_alloc(tmp_buffer, sizeof(uint8_t) * node->m_element_count);
            node->m_source_record_count = 0;

            if (node->m_source_records == NULL) {
                CPE_ERROR(svr->m_em, "%s: generate: alloc source records buf fail!", gift_svr_name(svr));
                return -1;
            }
        }
    }

    gift_svr_generator_gen_node(&ctx, require_count, nodes, node_count);

    return 0;
}

int gift_svr_generator_select_prefix(gift_svr_generator_t generator, char * output) {
    gift_svr_t svr = generator->m_svr;
    uint16_t source_count;
    uint16_t * source_list;
    uint16_t selected;
    uint8_t i;
    struct aom_obj_it obj_it;
    SVR_GIFT_GENERATE_RECORD const * record_common;
    uint8_t element_count = gift_svr_generator_char_scope_element_count(generator->m_prefix.m_scope);
    uint8_t element_min = gift_svr_generator_char_scope_element_min(generator->m_prefix.m_scope);

    /*将所有的可以选择的点投射在在一个连续数组 */
    mem_buffer_clear_data(gd_app_tmp_buffer(svr->m_app));
    source_count = pow((float)element_count, (float)generator->m_prefix.m_len);
    source_list = mem_buffer_alloc(gd_app_tmp_buffer(svr->m_app), sizeof(uint16_t) * source_count);
    if (source_list == NULL) {
        CPE_ERROR(svr->m_em, "%s: select prefix: alloc source buff fail, count=%d!", gift_svr_name(svr), source_count);
        return -1;
    }
    for(i = 0; i < source_count; ++i) source_list[i] = i;

    /*在可以选择的数组中剔除已经使用的 */
    aom_objs(svr->m_generate_record_mgr, &obj_it);
    while((record_common = aom_obj_it_next(&obj_it))) {
        uint16_t source_index = 0;
        uint16_t check_len;
        
        if (record_common->cdkey_len != generator->m_cdkey_len) continue;

        /*前缀长度比当前的长，只考虑前置的前缀 */
        check_len = strlen(record_common->prefix);
        if (check_len > generator->m_prefix.m_len) {
            check_len = generator->m_prefix.m_len;
        }

        /*前缀计算为选择数组的下标 */
        for(i = 0; i < check_len; ++i) {
            uint8_t c = record_common->prefix[i] ;
            if (c >= element_min && c < element_min + element_count) {
                source_index *= element_count;
                source_index += (c - element_min);
            }
            else {
                source_index = 0;
                break;
            }
        }

        if (source_index == 0) continue;

        if (check_len < generator->m_prefix.m_len) {
            /*如果前缀长度小，则这个前缀开始的所有点都需要被清理 */
            uint32_t used_count = (uint16_t)pow((float)element_count, (float)(generator->m_prefix.m_len - check_len));
            source_index *= used_count;
            for(i = 0; i < used_count; ++i) {
                source_list[source_index + i] = 0;
            }
        }
        else {
            source_list[source_index] = 0;
        }
    }

    /*规则化选择数组，将所有被清理的数据都剔除出去 */
    for(i = 0; i < source_count; ++i) {
        if (source_list[i] == 0) {
            while((source_count - 1 > i) && source_list[source_count - 1] == 0) source_count--;

            if (source_count - 1 > i) {
                source_list[i] = source_list[source_count - 1];
                source_count--;
            }
        }
    }

    if (source_count == 0) {
        CPE_ERROR(svr->m_em, "%s: select prefix: no left prefix can be use!", gift_svr_name(svr));
        return -1;
    }

    /*随机选择一个前缀 */
    selected = source_list[cpe_rand_ctx_generate(cpe_rand_ctx_dft(), source_count)];

    /*将选择的数据转换成为前缀 */
    for(i = 0; i < generator->m_prefix.m_len; ++i) {
        output[generator->m_prefix.m_len - 1 - i] = (selected % element_count) + element_min;
        selected /= element_count;
    }

    output[generator->m_prefix.m_len] = 0;
    
    return 0;
}

int gift_svr_generators_load(gift_svr_t svr, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, cfg);

    while((child_cfg = cfg_it_next(&child_it))) {
        if (gift_svr_generator_create(svr, cfg_name(child_cfg), child_cfg) == NULL) {
            return -1;
        }
    }
    
    return 0; 
}

uint8_t gift_svr_generator_prefix_match(gift_svr_generator_t generator, const char * prefix) {
    size_t len = strlen(prefix);
    uint8_t i;
    uint8_t element_count = gift_svr_generator_char_scope_element_count(generator->m_prefix.m_scope);
    uint8_t element_min = gift_svr_generator_char_scope_element_min(generator->m_prefix.m_scope);

    if (generator->m_prefix.m_len != len) return 0;

    for(i = 0; i < len; ++i) {
        uint8_t c = prefix[i] ;
        if (c < element_min || c >= (element_min + element_count)) return 0;
    }

    return 1;
}

const char * gift_svr_generator_scope_name(gift_svr_generator_char_scope_t scope) {
    switch(scope) {
    case gift_svr_generator_char_scope_num:
        return "num";
    case gift_svr_generator_char_scope_letter:
        return "letter";
    default:
        return "unknown";
    }
}

static int gift_svr_generator_char_scope_from_str(gift_svr_t svr, cfg_t cfg, const char * attr, gift_svr_generator_char_scope_t * scope) {
    const char * value;

    value = cfg_get_string(cfg, attr, NULL);
    if (value == NULL) {
        CPE_ERROR(svr->m_em, "%s: generate: scope not configured!", gift_svr_name(svr));
        return -1;
    }

    if (strcmp(value, "num") == 0) {
        *scope = gift_svr_generator_char_scope_num;
    }
    else if (strcmp(value, "letter") == 0) {
        *scope = gift_svr_generator_char_scope_letter;
    }
    else {
        CPE_ERROR(svr->m_em, "%s: generate: scope %s unknown!", gift_svr_name(svr), value);
        return -1;
    }
    
    return 0;
}

static uint8_t gift_svr_generator_char_scope_element_count(gift_svr_generator_char_scope_t scope) {
    switch(scope) {
    case gift_svr_generator_char_scope_num:
        return 9;
    case gift_svr_generator_char_scope_letter:
        return ('Z' - 'A') + 1;
    default:
        return 0;
    }
}

static uint8_t gift_svr_generator_char_scope_element_min(gift_svr_generator_char_scope_t scope) {
    switch(scope) {
    case gift_svr_generator_char_scope_num:
        return '0';
    case gift_svr_generator_char_scope_letter:
        return 'A';
    default:
        return 0;
    }
}
