#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "convert_ctx.h"
#include "convert_language.h"

int convert_build_language(convert_ctx_t ctx) {
    convert_language_t language;
    int rv = 0;
    cfg_t languages_seq;

    languages_seq = cfg_struct_add_seq(ctx->m_runing, "languages", cfg_replace);
    if (languages_seq == NULL) {
        CPE_ERROR(ctx->m_em, "convert_op_runing_gen_language_list: create languages seq fail!");
        return -1;
    }
    
    TAILQ_FOREACH(language, &ctx->m_languages, m_next) {
        if (cfg_seq_add_string(languages_seq, convert_language_name(language)) == NULL) {
            CPE_ERROR(ctx->m_em, "convert_op_runing_gen_language_list: add language %s fail!", convert_language_name(language));
            rv = -1;
        }
    }
    
    return rv;
}
