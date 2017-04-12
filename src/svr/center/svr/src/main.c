#include <assert.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include "argtable2.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/service.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/aom/aom_shm.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"

extern char g_metalib_svr_center_pro[];
static const char * generate_pid_file(const char * base_path, const char * app_name, error_monitor_t em);
static void svr_stop_all_sig_handler(int sig);

static int svr_main(const char * progname, const char * base_path, int argc, char * argv[], int shmkey) {
    struct mem_buffer buffer;
    gd_app_context_t ctx;
    char log_path[256];
    int rv;

    ctx = gd_app_context_create_main(NULL, 0, argc, argv);
    if (ctx == NULL) return -1;

    mem_buffer_init(&buffer, NULL);
    gd_app_set_root(ctx, dir_name_ex(progname, 2, &buffer));
    mem_buffer_clear(&buffer);

    gd_app_set_debug(ctx, 1);

    snprintf(log_path, sizeof(log_path), "%s/logs", base_path);
    if (gd_app_add_arg(ctx, "--log-dir") != 0
        || gd_app_add_arg(ctx, log_path) != 0) return -1;
    
    cfg_struct_add_int32(gd_app_cfg(ctx), "shmkey", shmkey, cfg_replace);

	rv = gd_app_run(ctx);

	gd_app_context_free(ctx);

    return rv;
}

int tool_shm_init(int shm_key, int shm_size, int force, error_monitor_t em) {
    LPDRMETA meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_cli_record");
    if (meta == NULL) {
        CPE_ERROR(em, "shm init: can`t find meta svr_center_cli_record!");
        return -1;
    }

    return aom_shm_init(meta, shm_key, shm_size, force, em);
}

