#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "dp_internal_types.h"

dp_req_t
dp_req_create(dp_mgr_t mgr, size_t capacity) {
    dp_req_t req;

    assert(mgr);

    req = mem_alloc(mgr->m_alloc, sizeof(struct dp_req) + capacity);
    if (req == NULL) return NULL;

    req->m_mgr = mgr;
    req->m_type = NULL;
    req->m_dumper = NULL;
    req->m_capacity = capacity;
    req->m_parent = NULL;
    req->m_manage_by_parent = 0;
    req->m_data = req + 1;
    req->m_data_capacity = capacity;
    req->m_data_size = 0;
    req->m_data_meta = NULL;

    TAILQ_INIT(&req->m_childs);

    return req;
}

dp_mgr_t dp_req_mgr(dp_req_t req) {
    assert(req);
    return req->m_mgr;
}

void dp_req_set_parent(dp_req_t child, dp_req_t parent) {
    assert(child);

    if (child->m_parent) {
        TAILQ_REMOVE(&child->m_parent->m_childs, child, m_brother);
        child->m_parent = NULL;
    }

    child->m_manage_by_parent = 0;

    if (parent) {
        TAILQ_INSERT_TAIL(&parent->m_childs, child, m_brother);
        child->m_parent = parent;
    }
}

void dp_req_add_to_parent(dp_req_t child, dp_req_t parent) {
    assert(child);
    assert(parent);

    if (child->m_parent) {
        TAILQ_REMOVE(&child->m_parent->m_childs, child, m_brother);
        child->m_parent = NULL;
    }

    TAILQ_INSERT_TAIL(&parent->m_childs, child, m_brother);
    child->m_parent = parent;
    child->m_manage_by_parent = 1;
}

int dp_req_manage_by_parent(dp_req_t req) {
    return req->m_manage_by_parent;
}

void dp_req_free(dp_req_t req) {
    if (req == NULL) return;

    while(!TAILQ_EMPTY(&req->m_childs)) {
        dp_req_t child = TAILQ_FIRST(&req->m_childs);
        if (child->m_manage_by_parent) {
            dp_req_free(child);
        }
        else {
            dp_req_set_parent(child, NULL);
        }
    }

    if (req->m_parent) {
        TAILQ_REMOVE(&req->m_parent->m_childs, req, m_brother);
    }

    mem_free(req->m_mgr->m_alloc, req);
}

void dp_req_set_type(dp_req_t req, const char * type) {
    req->m_type = type;
}

int dp_req_is_type(dp_req_t req, const char * type) {
    return req->m_type
        ? (strcmp(req->m_type, type) == 0 ? 1 : 0)
        : 0;
}

dp_req_t dp_req_parent(dp_req_t req) {
    return req->m_parent;
}

dp_req_t dp_req_parent_find(dp_req_t req, const char * typeName) {
    while(req) {
        if (req->m_parent && strcmp(req->m_type, typeName) == 0) return req;
        if (req->m_data_meta && strcmp(dr_meta_name(req->m_data_meta), typeName) == 0) return req;
        req = req->m_parent;
    }

    return NULL;
}

static dp_req_t dp_req_child_next(struct dp_req_it * it) {
    dp_req_t r = it->m_context;

    if (r) it->m_context = TAILQ_NEXT(r, m_brother);

    return r;
}

void dp_req_childs(dp_req_t req, dp_req_it_t it) {
    it->next = dp_req_child_next;
    it->m_context = TAILQ_FIRST(&req->m_childs);
}

void dp_req_child_clear(dp_req_t req) {
    while(!TAILQ_EMPTY(&req->m_childs)) {
        dp_req_t child = TAILQ_FIRST(&req->m_childs);
        if (child->m_manage_by_parent) {
            dp_req_free(child);
        }
        else {
            dp_req_set_parent(child, NULL);
        }
    }
}

dp_req_t dp_req_child_first(dp_req_t req) {
    return TAILQ_FIRST(&req->m_childs);
}

dp_req_t dp_req_child_find(dp_req_t req, const char * typeName) {
    dp_req_t child;

    TAILQ_FOREACH(child, &req->m_childs, m_brother) {
        if (child->m_type && strcmp(child->m_type, typeName) == 0) return child;
        if (child->m_data_meta && strcmp(dr_meta_name(child->m_data_meta), typeName) == 0) return child;
    }

    return NULL;
}

dp_req_t dp_req_brother_find(dp_req_t req, const char * typeName) {
    dp_req_t parent = req->m_parent;
    dp_req_t self = req;

    while(parent) {
        dp_req_t brother;

        TAILQ_FOREACH(brother, &parent->m_childs, m_brother) {
            if (brother == self) continue;

            if (brother->m_type && strcmp(brother->m_type, typeName) == 0) return brother;
            if (brother->m_data_meta && strcmp(dr_meta_name(brother->m_data_meta), typeName) == 0) return brother;
        }

        self = parent;
        parent = parent->m_parent;
    }

    return NULL;
}

const char * dp_req_type(dp_req_t req) {
    assert(req);
    return req->m_type;
}

void * dp_req_data(dp_req_t req) {
    assert(req);
    return req->m_data;
}

size_t dp_req_capacity(dp_req_t req) {
    return req->m_data_capacity;
}

size_t dp_req_size(dp_req_t req) {
    return req->m_data_size;
}

int dp_req_set_size(dp_req_t req, size_t size) {
    if (size >req->m_data_capacity) return -1;
    req->m_data_size = size;
    return 0;
}

void dp_req_set_buf(dp_req_t req, void * buf, size_t capacity) {
    if (buf) {
        req->m_data = buf;
        req->m_data_capacity = capacity;
        req->m_data_size = 0;
    }
    else {
        req->m_data = req + 1;
        req->m_data_capacity = req->m_capacity;
        req->m_data_size = 0;
    }
}

LPDRMETA dp_req_meta(dp_req_t req) {
    return req->m_data_meta;
}

void dp_req_set_meta(dp_req_t req, LPDRMETA meta) {
    req->m_data_meta = meta; 
}

void dp_req_data_clear(dp_req_t req) {
    req->m_data_size = 0;
    req->m_data_meta = NULL;
}

void dp_req_set_dumper(dp_req_t req, dp_req_dump_fun_t dumper) {
    req->m_dumper = dumper;
}

const char * dp_req_dump(dp_req_t req, mem_buffer_t buffer) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    dp_req_print(req, (write_stream_t)&stream, 0);
    stream_putc((write_stream_t)&stream, 0);
    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

void dp_req_print(dp_req_t req, write_stream_t s, int ident) {
    stream_putc_count(s, ' ', ident);

    if (req->m_dumper) {
        req->m_dumper(req, s);
    }
    else if (req->m_data_meta) {
        stream_printf(s, "%s(%d bytes): ", dr_meta_name(req->m_data_meta), (int)dp_req_size(req));
        /* struct error_monitor em; */
        /* cpe_error_monitor_init(&em, cpe_error_log_to_consol_and_flush, NULL); */
        dr_json_print(s, dp_req_data(req), dp_req_size(req), req->m_data_meta, DR_JSON_PRINT_MINIMIZE, NULL);
    }
    else if (req->m_type) {
        stream_printf(s, "%s(%d bytes)", cpe_hs_data(req->m_type), (int)req->m_data_size);
    }
    else {
        stream_printf(s, "(%d bytes)", (int)req->m_data_size);
    }
    
    stream_putc(s, '\n');
}

CPE_HS_DEF_VAR(dp_req_data_type_name, "dp_req_data");
