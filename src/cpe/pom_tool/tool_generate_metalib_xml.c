#include <assert.h>
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "tool_env.h"

int pom_tool_generate_metalib_xml(struct pom_tool_env * env, const char * filename) {
    if (env->m_input_metalib == NULL) {
        CPE_ERROR(env->m_em, "generate metalib xml: no metalib!");
        return -1;
    }

    return dr_save_lib_to_xml_file(env->m_input_metalib, filename, env->m_em);
}


