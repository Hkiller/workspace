#include <assert.h>
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils_xml/xml_utils.h"
#include "cpe/cfg/cfg.h"
#include "convert_ctx.h"

struct convert_load_res_cfg_ctx {
    cfg_t m_cfg;
    convert_ctx_t m_ctx;
    uint8_t m_in_sys_font;
    uint8_t m_in_art_font;
    char m_cur_tag_name[64];
};

static void convert_load_res_cfg_startElement(
        void* iprojutCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI,
        int nb_namespaces,
        const xmlChar** namespaces,
        int nb_attributes,
        int nb_defaulted,
        const xmlChar** attributes)
{
    struct convert_load_res_cfg_ctx * ctx = (struct convert_load_res_cfg_ctx *)(iprojutCtx);

    if (strcmp((const char *)localname, "SysFontList") == 0) {
        ctx->m_in_sys_font = 1;
    }
    else if (strcmp((const char *)localname, "ArtFontList") == 0) {
        ctx->m_in_art_font = 1;
    }
    else if (strcmp((const char *)localname, "File") == 0) {
        const char * list_name;
        cfg_t font_list;
        char path[256];
        
        if (ctx->m_in_sys_font) {
            list_name = "sys-fonts";
        }
        else if (ctx->m_in_art_font) {
            list_name = "art-fonts";
        }
        else {
            CPE_ERROR(ctx->m_ctx->m_em, "exportRuning_loadResCfg: read File in error state!");
            return;
        }

        font_list = cfg_find_cfg(ctx->m_cfg, list_name);
        if (font_list == NULL) {
            font_list = cfg_struct_add_seq(ctx->m_cfg, list_name, cfg_merge_use_new);
            if (font_list == NULL) {
                CPE_ERROR(ctx->m_ctx->m_em, "exportRuning_loadResCfg: create font list %s fail!", list_name);
                return;
            }
        }

        if (cpe_xml_find_attr_string(path, sizeof(path), "Name", nb_attributes, attributes, ctx->m_ctx->m_em) == NULL) {
            CPE_ERROR(ctx->m_ctx->m_em, "exportRuning_loadResCfg: read File fail!");
            return;
        }

        cfg_seq_add_string(font_list, path);
    }
    else {
        cpe_str_dup(ctx->m_cur_tag_name, sizeof(ctx->m_cur_tag_name), (const char*)localname);
    }
}

static void convert_load_res_cfg_endElement(
        void* iprojutCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct convert_load_res_cfg_ctx * ctx = (struct convert_load_res_cfg_ctx *)(iprojutCtx);
    ctx->m_cur_tag_name[0] = 0;

    if (strcmp((const char *)localname, "SysFontList") == 0) {
        ctx->m_in_sys_font = 0;
    }
    else if (strcmp((const char *)localname, "ArtFontList") == 0) {
        ctx->m_in_art_font = 0;
    }
}

static void convert_load_res_cfg_characters(void * iprojutCtx, const xmlChar *ch, int len) {
    struct convert_load_res_cfg_ctx * ctx = (struct convert_load_res_cfg_ctx *)(iprojutCtx);

    if (strcmp(ctx->m_cur_tag_name, "ActionTime") == 0) {
        uint32_t value;
        if (cpe_xml_read_value_uint32(&value, (const char *)ch, len) != 0) {
            CPE_ERROR(ctx->m_ctx->m_em, "exportRuning_loadResCfg: read ActionTime value fail!");
        }
        
        cfg_struct_add_uint32(ctx->m_cfg, "action-time", value, cfg_replace);
    }
}

static void convert_load_res_cfg_structed_error(void * iprojutCtx, xmlErrorPtr err) {
    struct convert_load_res_cfg_ctx * ctx = (struct convert_load_res_cfg_ctx *)(iprojutCtx);

    CPE_ERROR_SET_LEVEL(
        ctx->m_ctx->m_em,
        err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

    CPE_ERROR_SET_LINE(ctx->m_ctx->m_em, err->line);

    cpe_error_do_notify(ctx->m_ctx->m_em, "(%d) %s", err->code, err->message);
}

static xmlSAXHandler g_convert_load_res_cfg_callbacks = {
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
    , NULL /* uprojarsedEntityDeclSAXFunc uprojarsedEntityDecl */
    , NULL /* setDocumentLocatorSAXFunc setDocumentLocator */
    , NULL /* startDocumentSAXFunc startDocument */
    , NULL /* endDocumentSAXFunc endDocument */
    , NULL /* startElementSAXFunc startElement */
    , NULL /* endElementSAXFunc endElement */
    , NULL /* referenceSAXFunc reference */
    , convert_load_res_cfg_characters /* charactersSAXFunc characters */
    , NULL /* ignorableWhitespaceSAXFunc ignorableWhitespace */
    , NULL /* processingInstructionSAXFunc processingInstruction */
    , NULL /* commentSAXFunc comment */
    , NULL /* warningSAXFunc warning */
    , NULL /* errorSAXFunc error */
    , NULL /* fatalErrorSAXFunc fatalError; unused error() get all the errors * */
    , NULL /* getParameterEntitySAXFunc getParameterEntity */
    , NULL /* cdataBlockSAXFunc cdataBlock */
    , NULL /* externalSubsetSAXFunc externalSubset */
    , XML_SAX2_MAGIC /* unsigned int initialized */
    , NULL /* void *_private */
    , convert_load_res_cfg_startElement /* startElementNsSAX2Func startElementNs */
    , convert_load_res_cfg_endElement /* endElementNsSAX2Func endElementNs */
    , convert_load_res_cfg_structed_error /* xmlStructuredErrorFunc serror */
};

void convert_load_res_cfg_i(convert_ctx_t i_ctx) {
    struct convert_load_res_cfg_ctx ctx;
    char path[256];

    ctx.m_cfg = cfg_struct_add_struct(i_ctx->m_runing, "res-cfg", cfg_merge_use_new);
    assert(ctx.m_cfg);
    ctx.m_ctx = i_ctx;
    ctx.m_in_sys_font = 0;
    ctx.m_in_art_font = 0;
    ctx.m_cur_tag_name[0] = 0;

    snprintf(path, sizeof(path), "%s/res.cfg", i_ctx->m_root);

    if (xmlSAXUserParseFile(&g_convert_load_res_cfg_callbacks, &ctx, path) < 0) {
        CPE_ERROR(i_ctx->m_em, "parse fail!");
    }
}

int convert_load_res_cfg(convert_ctx_t ctx) {
    int ret = 0;

    if (ctx->m_em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, ctx->m_em, cpe_error_save_last_errno, &ret);
        convert_load_res_cfg_i(ctx);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, ctx->m_em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ctx->m_em = &logError;
        convert_load_res_cfg_i(ctx);
        ctx->m_em = NULL;
    }

    return ret;
}
