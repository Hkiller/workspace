#include <assert.h>
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "tool_env.h"

static int pom_tool_do_generate_lib_c(write_stream_t stream, struct pom_tool_env * env, const char * arg_name) {
    int rv;
    unsigned char * buf;
    unsigned char * write_pos;
    size_t size;
    int first_line;

    assert(stream);
    assert(env);
    assert(env->m_pom_grp_meta);

    rv = 0;

    size = pom_grp_meta_calc_bin_size(env->m_pom_grp_meta);
    buf = mem_alloc(NULL, size);
    if (buf == NULL) {
        CPE_ERROR(env->m_em, "generate lib c: alloc buf fail, size=%d!", (int)size);
        return -1;
    }
    pom_grp_meta_write_to_bin(buf, size, env->m_pom_grp_meta);

    first_line = 1;

    stream_printf(stream, "#include \"cpe/pal/pal_external.h\"\n EXPORT_DIRECTIVE\nchar %s[] = {", arg_name);

    write_pos = buf;
    while(size > 0) {
        size_t i;
        size_t line_size = size > 16 ? 16 : size;

        stream_printf(stream, "\n    ");

        if (first_line) {
            stream_printf(stream, "  ");
            first_line = 0;
        }
        else {
            stream_printf(stream, ", ");
        }

        for(i = 0; i < line_size; ++i, ++write_pos) {
            if (i > 0) { stream_printf(stream, ", "); }

            stream_printf(stream, "%s", "0x");
            stream_printf(stream, "%.2X", *write_pos);
        }

        size -= line_size;
    }

    stream_printf(stream, "\n};\n");
    mem_free(NULL, buf);
    return rv;
}

int pom_tool_generate_lib_c(struct pom_tool_env * env, const char * filename, const char * arg_name) {
    struct write_stream_file stream;
    FILE * fp;
    int rv;

    if (env->m_pom_grp_meta == NULL) {
        CPE_ERROR(env->m_em, "generate lib c: no pom-meta!");
        return -1;
    }

    fp = file_stream_open(filename, "w", env->m_em);
    if (fp == NULL) {
        CPE_ERROR(env->m_em, "open %s fro generate lib c fail!", filename);
        return -1;
    }

    write_stream_file_init(&stream, fp, env->m_em);

    rv = pom_tool_do_generate_lib_c((write_stream_t)&stream, env, arg_name);

    file_stream_close(fp, env->m_em);

    return rv;
}


