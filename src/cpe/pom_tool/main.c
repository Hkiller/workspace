#include <assert.h>
#include "argtable2.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_meta_build.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "tool_env.h"

dir_visit_next_op_t
accept_input_meta_file(const char * full, const char * base, void * ctx) {
    if (strcmp(file_name_suffix(base), "xml") == 0) {
        dr_metalib_builder_add_file((dr_metalib_builder_t)ctx, NULL, full);
    }
    return dir_visit_next_go;
}

struct dir_visitor g_input_meta_search_visitor = {
    NULL, NULL, accept_input_meta_file
};

void prepare_input_meta_group(
    dr_metalib_builder_t builder,
    struct arg_file * meta_group_root,
    struct arg_file * meta_group,
    error_monitor_t em)
{
    char path_buf[256];
    size_t path_len = 0;
    FILE * group_file;

    if (meta_group->count <= 0) return;

    path_len = strlen(meta_group_root->filename[0]);
    if (path_len + 5 > sizeof(path_buf)) {
        CPE_ERROR(em, "group input %s is too long!", meta_group->filename[0]);
    }

    snprintf(path_buf, sizeof(path_buf), "%s", meta_group_root->filename[0]);
    if (path_buf[path_len - 1] != '/') {
        ++path_len;
        path_buf[path_len - 1] = '/';
        path_buf[path_len] = 0;
    }

    group_file = file_stream_open(meta_group->filename[0], "r", em);
    if (group_file == NULL) {
        CPE_ERROR(em, "group input %s not exist!", meta_group->filename[0]);
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
        }
    }
}

void prepare_input_meta_file(dr_metalib_builder_t builder, struct arg_file * meta_file, error_monitor_t em) {
    int i;
    for(i = 0; i < meta_file->count; ++i) {
        const char * filename;
        size_t filename_len;

        filename = meta_file->filename[i];
        filename_len = strlen(filename);
        if (filename[filename_len - 1] == '\\' || filename[filename_len - 1] == '/') {
            ((char *)filename)[filename_len - 1] = 0;
        }

        if (dir_exist(filename, em)) {
            dir_search(&g_input_meta_search_visitor, builder, filename, 5, em, NULL);
        }
        else if (file_exist(filename, em)) {
            if (dr_metalib_builder_add_file(builder, NULL, filename) == NULL) {
                CPE_ERROR(em, "add input meta file %s fail!", filename);
            }
        }
        else {
            CPE_ERROR(em, "input meta file %s not exist!", filename);
        }
    }
}

int validate_meta_entry_id(struct pom_tool_env * env) {
    size_t meta_pos, meta_count;
    int error_count = 0;

    meta_count = dr_lib_meta_num(env->m_input_metalib);
    for(meta_pos = 0; meta_pos < meta_count; ++meta_pos) {
        size_t entry_pos, entry_count;
        LPDRMETA meta = dr_lib_meta_at(env->m_input_metalib, meta_pos);

        entry_count = dr_meta_entry_num(meta);
        for(entry_pos = 0; entry_pos < entry_count; ++entry_pos) {
            LPDRMETAENTRY entry = dr_meta_entry_at(meta, entry_pos);
            if (dr_entry_id(entry) == -1) {
                CPE_ERROR(env->m_em, "%s.%s id not set!", dr_meta_name(meta), dr_entry_name(entry));
                ++error_count;
            }
        }
    }

    return error_count > 0 ? -1 : 0;
}

static int env_init_meta(
    struct pom_tool_env * env, 
    struct arg_file * pom_meta_file,
    struct arg_str * pom_dr_name,
    struct arg_int * page_size,
    struct arg_file * dr_meta_file,
    struct arg_file * dr_meta_group_root,
    struct arg_file * dr_meta_group,
    struct arg_int  * dr_meta_align,
    struct arg_lit  * dr_meta_validate_entry_id)
{
    dr_metalib_builder_t builder;
    int build_rv;

    if (dr_meta_file->count == 0 && dr_meta_group->count == 0) {
        printf("no metalib file or group input!");
        return -1;
    }

    if (pom_meta_file->count == 0 && pom_dr_name->count == 0) {
        printf("no pom meta file input or dr meta name!");
        return -1;
    }

