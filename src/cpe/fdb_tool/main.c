#include <assert.h>
#include "argtable2.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/fdb/fdb_repo.h"

static int import_db(
    mem_allocrator_t alloc, error_monitor_t em,
    const char * meta_root, const char * meta_def,
    const char * schema_def, const char * data_def)
{
    vfs_mgr_t vfs = NULL;
    LPDRMETALIB metalib = NULL;
    fdb_repo_t repo = NULL;
    int rv = -1;
    
    vfs = vfs_mgr_create(alloc, em);
    if (vfs == NULL) {
        CPE_ERROR(em, "import_db: vfs_mgr_create fail!");
        goto COMPLETE;
    }

    if (meta_def) {
        metalib = dr_metalib_build_from_group(NULL, em, vfs, meta_root, meta_def);
    }
    else {
        metalib = dr_metalib_build_from_dir(NULL, em, vfs, meta_root);
    }
    
    if (metalib == NULL) {
        CPE_ERROR(em, "import_db: build metalib fail!");
        goto COMPLETE;
    }

    repo = fdb_repo_create(alloc, em);
    if (repo ==  NULL) {
        CPE_ERROR(em, "import_db: build metalib fail!");
        goto COMPLETE;
    }

    if (schema_def) {
        cfg_t schema  = cfg_create(alloc);
        if (schema == NULL) {
            CPE_ERROR(em, "import_db: create schema cfg fail!");
            goto COMPLETE;
        }

        if (cfg_yaml_read_file(schema, vfs, schema_def, cfg_merge_use_new, em) != 0) {
            CPE_ERROR(em, "import_db: read schema from %s fail!", schema_def);
            cfg_free(schema);
            goto COMPLETE;
        }

        if (fdb_repo_load_schema(repo, metalib, schema) != 0) {
            CPE_ERROR(em, "import_db: read db data from %s fail!", schema_def);
            cfg_free(schema);
            goto COMPLETE;
        }
    }

    if (data_def) {
        cfg_t data  = cfg_create(alloc);
        if (data == NULL) {
            CPE_ERROR(em, "import_db: create data cfg fail!");
            goto COMPLETE;
        }

        if (cfg_yaml_read_file_with_name(data, "root", vfs, data_def, cfg_merge_use_new, em) != 0) {
            CPE_ERROR(em, "import_db: read data from %s fail!", data_def);
            cfg_free(data);
            goto COMPLETE;
        }

        if (fdb_repo_load_schema_and_data(repo, metalib, cfg_child_only(data)) != 0) {
            CPE_ERROR(em, "import_db: read db data from %s fail!", data_def);
            cfg_free(data);
            goto COMPLETE;
        }
    }
    
    rv = 0;

COMPLETE:
    if (vfs) vfs_mgr_free(vfs);
    if (metalib) mem_free(alloc, metalib);
    if (repo) fdb_repo_free(repo);

    return rv;
}

int main(int argc, char * argv[]) {
    struct arg_rex  * import =            arg_rex1(NULL, NULL, "import", NULL, 0, NULL);
    struct arg_file * import_meta_root =  arg_file1(NULL, "meta-root", NULL, "root of input listed in group file");
    struct arg_file * import_meta_def =   arg_file0(NULL, "meta-def", NULL, "a file defined a list of input fild");
    struct arg_file * import_schema =     arg_file0(NULL,  "schema", NULL, "db define");
    struct arg_file * import_data =       arg_file0(NULL,  "data", NULL, "db define");
    struct arg_file * import_input =      arg_file1(NULL, "input", NULL, "db input dir");
    struct arg_file * import_output =     arg_file1(NULL, "output", NULL, "db output dir");
    struct arg_end  * import_end =        arg_end(20);
    void* import_argtable[] = { import, import_meta_root, import_meta_def, import_schema, import_data, import_input, import_output, import_end };
    int import_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv;

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = -1;

    import_nerrors = arg_parse(argc, argv, import_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    if (import_nerrors == 0) {
        rv = import_db(
            NULL, em,
            import_meta_root->filename[0], import_meta_def->count > 0 ? import_meta_def->filename[0] : NULL,
            import_schema->count > 0 ? import_schema->filename[0] : NULL,
            import_data->count > 0 ? import_data->filename[0] : NULL);
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        goto PRINT_HELP;
    }

    goto EXIT;

PRINT_HELP:
    printf("%s:\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, import_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(import_argtable, sizeof(import_argtable) / sizeof(import_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));

    return rv;
}
