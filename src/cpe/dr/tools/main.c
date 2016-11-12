#include "argtable2.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_validate.h"
#include "generate_ops.h"

struct arg_file * input;
struct arg_file * o_h;
struct arg_file * o_lib_c;
struct arg_str * o_lib_c_arg;
struct arg_file * o_lib_bin;
struct arg_file * i_group_root;
struct arg_file * i_group;
struct arg_lit * help;
struct arg_str * o_validate;
struct arg_int * i_align;
struct arg_end *end;
struct arg_lit * o_traits;
struct arg_file * o_traits_cpp;

dir_visit_next_op_t
accept_input_file(const char * full, const char * base, void * ctx) {
    if (strcmp(file_name_suffix(base), "xml") == 0) {
        dr_metalib_builder_add_file((dr_metalib_builder_t)ctx, NULL, full);
    }
    return dir_visit_next_go;
}

struct dir_visitor g_input_search_visitor = {
    NULL, NULL, accept_input_file
};

int prepare_input_group(dr_metalib_builder_t builder, error_monitor_t em) {
    char path_buf[256];
    size_t path_len = 0;
    int i;

    if (i_group->count <= 0) return 0;

    path_len = strlen(i_group_root->filename[0]);
    if (path_len + 5 > sizeof(path_buf)) {
        CPE_ERROR(em, "group input %s is too long!", i_group_root->filename[0]);
		return -1;
    }

    snprintf(path_buf, sizeof(path_buf), "%s", i_group_root->filename[0]);
    if (path_buf[path_len - 1] != '/') {
        ++path_len;
        path_buf[path_len - 1] = '/';
        path_buf[path_len] = 0;
    }

    for(i = 0; i < i_group->count; ++i) {
        FILE * group_file = file_stream_open(i_group->filename[i], "r", em);
        if (group_file == NULL) {
            CPE_ERROR(em, "group input %s not exist!", i_group->filename[i]);
            return -1;
        }

        while(fgets(path_buf + path_len, sizeof(path_buf) - path_len, group_file)) {
            size_t total_len;
            for(total_len = strlen(path_buf);
                total_len > 0
                    && (path_buf[total_len - 1] == '\n'
                        || path_buf[total_len - 1] == '\r');
                --total_len)
            {
                path_buf[total_len - 1] = 0;
            }

            if (file_exist(path_buf, em)) {
                dr_metalib_builder_add_file(builder, NULL, path_buf);
            }
            else {
                CPE_ERROR(em, "input %s not exist!", path_buf);
                return -1;
            }
        }
    }

	return 0;
}

void prepare_input(dr_metalib_builder_t builder, error_monitor_t em) {
    int i;
    for(i = 0; i < input->count; ++i) {
        const char * filename;
        size_t filename_len;

        filename = input->filename[i];
        filename_len = strlen(filename);
        if (filename[filename_len - 1] == '\\' || filename[filename_len - 1] == '/') {
            ((char *)filename)[filename_len - 1] = 0;
        }

        if (dir_exist(filename, em)) {
            dir_search(&g_input_search_visitor, builder, filename, 5, em, NULL);
        }
        else if (file_exist(input->filename[i], em)) {
            dr_metalib_builder_add_file(builder, NULL, filename);
        }
        else {
            CPE_ERROR(em, "input %s not exist!", filename);
        }
    }
}

static int do_generate_lib_bin(cpe_dr_generate_ctx_t ctx) {
    int i;
    int rv;

    rv = 0;

    for(i = 0; i < o_lib_bin->count; ++i) {
        FILE * fp;

        fp = file_stream_open(o_lib_bin->filename[i], "w", ctx->m_em);
        if (fp == NULL) {
            rv = -1;
            continue;
        }

        if (file_stream_write_from_buf(fp, ctx->m_metalib, dr_lib_size(ctx->m_metalib), ctx->m_em) <= 0) {
            rv = -1;
        }

        file_stream_close(fp, ctx->m_em);
    }

    return rv;
}

