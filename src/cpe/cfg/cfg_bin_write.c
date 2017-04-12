#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_queue.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/hash.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_ops.h"

struct cfg_bin_write_stack {
    cfg_t m_parent;
    int m_size_pos;
    struct cfg_it m_cfg_it;
};

struct cfg_bin_write_str_ele {
    const char * m_str;
    int32_t m_pos;
    struct cpe_hash_entry m_hh;
    TAILQ_ENTRY(cfg_bin_write_str_ele) m_next;
};

struct cfg_bin_write_ctx {
    mem_allocrator_t m_tmp_alloc;
    char * m_output_buf;
    size_t m_output_capacity;
    struct mem_buffer * m_output_alloc;
    size_t m_used_size;
    error_monitor_t m_em;
    struct cpe_hash_table m_string_hash;
    TAILQ_HEAD(cfg_bin_write_str_ele_list, cfg_bin_write_str_ele) m_string_list;
    int32_t m_string_pos;
};

static void * cfg_bin_write_append(struct cfg_bin_write_ctx * ctx, size_t capacity);
static int32_t cfg_bin_write_add_string(struct cfg_bin_write_ctx * ctx, const char *);
static int cfg_bin_write_ctx_init(struct cfg_bin_write_ctx * ctx, void * result, size_t result_capacity, mem_buffer_t result_buffer, error_monitor_t em);
static void cfg_bin_write_ctx_fini(struct cfg_bin_write_ctx * ctx);
static int cfg_bin_write_build_strings(struct cfg_bin_write_ctx * ctx);

#define CFG_WRITE_BIN_APPEND_TYPE(__type) \
    tmp_buf = cfg_bin_write_append(&ctx, sizeof(uint8_t));  \
    if (tmp_buf == NULL) goto WRITE_ERROR; \
    *(uint8_t*)tmp_buf = __type

#define CFG_WRITE_BIN_APPEND_STRING(__str) \
    tmp_str = cfg_bin_write_add_string(&ctx, __str); \
    if (tmp_str < 0) goto WRITE_ERROR; \
    tmp_buf = cfg_bin_write_append(&ctx, sizeof(tmp_str));  \
    if (tmp_buf == NULL) goto WRITE_ERROR; \
    CPE_COPY_HTON32(tmp_buf, &tmp_str)

#define CFG_WRITE_BIN_APPEND_BASIC(__type, __cfg)   \
    switch(__type) {                                \
    case CPE_CFG_TYPE_INT8:                             \
    case CPE_CFG_TYPE_UINT8:                            \
        tmp_buf = cfg_bin_write_append(&ctx, 1);                    \
        if (tmp_buf == NULL) goto WRITE_ERROR;                  \
        *(uint8_t*)tmp_buf = *(uint8_t*)cfg_data(__cfg);        \
        break;                                          \
    case CPE_CFG_TYPE_INT16:                            \
    case CPE_CFG_TYPE_UINT16:                           \
        tmp_buf = cfg_bin_write_append(&ctx, 2);                    \
        if (tmp_buf == NULL) goto WRITE_ERROR;                  \
        CPE_COPY_HTON16(tmp_buf, cfg_data(__cfg));              \
        break;                                          \
    case CPE_CFG_TYPE_INT32:                            \
    case CPE_CFG_TYPE_UINT32:                           \
    case CPE_CFG_TYPE_FLOAT:                            \
        tmp_buf = cfg_bin_write_append(&ctx, 4);                    \
        if (tmp_buf == NULL) goto WRITE_ERROR;                  \
        CPE_COPY_HTON32(tmp_buf, cfg_data(__cfg));              \
        break;                                          \
    case CPE_CFG_TYPE_INT64:                            \
    case CPE_CFG_TYPE_UINT64:                           \
    case CPE_CFG_TYPE_DOUBLE:                           \
        tmp_buf = cfg_bin_write_append(&ctx, 8);                    \
        if (tmp_buf == NULL) goto WRITE_ERROR;                  \
        CPE_COPY_HTON64(tmp_buf, cfg_data(__cfg));              \
        break;                                          \
    case CPE_CFG_TYPE_STRING:                           \
        tmp_str = cfg_bin_write_add_string(&ctx, (const char *)cfg_data(__cfg)); \
        if (tmp_str < 0) goto WRITE_ERROR; \
        tmp_buf = cfg_bin_write_append(&ctx, sizeof(tmp_str));  \
        if (tmp_buf == NULL) goto WRITE_ERROR; \
        CPE_COPY_HTON32(tmp_buf, &tmp_str);             \
        break;                                          \
    default:                                            \
        CPE_ERROR(ctx.m_em, "cfg_bin_write: append_basic: unknown basic type %d!", (__type)); \
        goto WRITE_ERROR;                                               \
    }

