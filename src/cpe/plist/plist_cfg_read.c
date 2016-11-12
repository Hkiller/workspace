#include <assert.h>
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg.h"
#include "cpe/plist/plist_cfg.h"

#define XML_PARSE_CTX_BUF_LEN CPE_DR_MACRO_LEN + 1

struct plist_cfg_parse_ctx {
    cfg_t m_root_cfg;
    cfg_policy_t m_policy;
    cfg_t m_cur_cfg;
    error_monitor_t m_em;
    char m_next_key[128];
    int m_next_type;
    int m_value_is_key;
    int m_is_first;
};

static void plist_cfg_xml_startElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI,
        int nb_namespaces,
        const xmlChar** namespaces,
        int nb_attributes,
        int nb_defaulted,
        const xmlChar** attributes)
{
    struct plist_cfg_parse_ctx * ctx = (struct plist_cfg_parse_ctx *)(inputCtx);

    if (strcmp((const char *)localname, "plist") == 0) {
        ctx->m_is_first = 1;
        ctx->m_cur_cfg = ctx->m_root_cfg;
        return;
    }

    if (strcmp((const char *)localname, "key") == 0) {
        ctx->m_value_is_key = 1;
        return;
    }

    if (ctx->m_cur_cfg == NULL) return;

    if (strcmp((const char *)localname, "dict") == 0) {
        if (ctx->m_is_first) {
            ctx->m_is_first = 0;
            return;
        }

        if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_STRUCT) {
            ctx->m_cur_cfg = cfg_struct_add_struct(ctx->m_cur_cfg, ctx->m_next_key, ctx->m_policy);
        }
        else if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_SEQUENCE) {
            ctx->m_cur_cfg = cfg_seq_add_struct(ctx->m_cur_cfg);
        }
    }
    else if (strcmp((const char *)localname, "array") == 0) {
        if (ctx->m_is_first) {
            CPE_ERROR(ctx->m_em, "not support array in root");
            return;
        }

        if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_STRUCT) {
            ctx->m_cur_cfg = cfg_struct_add_seq(ctx->m_cur_cfg, ctx->m_next_key, ctx->m_policy);
        }
        else if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_SEQUENCE) {
            ctx->m_cur_cfg = cfg_seq_add_seq(ctx->m_cur_cfg);
        }
    }
    else if (strcmp((const char *)localname, "string") == 0) {
        ctx->m_next_type = CPE_CFG_TYPE_STRING;
    }
    else if (strcmp((const char *)localname, "integer") == 0) {
        ctx->m_next_type = CPE_CFG_TYPE_INT64;
    }
    else if (strcmp((const char *)localname, "real") == 0) {
        ctx->m_next_type = CPE_CFG_TYPE_DOUBLE;
    }
    else if (strcmp((const char *)localname, "true") == 0) {
        if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_STRUCT) {
            cfg_struct_add_uint8(ctx->m_cur_cfg, ctx->m_next_key, 1, ctx->m_policy);
        }
        else if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_SEQUENCE) {
            cfg_seq_add_uint8(ctx->m_cur_cfg, 1);
        }
    }
    else if (strcmp((const char *)localname, "false") == 0) {
        if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_STRUCT) {
            cfg_struct_add_uint8(ctx->m_cur_cfg, ctx->m_next_key, 0, ctx->m_policy);
        }
        else if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_SEQUENCE) {
            cfg_seq_add_uint8(ctx->m_cur_cfg, 0);
        }
    }
}

