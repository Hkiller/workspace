#include <assert.h>
#include "curl/curl.h"
#include "yajl/yajl_tree.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_time.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/md5.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "convert_ctx.h"

struct ui_cache_pixel_tinypng_convert_ctx {
    convert_ctx_t m_ctx;
    char * m_cache_file;
    char m_apikey[128];
    char * m_result_url;
    mem_buffer_t m_response;
};

static int convert_tinypng_init(convert_ctx_t ctx);
static int convert_tinypng_calc_cache_file(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t input);
static int convert_tinypng_do_select_account(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx);
static int convert_tinypng_do_convert(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t input);
static int convert_tinypng_do_download(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx);
static int convert_tinypng_do_load_cache(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t output);
static int convert_tinypng_do_save_cache(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t output);

int convert_tinypng_convert(convert_ctx_t ctx, mem_buffer_t input, mem_buffer_t output, uint8_t * use_cache) {
    struct ui_cache_pixel_tinypng_convert_ctx cvt_ctx;

    if (!ctx->m_tinypng_init) {
        if (convert_tinypng_init(ctx) != 0) return -1;
    }
    
    bzero(&cvt_ctx, sizeof(cvt_ctx));
    cvt_ctx.m_ctx = ctx;
    cvt_ctx.m_cache_file = NULL;
    cvt_ctx.m_result_url = NULL;
    cvt_ctx.m_apikey[0] = 0;
    cvt_ctx.m_response = output;

    if (convert_tinypng_calc_cache_file(&cvt_ctx, input) != 0) goto SAVE_ERROR;

    if (vfs_file_exist(gd_app_vfs_mgr(ctx->m_app), cvt_ctx.m_cache_file)) {
        if (convert_tinypng_do_load_cache(&cvt_ctx, output) != 0) goto SAVE_ERROR;
        *use_cache = 1;
    }
    else {
        if (convert_tinypng_do_select_account(&cvt_ctx) != 0) goto SAVE_ERROR;
        if (convert_tinypng_do_convert(&cvt_ctx, input) != 0) goto SAVE_ERROR;
        if (convert_tinypng_do_download(&cvt_ctx) != 0) goto SAVE_ERROR;

        convert_tinypng_do_save_cache(&cvt_ctx, output);
        *use_cache = 0;
    }

    if (cvt_ctx.m_cache_file) mem_free(ctx->m_alloc, cvt_ctx.m_cache_file);
    if (cvt_ctx.m_result_url) mem_free(ctx->m_alloc, cvt_ctx.m_result_url);
    return 0;

SAVE_ERROR:
    if (cvt_ctx.m_cache_file) mem_free(ctx->m_alloc, cvt_ctx.m_cache_file);
    if (cvt_ctx.m_result_url) mem_free(ctx->m_alloc, cvt_ctx.m_result_url);
    return -1; 
}

static int convert_tinypng_init_load_saved_accounts(convert_ctx_t ctx) {
    mem_buffer_t tmp_buffer = gd_app_tmp_buffer(ctx->m_app);
    vfs_mgr_t vfs_mgr = gd_app_vfs_mgr(ctx->m_app);
    const char * path;
    uint16_t cur_year;
    uint16_t cur_mon;
    struct tm cur_tm;
    time_t cur_time;
    uint8_t need_init;
    cfg_t c;
    
    cur_time = time(0);
    localtime_r(&cur_time, &cur_tm);
    cur_year = cur_tm.tm_year + 1900;
    cur_mon = cur_tm.tm_mon + 1;

    mem_buffer_clear_data(tmp_buffer);
    mem_buffer_strcat(tmp_buffer, ctx->m_tinypng_cache);
    mem_buffer_strcat(tmp_buffer, "/accounts.yml");
    path = mem_buffer_make_continuous(tmp_buffer, 0);
    
    if (vfs_file_exist(vfs_mgr, path)) {
        uint16_t save_year;
        uint16_t save_mon;
        
        if (cfg_yaml_read_file(ctx->m_tinypng_data, vfs_mgr, path, cfg_merge_use_new, ctx->m_em) != 0) {
            CPE_ERROR(ctx->m_em, "convert: tinypng: read account data from %s fail!", path);
            return -1;
        }

        save_year = cfg_get_uint16(ctx->m_tinypng_data, "date.year", 0);
        save_mon = cfg_get_uint16(ctx->m_tinypng_data, "date.mon", 0);

        need_init = (save_year == cur_year && save_mon == cur_mon) ? 0 : 1;
    }
    else {
        need_init = 1;
    }

    if (!need_init) return 0;

    c = cfg_struct_add_struct(ctx->m_tinypng_data, "date", cfg_replace);
    cfg_struct_add_uint16(c, "year", cur_year, cfg_replace);
    cfg_struct_add_uint16(c, "mon", cur_mon, cfg_replace);
    c = cfg_struct_add_struct(ctx->m_tinypng_data, "accounts", cfg_replace);

    return 0;
}

