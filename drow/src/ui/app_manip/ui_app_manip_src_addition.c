#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "ui_app_manip_src_i.h"

struct ui_app_manip_collect_src_from_addition_ctx {
    ui_data_src_group_t m_src_group;
    ui_cache_group_t m_cache_group;
    cfg_t m_config;
    error_monitor_t m_em;
};

typedef int (*op_fun_t)(struct ui_app_manip_collect_src_from_addition_ctx * ctx, cfg_t cfg);

static cfg_t ui_app_manip_src_get_data_cfg(struct ui_app_manip_collect_src_from_addition_ctx * ctx);
static int ui_app_manip_collect_src_from_addition_scearch_process_entry(
    struct ui_app_manip_collect_src_from_addition_ctx * ctx, op_fun_t op, cfg_t cfg, char * root, char * p);

static int ui_app_manip_collect_src_from_addition_scearch_process_child(
    struct ui_app_manip_collect_src_from_addition_ctx * ctx, op_fun_t op, cfg_t cfg, char * root, char * child_name, char * p)
{
    size_t name_len = strlen(child_name);
                
    if (name_len == 0) {
        return ui_app_manip_collect_src_from_addition_scearch_process_entry(ctx, op, cfg, root, p);
    }
    else if (child_name[name_len - 1] == '*') {
        struct cfg_it child_it;
        cfg_t  child_cfg;
        int rv = 0;
            
        child_name[name_len - 1] = 0;
        
        cfg_it_init(&child_it, cfg);
        while((child_cfg = cfg_it_next(&child_it))) {
            const char * name = cfg_name(child_cfg);
            if (cpe_str_start_with(name, child_name)) {
                if (ui_app_manip_collect_src_from_addition_scearch_process_entry(ctx, op, child_cfg, root, p) != 0) {
                    rv = -1;
                }
            }                    
        }
        child_name[name_len - 1] = '*';

        return rv;
    }
    else {
        cfg_t child_cfg = cfg_find_cfg(cfg, child_name);
        if (child_cfg == NULL) return 0;
        return ui_app_manip_collect_src_from_addition_scearch_process_entry(ctx, op, child_cfg, root, p);
    }
}

static int ui_app_manip_collect_src_from_addition_scearch_process_entry(
    struct ui_app_manip_collect_src_from_addition_ctx * ctx, op_fun_t op, cfg_t cfg, char * root, char * p)
{
    if (cfg_type(cfg) == CPE_CFG_TYPE_SEQUENCE) {
        struct cfg_it child_it;
        cfg_t  child_cfg;
        int rv = 0;
            
        cfg_it_init(&child_it, cfg);
        while((child_cfg = cfg_it_next(&child_it))) {
            if (ui_app_manip_collect_src_from_addition_scearch_process_entry(ctx, op, child_cfg, root, p) != 0) {
                rv = -1;
            }
        }

        return rv;
    }
    else {
        if (p[0] == 0) {
            return op(ctx, cfg);
        }
        else {
            char * sep;

            sep = strchr(p, '.');
            if (sep) {
                int rv;

                *sep = 0;
                rv = ui_app_manip_collect_src_from_addition_scearch_process_child(ctx, op, cfg, root, p, sep + 1);
                *sep = '.';

                return rv;
            }
            else {
                return ui_app_manip_collect_src_from_addition_scearch_process_child(ctx, op, cfg, root, p, p + strlen(p));
            }
        }
    }
}

static int ui_app_manip_collect_src_from_addition_scearch(struct ui_app_manip_collect_src_from_addition_ctx * ctx, op_fun_t op) {
    cfg_t list_cfg;
    struct cfg_it child_it;
    cfg_t child_cfg;
    int rv = 0;
    char entry[256];

    cpe_str_dup(entry, sizeof(entry), cfg_get_string(ctx->m_config, "entry", ""));
    
    list_cfg = ui_app_manip_src_get_data_cfg(ctx);
    if (list_cfg == NULL) return -1;

    cfg_it_init(&child_it, list_cfg);
    while((child_cfg = cfg_it_next(&child_it))) {
        if (ui_app_manip_collect_src_from_addition_scearch_process_entry(ctx, op, child_cfg, entry, entry) != 0) {
            rv = -1;
        }
    }
    
    return rv;
}

static int ui_app_manip_src_search_data_cfg(struct ui_app_manip_collect_src_from_addition_ctx * ctx, op_fun_t op) {
    const char * base = cfg_get_string(ctx->m_config, "base", NULL);
    const char * path = cfg_get_string(ctx->m_config, "path", NULL);
    cfg_t root_cfg;
    char path_buf[256];
        
    if (path == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_src_get_data_cfg: path not configure");
        return -1;
    }

    if (base == NULL) {
        root_cfg = ctx->m_config;
        while(cfg_parent(root_cfg)) root_cfg = cfg_parent(root_cfg);
    }
    else if (strcmp(base, "etc") == 0) {
        root_cfg = gd_app_cfg(ui_data_mgr_app(ui_data_src_group_mgr(ctx->m_src_group)));
    }
    else {
        CPE_ERROR(ctx->m_em, "ui_app_manip_src_get_data_cfg: base %s unknown", base);
        return -1;
    }

    snprintf(path_buf, sizeof(path_buf), "%s", path);

    return ui_app_manip_collect_src_from_addition_scearch_process_entry(ctx, op, root_cfg, path_buf, path_buf);
}