int generate_shm_key(const char * pidfile, error_monitor_t em) {
    int key;

    key = cpe_shm_key_gen(pidfile, 'a');
    if (key == -1) {
        CPE_ERROR(em, "generate shm key at %s fail, errno=%d(%s)", pidfile, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    return key;
}

static int generate_base_dir_and_app(
    char * path, size_t path_size,
    char * app, size_t app_size,
    const char * progname)
{
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(path, path_size);
    const char * dir_end_1 = NULL;
    const char * dir_end_2 = NULL;
    const char * dir_end_3 = NULL;
    const char * dir_end_4 = NULL;
    const char * p;
    const char * home;
    int home_len;

    for(p = strchr(progname, '/'); p; p = strchr(p + 1, '/')) {
        dir_end_1 = dir_end_2;
        dir_end_2 = dir_end_3;
        dir_end_3 = dir_end_4;
        dir_end_4 = p;
    }

    home = getenv("HOME");
    home_len = strlen(home);

    cpe_str_buf_append(&str_buf, home, home_len);
    if (home[home_len - 1] != '/') {
        cpe_str_buf_cat(&str_buf, "/");
    }
    cpe_str_buf_cat(&str_buf, ".");
    cpe_str_buf_append(&str_buf, dir_end_1 + 1, dir_end_2 - dir_end_1 - 1);

    cpe_str_dup(app, app_size, dir_end_4 + 1);

    return cpe_str_buf_is_overflow(&str_buf) ? -1 : 0;
}

int main(int argc, char * argv[]) {
    int rv;

    /*run*/
    struct arg_rex  * run = arg_rex1(NULL, NULL, "run", NULL, 0, NULL);
    struct arg_str *  run_shm_size = arg_str1(NULL, "shm-size", NULL,    "shm size");
    struct arg_int *  run_shm_force = arg_int0(NULL, "force", NULL,    "force");
    struct arg_str *  run_listen = arg_str0(NULL, "listen", NULL,    "listen");
    struct arg_end  * run_end = arg_end(20);
    void* run_argtable[] = { run, run_shm_size, run_listen, run_end };
    int run_nerrors;

    /*start service*/
    struct arg_rex  * start = arg_rex1(NULL, NULL, "start", NULL, 0, NULL);
    struct arg_str *  start_shm_size = arg_str1(NULL, "shm-size", NULL,    "shm size");
    struct arg_int *  start_shm_force = arg_int0(NULL, "force", NULL,    "force");
    struct arg_str *  start_listen = arg_str0(NULL, "listen", NULL,    "listen");
    struct arg_end  * start_end = arg_end(20);
    void* start_argtable[] = { start, start_shm_size, start_shm_force, start_listen, start_end };
    int start_nerrors;

    /*stop service*/
    struct arg_rex  * stop = arg_rex1(NULL, NULL, "stop", NULL, 0, NULL);
    struct arg_end  * stop_end = arg_end(20);
    void* stop_argtable[] = { stop, stop_end };
    int stop_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;
    char progname[256];
    char base_path[256];
    char app_name[64];

    if (argv[0][0] == '/') {
        cpe_str_dup(progname, sizeof(progname), argv[0]);
    }
    else {
        snprintf(progname, sizeof(progname), "%s/%s", getcwd(NULL, 0), argv[0]);
    }
    file_name_normalize(progname);
    argv[0] = progname;

    run_nerrors = arg_parse(argc, argv, run_argtable);
    start_nerrors = arg_parse(argc, argv, start_argtable);
    stop_nerrors = arg_parse(argc, argv, stop_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    if (generate_base_dir_and_app(base_path, sizeof(base_path), app_name, sizeof(app_name), progname) != 0) return -1;

    rv = 0;
    if (run->count) {
        const char * pid_file;

        if (run_nerrors) goto PRINT_HELP;
        
        pid_file = generate_pid_file(base_path, app_name, em);
        if (pid_file == NULL) {
            rv = -1;
        }
        else {
            if (cpe_check_and_write_pid(pid_file, em) != 0) {
                printf("%s is already runing!\n", file_name_no_dir(progname));
                rv = -1;
            }
            else {
                int shmkey;
                uint64_t shmsize;

                shmkey = generate_shm_key(pid_file, em);
                if (shmkey == -1) return -1;

                if (cpe_str_parse_byte_size(&shmsize, start_shm_size->sval[0]) != 0) {
                    printf("%s shm-size " FMT_UINT64_T " format error!\n", progname, shmsize);
                    return -1;
                }
                
                if (tool_shm_init(shmkey, (int)shmsize, start_shm_force->ival[0], em) != 0) return -1;

                signal(SIGHUP, svr_stop_all_sig_handler);
                signal(SIGUSR1, svr_stop_all_sig_handler);
                
                rv = svr_main(progname, base_path, argc, argv, shmkey);
            }
        }
    }
    else if (start->count) {
        const char * pid_file;

        if (start_nerrors) goto PRINT_HELP;
        
        pid_file = generate_pid_file(base_path, app_name, em);
        if (pid_file == NULL) {
            rv = -1;
        }
        else {
            int shmkey;
            uint64_t shmsize;

            shmkey = generate_shm_key(pid_file, em);
            if (shmkey == -1) return -1;

            if (cpe_str_parse_byte_size(&shmsize, run_shm_size->sval[0]) != 0) {
                printf("%s shm-size " FMT_UINT64_T " format error!\n", progname, shmsize);
                return -1;
            }

            cpe_daemonize(em);
            
            if (tool_shm_init(shmkey, (int)shmsize, run_shm_force->ival[0], em) != 0) return -1;

            if (cpe_check_and_write_pid(pid_file, em) != 0) {
                printf("%s is already runing!\n", file_name_no_dir(progname));
                rv = -1;
            }
            else {
                signal(SIGUSR1, svr_stop_all_sig_handler);
                
                rv = svr_main(progname, base_path, argc, argv, shmkey);
            }
        }
    }
    else if (stop->count) {
        const char * pid_file;

        if (stop_nerrors) goto PRINT_HELP;

        pid_file = generate_pid_file(base_path, app_name, em);
        if (pid_file == NULL) {
            rv = -1;
        }
        else {
            rv = cpe_kill_by_pidfile(pid_file, SIGUSR1, em);
        }
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        rv = -1;
        goto PRINT_HELP;
    }

    goto EXIT;

PRINT_HELP:
    printf("%s:\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, start_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, stop_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 4: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(run_argtable, sizeof(run_argtable) / sizeof(run_argtable[0]));
    arg_freetable(start_argtable, sizeof(start_argtable) / sizeof(start_argtable[0]));
    arg_freetable(stop_argtable, sizeof(stop_argtable) / sizeof(stop_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));
    return rv;
}

static const char * generate_pid_file(const char * base_path, const char * app_name, error_monitor_t em) {
    static char buf[256];
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, sizeof(buf));
    
    cpe_str_buf_cat(&str_buf, base_path);

    if (dir_mk_recursion(buf, DIR_DEFAULT_MODE, em, NULL) != 0) {
        printf("create pid dir %s fail, error=%d (%s)\n", buf, errno, strerror(errno));
        return NULL;
    }
    
    cpe_str_buf_cat(&str_buf, "/");
    cpe_str_buf_cat(&str_buf, app_name);
    cpe_str_buf_cat(&str_buf, ".pid");

    return cpe_str_buf_is_overflow(&str_buf) ? NULL : buf;
}

static void svr_stop_all_sig_handler(int sig) {
    gd_app_context_t app;

    app = gd_app_ins();
    assert(app);

    APP_CTX_INFO(app, "svr_stop_all_sig_handler: receive sig %d, begin stop", sig);

    /* g_set_svr_mon->m_stop_apps = 1; */
    gd_app_notify_stop(app);
}