static int cfg_bin_write_i(void * result, size_t result_capacity, mem_buffer_t result_buffer, cfg_t cfg, error_monitor_t em) {
    struct cfg_bin_write_stack processStack[64];
    int stackPos;
    struct cfg_bin_write_ctx ctx;
    struct cfg_format_bin_head * head;
    size_t data_total_len;
    void * tmp_buf;
    int32_t tmp_str;
    int cur_type;
    uint32_t tmp_size;

    if (cfg_bin_write_ctx_init(&ctx, result, result_capacity, result_buffer, em) != 0) return -1;

    if (cfg_bin_write_append(&ctx, sizeof(struct cfg_format_bin_head)) == NULL) goto WRITE_ERROR;

    cur_type = cfg_type(cfg);
    CFG_WRITE_BIN_APPEND_TYPE(cur_type);
    if (cur_type == CPE_CFG_TYPE_SEQUENCE || cur_type == CPE_CFG_TYPE_STRUCT) {
        processStack[0].m_size_pos = (int)ctx.m_used_size;
        processStack[0].m_parent = cfg;
        cfg_it_init(&processStack[0].m_cfg_it, cfg);
        stackPos = 0;

        if (cfg_bin_write_append(&ctx, sizeof(uint32_t)) == NULL) goto WRITE_ERROR;
    }
    else {
        CFG_WRITE_BIN_APPEND_BASIC(cur_type, cfg);
        stackPos = -1;
    }
    
    while(stackPos >= 0) {
        struct cfg_bin_write_stack * cur_stack;
        cfg_t cur_cfg;

    WRITE_GO_NEXT:

        assert(stackPos < CPE_ARRAY_SIZE(processStack));

        cur_stack = &processStack[stackPos];

        switch(cfg_type(cur_stack->m_parent)) {
        case CPE_CFG_TYPE_SEQUENCE:
            while((cur_cfg = cfg_it_next(&cur_stack->m_cfg_it))) {
                cur_type = cfg_type(cur_cfg);

                CFG_WRITE_BIN_APPEND_TYPE(cur_type);

                if (cfg_type_is_value(cur_type)) {
                    CFG_WRITE_BIN_APPEND_BASIC(cur_type, cur_cfg);
                }
                else {
                    if (stackPos + 1 >= CPE_ARRAY_SIZE(processStack)) {
                        CPE_ERROR(ctx.m_em, "cfg_bin_write: max level reached!");
                        goto WRITE_ERROR;
                    }
                    else {
                        struct cfg_bin_write_stack * new_stack;

                        stackPos++;
                        new_stack = processStack + stackPos;
                        new_stack->m_parent = cur_cfg;
                        new_stack->m_size_pos = (int)ctx.m_used_size;
                        cfg_it_init(&new_stack->m_cfg_it, new_stack->m_parent);

                        if (cfg_bin_write_append(&ctx, sizeof(uint32_t)) == NULL) goto WRITE_ERROR;

                        goto WRITE_GO_NEXT;
                    }
                }
            }
            break;
        case CPE_CFG_TYPE_STRUCT:
            while((cur_cfg = cfg_it_next(&cur_stack->m_cfg_it))) {
                cur_type = cfg_type(cur_cfg);

                CFG_WRITE_BIN_APPEND_TYPE(cur_type);
                CFG_WRITE_BIN_APPEND_STRING(cfg_name(cur_cfg));

                if (cfg_type_is_value(cur_type)) {
                    CFG_WRITE_BIN_APPEND_BASIC(cur_type, cur_cfg);
                }
                else {
                    if (stackPos + 1 >= CPE_ARRAY_SIZE(processStack)) {
                        CPE_ERROR(ctx.m_em, "cfg_bin_write: max level reached!");
                        goto WRITE_ERROR;
                    }
                    else {
                        struct cfg_bin_write_stack * new_stack;

                        stackPos++;
                        new_stack = processStack + stackPos;
                        new_stack->m_parent = cur_cfg;
                        new_stack->m_size_pos = (int)ctx.m_used_size;
                        cfg_it_init(&new_stack->m_cfg_it, new_stack->m_parent);

                        if (cfg_bin_write_append(&ctx, sizeof(uint32_t)) == NULL) goto WRITE_ERROR;

                        goto WRITE_GO_NEXT;
                    }
                }
            }
            break;
        default:
            CPE_ERROR(ctx.m_em, "cfg_bin_write: not support write basic value!");
            goto  WRITE_ERROR;
        }

        tmp_size = (uint32_t)(ctx.m_used_size - sizeof(struct cfg_format_bin_head));
        CPE_COPY_HTON32(ctx.m_output_buf + cur_stack->m_size_pos, &tmp_size);

        --stackPos;
    }

    data_total_len = ctx.m_used_size;

    /*构造string表 */
    if (cfg_bin_write_build_strings(&ctx) != 0) goto WRITE_ERROR;

    /*填写head */
    head = (struct cfg_format_bin_head *)ctx.m_output_buf;
    assert(head);

    memcpy(&head->m_magic, CFG_FORMAT_BIN_MATIC, sizeof(head->m_magic));
    head->m_data_start = sizeof(struct cfg_format_bin_head);
    head->m_data_capacity = (uint32_t)(data_total_len - head->m_data_start);
    head->m_strpool_start = (uint32_t)data_total_len;
    head->m_strpool_capacity = (uint32_t)(ctx.m_used_size - head->m_strpool_start);

    CPE_SWAP_HTON32(&head->m_data_start);
    CPE_SWAP_HTON32(&head->m_data_capacity);
    CPE_SWAP_HTON32(&head->m_strpool_start);
    CPE_SWAP_HTON32(&head->m_strpool_capacity);

    /*清理 */
    data_total_len = ctx.m_used_size;
    cfg_bin_write_ctx_fini(&ctx);

    return (int)data_total_len;

WRITE_ERROR:
    cfg_bin_write_ctx_fini(&ctx);

    return -1;
}

