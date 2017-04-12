#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "convert_language.h"

convert_language_t convert_language_create(convert_ctx_t ctx, ui_data_language_t data_language) {
    convert_language_t language;
    char file_buf[256];
    
    language = mem_alloc(gd_app_alloc(ctx->m_app), sizeof(struct convert_language));
    if (language == NULL) {
        CPE_ERROR(ctx->m_em, "convert_language_create: alloc fail!");
        return NULL;
    }

    language->m_ctx = ctx;
    language->m_data_language = data_language;
    language->m_is_active = 0;
    language->m_string_table = ui_string_table_create(gd_app_alloc(ctx->m_app), ctx->m_em);
    if (language->m_string_table == NULL) {
        CPE_ERROR(ctx->m_em, "convert_language_create: string table create fail!");
        mem_free(gd_app_alloc(ctx->m_app), language);
        return NULL;
    }

    snprintf(
        file_buf, sizeof(file_buf), "%s/../language/strings_%s.stb",
        ui_data_src_data(ui_data_mgr_src_root(ctx->m_data_mgr)),
        ui_data_language_name(data_language));
    if (ui_string_table_load_file(language->m_string_table, gd_app_vfs_mgr(ctx->m_app), file_buf) != 0) {
        CPE_ERROR(
            ctx->m_em, "convert_language_create: language %s load strings load from %s fail!",
            ui_data_language_name(data_language), file_buf);
        ui_string_table_free(language->m_string_table);
        mem_free(gd_app_alloc(ctx->m_app), language);
        return NULL;
    }
    
    ctx->m_language_count++;
    TAILQ_INSERT_TAIL(&ctx->m_languages, language, m_next);
    return language;
}

void convert_language_free(convert_language_t language) {
    convert_ctx_t ctx = language->m_ctx;
    
    ui_string_table_free(language->m_string_table);
    language->m_string_table = NULL;

    assert(ctx->m_language_count > 0);
    ctx->m_language_count--;

    TAILQ_REMOVE(&ctx->m_languages, language, m_next);
    
    mem_free(gd_app_alloc(ctx->m_app), language);
}

const char * convert_language_name(convert_language_t language) {
    return ui_data_language_name(language->m_data_language);
}

convert_language_t
convert_language_find(convert_ctx_t ctx, ui_data_language_t data_language) {
    convert_language_t language;

    TAILQ_FOREACH(language, &ctx->m_languages, m_next) {
        if (language->m_data_language == data_language) return language;
    }

    return NULL;
}

convert_language_t convert_language_find_by_name(convert_ctx_t ctx, const char * str_language) {
    convert_language_t language;

    TAILQ_FOREACH(language, &ctx->m_languages, m_next) {
        if (strcmp(ui_data_language_name(language->m_data_language), str_language) == 0) return language;
    }

    return NULL;
}
