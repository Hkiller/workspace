#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/net_trans/net_trans_detail.h"
#include "net_trans_task_i.h"

static size_t net_trans_task_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata);
static int net_tranks_task_prog_cb(void *p, double dltotal, double dlnow, double ult, double uln);

net_trans_task_t net_trans_task_create(net_trans_group_t group, size_t capacity) {
    net_trans_manage_t mgr = group->m_mgr;
    net_trans_task_t task;

    task = mem_alloc(mgr->m_alloc, sizeof(struct net_trans_task) + capacity);
    if (task == NULL) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create task: alloc fail!", net_trans_manage_name(mgr), group->m_name);
        return NULL;
    }

    task->m_id = ++mgr->m_max_id;
    task->m_group = group;
    task->m_capacity = capacity;
    task->m_is_free = 0;
    task->m_in_callback = 0;
    task->m_sockfd = -1;
    task->m_evset = 0;
    task->m_state = net_trans_task_init;
    task->m_result = net_trans_result_unknown;
    task->m_errno = 0;
    task->m_watch.data = task;
    task->m_commit_op = NULL;
    task->m_commit_ctx = NULL;
    task->m_commit_ctx_free = NULL;
    task->m_progress_op = NULL;
    task->m_progress_ctx = NULL;
    task->m_progress_ctx_free = NULL;
    task->m_write_op = NULL;
    task->m_write_ctx = NULL;
    task->m_write_ctx_free = NULL;
    task->m_header = NULL;
    
    task->m_handler = curl_easy_init();
    if (task->m_handler == NULL) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create task: curl_easy_init fail!", net_trans_manage_name(mgr), group->m_name);
        mem_free(mgr->m_alloc, task);
        return NULL;
    }
    curl_easy_setopt(task->m_handler, CURLOPT_PRIVATE, task);

    curl_easy_setopt(task->m_handler, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(task->m_handler, CURLOPT_DNS_CACHE_TIMEOUT, mgr->m_cfg_dns_cache_timeout);
	curl_easy_setopt(task->m_handler, CURLOPT_CONNECTTIMEOUT_MS, group->m_connect_timeout_ms);
    curl_easy_setopt(task->m_handler, CURLOPT_TIMEOUT_MS, group->m_transfer_timeout_ms);
	curl_easy_setopt(task->m_handler, CURLOPT_FORBID_REUSE, group->m_forbid_reuse);

    net_trans_task_set_debug(task, mgr->m_debug >= 2 ? 1 : 0);
    
	curl_easy_setopt(task->m_handler, CURLOPT_WRITEFUNCTION, net_trans_task_write_cb);
	curl_easy_setopt(task->m_handler, CURLOPT_WRITEDATA, task);

    curl_easy_setopt(task->m_handler, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(task->m_handler, CURLOPT_PROGRESSFUNCTION, net_tranks_task_prog_cb);
    curl_easy_setopt(task->m_handler, CURLOPT_PROGRESSDATA, task);

    cpe_hash_entry_init(&task->m_hh_for_mgr);
    if (cpe_hash_table_insert_unique(&mgr->m_tasks, task) != 0) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create task: task id duplicate!", net_trans_manage_name(mgr), group->m_name);
        curl_easy_cleanup(task->m_handler);
        mem_free(mgr->m_alloc, task);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&group->m_tasks, task, m_next_for_group);

    mem_buffer_init(&task->m_buffer, mgr->m_alloc);

    bzero(task + 1, capacity);

    if (mgr->m_debug) {
        CPE_INFO(mgr->m_em, "%s: task %d (%s): create!", net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
    }

    return task;
}

void net_trans_task_free(net_trans_task_t task) {
    net_trans_group_t group = task->m_group;
    net_trans_manage_t mgr = group->m_mgr;

    if (task->m_state != net_trans_task_done) {
        task->m_is_free = 1;
        net_trans_task_set_done(task, net_trans_result_cancel, -1);
        return;
    }

    if (task->m_in_callback) {
        task->m_is_free = 1;
        return;
    }

    if (mgr->m_debug) {
        CPE_INFO(mgr->m_em, "%s: task %d (%s): free!", net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
    }

    curl_easy_cleanup(task->m_handler);
    task->m_handler = NULL;

    TAILQ_REMOVE(&group->m_tasks, task, m_next_for_group);
    cpe_hash_table_remove_by_ins(&mgr->m_tasks, task);

    if (task->m_evset) {
        ev_io_stop(mgr->m_loop, &task->m_watch);
        task->m_evset = 0;
    }

    task->m_sockfd = -1;

    mem_buffer_clear(&task->m_buffer);

    if (task->m_commit_ctx && task->m_commit_ctx_free) {
        task->m_commit_ctx_free(task->m_commit_ctx);
    }
    task->m_commit_op = NULL;
    task->m_commit_ctx = NULL;
    task->m_commit_ctx_free = NULL;

    if (task->m_progress_ctx && task->m_progress_ctx_free) {
        task->m_progress_ctx_free(task->m_progress_ctx);
    }
    task->m_progress_op = NULL;
    task->m_progress_ctx = NULL;
    task->m_progress_ctx_free = NULL;
    
    if (task->m_write_ctx && task->m_write_ctx_free) {
        task->m_write_ctx_free(task->m_write_ctx);
    }
    task->m_write_op = NULL;
    task->m_write_ctx = NULL;
    task->m_write_ctx_free = NULL;

    if (task->m_header) {
        curl_slist_free_all(task->m_header);
        task->m_header = NULL;
    }
    
    mem_free(mgr->m_alloc, task);
}

int net_trans_task_start(net_trans_task_t task) {
    net_trans_manage_t mgr = task->m_group->m_mgr;
    int rc;

    assert(!task->m_is_free);

    if (task->m_state == net_trans_task_working) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t start in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            net_trans_task_state_str(task->m_state));
        return -1;
    }

    if (task->m_state == net_trans_task_done) {
        CURL * new_handler = curl_easy_duphandle(task->m_handler);
        if (new_handler == NULL) {
            CPE_ERROR(
                mgr->m_em, "%s: task %d (%s): duphandler fail!",
                net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
            return -1;
        }

        if (mgr->m_debug) {
            CPE_INFO(mgr->m_em, "%s: task %d (%s): duphandler!", net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
        }

        curl_easy_cleanup(task->m_handler);
        task->m_handler = new_handler;
    }

    rc = curl_multi_add_handle(mgr->m_multi_handle, task->m_handler);
    if (rc != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): curl_multi_add_handle error, rc=%d",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, rc);
        return -1;
    }
    task->m_state = net_trans_task_working;

    mgr->m_still_running = 1;

    if (mgr->m_debug) {
        CPE_INFO(mgr->m_em, "%s: task %d (%s): start!", net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
    }

    return 0;
}

int net_trans_task_restart(net_trans_task_t task) {
    net_trans_manage_t mgr = task->m_group->m_mgr;
    CURL * new_handler;

    if (task->m_state != net_trans_task_done) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): can`t restart in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            net_trans_task_state_str(task->m_state));
        return -1;
    }

    new_handler = curl_easy_duphandle(task->m_handler);
    if (new_handler == NULL) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): duphandler fail!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
        return -1;
    }

    curl_easy_cleanup(task->m_handler);
    task->m_handler = new_handler;
    task->m_sockfd = -1;
    task->m_state = net_trans_task_init;
    task->m_result = net_trans_result_unknown;
    task->m_errno = 0;

    return 0;
}

uint32_t net_trans_task_id(net_trans_task_t task) {
    return task->m_id;
}

net_trans_group_t net_trans_task_group(net_trans_task_t task) {
    return task->m_group;
}

net_trans_manage_t net_trans_task_manage(net_trans_task_t task) {
    return task->m_group->m_mgr;
}

net_trans_task_state_t net_trans_task_state(net_trans_task_t task) {
    return task->m_state;
}

net_trans_task_result_t net_trans_task_result(net_trans_task_t task) {
    if (task->m_state != net_trans_task_done) return net_trans_result_unknown;

    return task->m_result;
}

net_trans_errno_t net_trans_task_errno(net_trans_task_t task) {
    return task->m_errno;
}

void * net_trans_task_data(net_trans_task_t task) {
    return task + 1;
}

size_t net_trans_task_data_capacity(net_trans_task_t task) {
    return task->m_capacity;
}

mem_buffer_t net_trans_task_buffer(net_trans_task_t task) {
    return &task->m_buffer;
}

char * net_trans_task_buffer_to_string(net_trans_task_t task) {
    net_trans_manage_t mgr = task->m_group->m_mgr;
    char * r;
    size_t r_size;

    if (task->m_state != net_trans_task_done) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): state is %s: task not complete, can`t get result string",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, net_trans_task_state_str(task->m_state));
        return NULL;
    }

    r_size = mem_buffer_size(&task->m_buffer);
    r = mem_buffer_make_continuous(&task->m_buffer, 1);
    if (r == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): get buff fail!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
        return NULL;
    }

    r[r_size] = 0;
    return r;
}