static cfg_t ui_app_manip_src_get_data_cfg(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    const char * base = cfg_get_string(ctx->m_config, "base", NULL);
    const char * path = cfg_get_string(ctx->m_config, "path", NULL);
    cfg_t root_cfg;
    cfg_t target_cfg;
        
    if (path == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_src_get_data_cfg: path not configure");
        return NULL;
    }

    if (base == NULL) {
        root_cfg = ctx->m_config;
        while(cfg_parent(root_cfg)) root_cfg = cfg_parent(root_cfg);
    }
    else if (strcmp(base, "etc") == 0) {
        root_cfg = gd_app_cfg(ui_data_mgr_app(ui_data_src_group_mgr(ctx->m_src_group)));
    }
    else {
        CPE_ERROR(ctx->m_em, "ui_app_manip_src_get_data_cfg: base %s unknown", base);
        return NULL;
    }

    target_cfg = cfg_find_cfg(root_cfg, path);
    if (target_cfg == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_src_get_data_cfg: path %s not exist in base %s", path, base ? base : "default");
        return NULL;
    }

    return target_cfg;
}

int ui_app_manip_collect_src_from_addition_src(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    const char * sep;
    const char * path = cfg_get_string(ctx->m_config, "path", NULL);
    if (path == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_collect_src_from_addition: src: path not configure");
        return -1;
    }

    sep = strrchr(path, '/');
    if (sep == NULL) sep = path;

    if (strchr(sep, '.') == NULL) {
        return ui_data_src_group_add_src_by_path(ctx->m_src_group, path, ui_data_src_type_dir);
    }
    else {
        return ui_app_manip_collect_src_by_res(ctx->m_src_group, ctx->m_cache_group, path, ctx->m_em);
    }
}

/**/
int ui_app_manip_collect_src_from_addition_fsm(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    cfg_t fsm_cfg;
    
    fsm_cfg = ui_app_manip_src_get_data_cfg(ctx);
    if (fsm_cfg == NULL) return -1;

    return ui_app_manip_collect_src_from_fsm(ctx->m_src_group, ctx->m_cache_group, fsm_cfg, ctx->m_em);
}

/**/
int ui_app_manip_collect_src_from_addition_entity(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    cfg_t entity_cfg;
    
    entity_cfg = ui_app_manip_src_get_data_cfg(ctx);
    if (entity_cfg == NULL) return -1;

    return ui_app_manip_collect_src_from_entity(ctx->m_src_group, ctx->m_cache_group, entity_cfg, ctx->m_em);
}

/**/
static int do_collect_from_entity(struct ui_app_manip_collect_src_from_addition_ctx * ctx, cfg_t cfg) {
    return ui_app_manip_collect_src_from_entity(ctx->m_src_group, ctx->m_cache_group, cfg, ctx->m_em);
}

int ui_app_manip_collect_src_from_addition_entity_list(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    return ui_app_manip_src_search_data_cfg(ctx, do_collect_from_entity);    
}

/**/
static int do_collect_from_fsm(struct ui_app_manip_collect_src_from_addition_ctx * ctx, cfg_t cfg) {
    return ui_app_manip_collect_src_from_fsm(ctx->m_src_group, ctx->m_cache_group, cfg, ctx->m_em);
}

int ui_app_manip_collect_src_from_addition_fsm_list(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    return ui_app_manip_src_search_data_cfg(ctx, do_collect_from_fsm);
}

/**/
static int do_collect_from_entity_ref(struct ui_app_manip_collect_src_from_addition_ctx * ctx, cfg_t cfg) {
    cfg_t root;
    const char * path;
    cfg_t entity_cfg;
    char full_path[256];
    
    path = cfg_as_string(cfg, NULL);
    if (path == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_collect_src_from_entity_ref: path format error!");
        return -1;
    }

    root = cfg;
    while(cfg_parent(root)) root = cfg_parent(root);

    snprintf(
        full_path, sizeof(full_path), "%s%s%s",
        cfg_get_string(ctx->m_config, "prefix", ""), path, cfg_get_string(ctx->m_config, "postfix", ""));

    entity_cfg = cfg_find_cfg(root, full_path);
    if (entity_cfg == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_collect_src_from_entity_ref: entity cfg %s not exist!", full_path);
        return -1;
    }

    return ui_app_manip_collect_src_from_entity(ctx->m_src_group, ctx->m_cache_group, entity_cfg, ctx->m_em);
}