int cfg_bin_write(void * result, size_t result_capacity, cfg_t cfg, error_monitor_t em) {
    int err = 0;
    int r;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &err);
        r = cfg_bin_write_i(result, result_capacity, NULL, cfg, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &err);
        r = cfg_bin_write_i(NULL, 0, result, cfg, &logError);
    }

    return r < 0 ? err : r;
}

int cfg_bin_write_to_buffer(struct mem_buffer * result, cfg_t cfg, error_monitor_t em) {
    int err = 0;
    int r;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &err);
        r = cfg_bin_write_i(NULL, 0, result, cfg, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &err);
        r = cfg_bin_write_i(NULL, 0, result, cfg, &logError);
    }

    return r < 0 ? err : r;
}

static void * cfg_bin_write_append(struct cfg_bin_write_ctx * ctx, size_t require) {
    size_t total_size;
    void * r;

    assert(ctx);

    total_size = ctx->m_used_size + require;

    if (total_size > ctx->m_output_capacity) {
        if (ctx->m_output_alloc) {
            uint32_t new_capacity = ctx->m_output_capacity < 1024 ? 1024u : (uint32_t)(ctx->m_output_capacity * 2);
            while (new_capacity < total_size) {
                new_capacity = new_capacity * 2;
            }

            ctx->m_output_buf = mem_buffer_make_continuous(ctx->m_output_alloc, new_capacity - ctx->m_used_size);
            if (ctx->m_output_buf == NULL) {
                CPE_ERROR(ctx->m_em, "dr_pbuf_read: resize to %d fail!", (int)new_capacity);
                return NULL;
            }
            ctx->m_output_capacity = new_capacity;
        }
        else {
            CPE_ERROR_EX(ctx->m_em, dr_code_error_not_enough_output, "dr_pbuf_read: not enouth output!");
            return NULL;
        }
    }

    r = ctx->m_output_buf + ctx->m_used_size;
    ctx->m_used_size += require;

    if (ctx->m_output_alloc) {
        if (mem_buffer_set_size(ctx->m_output_alloc, ctx->m_used_size) != 0) {
            CPE_ERROR(ctx->m_em, "dr_pbuf_read: set size to %d fail!", (int)ctx->m_used_size);
            return NULL;
        }
    }

    return r;
}