    if (pom_meta_file->count >= 1 && pom_dr_name->count >= 1) {
        printf("both pom meta file input and dr meta name!");
        return -1;
    }

    builder = dr_metalib_builder_create(NULL, env->m_em);
    if (builder == NULL) {
        CPE_ERROR(env->m_em, "create metalib builder fail!\n");
        return -1;
    }

    if (dr_meta_align->count) {
        int align = dr_meta_align->ival[0];
        if (align != 1 && align != 2 && align != 4 && align != 8) {
            CPE_ERROR(env->m_em, "default align %d is invalid!", align);
            return -1;
        }

        dr_inbuild_set_dft_align(dr_metalib_bilder_lib(builder), (uint8_t)align);
    }

    prepare_input_meta_file(builder, dr_meta_file, env->m_em);
    prepare_input_meta_group(builder, dr_meta_group_root, dr_meta_group, env->m_em);

    dr_metalib_builder_analize(builder);
    build_rv = dr_inbuild_build_lib(
        &env->m_input_meta_buffer,
        dr_metalib_bilder_lib(builder),
        env->m_em);
    dr_metalib_builder_free(builder);

    if (build_rv == 0) {
        env->m_input_metalib = (LPDRMETALIB)mem_buffer_make_continuous(&env->m_input_meta_buffer, 0);
    }
    else {
        CPE_ERROR(env->m_em, "build meta lib fail!\n");
        return -1;
    }

    env->m_pom_cfg = NULL;

    if (pom_meta_file->count > 0) {
        env->m_pom_cfg = cfg_create(NULL);
        if (env->m_pom_cfg == NULL) {
            CPE_ERROR(env->m_em, "create cfg fail!");
            return -1;
        }

        if (cfg_read_file(env->m_pom_cfg, pom_meta_file->filename[0], cfg_replace, env->m_em) != 0) {
            CPE_ERROR(env->m_em, "read pom meta from %s fail!", pom_meta_file->filename[0]);
            return -1;
        }

        env->m_pom_grp_meta = pom_grp_meta_build_from_cfg(NULL, (page_size && page_size->count) ? page_size->ival[0] : USHRT_MAX, cfg_child_only(env->m_pom_cfg), env->m_input_metalib, env->m_em);
        if (env->m_pom_grp_meta == NULL) {
            CPE_ERROR(env->m_em, "create pom meta from %s fail!", pom_meta_file->filename[0]);
            return -1;
        }
    }
    else {
        LPDRMETA dr_meta = dr_lib_find_meta_by_name(env->m_input_metalib, pom_dr_name->sval[0]);
        if (dr_meta == NULL) {
            CPE_ERROR(env->m_em, "meta %s not exist in metalib!", pom_dr_name->sval[0]);
            return -1;
        }

        env->m_pom_grp_meta = pom_grp_meta_build_from_meta(NULL, (page_size && page_size->count) ? page_size->ival[0] : USHRT_MAX, dr_meta, env->m_em);
        if (env->m_pom_grp_meta == NULL) {
            CPE_ERROR(env->m_em, "create pom meta from %s fail!", pom_meta_file->filename[0]);
            return -1;
        }
    }

    if (dr_meta_validate_entry_id->count && validate_meta_entry_id(env) != 0) return -1;

    return 0;
}

