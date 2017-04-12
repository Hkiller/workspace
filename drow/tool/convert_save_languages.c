#include <assert.h>
#include <errno.h>
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_dir.h"
#include "convert_ctx.h"
#include "convert_language.h"

int convert_save_languages(convert_ctx_t ctx) {
    convert_language_t language;
    vfs_mgr_t vfs = gd_app_vfs_mgr(ctx->m_app);
    int rv = 0;
    char to_path[128];

    snprintf(to_path, sizeof(to_path), "%s/language", ctx->m_output);
    if (vfs_dir_mk_recursion(vfs, to_path) != 0) {
        return -1;
    }
        
    TAILQ_FOREACH(language, &ctx->m_languages, m_next) {
        if (!language->m_is_active) continue;
        
        snprintf(to_path, sizeof(to_path), "%s/language/strings_%s.stb", ctx->m_output, convert_language_name(language));
        if (ui_string_table_write_file(language->m_string_table, vfs, to_path) != 0) rv = -1;
    }

    return rv;
}