static int convert_tinypng_init_load_input_accounts(convert_ctx_t ctx) {
    cfg_t account_cfg = cfg_find_cfg(ctx->m_tinypng_data, "accounts");
    mem_buffer_t tmp_buffer = gd_app_tmp_buffer(ctx->m_app);
    char * data_p;
    char * sep;
    
    if (ctx->m_tinypng_accounts == NULL) return 0;
    
    mem_buffer_clear_data(tmp_buffer);
    if (vfs_file_load_to_buffer_by_path(tmp_buffer, gd_app_vfs_mgr(ctx->m_app), ctx->m_tinypng_accounts) < 0) {
        CPE_ERROR(ctx->m_em, "convert: tinypng: read input accounts from %s fail!", ctx->m_tinypng_accounts);
        return -1;
    }

    mem_buffer_append_char(tmp_buffer, 0);
    data_p = mem_buffer_make_continuous(tmp_buffer, 0);

    
    while((sep = strchr(data_p, '\n'))) {
        * cpe_str_trim_tail(sep, data_p) = 0;

        if (data_p[0]) {
            cfg_struct_add_uint32(account_cfg, data_p, 0, cfg_merge_use_exist);
        }
        
        data_p = cpe_str_trim_head(sep + 1);
    }

    if (data_p[0]) {
        cfg_struct_add_uint32(account_cfg, data_p, 0, cfg_merge_use_exist);
    }
    
    return 0;
}

static int convert_tinypng_save_accounts(convert_ctx_t ctx) {
    mem_buffer_t tmp_buffer = gd_app_tmp_buffer(ctx->m_app);
    const char * path;

    mem_buffer_clear_data(tmp_buffer);
    mem_buffer_strcat(tmp_buffer, ctx->m_tinypng_cache);
    mem_buffer_strcat(tmp_buffer, "/accounts.yml");
    path = mem_buffer_make_continuous(tmp_buffer, 0);
    
    if (cfg_yaml_write_file(ctx->m_tinypng_data, gd_app_vfs_mgr(ctx->m_app), path, ctx->m_em) != 0) {
        CPE_ERROR(ctx->m_em, "tinypng: save accounts to %s fail!", path);
        return -1;
    }

    return 0;
}

static int convert_tinypng_init(convert_ctx_t ctx) {
    mem_buffer_t tmp_buffer = gd_app_tmp_buffer(ctx->m_app);
    
    if (ctx->m_tinypng_init) return 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    mem_buffer_clear_data(tmp_buffer);
    mem_buffer_strcat(tmp_buffer, getenv("HOME"));
    mem_buffer_strcat(tmp_buffer, "/.tinypng");

    ctx->m_tinypng_cache = cpe_str_mem_dup(ctx->m_alloc, mem_buffer_make_continuous(tmp_buffer, 0));

    if (vfs_dir_mk_recursion(gd_app_vfs_mgr(ctx->m_app), ctx->m_tinypng_cache) != 0) {
        CPE_ERROR(ctx->m_em, "convert: tinypng: create cache dir %s fail!", ctx->m_tinypng_cache);
        goto INIT_FAIL;
    }

    ctx->m_tinypng_data = cfg_create(ctx->m_alloc);
    if (ctx->m_tinypng_data == NULL) {
        CPE_ERROR(ctx->m_em, "convert: tinypng: create cache dir %s fail!", ctx->m_tinypng_cache);
        goto INIT_FAIL;
    }

    if (convert_tinypng_init_load_saved_accounts(ctx) != 0
        || convert_tinypng_init_load_input_accounts(ctx) != 0
        || convert_tinypng_save_accounts(ctx) != 0
        )
    {
        goto INIT_FAIL;
    }

    ctx->m_tinypng_init = 1; 
    return 0;

INIT_FAIL:
    if (ctx->m_tinypng_cache) {
        mem_free(ctx->m_alloc, ctx->m_tinypng_cache);
        ctx->m_tinypng_cache = NULL;
    }

    if (ctx->m_tinypng_data) {
        cfg_free(ctx->m_tinypng_data);
        ctx->m_tinypng_data = NULL;
    }
    
    return -1;
}

