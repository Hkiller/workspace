#include <assert.h>
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils_xml/xml_utils.h"
#include "cpe/vfs/vfs_file.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin/tiledmap/plugin_tiledmap_module.h"
#include "plugin_tiledmap_manip_i.h"

struct plugin_tiledmap_map_scene_build_ctx {
    plugin_tiledmap_manip_t m_module;
    error_monitor_t m_em;
    plugin_tiledmap_data_scene_t m_scene;
    plugin_tiledmap_data_layer_t m_layer;
    TILEDMAP_LAYER * m_layer_data;
    plugin_tiledmap_data_tile_t m_tile;
    TILEDMAP_TILE * m_tile_data;
    char m_cur_tag_name[64];
};

static void plugin_tiledmap_map_scene_build_startElement(
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
    struct plugin_tiledmap_map_scene_build_ctx * ctx = (struct plugin_tiledmap_map_scene_build_ctx *)(iprojutCtx);

    if (strcmp((const char *)localname, "SceneLayer") == 0) {
        int32_t cell_row;
        int32_t cell_col;

        ctx->m_layer = plugin_tiledmap_data_layer_create(ctx->m_scene);
        if (ctx->m_layer == NULL) {
            CPE_ERROR(ctx->m_em, "create layer fail!");
            return;
        }
        ctx->m_layer_data = plugin_tiledmap_data_layer_data(ctx->m_layer);

        if (cpe_xml_find_attr_string(
                ctx->m_layer_data->name, sizeof(ctx->m_layer_data->name),
                "Name", nb_attributes, attributes, ctx->m_em)
            == NULL)
        {
            CPE_ERROR(ctx->m_em, "load layer name!");
            return;
        }


        if (cpe_xml_find_attr_uint32(&ctx->m_layer_data->cell_w, "CellW", nb_attributes, attributes, ctx->m_em) != 0) {
            CPE_ERROR(ctx->m_em, "load CellW fail!");
            return;
        }

        if (cpe_xml_find_attr_uint32(&ctx->m_layer_data->cell_h, "CellH", nb_attributes, attributes, ctx->m_em) != 0) {
            CPE_ERROR(ctx->m_em, "load CellH fail!");
            return;
        }

        if (cpe_xml_find_attr_int32(&cell_row, "CellR", nb_attributes, attributes, ctx->m_em) != 0) {
            CPE_ERROR(ctx->m_em, "load CellR fail!");
            return;
        }

        if (cpe_xml_find_attr_int32(&cell_col, "CellC", nb_attributes, attributes, ctx->m_em) != 0) {
            CPE_ERROR(ctx->m_em, "load CellC fail!");
            return;
        }

        ctx->m_layer_data->cell_row_begin = cell_row < 0 ? cell_row : 0;
        ctx->m_layer_data->cell_row_end = cell_row > 0 ? cell_row : 0;
        ctx->m_layer_data->cell_col_begin = cell_col < 0 ? cell_col : 0;
        ctx->m_layer_data->cell_col_end = cell_col > 0 ? cell_col : 0;
    }
    else if (strcmp((const char *)localname, "R2DSFrameRef") == 0 || strcmp((const char *)localname, "R2DSImageRef") == 0) {
        if (ctx->m_layer) {
            ctx->m_tile = plugin_tiledmap_data_tile_create(ctx->m_layer);
            if (ctx->m_tile == NULL) {
                CPE_ERROR(ctx->m_em, "create tile fail!");
                return;
            }
            ctx->m_tile_data = plugin_tiledmap_data_tile_data(ctx->m_tile);

            if (cpe_xml_find_attr_string(ctx->m_tile_data->name, sizeof(ctx->m_tile_data->name), "Name", nb_attributes, attributes, ctx->m_em) == NULL) return;
            if (strcmp((const char *)localname, "R2DSFrameRef") == 0) {
                ctx->m_tile_data->ref_type =  tiledmap_tile_ref_type_frame;
                if (cpe_xml_find_attr_uint32(&ctx->m_tile_data->ref_data.frame.sprite_id, "Resfile", nb_attributes, attributes, ctx->m_em) != 0) return;
                if (cpe_xml_find_attr_uint32(&ctx->m_tile_data->ref_data.frame.frame_id, "Frameid", nb_attributes, attributes, ctx->m_em) != 0) return;
            }
            else {
                ctx->m_tile_data->ref_type =  tiledmap_tile_ref_type_img;
                if (cpe_xml_find_attr_uint32(&ctx->m_tile_data->ref_data.img.module_id, "Resfile", nb_attributes, attributes, ctx->m_em) != 0) return;
                if (cpe_xml_find_attr_uint32(&ctx->m_tile_data->ref_data.img.img_block_id, "Imageid", nb_attributes, attributes, ctx->m_em) != 0) return;
            }
        }
    }
    else if (strcmp((const char *)localname, "WorldTrans") == 0) {
        if (ctx->m_tile_data) {
            if (cpe_xml_find_attr_float(&ctx->m_tile_data->pos.x, "x", nb_attributes, attributes, ctx->m_em) != 0
                || cpe_xml_find_attr_float(&ctx->m_tile_data->pos.y, "y", nb_attributes, attributes, ctx->m_em) != 0
                )
            {
                return;
            }
        }
    }
    else if (strcmp((const char *)localname, "WorldScale") == 0) {
        if (ctx->m_tile_data) {
            if (cpe_xml_find_attr_float(&ctx->m_tile_data->scale.x, "x", nb_attributes, attributes, ctx->m_em) != 0
                || cpe_xml_find_attr_float(&ctx->m_tile_data->scale.y, "y", nb_attributes, attributes, ctx->m_em) != 0
                )
            {
                return;
            }
        }
    }
    else {
        cpe_str_dup(ctx->m_cur_tag_name, sizeof(ctx->m_cur_tag_name), (const char *)localname);
    }
}