static int do_generate_lib_c(cpe_dr_generate_ctx_t ctx) {
    struct write_stream_file stream;
    int i;
    int rv;

    rv = 0;

    for(i = 0; i < o_lib_c->count; ++i) {
        FILE * fp;

        if (o_lib_c_arg->count == 0) {
            CPE_ERROR(ctx->m_em, "not arg name setted!");
            return -1;
        }

        fp = file_stream_open(o_lib_c->filename[i], "w", ctx->m_em);
        if (fp == NULL) {
            rv = -1;
            continue;
        }

        write_stream_file_init(&stream, fp, ctx->m_em);

        cpe_dr_generate_lib_c((write_stream_t)&stream, o_lib_c_arg->sval[0], ctx);

        file_stream_close(fp, ctx->m_em);
    }

    return rv;
}

static int do_generate_traits_cpp(cpe_dr_generate_ctx_t ctx) {
    struct write_stream_file stream;
    int i;
    int rv;

    rv = 0;

    for(i = 0; i < o_traits_cpp->count; ++i) {
        FILE * fp;

        if (o_lib_c_arg->count == 0) {
            CPE_ERROR(ctx->m_em, "not arg name setted!");
            return -1;
        }

        fp = file_stream_open(o_traits_cpp->filename[i], "w", ctx->m_em);
        if (fp == NULL) {
            rv = -1;
            continue;
        }

        write_stream_file_init(&stream, fp, ctx->m_em);

        cpe_dr_generate_traits_cpp((write_stream_t)&stream, o_lib_c_arg->sval[0], ctx);

        file_stream_close(fp, ctx->m_em);
    }

    return rv;
}

static int do_generate_h(cpe_dr_generate_ctx_t ctx) {
    struct dr_metalib_source_it source_it;
    dr_metalib_source_t source;
    struct mem_buffer buffer;
    struct write_stream_file stream;
    int i;
    FILE * fp;
    int rv;

    mem_buffer_init(&buffer, 0);

    rv = 0;
    for(i = 0; i < o_h->count; ++i) {
        dr_metalib_builder_sources(&source_it, ctx->m_builder);
        while((source = dr_metalib_source_next(&source_it))) {
            const char * file_name;

            if (dr_metalib_source_from(source) != dr_metalib_source_from_user) continue;

            file_name = dr_metalib_source_file(source);
            if (file_name == NULL) continue;

            mem_buffer_clear_data(&buffer);
            mem_buffer_strcat(&buffer, o_h->filename[i]);
            mem_buffer_strcat(&buffer, "/");
            file_name_append_base(&buffer, file_name);
            mem_buffer_strcat(&buffer, ".h");

            fp = file_stream_open((char *)mem_buffer_make_continuous(&buffer, 0), "w", ctx->m_em);
            if (fp == NULL) {
                rv = -1;
                continue;
            }

            write_stream_file_init(&stream, fp, ctx->m_em);
            cpe_dr_generate_h((write_stream_t)&stream, source, o_traits->count > 0 ? 1 : 0, ctx);
            file_stream_close(fp, ctx->m_em);
        }
    }

    mem_buffer_clear(&buffer);

    return rv;
}

int do_validate(cpe_dr_generate_ctx_t ctx) {
    int rv;
    int i;

    rv = 0;

    for(i = 0; i < o_validate->count; ++i) {
        const char * validate_name;

        validate_name = o_validate->sval[i];
        if (strcmp(validate_name, "align") == 0) {
            if (dr_metalib_validate_align(ctx->m_em, ctx->m_metalib) != 0) {
                rv = -1;
            }
        }
        else {
            CPE_ERROR(ctx->m_em, "validate %s is unknown!", validate_name);
            rv = -1;
        }
    }

    return rv;
}