static void plist_cfg_xml_endElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct plist_cfg_parse_ctx * ctx = (struct plist_cfg_parse_ctx *)(inputCtx);

    if (strcmp((const char *)localname, "plist") == 0) {
        ctx->m_cur_cfg = NULL;
    }
    else if (strcmp((const char *)localname, "key") == 0) {
        ctx->m_value_is_key = 0;
    }
    else if (strcmp((const char *)localname, "dict") == 0) {
        if (ctx->m_cur_cfg == ctx->m_root_cfg) {
            ctx->m_cur_cfg = NULL;
        }
        else {
            ctx->m_cur_cfg = cfg_parent(ctx->m_cur_cfg);
        }
        ctx->m_next_type = -1;
    }
    else if (strcmp((const char *)localname, "array") == 0) {
        if (ctx->m_cur_cfg == ctx->m_root_cfg) {
            ctx->m_cur_cfg = NULL;
        }
        else {
            ctx->m_cur_cfg = cfg_parent(ctx->m_cur_cfg);
        }
        ctx->m_next_type = -1;
    }
    else if (strcmp((const char *)localname, "string") == 0) {
        ctx->m_next_type = -1;
    }
    else if (strcmp((const char *)localname, "real") == 0) {
        ctx->m_next_type = -1;
    }
    else if (strcmp((const char *)localname, "integer") == 0) {
        ctx->m_next_type = -1;
    }
    else if (strcmp((const char *)localname, "true") == 0) {
        ctx->m_next_type = -1;
    }
    else if (strcmp((const char *)localname, "false") == 0) {
        ctx->m_next_type = -1;
    }
    else {
        CPE_ERROR(ctx->m_em, "not support tak %s", (const char *)localname);
        ctx->m_next_type = -1;
    }
}

static void plist_cfg_xml_characters(void * inputCtx, const xmlChar *ch, int len) {
    struct plist_cfg_parse_ctx * ctx = (struct plist_cfg_parse_ctx *)(inputCtx);

    if (ctx->m_value_is_key) {
        if (len + 1 > sizeof(ctx->m_next_key)) len = sizeof(ctx->m_next_key) -1;
        memcpy(ctx->m_next_key, ch, len);
        ctx->m_next_key[len] = 0;
    }
    else {
        switch(ctx->m_next_type) {
        case CPE_CFG_TYPE_STRING:
            if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_STRUCT) {
                cfg_struct_add_string_len(ctx->m_cur_cfg, ctx->m_next_key, (const char *)ch, (size_t)len, ctx->m_policy);
            }
            else if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_SEQUENCE) {
                cfg_seq_add_string_len(ctx->m_cur_cfg, (const char *)ch, (size_t)len);
            }
            break;
        case CPE_CFG_TYPE_INT64:
            if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_STRUCT) {
                cfg_struct_add_int64(ctx->m_cur_cfg, ctx->m_next_key, (int64_t)atoi((const char *)ch), ctx->m_policy);
            }
            else if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_SEQUENCE) {
                cfg_seq_add_int64(ctx->m_cur_cfg, (int64_t)atoi((const char *)ch));
            }
            break;
        case CPE_CFG_TYPE_DOUBLE:
            if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_STRUCT) {
                cfg_struct_add_double(ctx->m_cur_cfg, ctx->m_next_key, strtod((const char *)ch, NULL), ctx->m_policy);
            }
            else if (cfg_type(ctx->m_cur_cfg) == CPE_CFG_TYPE_SEQUENCE) {
                cfg_seq_add_double(ctx->m_cur_cfg, strtod((const char *)ch, NULL));
            }
            break;
        default:
            break;
        }
    }
}

static void plist_cfg_xml_warningSAX(void * inputCtx, const char *msg, ...) {
    struct plist_cfg_parse_ctx * ctx = (struct plist_cfg_parse_ctx *)(inputCtx);
    va_list ap;

    va_start(ap, msg);

    CPE_ERROR_SET_LEVEL(ctx->m_em, CPE_EL_WARNING);
    CPE_ERROR_SET_ERRNO(ctx->m_em, dr_code_error_format_error);
    cpe_error_do_notify_var(ctx->m_em, msg, ap);

    va_end(ap);
}

static void plist_cfg_xml_errorSAX(void * inputCtx, const char *msg, ...) {
    struct plist_cfg_parse_ctx * ctx = (struct plist_cfg_parse_ctx *)(inputCtx);
    va_list ap;

    va_start(ap, msg);

    CPE_ERROR_SET_LEVEL(ctx->m_em, CPE_EL_ERROR);
    CPE_ERROR_SET_ERRNO(ctx->m_em, dr_code_error_format_error);
    cpe_error_do_notify_var(ctx->m_em, msg, ap);

    va_end(ap);
}