int main(int argc, char * argv[]) {
    /*mk meta bin*/
    struct arg_rex  * mk_clib =     arg_rex1(NULL, NULL, "mk-clib", NULL, 0, NULL);
    struct arg_file  * mk_clib_from_pom_meta =     arg_file0(NULL, "from-pom-meta", NULL, "input pom meta file");
    struct arg_str  * mk_clib_from_dr_name =     arg_str0(NULL, "from-dr-name", NULL, "input dr meta name");
    struct arg_file  * mk_clib_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * mk_clib_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * mk_clib_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_int  * mk_clib_align =     arg_int0(NULL, "align", NULL,  "meta align");
    struct arg_lit  * mk_clib_validate_entry_id =     arg_lit0(NULL, "validate-entry-id", "validate entry id");
    struct arg_int  * mk_clib_page_size =     arg_int1(NULL, "page-size", NULL, "object page size");
    struct arg_file  * mk_clib_o_file =     arg_file1(NULL, "output-lib-c", NULL, "output lib c file");
    struct arg_str  * mk_clib_o_argname =     arg_str1(NULL, "output-lib-c-arg", NULL, "output c value arg name");
    struct arg_end  * mk_clib_end = arg_end(20);
    void* mk_clib_argtable[] = { 
        mk_clib, mk_clib_from_pom_meta, mk_clib_from_dr_name, mk_clib_page_size,
        mk_clib_dr_file, mk_clib_dr_group_root, mk_clib_dr_group, mk_clib_align, mk_clib_validate_entry_id,
        mk_clib_o_file, mk_clib_o_argname,
        mk_clib_end
    };
    int mk_clib_nerrors;

    /*mk metalib xml*/
    struct arg_rex  * metalib_xml =     arg_rex1(NULL, NULL, "^metalib-xml$", NULL, 0, NULL);
    struct arg_file  * metalib_xml_from_pom_meta =     arg_file0(NULL, "from-pom-meta", NULL, "input pom meta file");
    struct arg_str  * metalib_xml_from_dr_name =     arg_str0(NULL, "from-dr-name", NULL, "input dr meta name");
    struct arg_file  * metalib_xml_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * metalib_xml_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * metalib_xml_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_int  * metalib_xml_align =     arg_int0(NULL, "align", NULL,  "meta align");
    struct arg_lit  * metalib_xml_validate_entry_id =     arg_lit0(NULL, "validate-entry-id", "validate entry id");
    struct arg_file  * metalib_xml_o_file =     arg_file1(NULL, "output-metalib-xml", NULL, "output metalib xml file");
    struct arg_end  * metalib_xml_end = arg_end(20);
    void* metalib_xml_argtable[] = { 
        metalib_xml, metalib_xml_from_pom_meta, metalib_xml_from_dr_name,
        metalib_xml_dr_file, metalib_xml_dr_group_root, metalib_xml_dr_group, metalib_xml_align, metalib_xml_validate_entry_id,
        metalib_xml_o_file,
        metalib_xml_end
    };
    int metalib_xml_nerrors;

    /*mk store_metalib xml*/
    struct arg_rex  * store_metalib_xml =     arg_rex1(NULL, NULL, "^store-metalib-xml$", NULL, 0, NULL);
    struct arg_file  * store_metalib_xml_from_pom_meta =     arg_file1(NULL, "from-pom-meta<", NULL, "input pom meta file");
    struct arg_str  * store_metalib_xml_from_dr_name =     arg_str1(NULL, "from-dr-name<", NULL, "input dr meta name");
    struct arg_file  * store_metalib_xml_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * store_metalib_xml_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * store_metalib_xml_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_int  * store_metalib_xml_align =     arg_int0(NULL, "align", NULL,  "meta align");
    struct arg_lit  * store_metalib_xml_validate_entry_id =     arg_lit0(NULL, "validate-entry-id", "validate entry id");
    struct arg_file  * store_metalib_xml_o_file =     arg_file1(NULL, "output-metalib-xml", NULL, "output metalib xml file");
    struct arg_end  * store_metalib_xml_end = arg_end(20);
    void* store_metalib_xml_argtable[] = { 
        store_metalib_xml, store_metalib_xml_from_pom_meta, store_metalib_xml_from_dr_name,
        store_metalib_xml_dr_file, store_metalib_xml_dr_group_root, store_metalib_xml_dr_group,
        store_metalib_xml_align, store_metalib_xml_validate_entry_id,
        store_metalib_xml_o_file,
        store_metalib_xml_end
    };
    int store_metalib_xml_nerrors;

    /*mk hpp*/
    struct arg_rex  * mk_hpp =     arg_rex1(NULL, NULL, "mk-hpp", NULL, 0, NULL);
    struct arg_file  * mk_hpp_from_pom_meta =     arg_file0(NULL, "from-pom-meta", NULL, "input pom meta file");
    struct arg_str  * mk_hpp_from_dr_name =     arg_str0(NULL, "from-dr-name", NULL, "input dr meta name");
    struct arg_file  * mk_hpp_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * mk_hpp_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * mk_hpp_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_int  * mk_hpp_align =     arg_int0(NULL, "align", NULL,  "meta align");
    struct arg_lit  * mk_hpp_validate_entry_id =     arg_lit0(NULL, "validate-entry-id", "validate entry id");
    struct arg_file  * mk_hpp_o_file =     arg_file1(NULL, "output-hpp", NULL, "output hpp file");
    struct arg_str  * mk_hpp_o_classname =     arg_str1(NULL, "class-name", NULL, "output class name");
    struct arg_str  * mk_hpp_o_namespace =     arg_str0(NULL, "namespace", NULL, "output class namespace");
    struct arg_end  * mk_hpp_end = arg_end(20);
    void* mk_hpp_argtable[] = { 
        mk_hpp, mk_hpp_from_pom_meta, mk_hpp_from_dr_name, 
        mk_hpp_dr_file, mk_hpp_dr_group_root, mk_hpp_dr_group, mk_hpp_align, mk_hpp_validate_entry_id,
        mk_hpp_o_file, mk_hpp_o_classname, mk_hpp_o_namespace,
        mk_hpp_end
    };
    int mk_hpp_nerrors;

    /*init shm bin*/
    struct arg_rex  * shm_init =     arg_rex1(NULL, NULL, "shm-init", NULL, 0, NULL);
    struct arg_file  * shm_init_from_pom_meta =     arg_file0(NULL, "from-pom-meta", NULL, "input pom meta file");
    struct arg_str  * shm_init_from_dr_name =     arg_str0(NULL, "from-dr-name", NULL, "input dr meta name");
    struct arg_file  * shm_init_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * shm_init_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * shm_init_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_int  * shm_init_align =     arg_int0(NULL, "align", NULL,  "meta align");
    struct arg_lit  * shm_init_validate_entry_id =     arg_lit0(NULL, "validate-entry-id", "validate entry id");
    struct arg_int  * shm_init_page_size =     arg_int1(NULL, "page-size", NULL, "object page size");
    struct arg_int  * shm_init_shm_key =     arg_int0(NULL, "shm-key", NULL, "shm key");
    struct arg_end  * shm_init_end = arg_end(20);
    void* shm_init_argtable[] = { 
        shm_init, shm_init_from_pom_meta, shm_init_from_dr_name, shm_init_page_size,
        shm_init_dr_file, shm_init_dr_group_root, shm_init_dr_group, shm_init_align, shm_init_validate_entry_id,
        shm_init_shm_key,
        shm_init_end
    };
    int shm_init_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv;
    struct pom_tool_env env;

    bzero(&env, sizeof(env));

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    env.m_em = em;
    mem_buffer_init(&env.m_input_meta_buffer, 0);

    rv = -1;

    assert(arg_nullcheck(mk_clib_argtable) == 0);
    assert(arg_nullcheck(shm_init_argtable) == 0);

    mk_clib_nerrors = arg_parse(argc, argv, mk_clib_argtable);
    metalib_xml_nerrors = arg_parse(argc, argv, metalib_xml_argtable);
    store_metalib_xml_nerrors = arg_parse(argc, argv, store_metalib_xml_argtable);
    mk_hpp_nerrors = arg_parse(argc, argv, mk_hpp_argtable);
    shm_init_nerrors = arg_parse(argc, argv, shm_init_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    if (shm_init_nerrors == 0) {
        /* int shmkey = generate_shm_key(shm_init_key->count ? shm_init_key->sval[0] : NULL, em); */
        /* if (shmkey == -1) return -1; */

        rv = 0;
        /* tool_shm_init( */
            /* shmkey, */
            /* shm_init_size->count ? shm_init_size->ival[0] : 512 * 1024 * 1024, */
            /* shm_init_force->count, */
            /* em); */
    }
    else if (mk_clib_nerrors == 0) {
        if (env_init_meta(
                &env, 
                mk_clib_from_pom_meta,
                mk_clib_from_dr_name,
                mk_clib_page_size,
                mk_clib_dr_file,
                mk_clib_dr_group_root,
                mk_clib_dr_group,
                mk_clib_align,
                mk_clib_validate_entry_id) != 0)
        {
            rv = -1;
            goto EXIT;
        }

        rv = pom_tool_generate_lib_c(&env, mk_clib_o_file->filename[0], mk_clib_o_argname->sval[0]);
    }
    else if (metalib_xml_nerrors == 0) {
        if (env_init_meta(
                &env, 
                metalib_xml_from_pom_meta,
                metalib_xml_from_dr_name,
                NULL,
                metalib_xml_dr_file,
                metalib_xml_dr_group_root,
                metalib_xml_dr_group,
                metalib_xml_align,
                metalib_xml_validate_entry_id) != 0)
        {
            rv = -1;
            goto EXIT;
        }

        rv = pom_tool_generate_metalib_xml(&env, metalib_xml_o_file->filename[0]);
    }
    else if (store_metalib_xml_nerrors == 0) {
        if (env_init_meta(
                &env, 
                store_metalib_xml_from_pom_meta,
                store_metalib_xml_from_dr_name,
                NULL,
                store_metalib_xml_dr_file,
                store_metalib_xml_dr_group_root,
                store_metalib_xml_dr_group,
                store_metalib_xml_align,
                store_metalib_xml_validate_entry_id) != 0)
        {
            rv = -1;
            goto EXIT;
        }

        rv = pom_tool_generate_store_metalib_xml(
            &env,
            store_metalib_xml_o_file->filename[0]);
    }
    else if (mk_hpp_nerrors == 0) {
        if (env_init_meta(
                &env, 
                mk_hpp_from_pom_meta,
                mk_hpp_from_dr_name,
                NULL,
                mk_hpp_dr_file,
                mk_hpp_dr_group_root,
                mk_hpp_dr_group,
                mk_hpp_align,
                mk_hpp_validate_entry_id) != 0)
        {
            rv = -1;
            goto EXIT;
        }

        rv = pom_tool_generate_hpp(
            &env,
            mk_hpp_o_file->filename[0],
            mk_hpp_o_classname->sval[0],
            mk_hpp_o_namespace->count ? mk_hpp_o_namespace->sval[0] : "");
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        rv = -1;
        if (shm_init->count) {
            arg_print_errors(stdout, shm_init_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, shm_init_argtable, "\n");
        }
        else if (mk_clib->count) {
            arg_print_errors(stdout, mk_clib_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, mk_clib_argtable, "\n");
        }
        else if (metalib_xml->count) {
            arg_print_errors(stdout, metalib_xml_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, metalib_xml_argtable, "\n");
        }
        else if (store_metalib_xml->count) {
            arg_print_errors(stdout, store_metalib_xml_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, store_metalib_xml_argtable, "\n");
        }
        else if (mk_hpp->count) {
            arg_print_errors(stdout, mk_hpp_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, mk_hpp_argtable, "\n");
        }
        else {
            goto PRINT_HELP;
        }
    }

    goto EXIT;

PRINT_HELP:
    printf("%s: missing <mk-clib|mk-hpp|metalib-xml|shm-init> command.\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, mk_clib_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, metalib_xml_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, store_metalib_xml_argtable, "\n");
    printf("usage 4: %s ", argv[0]); arg_print_syntax(stdout, mk_hpp_argtable, "\n");
    printf("usage 5: %s ", argv[0]); arg_print_syntax(stdout, shm_init_argtable, "\n");
    printf("usage 6: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(mk_clib_argtable, sizeof(mk_clib_argtable) / sizeof(mk_clib_argtable[0]));
    arg_freetable(metalib_xml_argtable, sizeof(metalib_xml_argtable) / sizeof(metalib_xml_argtable[0]));
    arg_freetable(store_metalib_xml_argtable, sizeof(store_metalib_xml_argtable) / sizeof(store_metalib_xml_argtable[0]));
    arg_freetable(mk_hpp_argtable, sizeof(mk_hpp_argtable) / sizeof(mk_hpp_argtable[0]));
    arg_freetable(shm_init_argtable, sizeof(shm_init_argtable) / sizeof(shm_init_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));

    mem_buffer_clear(&env.m_input_meta_buffer);
    if (env.m_pom_cfg) cfg_free(env.m_pom_cfg);
    if (env.m_pom_grp_meta) pom_grp_meta_free(env.m_pom_grp_meta);

    return rv;
}