int ui_app_manip_collect_src_from_addition_entity_ref_list(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    return ui_app_manip_collect_src_from_addition_scearch(ctx, do_collect_from_entity_ref);
}

/**/
static int do_collect_from_src_ref(struct ui_app_manip_collect_src_from_addition_ctx * ctx, cfg_t cfg) {
    const char * path;
    char cvt_buf[128];
    char full_path[256];
    
    path = cfg_as_string_cvt(cfg, NULL, cvt_buf, sizeof(cvt_buf));
    if (path == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_collect_src_from_src_ref: path format error!");
        return -1;
    }

    snprintf(
        full_path, sizeof(full_path), "%s%s%s",
        cfg_get_string(ctx->m_config, "prefix", ""), path, cfg_get_string(ctx->m_config, "postfix", ""));

    return ui_app_manip_collect_src_by_res(ctx->m_src_group, ctx->m_cache_group, full_path, ctx->m_em);
}

int ui_app_manip_collect_src_from_addition_src_ref_list(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    return ui_app_manip_collect_src_from_addition_scearch(ctx, do_collect_from_src_ref);
}

/**/
static int do_collect_from_texture(struct ui_app_manip_collect_src_from_addition_ctx * ctx, cfg_t cfg) {
    const char * path;
    char full_path[256];
    
    path = cfg_as_string(cfg, NULL);
    if (path == NULL) {
        CPE_ERROR(ctx->m_em, "ui_app_manip_collect_src_from_texture: path format error!");
        return -1;
    }

    snprintf(
        full_path, sizeof(full_path), "%s%s%s",
        cfg_get_string(ctx->m_config, "prefix", ""), path, cfg_get_string(ctx->m_config, "postfix", ""));

    return ui_cache_group_add_res_by_path(ctx->m_cache_group, full_path);
}

int ui_app_manip_collect_src_from_addition_texture_list(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    return ui_app_manip_collect_src_from_addition_scearch(ctx, do_collect_from_texture);
}

int ui_app_manip_collect_src_from_addition_scene(struct ui_app_manip_collect_src_from_addition_ctx * ctx) {
    cfg_t scene_cfg;

    scene_cfg = ui_app_manip_src_get_data_cfg(ctx);
    if (scene_cfg == NULL) return -1;
    
    return ui_app_manip_collect_src_from_scene(ctx->m_src_group, ctx->m_cache_group, scene_cfg, ctx->m_em);
}

int ui_app_manip_collect_src_from_addition(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t addition_cfg, error_monitor_t em) {
    struct ui_app_manip_collect_src_from_addition_ctx ctx = { src_group, cache_group, addition_cfg, em };
    const char * addition_type = cfg_get_string(addition_cfg, "type", NULL);
    if (addition_type == NULL) {
        CPE_ERROR(em, "ui_app_manip_collect_src_from_addition: addition type not configure");
        return -1;
    }

    if (strcmp(addition_type, "scene") == 0) {
        return ui_app_manip_collect_src_from_addition_scene(&ctx);
    }
    else if (strcmp(addition_type, "entity") == 0) {
        return ui_app_manip_collect_src_from_addition_entity(&ctx);
    }
    else if (strcmp(addition_type, "fsm-list") == 0) {
        return ui_app_manip_collect_src_from_addition_fsm_list(&ctx);
    }
    else if (strcmp(addition_type, "entity-list") == 0) {
        return ui_app_manip_collect_src_from_addition_entity_list(&ctx);
    }
    else if (strcmp(addition_type, "entity-ref-list") == 0) {
        return ui_app_manip_collect_src_from_addition_entity_ref_list(&ctx);
    }
    else if (strcmp(addition_type, "fsm") == 0) {
        return ui_app_manip_collect_src_from_addition_fsm(&ctx);
    }
    else if (strcmp(addition_type, "src") == 0) {
        return ui_app_manip_collect_src_from_addition_src(&ctx);
    }
    else if (strcmp(addition_type, "src-ref-list") == 0) {
        return ui_app_manip_collect_src_from_addition_src_ref_list(&ctx);
    }
    else if (strcmp(addition_type, "texture-list") == 0) {
        return ui_app_manip_collect_src_from_addition_texture_list(&ctx);
    }
    else {
        CPE_ERROR(em, "ui_app_manip_collect_src_from_addition: addition type %s unknown", addition_type);
        return -1;
    }
}

int ui_app_manip_collect_src_from_additions(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t additions_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    int rv = 0;
    
    cfg_it_init(&childs, additions_cfg);

    while((child_cfg = cfg_it_next(&childs))) {
        if (ui_app_manip_collect_src_from_addition(src_group, cache_group, child_cfg, em) != 0) {
            rv = -1;
        }
    }

    return rv;
}