static xmlSAXHandler g_plist_cfg_callbacks = {
    NULL /* internalSubsetSAXFunc internalSubset */
    , NULL /* isStandaloneSAXFunc isStandalone */
    , NULL /* hasInternalSubsetSAXFunc hasInternalSubset */
    , NULL /* hasExternalSubsetSAXFunc hasExternalSubset */
    , NULL /* resolveEntitySAXFunc resolveEntity */
    , NULL /* getEntitySAXFunc getEntity */
    , NULL /* entityDeclSAXFunc entityDecl */
    , NULL /* notationDeclSAXFunc notationDecl */
    , NULL /* attributeDeclSAXFunc attributeDecl */
    , NULL /* elementDeclSAXFunc elementDecl */
    , NULL /* unparsedEntityDeclSAXFunc unparsedEntityDecl */
    , NULL /* setDocumentLocatorSAXFunc setDocumentLocator */
    , NULL /* startDocumentSAXFunc startDocument */
    , NULL /* endDocumentSAXFunc endDocument */
    , NULL /* startElementSAXFunc startElement */
    , NULL /* endElementSAXFunc endElement */
    , NULL /* referenceSAXFunc reference */
    , plist_cfg_xml_characters /* charactersSAXFunc characters */
    , NULL /* ignorableWhitespaceSAXFunc ignorableWhitespace */
    , NULL /* processingInstructionSAXFunc processingInstruction */
    , NULL /* commentSAXFunc comment */
    , plist_cfg_xml_warningSAX /* warningSAXFunc warning */
    , plist_cfg_xml_errorSAX /* errorSAXFunc error */
    , NULL /* fatalErrorSAXFunc fatalError; unused error() get all the errors * */
    , NULL /* getParameterEntitySAXFunc getParameterEntity */
    , NULL /* cdataBlockSAXFunc cdataBlock */
    , NULL /* externalSubsetSAXFunc externalSubset */
    , XML_SAX2_MAGIC /* unsigned int initialized */
    , NULL /* void *_private */
    , plist_cfg_xml_startElement /* startElementNsSAX2Func startElementNs */
    , plist_cfg_xml_endElement /* endElementNsSAX2Func endElementNs */
    , NULL /* xmlStructuredErrorFunc serror */
};

static void plist_cfg_read_i(cfg_t cfg, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    struct plist_cfg_parse_ctx ctx;
    xmlParserCtxtPtr parseCtx = NULL;
    char buf[1024];
    int sz;

    bzero(&ctx, sizeof(struct plist_cfg_parse_ctx));

    ctx.m_root_cfg = cfg;
    ctx.m_policy = policy;
    ctx.m_em = em;
    ctx.m_next_type = -1;

    sz = stream_read(stream, buf, sizeof(buf));
    if (sz > 0) {
        parseCtx = xmlCreatePushParserCtxt(&g_plist_cfg_callbacks, &ctx, buf, sz, NULL);
        if (parseCtx == NULL) {
            CPE_ERROR_EX(em, dr_code_error_internal, "can`t alloc memory for xml parser");
            return;
        }

        while ((sz = stream_read(stream, buf, sizeof(buf))) > 0) {
            xmlParseChunk(parseCtx, buf, sz, 0);
        }

        xmlParseChunk(parseCtx, buf, 0, 1);
    }

    xmlFreeParserCtxt(parseCtx);
}

int plist_cfg_read(cfg_t cfg, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        plist_cfg_read_i(cfg, stream, policy, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        plist_cfg_read_i(cfg, stream, policy, &logError);
    }

    return ret;
}

cfg_t plist_cfg_load_dict_from_file(const char * file_path, error_monitor_t em) {
    cfg_t r;
    struct read_stream_file rs;
    FILE * fp;

    r = cfg_create(NULL);
    if (r == NULL) {
        CPE_ERROR(em, "plist_cfg_load_dict_from_file: alloc cfg fail!");
        return NULL;
    }

    fp = file_stream_open(file_path, "r", em);
    if (fp == NULL) {
        CPE_ERROR(em, "plist_cfg_load_dict_from_file: open file %s fail!", file_path);
        cfg_free(r);
        return NULL;
    }

    read_stream_file_init(&rs, fp, em);

    if (plist_cfg_read(r, (read_stream_t)&rs, cfg_merge_use_new, em) != 0) {
        CPE_ERROR(em, "plist_cfg_load_dict_from_file: read from file %s fail!", file_path);
        file_stream_close(fp, em);
        cfg_free(r);
        return NULL;
    }

    file_stream_close(fp, em);
    return r;
}
