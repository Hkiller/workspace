#include <assert.h>
#include "argtable2.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/spack/spack_builder.h"

int main(int argc, char * argv[]) {
    /*inpack */
    struct arg_rex  * pack =            arg_rex1(NULL, NULL, "pack", NULL, 0, NULL);
    struct arg_file * pack_root =  arg_file1(NULL, "root", NULL, "input dirs");
    struct arg_file * pack_input =  arg_filen(NULL, "input", "i", 1, 100, "input dirs");
    struct arg_file * pack_output =  arg_file1(NULL, "output", "o", "output file name");
    struct arg_end  * pack_end =        arg_end(20);
    void* pack_argtable[] = { pack, pack_root, pack_input, pack_output, pack_end };
    int pack_nerrors;

    /*unpack*/
    struct arg_rex  * unpack =            arg_rex1(NULL, NULL, "unpack", NULL, 0, NULL);
    struct arg_file * unpack_input =  arg_file1(NULL, "input", "i", "input file");
    struct arg_file * unpack_output =  arg_file0(NULL, "output", "o", "output dir");
    struct arg_end  * unpack_end =        arg_end(20);
    void* unpack_argtable[] = { unpack, unpack_input, unpack_output, unpack_end };
    int unpack_nerrors;
    
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

    pack_nerrors = arg_parse(argc, argv, pack_argtable);
    unpack_nerrors = arg_parse(argc, argv, unpack_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    if (pack_nerrors == 0) {
        vfs_mgr_t vfs_mgr;
        spack_builder_t builder;
        int i;
        struct write_stream_file ws;
        FILE * fp;
        struct mem_buffer path_buffer;

        mem_buffer_init(&path_buffer, NULL);
        
        vfs_mgr = vfs_mgr_create(NULL, em);
        if (vfs_mgr == NULL) {
            CPE_ERROR(em, "create vfs mgr fail!");
            mem_buffer_clear(&path_buffer);
            goto EXIT;
        }

        builder = spack_builder_create(vfs_mgr, pack_root->filename[0], NULL, em);
        if (builder == NULL) {
            CPE_ERROR(em, "create builder fail!");
            mem_buffer_clear(&path_buffer);
            vfs_mgr_free(vfs_mgr);
            goto EXIT;
        }

        for (i = 0; i < pack_input->count; ++i) {
            const char * file = pack_input->filename[i];
            mem_buffer_clear_data(&path_buffer);
            mem_buffer_printf(&path_buffer, "%s/%s", pack_root->filename[0], file);
            file = mem_buffer_make_continuous(&path_buffer, 0);
            //printf("xxxxx: file = %s\n", file);
            if (spack_builder_add(builder, file) != 0) {
                CPE_ERROR(em, "add dir/file %s fail!", file);
                mem_buffer_clear(&path_buffer);
                vfs_mgr_free(vfs_mgr);
                spack_builder_free(builder);
                goto EXIT;
            }
        }
        
        fp = file_stream_open(pack_output->filename[0], "wb", em);
        if (fp == NULL) {
            CPE_ERROR(em, "open output file %s fail!", pack_output->filename[0]);
            mem_buffer_clear(&path_buffer);
            vfs_mgr_free(vfs_mgr);
            spack_builder_free(builder);
            goto EXIT;
        }

        write_stream_file_init(&ws, fp, em);

        if (spack_builder_build(builder, (write_stream_t)&ws) != 0) {
            CPE_ERROR(em, "builder fail!");
            mem_buffer_clear(&path_buffer);
            file_stream_close(fp, em);
            vfs_mgr_free(vfs_mgr);
            spack_builder_free(builder);
            goto EXIT;
        }

        mem_buffer_clear(&path_buffer);
        file_stream_close(fp, em);
        vfs_mgr_free(vfs_mgr);
        spack_builder_free(builder);
        rv = 0;
    }
    else if (unpack_nerrors == 0) {
        /* rv = import_db( */
        /*     NULL, em, */
        /*     import_meta_root->filename[0], import_meta_def->count > 0 ? import_meta_def->filename[0] : NULL, */
        /*     import_schema->count > 0 ? import_schema->filename[0] : NULL, */
        /*     import_data->count > 0 ? import_data->filename[0] : NULL); */
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
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, pack_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, unpack_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(pack_argtable, sizeof(pack_argtable) / sizeof(pack_argtable[0]));
    arg_freetable(unpack_argtable, sizeof(unpack_argtable) / sizeof(unpack_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));

    return rv;
}