void net_trans_task_set_commit_op(net_trans_task_t task, net_trans_task_commit_op_t op, void * ctx, void (*ctx_free)(void *)) {
    if (task->m_commit_ctx && task->m_commit_ctx_free) {
        task->m_commit_ctx_free(task->m_commit_ctx);
    }
    
    task->m_commit_op = op;
    task->m_commit_ctx = ctx;
    task->m_commit_ctx_free = ctx_free;
}

void net_trans_task_set_progress_op(net_trans_task_t task, net_trans_task_progress_op_t op, void * ctx, void (*ctx_free)(void *)) {
    if (task->m_progress_ctx && task->m_progress_ctx_free) {
        task->m_progress_ctx_free(task->m_progress_ctx);
    }
    
    task->m_progress_op = op;
    task->m_progress_ctx = ctx;
    task->m_progress_ctx_free = ctx_free;
}

void net_trans_task_set_write_op(net_trans_task_t task, net_trans_task_write_op_t op, void * ctx, void (*ctx_free)(void *)) {
    if (task->m_write_ctx && task->m_write_ctx_free) {
        task->m_write_ctx_free(task->m_write_ctx);
    }
    
    task->m_write_op = op;
    task->m_write_ctx = ctx;
    task->m_write_ctx_free = ctx_free;
}