static int32_t cfg_bin_write_add_string(struct cfg_bin_write_ctx * ctx, const char * str) {
    struct cfg_bin_write_str_ele key;
    struct cfg_bin_write_str_ele * ele;

    key.m_str = str;

    ele = cpe_hash_table_find(&ctx->m_string_hash, &key);
    if (ele == NULL) {
        ele = mem_alloc(ctx->m_tmp_alloc, sizeof(struct cfg_bin_write_str_ele));
        if (ele == NULL) {
            CPE_ERROR(ctx->m_em, "cfg_bin_write: add_string: alloc string_ele fail!");
            return -1;
        }

        ele->m_str = str;
        ele->m_pos = ctx->m_string_pos;

        ctx->m_string_pos += strlen(str) + 1;

        TAILQ_INSERT_TAIL(&ctx->m_string_list, ele, m_next);
        cpe_hash_entry_init(&ele->m_hh);
        cpe_hash_table_insert_unique(&ctx->m_string_hash, ele);
    }

    return ele->m_pos;
}

static int cfg_bin_write_build_strings(struct cfg_bin_write_ctx * ctx) {
    struct cfg_bin_write_str_ele * ele;

    TAILQ_FOREACH(ele, &ctx->m_string_list, m_next) {
        size_t len = strlen(ele->m_str) + 1;
        void * buf = cfg_bin_write_append(ctx, len);
        if (buf == NULL) {
            CPE_ERROR(ctx->m_em, "cfg_bin_write: build_strings: alloc buf fail, len=%d", (int)len);
            return -1;
        }
        memcpy(buf, ele->m_str, len);
    }

    return 0;
}

static uint32_t cfg_bin_write_str_ele_hash(const struct cfg_bin_write_str_ele * ele) {
    return cpe_hash_str(ele->m_str, strlen(ele->m_str));
}

static uint32_t cfg_bin_write_str_ele_eq(const struct cfg_bin_write_str_ele * l, const struct cfg_bin_write_str_ele * r) {
    return strcmp(l->m_str, r->m_str) == 0;
}

static int cfg_bin_write_ctx_init(
    struct cfg_bin_write_ctx * ctx,
    void * result, size_t result_capacity, mem_buffer_t result_buffer, error_monitor_t em)
{
    ctx->m_tmp_alloc = NULL;
    ctx->m_output_buf = result;
    ctx->m_output_capacity = result_capacity;
    ctx->m_output_alloc = result_buffer;
    ctx->m_em = em;
    ctx->m_used_size = 0;
    ctx->m_string_pos = 0;

    TAILQ_INIT(&ctx->m_string_list);

    if (cpe_hash_table_init(
            &ctx->m_string_hash,
            ctx->m_tmp_alloc,
            (cpe_hash_fun_t) cfg_bin_write_str_ele_hash,
            (cpe_hash_eq_t) cfg_bin_write_str_ele_eq,
            CPE_HASH_OBJ2ENTRY(cfg_bin_write_str_ele, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "cfg_bin_write: init string hash fail!");
        return -1;
    }

    return 0;
}

static void cfg_bin_write_ctx_fini(struct cfg_bin_write_ctx * ctx) {
    while(!TAILQ_EMPTY(&ctx->m_string_list)) {
        struct cfg_bin_write_str_ele * ele = TAILQ_FIRST(&ctx->m_string_list);
        TAILQ_REMOVE(&ctx->m_string_list, ele, m_next);
        cpe_hash_table_remove_by_ins(&ctx->m_string_hash, ele);
        mem_free(ctx->m_tmp_alloc, ele);
    }

    cpe_hash_table_fini(&ctx->m_string_hash);
}

int cfg_bin_write_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em) {
    struct mem_buffer write_buff;
    int size;
    vfs_file_t output_file;
    
    mem_buffer_init(&write_buff, NULL);
    
    size = cfg_bin_write_to_buffer(&write_buff, cfg, em);
    if (size < 0) {
        CPE_ERROR(em, "cfg_bin_write_file: write to bin fail, rv=%d!", size);
        mem_buffer_clear(&write_buff);
        return -1;
    }
    assert(mem_buffer_size(&write_buff) == (size_t)size);
    
    output_file = vfs_file_open(vfs, path, "wb");
    if (output_file == NULL) {
        CPE_ERROR(
            em, "cfg_bin_write_file:: create file %s fail, errno=%d (%s)!",
            path, errno, strerror(errno));
        mem_buffer_clear(&write_buff);
        return -1;
    }

    if (vfs_file_write_from_buffer(output_file, &write_buff) < 0) {
        CPE_ERROR(
            em, "cfg_bin_write_file: write to %s fail, errno=%d (%s)!", path, errno, strerror(errno));
        vfs_file_close(output_file);
        mem_buffer_clear(&write_buff);
        return -1;
    }

    vfs_file_close(output_file);
    mem_buffer_clear(&write_buff);
    
    return 0;
}
