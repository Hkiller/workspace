#include <errno.h>
#include "yaml.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cfg_internal_ops.h"

#define CFG_YAML_TAG_BUF_LEN 128

struct cfg_yaml_write_ctx {
    yaml_document_t m_document;

    error_monitor_t m_em;
    char m_tag_buffer[CFG_YAML_TAG_BUF_LEN];
};

static int cfg_write_to_stream_handler(void *data, unsigned char *buffer, size_t size) {
    int writeSize;

    writeSize = stream_write((write_stream_t)data, (char*)buffer, size);

    return writeSize == size ? 1 : 0;
}

static int cfg_yaml_write_ctx_init(
    struct cfg_yaml_write_ctx * ctx,
    error_monitor_t em)
{
    bzero(ctx, sizeof(struct cfg_yaml_write_ctx));

    if (!yaml_document_initialize(&ctx->m_document, NULL, NULL, NULL, 0, 0)) {
        CPE_ERROR(ctx->m_em, "yaml document initialize fail!");
        return -1;
    }

    ctx->m_em = em;

    return 0;
}

static void cfg_yaml_write_ctx_fini(struct cfg_yaml_write_ctx * ctx) {
    yaml_document_delete(&ctx->m_document);
}

static void cfg_yaml_notify_document_error(struct cfg_yaml_write_ctx * ctx) {
    CPE_ERROR(ctx->m_em, "Memory error: Not enough memory for creating a document");
}

static int cfg_yaml_do_write_cfg(struct cfg_yaml_write_ctx * ctx, cfg_t cfg);

static int cfg_yaml_do_write_cfg_scalar(struct cfg_yaml_write_ctx * ctx, cfg_t cfg) {
    int v;

    if (cfg->m_type == CPE_CFG_TYPE_STRING) {
        v = yaml_document_add_scalar( 
            &ctx->m_document,
            (yaml_char_t *)YAML_STR_TAG,
            (yaml_char_t *)cfg_data(cfg),
            -1,
            YAML_SINGLE_QUOTED_SCALAR_STYLE);
    }
    else {
        char buf[20 + 1];
        struct write_stream_mem bufS = CPE_WRITE_STREAM_MEM_INITIALIZER(buf, 20);
        int len = dr_ctype_print_to_stream((write_stream_t)&bufS, cfg_data(cfg), cfg->m_type, ctx->m_em);
        if (len > 0) {
            const char * tag = NULL;

            buf[len] = 0;

            if (cfg->m_type != CPE_CFG_TYPE_INT32 && cfg->m_type != CPE_CFG_TYPE_FLOAT) {
                snprintf(ctx->m_tag_buffer, CFG_YAML_TAG_BUF_LEN, "!%s", dr_type_name(cfg->m_type));
                tag = ctx->m_tag_buffer;
            }

            v = yaml_document_add_scalar( 
                &ctx->m_document,
                (yaml_char_t*)tag,
                (yaml_char_t *)buf,
                -1,
                YAML_PLAIN_SCALAR_STYLE);
        }
        else {
            v = 0;
        }
    }

    if (!v) {
        cfg_yaml_notify_document_error(ctx);
    }

    return v;
}

static int cfg_yaml_do_write_cfg_struct(struct cfg_yaml_write_ctx * ctx, struct cfg_struct * cfg) {
    struct cfg_struct_item * item;

    int mapNode = yaml_document_add_mapping(&ctx->m_document, NULL, YAML_BLOCK_MAPPING_STYLE);
    if (!mapNode) {
        cfg_yaml_notify_document_error(ctx);
        return mapNode;
    }

    RB_FOREACH(item, cfg_struct_item_tree, &cfg->m_items) {
        int key = yaml_document_add_scalar(
            &ctx->m_document, NULL, (yaml_char_t *) item->m_name, -1, YAML_PLAIN_SCALAR_STYLE);
        int value = cfg_yaml_do_write_cfg(ctx, &item->m_data);

        if (key && value) {
            if (!yaml_document_append_mapping_pair(&ctx->m_document, mapNode, key, value)) {
                cfg_yaml_notify_document_error(ctx);
            }
        }
    }

    return mapNode;
}