static int convert_tinypng_calc_cache_file(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t input) {
    struct cpe_md5_ctx md5_ctx;
    mem_buffer_t tmp_buffer = gd_app_tmp_buffer(cvt_ctx->m_ctx->m_app);
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(tmp_buffer);
    
    cpe_md5_ctx_init(&md5_ctx);
    cpe_md5_ctx_update(&md5_ctx, mem_buffer_make_continuous(input, 0), mem_buffer_size(input));
    cpe_md5_ctx_final(&md5_ctx);

    mem_buffer_clear_data(tmp_buffer);

    stream_printf((write_stream_t)&s, "%s/", cvt_ctx->m_ctx->m_tinypng_cache);
    cpe_md5_print((write_stream_t)&s, &md5_ctx.value);
    stream_printf((write_stream_t)&s, ".png");
    stream_putc((write_stream_t)&s, 0);

    cvt_ctx->m_cache_file = cpe_str_mem_dup(cvt_ctx->m_ctx->m_alloc, mem_buffer_make_continuous(tmp_buffer, 0));
    
    return 0;
}

static int convert_tinypng_do_select_account(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx) {
    cfg_t accounts_cfg = cfg_find_cfg(cvt_ctx->m_ctx->m_tinypng_data, "accounts");
    cfg_t cfg;
    struct cfg_it cfg_it;
    cfg_t selected;

    selected = NULL;
    
    cfg_it_init(&cfg_it, accounts_cfg);
    while((cfg = cfg_it_next(&cfg_it))) {
        if (selected == NULL || cfg_as_uint32(cfg, 65535) < cfg_as_uint32(selected, 65535) ) {
            selected = cfg;
        }
    }

    if (selected == NULL) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "tinypng: select count fail!");
        return -1;
    }

    snprintf(cvt_ctx->m_apikey, sizeof(cvt_ctx->m_apikey), "api:%s", cfg_name(selected));

    cfg_struct_add_uint32(accounts_cfg, cvt_ctx->m_apikey + 4, cfg_as_uint32(selected, 65535) + 1, cfg_replace);
    
    convert_tinypng_save_accounts(cvt_ctx->m_ctx);
               
    return 0;
}

static int convert_tinypng_do_load_cache(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t output) {
    vfs_mgr_t vfs_mgr = gd_app_vfs_mgr(cvt_ctx->m_ctx->m_app);
    vfs_file_t cache_file = vfs_file_open(vfs_mgr, cvt_ctx->m_cache_file, "rb");
    if (cache_file == NULL) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "open cache file %s fail!", cvt_ctx->m_cache_file);
        return -1;
    }
        
    if (vfs_file_load_to_buffer(output, cache_file) < 0) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "open cache file %s fail!", cvt_ctx->m_cache_file);
        vfs_file_close(cache_file);
        return -1;
    }
    
    vfs_file_close(cache_file);
    return 0;
}

static int convert_tinypng_do_save_cache(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t output) {
    vfs_file_t cache_file;
        
    cache_file = vfs_file_open(gd_app_vfs_mgr(cvt_ctx->m_ctx->m_app), cvt_ctx->m_cache_file, "wb");
    if (cache_file == NULL) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "open cache file %s for write fail!", cvt_ctx->m_cache_file);
        return -1;
    }

    if (vfs_file_write_from_buffer(cache_file, output) < 0) {
        vfs_file_close(cache_file);
        vfs_file_rm(gd_app_vfs_mgr(cvt_ctx->m_ctx->m_app), cvt_ctx->m_cache_file);
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "open cache file %s fail!", cvt_ctx->m_cache_file);
        return -1;
    }
    
    vfs_file_close(cache_file);
    return 0;
}

