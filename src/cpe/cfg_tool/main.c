#include <assert.h>
#include "argtable2.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"

int combine_cfg(
    const char * input_dir,
    const char * * merge_dir_begin, const char * * merge_dir_end,
    const char * * set_begin, const char * * set_end,
    const char * * add_begin, const char * * add_end,
    const char * output_file_name, const char * format, error_monitor_t em)
{
    vfs_mgr_t vfs = NULL;
    cfg_t cfg = NULL;
    vfs_file_t output_file = NULL;
    int rv = -1;
    struct mem_buffer path_buf;
    const char * path_dir;
    
    mem_buffer_init(&path_buf, NULL);
    
    vfs = vfs_mgr_create(NULL, em);
    if (vfs == NULL) {
        CPE_ERROR(em, "conbine cfg: create vfs fail!");
        goto COMBINE_CFG_COMPLETE;
    }
    
    cfg = cfg_create(NULL);
    if (cfg == NULL) {
        CPE_ERROR(em, "conbine cfg: create cfg fail!");
        goto COMBINE_CFG_COMPLETE;
    }
    
    if (cfg_read_dir(cfg, vfs, input_dir, cfg_merge_use_new, em, NULL) != 0) {
        CPE_ERROR(em, "conbine cfg: read from dir %s fail!", input_dir);
        goto COMBINE_CFG_COMPLETE;
    }

    for(; merge_dir_begin < merge_dir_end; ++merge_dir_begin) {
        if (cfg_read_dir(cfg, vfs, *merge_dir_begin, cfg_merge_use_exist, em, NULL) != 0) {
            CPE_ERROR(em, "conbine cfg: read from dir %s fail!", *merge_dir_begin);
            goto COMBINE_CFG_COMPLETE;
        }
    }

    for(; set_begin < set_end; ++set_begin) {
        const char * sep;
        char path_buf[128];

        sep = strchr(*set_begin, '=');
        if (sep == NULL) {
            CPE_ERROR(em, "conbine cfg: set: set data %s format error!", *set_begin);
            goto COMBINE_CFG_COMPLETE;
        }

        if (cpe_str_dup_range(path_buf, sizeof(path_buf), *set_begin, sep) == NULL) {
            CPE_ERROR(em, "conbine cfg: set: set data %s path overflow!", *set_begin);
            goto COMBINE_CFG_COMPLETE;
        }
        
        if (cfg_add_string(cfg, path_buf, sep + 1, em) == NULL) {
            CPE_ERROR(em, "conbine cfg: set: set %s ==> %s fail!", path_buf, sep + 1);
            goto COMBINE_CFG_COMPLETE;
        }
    }

    for(; add_begin < add_end; ++add_begin) {
        const char * sep;
        char path_buf[128];
        cfg_t seq_cfg;
        
        sep = strchr(*add_begin, '=');
        if (sep == NULL) {
            CPE_ERROR(em, "conbine cfg: add: add data %s format error!", *add_begin);
            goto COMBINE_CFG_COMPLETE;
        }

        if (cpe_str_dup_range(path_buf, sizeof(path_buf), *add_begin, sep) == NULL) {
            CPE_ERROR(em, "conbine cfg: add: add data %s path overflow!", *add_begin);
            goto COMBINE_CFG_COMPLETE;
        }

        seq_cfg = cfg_find_cfg(cfg, path_buf);
        if (seq_cfg) {
            if (cfg_type(seq_cfg) != CPE_CFG_TYPE_SEQUENCE) {
                CPE_ERROR(em, "conbine cfg: add: %s is not sequence!", path_buf);
                goto COMBINE_CFG_COMPLETE;
            }
        }
        else {
            seq_cfg = cfg_add_seq(cfg, path_buf, em);
            if (seq_cfg == NULL) {
                CPE_ERROR(em, "conbine cfg: add: create %s sequence fail!", path_buf);
                goto COMBINE_CFG_COMPLETE;
            }
        }
        
        if (cfg_seq_add_string(seq_cfg, sep + 1) == NULL) {
            CPE_ERROR(em, "conbine cfg: add: add to %s %s fail!", path_buf, sep + 1);
            goto COMBINE_CFG_COMPLETE;
        }
    }

    path_dir = dir_name(output_file_name, &path_buf);
    if (path_dir) {
        int rv = vfs_dir_mk_recursion(vfs, path_dir);
        if (rv != 0) {
            CPE_ERROR(em, "conbine cfg: create output dir %s fail, rv=%d!", path_dir, rv);
            goto COMBINE_CFG_COMPLETE;
        }
    }
    
    output_file = vfs_file_open(vfs, output_file_name, "wb");
    if (output_file == NULL) {
        CPE_ERROR(em, "conbine cfg: create output file %s fail!", output_file_name);
        goto COMBINE_CFG_COMPLETE;
    }

    if (strcmp(format, "yml") == 0) {
        struct vfs_write_stream fs;

        vfs_write_stream_init(&fs, output_file);

        if (cfg_yaml_write((write_stream_t)&fs, cfg, em) != 0) {
            CPE_ERROR(em, "conbine cfg: write result(yml) file to %s fail!", output_file_name);
            goto COMBINE_CFG_COMPLETE;
        }
    }
    else if (strcmp(format, "bin") == 0) {
        struct mem_buffer buffer;

        mem_buffer_init(&buffer, NULL);

        if (cfg_bin_write_to_buffer(&buffer, cfg, em) <= 0) {
            CPE_ERROR(em, "conbine cfg: write result(bin) to buffer fail!");
            mem_buffer_clear(&buffer);
            goto COMBINE_CFG_COMPLETE;
        }

        if (vfs_file_write_from_buffer(output_file, &buffer) < 0) {
            CPE_ERROR(em, "conbine cfg: write result(bin) file to %s fail!", output_file_name);
            mem_buffer_clear(&buffer);
            goto COMBINE_CFG_COMPLETE;
        }

        mem_buffer_clear(&buffer);
    }
    else {
        CPE_ERROR(em, "conbine cfg: unknown format %s", format);
        goto COMBINE_CFG_COMPLETE;
    }
    
    rv = 0;

COMBINE_CFG_COMPLETE:
    mem_buffer_clear(&path_buf);
    
    if (output_file) vfs_file_close(output_file);
    if (cfg) cfg_free(cfg);
    if (vfs) vfs_mgr_free(vfs);
    
    return rv;
}

