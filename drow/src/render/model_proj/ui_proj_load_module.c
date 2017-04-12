#include <assert.h>
#include "libxml/xmlstring.h"
#include "libxml/parser.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "render/utils/ui_string_table_builder.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "ui_proj_load_utils.h"
#include "ui_proj_utils.h"

struct ui_proj_load_module_ctx {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    error_monitor_t m_em;
    ui_string_table_builder_t m_string_table;
    char * m_use_img;
    ui_data_module_t m_module;
    ui_data_img_block_t m_cur_img_block;
};

static void ui_proj_load_module_startElement(
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
    struct ui_proj_load_module_ctx * ctx = (struct ui_proj_load_module_ctx *)(iprojutCtx);
    UI_IMG_BLOCK * block = ctx->m_cur_img_block ? ui_data_img_block_data(ctx->m_cur_img_block) : NULL;

    if (strcmp((const char *)localname, "Image") == 0) {
        uint32_t id;

        ctx->m_cur_img_block = ui_data_img_block_create(ctx->m_module);
        if (ctx->m_cur_img_block  == NULL) {
            CPE_ERROR(ctx->m_em, "create img block fail!");
            return;
        }

        UI_R_XML_READ_ATTR_UINT32(id, "ID");
        if (ui_data_img_block_set_id(ctx->m_cur_img_block, id) != 0 ) {
            CPE_ERROR(ctx->m_em, "create img block: id duplicate!");
            return;
        }

        block = ui_data_img_block_data(ctx->m_cur_img_block);

        UI_R_XML_READ_ATTR_STRING_ID(block->name_id, "Name");

        UI_R_XML_READ_ATTR_UINT32(block->src_x, "SrcX");
        UI_R_XML_READ_ATTR_UINT32(block->src_y, "SrcY");

        UI_R_XML_READ_ATTR_UINT32(block->src_w, "SrcW");
        UI_R_XML_READ_ATTR_UINT32(block->src_h, "SrcH");

        UI_R_XML_READ_ATTR_UINT16(block->flag, "Flag");

        block->use_img = ui_string_table_builder_msg_alloc(ctx->m_string_table, ctx->m_use_img);
    }
    else if (strcmp((const char *)localname, "Rect") == 0) {
        if (block) {
            UI_R_XML_READ_ATTR_RECT(&block->rect);
        }
    }
    else if (strcmp((const char *)localname, "SCTex") == 0) {
        int write_pos;
        char buff[256];
        char buff2[128];

        UI_R_XML_READ_ATTR_STRING(buff, "Path");
        write_pos = strlen(buff);

        if (write_pos > 0 && buff[write_pos - 1] != '/') {
            buff[write_pos++] = '/';
        }

        UI_R_XML_READ_ATTR_STRING(buff2, "File");
        cpe_str_dup(buff + write_pos, sizeof(buff) - write_pos, buff2);
        
        ctx->m_use_img = cpe_str_mem_dup(NULL,buff);
    }
}

static void ui_proj_load_module_endElement(
        void* iprojutCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct ui_proj_load_module_ctx * ctx = (struct ui_proj_load_module_ctx *)(iprojutCtx);

    if (strcmp((const char *)localname, "Image") == 0) {
        ctx->m_cur_img_block = NULL;
    }
}

static void ui_proj_load_module_characters(void * iprojutCtx, const xmlChar *ch, int len) {
}

static void ui_proj_load_module_structed_error(void * iprojutCtx, xmlErrorPtr err) {
    struct ui_proj_load_module_ctx * ctx = (struct ui_proj_load_module_ctx *)(iprojutCtx);

    if (err->code == XML_ERR_DOCUMENT_END) {
        ((xmlParserCtxtPtr)err->ctxt)->wellFormed = 1;
        xmlCtxtResetLastError(err->ctxt);
    }
    else {
        CPE_ERROR_SET_LEVEL(
            ctx->m_em,
            err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

        CPE_ERROR_SET_LINE(ctx->m_em, err->line);

        cpe_error_do_notify(ctx->m_em, "(%d) %s", err->code, err->message);
    }
}

static xmlSAXHandler g_ui_proj_load_module_callbacks = {
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
    , ui_proj_load_module_characters /* charactersSAXFunc characters */
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
    , ui_proj_load_module_startElement /* startElementNsSAX2Func startElementNs */
    , ui_proj_load_module_endElement /* endElementNsSAX2Func endElementNs */
    , ui_proj_load_module_structed_error /* xmlStructuredErrorFunc serror */
};

void ui_data_proj_load_module_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    struct ui_proj_load_module_ctx ctx;
    struct mem_buffer path_buff;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    const char * path;

    ctx.m_mgr = mgr;
    ctx.m_src = src;
    ctx.m_em = em;
    ctx.m_use_img = NULL;
    ctx.m_module = NULL;
    ctx.m_cur_img_block = NULL;

    ctx.m_module = ui_data_module_create(mgr, src);
    if (ctx.m_module == NULL) {
        CPE_ERROR(em, "create module fail");
        return;
    }

    ctx.m_string_table = ui_data_src_strings_build_begin(src);
    if (ctx.m_string_table == NULL) {
        CPE_ERROR(em, "proj load module: create string table builder fail");
        ui_data_module_free(ctx.m_module);
        return;
    }
    
    mem_buffer_init(&path_buff, NULL);

    stream_printf((write_stream_t)&stream, "%s/", ui_data_src_data(ui_data_mgr_src_root(mgr)));
    ui_data_src_path_print((write_stream_t)&stream, src);
    stream_printf((write_stream_t)&stream, ".%s", ui_data_proj_postfix(ui_data_src_type(src)));
    stream_putc((write_stream_t)&stream, 0);
    path = mem_buffer_make_continuous(&path_buff, 0);

    xmlSAXUserParseFile(&g_ui_proj_load_module_callbacks, &ctx, path);

    if (ctx.m_use_img) {
        mem_free(NULL, ctx.m_use_img);
    }

    mem_buffer_clear(&path_buff);
}

int ui_data_proj_load_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_proj_load_module_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_proj_load_module_i(ctx, mgr, src, &logError);
    }

    return ret;
}
