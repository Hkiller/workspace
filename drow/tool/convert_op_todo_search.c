#include <assert.h>
#include "render/model/ui_data_src.h"
#include "render/model/ui_object_ref.h"
#include "convert_ctx.h"

int convert_op_todo_search(convert_ctx_t ctx) {
    struct cfg_it todo_it;
    cfg_t todo_cfg;
    int rv = 0;
    
    cfg_it_init(&todo_it, cfg_find_cfg(ctx->m_todo, "resources"));
    while((todo_cfg = cfg_it_next(&todo_it))) {
        UI_OBJECT_URL url_buf;
        UI_OBJECT_URL * url;
        const char * todo_str;
        ui_data_src_t src;

        todo_str = cfg_as_string(todo_cfg, NULL);
        if (todo_str == NULL) {
            CPE_ERROR(ctx->m_em, "todo config format error!");
            rv = -1;
            continue;
        }

        url = ui_object_ref_parse(todo_str, &url_buf, ctx->m_em);
        if (url == NULL) {
            CPE_ERROR(ctx->m_em, "todo %s url format error!", todo_str);
            rv = -1;
            continue;
        }

        src = ui_data_src_find_by_url(ctx->m_data_mgr, url);
        if (src == NULL) {
            CPE_ERROR(ctx->m_em, "todo %s src not exist!", todo_str);
            rv = -1;
            continue;
        }

        //TODO: Loki
        /* if (ui_data_src_in_group(src, ctx->m_all_srcs)) { */
        /*     CPE_INFO(ctx->m_em, "todo %s is already used!", todo_str); */
        /*     continue; */
        /* } */

        if (ui_data_src_group_add_src(ctx->m_ignore_srcs, src) != 0) {
            CPE_ERROR(ctx->m_em, "todo %s add fail!", todo_str);
            rv = -1;
            continue;
        }
    }

    return rv;
}
