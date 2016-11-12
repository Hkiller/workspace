#include "argtable2.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/service.h"
#include "gd/app/app_context.h"

static int svr_main(int argc, char * argv[]) {
    gd_app_context_t ctx;
    int rv;

    ctx = gd_app_context_create_main(NULL, 0, argc, argv);
    if (ctx == NULL) return -1;

    gd_app_set_debug(ctx, 1);

	rv = gd_app_run(ctx);

	gd_app_context_free(ctx);

    return rv;
}

int main(int argc, char * argv[]) {
    int rv;

    /*run*/
    struct arg_rex  * run = arg_rex1(NULL, NULL, "run", NULL, 0, NULL);
    struct arg_file * run_pidfile = arg_file1(NULL, "pidfile", NULL, "pid file path");
    struct arg_file * run_root = arg_file1(NULL, "root", NULL, "root dir");
    struct arg_int *  run_app_id = arg_int0(NULL, "app-id", NULL,    "app id");
    struct arg_str * run_db = arg_strn(NULL, "db-svr", " <ip:port>", 1, -1, "db server");
    struct arg_str * run_db_ns = arg_str1(NULL, "db-ns", NULL, "db namespace");
    struct arg_str * run_db_buf_size = arg_str1(NULL, "db-buf-size", NULL, "db buf size");
    struct arg_str *  run_log_dir = arg_str0(NULL, "log-dir", NULL,    "log path");
    struct arg_end  * run_end = arg_end(20);
    void* run_argtable[] = { run, run_pidfile, run_root, run_app_id, run_db, run_db_ns, run_db_buf_size, run_log_dir, run_end };
    int run_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    run_nerrors = arg_parse(argc, argv, run_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    rv = 0;
    if (run_nerrors == 0) {
        rv = svr_main(argc, argv);
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        rv = -1;
        if (run->count) {
            arg_print_errors(stdout, run_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, run_argtable, "\n");
        }
        else {
            goto PRINT_HELP;
        }
    }

    goto EXIT;

PRINT_HELP:
    printf("%s: missing run command.\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(run_argtable, sizeof(run_argtable) / sizeof(run_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));
    return rv;
}