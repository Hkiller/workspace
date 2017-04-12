#include <assert.h>
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "tool_env.h"

int pom_tool_generate_store_metalib_xml(struct pom_tool_env * env, const char * filename) {
    struct mem_buffer buffer;
    int rv;

    if (env->m_pom_grp_meta == NULL) {
        CPE_ERROR(env->m_em, "generate store metalib xml: no pom grp meta!");
        return -1;
    }

    rv = 0;

    mem_buffer_init(&buffer, NULL);

    if (pom_grp_meta_build_store_meta(&buffer, env->m_pom_grp_meta, env->m_em) != 0) {
        CPE_ERROR(env->m_em, "generate store metalib xml: build store metalib fail!");
        rv = -1;
        goto COMPLETE;
    }

    if (dr_save_lib_to_xml_file((LPDRMETALIB)mem_buffer_make_continuous(&buffer, 0), filename, env->m_em) != 0) {
        CPE_ERROR(env->m_em, "generate store metalib xml: write metalib fail!");
        rv = -1;
        goto COMPLETE;
    }

COMPLETE:
    mem_buffer_clear(&buffer);
    return rv;
}