void net_trans_task_set_debug(net_trans_task_t task, uint8_t is_debug) {
    if (is_debug) {
        curl_easy_setopt(task->m_handler, CURLOPT_STDERR, stderr );
        curl_easy_setopt(task->m_handler, CURLOPT_VERBOSE, 1L);
    }
    else {
        curl_easy_setopt(task->m_handler, CURLOPT_VERBOSE, 0L);
    }
}

int net_trans_task_set_get(net_trans_task_t task, const char * uri) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

    if (task->m_state == net_trans_task_working) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t set get %s in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            uri, net_trans_task_state_str(task->m_state));
        return -1;
    }

    if (curl_easy_setopt(task->m_handler, CURLOPT_POST, 0L) != (int)CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_URL, uri) != (int)CURLM_OK)
    {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): set query to %s fail!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, uri);
        return -1;
    }

    if (cpe_str_start_with(uri, "https")) {
        curl_easy_setopt(task->m_handler, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(task->m_handler, CURLOPT_SSL_VERIFYHOST, 0);
    }

    return 0;
}

int net_trans_task_set_post_to(net_trans_task_t task, const char * uri, const char * data, size_t data_len) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

    if (task->m_state == net_trans_task_working) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t set post %d data to %s in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            (int)data_len, uri, net_trans_task_state_str(task->m_state));
        return -1;
    }

    if (curl_easy_setopt(task->m_handler, CURLOPT_POST, 1L) != (int)CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_POSTFIELDSIZE, (int)data_len) != (int)CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_COPYPOSTFIELDS, data) != (int)CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_URL, uri) != (int)CURLM_OK)
    {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): set post %d data to %s fail!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)data_len, uri);
        return -1;
    }

    if (cpe_str_start_with(uri, "https") == 0) {
        curl_easy_setopt(task->m_handler, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(task->m_handler, CURLOPT_SSL_VERIFYHOST, 0);
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): set post %d data to %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)data_len, uri);
    }

    return 0;
}

int net_trans_task_set_ssl_cainfo(net_trans_task_t task, const char * ca_file) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

    if (task->m_state == net_trans_task_working) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t set ssl cainfo %s in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            ca_file, net_trans_task_state_str(task->m_state));
        return -1;
    }

    if (curl_easy_setopt(task->m_handler, CURLOPT_SSL_VERIFYPEER, 1) != (int)CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_CAINFO, ca_file) != (int)CURLM_OK
        )
    {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): set ssl cainfo %s error!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, ca_file);
        return -1;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): set ca info %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, ca_file);
    }

    return 0;
}

int net_trans_task_set_done(net_trans_task_t task, net_trans_task_result_t result, int err) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

    if (task->m_state != net_trans_task_working) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t done in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            net_trans_task_state_str(task->m_state));
        return -1;
    }

    assert(task->m_state == net_trans_task_working);
    curl_multi_remove_handle(mgr->m_multi_handle, task->m_handler);

    task->m_result = result;
    task->m_errno = (net_trans_errno_t)err;
    task->m_state = net_trans_task_done;

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): done, result is %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            net_trans_task_result_str(task->m_result));
    }

    if (task->m_commit_op) {
        task->m_in_callback = 1;
        task->m_commit_op(task, task->m_commit_ctx);
        task->m_in_callback = 0;

        if (task->m_is_free || task->m_state == net_trans_task_done) {
            task->m_is_free = 0;
            net_trans_task_free(task);
        }
    }
    else {
        net_trans_task_free(task);
    }

    return 0;
}