static void plugin_tiledmap_map_scene_build_endElement(void* iprojutCtx, const xmlChar* localname, const xmlChar* prefix, const xmlChar* URI) {
    struct plugin_tiledmap_map_scene_build_ctx * ctx = (struct plugin_tiledmap_map_scene_build_ctx *)(iprojutCtx);

    if (strcmp((const char *)localname, "SceneLayer") == 0) {
        ctx->m_layer = NULL;
        ctx->m_layer_data = NULL;
        ctx->m_tile = NULL;
        ctx->m_tile_data = NULL;
    }
    else if (strcmp((const char *)localname, "R2DSFrameRef") == 0 || strcmp((const char *)localname, "R2DSImageRef") == 0) {
        ctx->m_tile = NULL;
        ctx->m_tile_data = NULL;
    }

    ctx->m_cur_tag_name[0] = 0;
}

static void plugin_tiledmap_map_scene_build_characters(void * iprojutCtx, const xmlChar *ch, int len) {
    struct plugin_tiledmap_map_scene_build_ctx * ctx = (struct plugin_tiledmap_map_scene_build_ctx *)(iprojutCtx);

    if (strcmp(ctx->m_cur_tag_name, "WorldFlips") == 0) {
        if (ctx->m_tile_data) {
            uint32_t v;
            if (cpe_xml_read_value_uint32(&v, (const char *)ch, len) != 0) {
                CPE_ERROR(ctx->m_em, "read WorldFlips fail!");
            }
            else {
                if(v == 0) {
                    ctx->m_tile_data->flip_type = tiledmap_tile_flip_type_none;
                }
                else if(v == 1) {
                    ctx->m_tile_data->flip_type = tiledmap_tile_flip_type_x;
                }
                else if(v == 2) {
                    ctx->m_tile_data->flip_type = tiledmap_tile_flip_type_y;
                }
                else if(v == 3) {
                    ctx->m_tile_data->flip_type = tiledmap_tile_flip_type_xy;
                }
                else {
                    CPE_ERROR(ctx->m_em, "WorldFlips %d is unknown!", v);
                }
            }
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "WorldAngle") == 0) {
        if (ctx->m_tile_data) {
            float angle;
            if (cpe_xml_read_value_float(&angle, (const char *)ch, len) != 0) return;

            if (angle == 0.0f) {
                ctx->m_tile_data->angle_type = tiledmap_tile_angle_type_none;
            }
            else if (angle == 90.0f) {
                ctx->m_tile_data->angle_type = tiledmap_tile_angle_type_90;
            }
            else if (angle == 180.0f) {
                ctx->m_tile_data->angle_type = tiledmap_tile_angle_type_180;
            }
            else if (angle == 270.0f) {
                ctx->m_tile_data->angle_type = tiledmap_tile_angle_type_270;
            }
            else {
                CPE_ERROR(ctx->m_em, "WorldAngle %f not support!", angle);
            }
        }
    }
}