static int cfg_yaml_do_write_cfg_seq(struct cfg_yaml_write_ctx * ctx, struct cfg_seq * cfg) {
    int i = 0;
    int count = cfg->m_count;
    struct cfg_seq_block * b = &cfg->m_block_head;

    int seqNode = yaml_document_add_sequence(&ctx->m_document, NULL, YAML_BLOCK_SEQUENCE_STYLE);
    if (!seqNode) {
        cfg_yaml_notify_document_error(ctx);
        return seqNode;
    }

    for(; count > 0 && b; ++i, --count) {
        int subNode;
        if (i >= CPE_CFG_SEQ_BLOCK_ITEM_COUNT) {
            i = 0;
            b = b->m_next;
            if (b == NULL) break;
        }

        subNode = cfg_yaml_do_write_cfg(ctx, b->m_items[i]);
        if (!subNode) {
            continue;
        }

        if (!yaml_document_append_sequence_item(&ctx->m_document, seqNode, subNode)) {
            cfg_yaml_notify_document_error(ctx);
        }
    }
    
    return seqNode;
}

static int cfg_yaml_do_write_cfg(struct cfg_yaml_write_ctx * ctx, cfg_t cfg) {
    switch(cfg->m_type) {
    case CPE_CFG_TYPE_STRUCT:
        return cfg_yaml_do_write_cfg_struct(ctx, (struct cfg_struct *)cfg);
    case CPE_CFG_TYPE_SEQUENCE:
        return cfg_yaml_do_write_cfg_seq(ctx, (struct cfg_seq *)cfg);
    default:
        return cfg_yaml_do_write_cfg_scalar(ctx, cfg);
    }
}

static void cfg_yaml_notify_emitter_error(yaml_emitter_t * emitter, error_monitor_t em) {
    switch (emitter->error) {
    case YAML_MEMORY_ERROR:
        CPE_ERROR(em, "Memory error: Not enough memory for emitting\n");
        break;
    case YAML_WRITER_ERROR:
        CPE_ERROR(em, "Writer error: %s\n", emitter->problem);
        break;
    case YAML_EMITTER_ERROR:
        CPE_ERROR(em, "Emitter error: %s\n", emitter->problem);
        break;
    default:
        /* Couldn't happen. */
        CPE_ERROR(em, "Internal error\n");
        break;
    }
}

static void cfg_dump_document_stream(struct cfg_yaml_write_ctx * ctx, write_stream_t stream) {
    yaml_emitter_t emitter;
    bzero(&emitter, sizeof(emitter));

    if (!yaml_emitter_initialize(&emitter)) {
        CPE_ERROR(ctx->m_em, "yaml emmit initialize fail!");
        return;
    }

    yaml_emitter_set_output(&emitter, cfg_write_to_stream_handler, stream);
    /* yaml_emitter_set_canonical(&emitter, canonical); */
    yaml_emitter_set_unicode(&emitter, YAML_UTF8_ENCODING);
    yaml_emitter_set_indent(&emitter, 4);

    if (!yaml_emitter_open(&emitter)
        || !yaml_emitter_dump(&emitter, &ctx->m_document)
        || !yaml_emitter_close(&emitter))
    {
        cfg_yaml_notify_emitter_error(&emitter, ctx->m_em);
    }

    yaml_emitter_delete(&emitter);
}

static void cfg_yaml_write_i(write_stream_t stream, cfg_t cfg, error_monitor_t em) {
    struct cfg_yaml_write_ctx ctx;

    if (cfg_yaml_write_ctx_init(&ctx, em) != 0) return;

    cfg_yaml_do_write_cfg(&ctx, cfg);

    cfg_dump_document_stream(&ctx, stream);

    cfg_yaml_write_ctx_fini(&ctx);
}

int cfg_yaml_write(write_stream_t stream, cfg_t cfg, error_monitor_t em) {
    int ret = 0;
    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        cfg_yaml_write_i(stream, cfg, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        cfg_yaml_write_i(stream, cfg, &logError);
    }

    return ret;
}

int cfg_yaml_write_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em) {
    struct vfs_write_stream fs;
    vfs_file_t output_file;
    
    output_file = vfs_file_open(vfs, path, "wb");
    if (output_file == NULL) {
        CPE_ERROR(
            em, "cfg_bin_write_file:: create file %s fail, errno=%d (%s)!",
            path, errno, strerror(errno));
        return -1;
    }

    vfs_write_stream_init(&fs, output_file);
    if (cfg_yaml_write((write_stream_t)&fs, cfg, em) < 0) {
        CPE_ERROR(
            em, "cfg_bin_write_file: write to %s fail, errno=%d (%s)!", path, errno, strerror(errno));
        vfs_file_close(output_file);
        return -1;
    }

    vfs_file_close(output_file);
    
    return 0;
}
