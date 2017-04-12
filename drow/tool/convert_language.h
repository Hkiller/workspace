#ifndef UI_MODEL_TOOL_CONVERT_LANGUAGE_H
#define UI_MODEL_TOOL_CONVERT_LANGUAGE_H
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_language.h"
#include "convert_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

struct convert_language {
    convert_ctx_t m_ctx;
    TAILQ_ENTRY(convert_language) m_next;
    ui_string_table_t m_string_table;
    ui_data_language_t m_data_language;
    uint8_t m_is_active;
};

convert_language_t convert_language_create(convert_ctx_t ctx, ui_data_language_t data_language);
void convert_language_free(convert_language_t language);
const char * convert_language_name(convert_language_t language);

convert_language_t convert_language_find(convert_ctx_t ctx, ui_data_language_t data_language);
convert_language_t convert_language_find_by_name(convert_ctx_t ctx, const char * str_language);

#ifdef __cplusplus
}
#endif

#endif
