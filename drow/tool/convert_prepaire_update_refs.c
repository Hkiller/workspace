#include <assert.h>
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_layout.h"
#include "plugin/spine/plugin_spine_data_skeleton.h"
#include "convert_ctx.h"

static int convert_prepaire_update_refs_one(ui_data_src_t src, convert_ctx_t ctx) {
    int rv = 0;
    
    switch(ui_data_src_type(src)) {
    case ui_data_src_type_dir: {
        struct ui_data_src_it child_it;
        ui_data_src_t child;
        ui_data_src_childs(&child_it, src);

        while((child = ui_data_src_it_next(&child_it))) {
            if (convert_prepaire_update_refs_one(child, ctx) != 0) rv = -1;
        }
        break;
    }
    default:
        ui_data_src_update_using(src);
        break;
    }

    return rv;
}

int convert_prepaire_update_refs(convert_ctx_t ctx) {
    return convert_prepaire_update_refs_one(ui_data_mgr_src_root(ctx->m_data_mgr), ctx);
}