static size_t convert_tinypng_save_response(void *buffer, size_t size, size_t nmemb, void *stream) {
    struct ui_cache_pixel_tinypng_convert_ctx * ctx = stream;
    mem_buffer_append(ctx->m_response, buffer, size * nmemb);
    return size * nmemb;
}

static CURL * convert_tinypng_create_curl(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx) {
    CURL *curl = NULL;

    curl = curl_easy_init();
    if (curl == NULL) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "init curl fail!");
        return NULL;
    }

    /*fot debug*/
    /* curl_easy_setopt(curl, CURLOPT_STDERR, stderr); */
    /* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, convert_tinypng_save_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, cvt_ctx);
    curl_easy_setopt(curl, CURLOPT_USERPWD, cvt_ctx->m_apikey);

    mem_buffer_clear_data(cvt_ctx->m_response);
    
    return curl;
}

static int convert_tinypng_do_convert(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx, mem_buffer_t input) {
    CURL *curl = NULL;
    CURLcode curl_rv;
    struct curl_slist * headers = NULL;
    const char * convert_response;
    yajl_val data_tree = NULL;
    char error_buf[128];
    yajl_val val_output, val;
    
    curl = convert_tinypng_create_curl(cvt_ctx);
    if (curl == NULL) return -1;

    headers = curl_slist_append(headers, "Expect:"); /*禁止 Expect: 100-continue*/

    /*发送转换请求 */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, mem_buffer_size(input));
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, mem_buffer_make_continuous(input, 0));
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.tinify.com/shrink");
    
    curl_rv = curl_easy_perform(curl);
    if (curl_rv != CURLM_OK) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "save_res_tinypng: conrever error, %s!", curl_easy_strerror(curl_rv));
        goto CONVERT_ERROR;
    }
    
    mem_buffer_append_char(cvt_ctx->m_response, 0);
    convert_response = (const char *)mem_buffer_make_continuous(cvt_ctx->m_response, 0);

    data_tree  = yajl_tree_parse(convert_response, error_buf, sizeof(error_buf));
    if(data_tree == NULL){
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "parse response(%s) fail(%s)!", convert_response, error_buf);
        goto CONVERT_ERROR;
    }

    val_output = yajl_tree_get_2(data_tree, "output", yajl_t_object);
    if (val_output == NULL) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "response(%s) no output", convert_response);
        goto CONVERT_ERROR;
    }

    val = yajl_tree_get_2(val_output, "url", yajl_t_string);
    if (val == NULL) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "response(%s) no output.url", convert_response);
        goto CONVERT_ERROR;
    }

    assert(cvt_ctx->m_result_url == NULL);
    cvt_ctx->m_result_url = cpe_str_mem_dup(cvt_ctx->m_ctx->m_alloc, YAJL_GET_STRING(val));
    yajl_tree_free(data_tree);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 0;

CONVERT_ERROR:
    if (data_tree) yajl_tree_free(data_tree);
    if (headers) curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return -1;
}

static int convert_tinypng_do_download(struct ui_cache_pixel_tinypng_convert_ctx * cvt_ctx) {
    CURL *curl = NULL;
    CURLcode curl_rv;
    
    curl = convert_tinypng_create_curl(cvt_ctx);
    if (curl == NULL) return -1;

    /*发送转换请求 */
    curl_easy_setopt(curl, CURLOPT_URL, cvt_ctx->m_result_url);

    curl_rv = curl_easy_perform(curl);
    if (curl_rv != CURLM_OK) {
        CPE_ERROR(cvt_ctx->m_ctx->m_em, "download fail(%s)!", curl_easy_strerror(curl_rv));
        goto DOWNLOAD_ERROR;
    }
    
    curl_easy_cleanup(curl);
    return 0;
    
DOWNLOAD_ERROR:
    curl_easy_cleanup(curl);
    return -1;
}