int main(int argc, char * argv[]) {
    /*mk hpp*/
    struct arg_rex  * combine =     arg_rex1(NULL, NULL, "combine", NULL, 0, NULL);
    struct arg_file  * combine_input =     arg_file1(NULL, "input", NULL, "input cfg root");
    struct arg_file  * combine_merge =     arg_filen(NULL, "merge", NULL, 0, 100, "merge cfg root");
    struct arg_str  * combine_set =     arg_strn(NULL, "set", NULL, 0, 100, "set args");
    struct arg_str  * combine_add =     arg_strn(NULL, "add", NULL, 0, 100, "add args");
    struct arg_file  * combine_output =     arg_file1(NULL, "output", NULL, "output cfg file");
    struct arg_str  * combine_format =     arg_str1(NULL, "format", "(yml|bin)", "output format");
    struct arg_end  * combine_end = arg_end(20);
    void* combine_argtable[] = { combine, combine_input, combine_merge, combine_set, combine_add, combine_output, combine_format, combine_end };
    int combine_nerrors;

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

    combine_nerrors = arg_parse(argc, argv, combine_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    if (combine_nerrors == 0) {
        rv = combine_cfg(
            combine_input->filename[0],
            combine_merge->filename, combine_merge->filename + combine_merge->count,
            combine_set->sval, combine_set->sval + combine_set->count,
            combine_add->sval, combine_add->sval + combine_add->count,
            combine_output->filename[0], combine_format->sval[0], em);
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
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, combine_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(combine_argtable, sizeof(combine_argtable) / sizeof(combine_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));

    return rv;
}