const char * net_trans_task_state_str(net_trans_task_state_t state) {
    switch(state) {
    case net_trans_task_init:
        return "init";
    case net_trans_task_working:
        return "working";
    case net_trans_task_done:
        return "done";
    default:
        return "unknown";
    }
}

const char * net_trans_task_result_str(net_trans_task_result_t result) {
    switch(result) {
    case net_trans_result_unknown:
        return "unknown";
    case net_trans_result_ok:
        return "ok";
    case net_trans_result_error:
        return "error";
    case net_trans_result_timeout:
        return "timeout";
    case net_trans_result_cancel:
        return "cancel";
    default:
        return "!!bad task result!!";
    }
}

int net_trans_task_set_skip_data(net_trans_task_t task, ssize_t skip_length) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

    if (curl_easy_setopt(task->m_handler, CURLOPT_RESUME_FROM_LARGE, skip_length > 0 ? (curl_off_t)skip_length : (curl_off_t)0) != (int)CURLM_OK) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): set skip data %d error!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)skip_length);
        return -1;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): set skip data %d!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)skip_length);
    }

    return 0;
}

int net_trans_task_set_timeout(net_trans_task_t task, uint64_t timeout_ms) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

	if (curl_easy_setopt(task->m_handler, CURLOPT_TIMEOUT_MS, timeout_ms) != (int)CURLM_OK) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): set timeout " FMT_UINT64_T " error!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, timeout_ms);
        return -1;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): set timeout " FMT_UINT64_T "!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, timeout_ms);
    }

    return 0;
}

int net_trans_task_set_useragent(net_trans_task_t task, const char * agent) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

	if (curl_easy_setopt(task->m_handler, CURLOPT_USERAGENT, agent) != (int)CURLM_OK) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): set useragent %s error!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, agent);
        return -1;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): set useragent %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, agent);
    }

    return 0;
}

int net_trans_task_append_header(net_trans_task_t task, const char * header_one) {
    net_trans_manage_t mgr = task->m_group->m_mgr;
    
    if (task->m_header == NULL) {
        task->m_header = curl_slist_append(NULL, header_one);

        if (task->m_header == NULL) {
            CPE_ERROR(
                mgr->m_em, "%s: task %d (%s): append header %s create slist fail!",
                net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, header_one);
            return -1;
        }

        if (curl_easy_setopt(task->m_handler, CURLOPT_HTTPHEADER, task->m_header) != (int)CURLM_OK) {
            CPE_ERROR(
                mgr->m_em, "%s: task %d (%s): append header %s set opt fail!",
                net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, header_one);
            return -1;
        }
    }
    else {
        if (curl_slist_append(task->m_header, header_one) == NULL) {
            CPE_ERROR(
                mgr->m_em, "%s: task %d (%s): append header %s add to slist fail!",
                net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, header_one);
            return -1;
        }
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): append header %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, header_one);
    }

    return 0;
}

static size_t net_trans_task_write_cb(char *ptr, size_t size, size_t nmemb, void * userdata) {
	net_trans_task_t task = userdata;
    net_trans_manage_t mgr = task->m_group->m_mgr;
	size_t total_length = size * nmemb;
    ssize_t write_size;

    if (task->m_write_op) {
        task->m_in_callback = 1;
        task->m_write_op(task, task->m_write_ctx, ptr, total_length);
        task->m_in_callback = 0;

        if (task->m_is_free) {
            task->m_is_free = 0;
            net_trans_task_free(task);
        }

        return total_length;
    }
    else {
        write_size = mem_buffer_append(&task->m_buffer, ptr, total_length);
        if (write_size != (ssize_t)total_length) {
            CPE_ERROR(
                mgr->m_em, "%s: task %d (%s): append %d data fail, return %d!",
                net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)total_length, (int)write_size);
        }
        else {
            if (mgr->m_debug) {
                CPE_INFO(
                    mgr->m_em, "%s: task %d (%s): receive %d data!",
                    net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)total_length);
            }
        }
    }
    
    return total_length;
}

static int net_tranks_task_prog_cb(void *p, double dltotal, double dlnow, double ult, double uln) {
    net_trans_task_t task = (net_trans_task_t)p;
    (void)ult;
    (void)uln;

    if (task->m_progress_op) {
        task->m_in_callback = 1;
        task->m_progress_op(task, task->m_progress_ctx, dltotal, dlnow);
        task->m_in_callback = 0;

        if (task->m_is_free) {
            task->m_is_free = 0;
            net_trans_task_free(task);
        }
    }
    
    return 0;
}

uint32_t net_trans_task_hash(net_trans_task_t task) {
    return task->m_id;
}

int net_trans_task_eq(net_trans_task_t l, net_trans_task_t r) {
    return l->m_id == r->m_id;
}