int tools_main(error_monitor_t em) {
    struct cpe_dr_generate_ctx ctx;
    struct mem_buffer buffer;
    int rv;

    ctx.m_builder = NULL;
    ctx.m_metalib = NULL;
    ctx.m_em = em;

    rv = 0;

    ctx.m_builder = dr_metalib_builder_create(NULL, em);
    if (ctx.m_builder == NULL) {
        CPE_ERROR(em, "create metalib builder fail!");
        return -1;
    }

    if (i_align->count) {
        int align = i_align->ival[0];
        if (align != 1 && align != 2 && align != 4 && align != 8) {
            CPE_ERROR(em, "default align %d is invalid!", align);
            return -1;
        }

        dr_inbuild_set_dft_align(dr_metalib_bilder_lib(ctx.m_builder), (uint8_t)align);
    }

    prepare_input(ctx.m_builder, em);
    if (prepare_input_group(ctx.m_builder, em) != 0) return -1;

    if (dr_inbuild_tsort(dr_metalib_bilder_lib(ctx.m_builder), em) != 0) return -1;
    
    dr_metalib_builder_analize(ctx.m_builder);

    mem_buffer_init(&buffer, 0);
    if (dr_inbuild_build_lib(&buffer, dr_metalib_bilder_lib(ctx.m_builder), em) == 0) {
        ctx.m_metalib = (LPDRMETALIB)mem_buffer_make_continuous(&buffer, 0);

        if (ctx.m_metalib) {
            if (do_validate(&ctx) != 0) rv = -1;
            if (do_generate_h(&ctx) != 0) rv = -1;
            if (do_generate_lib_bin(&ctx) != 0) rv = -1;
            if (do_generate_lib_c(&ctx) != 0) rv = -1;
            if (do_generate_traits_cpp(&ctx) != 0) rv = -1;
        }
    }
    else {
        rv = -1;
    }

    mem_buffer_clear(&buffer);
    dr_metalib_builder_free(ctx.m_builder);

    return rv;
}

int main(int argc, char * argv[]) {
    void* argtable[] = {
                input = arg_filen(   "i",   "input",              "<string>", 0, 1000,    "input file")
        ,  o_validate = arg_strn(   "v",  "validate",     "<string>",         0, 10,   "validate operations")
        ,  i_align = arg_int0(   NULL,  "align",     "<int>",   "default align")
        ,         o_h = arg_file0(   NULL,  "output-h",           "<string>",            "output h file dir")
        ,     o_lib_c = arg_file0(   NULL,  "output-lib-c",       "<string>",            "output c lib file")
        , o_lib_c_arg = arg_str0(   NULL,  "output-lib-c-arg",    "<string>",            "output c lib file")
        ,   o_lib_bin = arg_file0(   NULL,  "output-lib-bin",     "<string>",            "output c lib file")
        , i_group_root = arg_file0(   NULL,  "input-group-root",     "<string>",            "root of input listed in group file")
        , i_group = arg_filen(   NULL,  "input-group",     "<string>",       0, 100,     "a file defined a list of input fild")
        ,        help = arg_lit0(   NULL,  "help",                                   "print this help and exit")
        ,    o_traits = arg_lit0(   NULL,  "with-traits",                         "generate traits in .h")
        , o_traits_cpp = arg_file0(   NULL,  "output-traits-cpp",       "<string>",            "output traits cpp file")
        ,         end = arg_end(20)
    };

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv;
    int nerrors;

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = -1;

    if (arg_nullcheck(argtable) != 0) {
        CPE_ERROR(em, "init arg table fail!");
        goto exit;
    }

    nerrors = arg_parse(argc,argv,argtable);

    if (help->count > 0) {
        printf("Usage: %s", argv[0]);
        arg_print_syntax(stdout,argtable,"\n");
        rv = 0;
        goto exit;
    }

    if (nerrors > 0) {
        arg_print_errors(stdout, end, argv[0]);
        printf("Try '%s --help' for more information.\n", argv[0]);
        goto exit;
    }

    rv = tools_main(em);

exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return rv;
}