static void plugin_tiledmap_map_scene_build_structed_error(void * iprojutCtx, xmlErrorPtr err) {
    struct plugin_tiledmap_map_scene_build_ctx * ctx = (struct plugin_tiledmap_map_scene_build_ctx *)(iprojutCtx);

    CPE_ERROR_SET_LEVEL(
        ctx->m_em,
        err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

    CPE_ERROR_SET_LINE(ctx->m_em, err->line);

    cpe_error_do_notify(ctx->m_em, "(%d) %s", err->code, err->message);
}

static xmlSAXHandler g_plugin_tiledmap_map_scene_build_callbacks = {
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
    , plugin_tiledmap_map_scene_build_characters /* charactersSAXFunc characters */
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
    , plugin_tiledmap_map_scene_build_startElement /* startElementNsSAX2Func startElementNs */
    , plugin_tiledmap_map_scene_build_endElement /* endElementNsSAX2Func endElementNs */
    , plugin_tiledmap_map_scene_build_structed_error /* xmlStructuredErrorFunc serror */
};

static void plugin_tiledmap_map_scene_data_build_i(void * ctx, ui_data_src_t src, vfs_file_t fp,  error_monitor_t em) {
    plugin_tiledmap_manip_t manip = ctx;
    struct plugin_tiledmap_map_scene_build_ctx build_ctx;
    struct mem_buffer data_buffer;

    mem_buffer_init(&data_buffer, manip->m_alloc);
    
    bzero(&build_ctx, sizeof(build_ctx));
    build_ctx.m_module = manip;
    build_ctx.m_em = em;
    build_ctx.m_scene = plugin_tiledmap_data_scene_create(manip->m_tiledmap_module, src);
    if (build_ctx.m_scene == NULL) {
        CPE_ERROR(em, "create scene fail!");
        goto COMPLETE;
    }

    if (vfs_file_load_to_buffer(&data_buffer, fp) < 0) {
        CPE_ERROR(em, "load scene data fail!");
        goto COMPLETE;
    }
    
    if (xmlSAXUserParseMemory(
            &g_plugin_tiledmap_map_scene_build_callbacks, &build_ctx,
            mem_buffer_make_continuous(&data_buffer, 0), mem_buffer_size(&data_buffer))
        < 0)
    {
        CPE_ERROR(em, "parse data fail!");
        goto COMPLETE;
    }

COMPLETE:
    mem_buffer_clear(&data_buffer);
}

static int plugin_tiledmap_data_scene_do_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        plugin_tiledmap_map_scene_data_build_i(ctx, src, fp, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        plugin_tiledmap_map_scene_data_build_i(ctx, src, fp, &logError);
    }

    return ret;
}

int plugin_tiledmap_scene_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "map", plugin_tiledmap_data_scene_do_load, ctx, em);
}

int plugin_tiledmap_scene_proj_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "map", em);
}
