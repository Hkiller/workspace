#include <assert.h>
#include "cpe/pal/pal_time.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/random.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_data.h"
#include "render/utils/ui_string_table_builder.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_i.h"

static int ui_ed_src_do_load(ui_ed_mgr_t ed_mgr, ui_ed_src_t ed_src);

ui_ed_src_t ui_ed_src_create_i(ui_ed_mgr_t ed_mgr, ui_data_src_t data_src, ui_ed_src_state_t state) {
    ui_ed_src_t ed_src;

    assert(data_src);
    assert(ed_mgr);

    ed_src = mem_alloc(ed_mgr->m_alloc, sizeof(struct ui_ed_src));
    if (ed_src == NULL) {
        CPE_ERROR(ed_mgr->m_em, "create obj cache fail!");
        return NULL;
    }

    ed_src->m_ed_mgr = ed_mgr;
    ed_src->m_root_obj = NULL;
    ed_src->m_data_src = data_src;
    ed_src->m_data.src_id = ui_data_src_id(data_src);
    ed_src->m_state = state;

    cpe_hash_entry_init(&ed_src->m_hh_for_mgr);
    if (cpe_hash_table_insert_unique(&ed_mgr->m_ed_srcs, ed_src) != 0) {
        CPE_ERROR(ed_mgr->m_em, "create obj insert fail, src id %d duplicate!", ed_src->m_data.src_id);
        mem_free(ed_mgr->m_alloc, ed_src);
        return NULL;
    }

    TAILQ_INIT(&ed_src->m_objs);
    ed_src->m_root_obj =
        ui_ed_obj_create_i(ed_src, NULL, ui_ed_obj_type_src, ed_src, &ed_src->m_data, sizeof(ed_src->m_data));
    if (ed_src->m_root_obj == NULL) {
        CPE_ERROR(ed_mgr->m_em, "create obj cache fail!");
        cpe_hash_table_remove_by_ins(&ed_mgr->m_ed_srcs, ed_src);
        mem_free(ed_mgr->m_alloc, ed_src);
        return NULL;
    }

    return ed_src;
}

void ui_ed_src_free_i(ui_ed_src_t ed_src) {
    ui_ed_mgr_t ed_mgr;

    ed_mgr = ed_src->m_ed_mgr;

    ui_ed_obj_free_i(ed_src->m_root_obj);
    ed_src->m_root_obj = NULL;
    assert(TAILQ_EMPTY(&ed_src->m_objs));

    cpe_hash_table_remove_by_ins(&ed_mgr->m_ed_srcs, ed_src);

    mem_free(ed_mgr->m_alloc, ed_src);
}

uint32_t ui_ed_src_id(ui_ed_src_t src) {
    return ui_data_src_id(src->m_data_src);
}

ui_data_src_type_t ui_ed_src_type(ui_ed_src_t src) {
    return ui_data_src_type(src->m_data_src);
}

ui_ed_src_state_t ui_ed_src_state(ui_ed_src_t ed_src) {
    return ed_src->m_state;
}

void * ui_ed_src_product(ui_ed_src_t ed_src) {
    return ui_data_src_product(ed_src->m_data_src);
}

ui_data_src_t ui_ed_src_data(ui_ed_src_t src) {
    return src->m_data_src;
}

ui_ed_src_t ui_ed_src_find_by_id(ui_ed_mgr_t ed_mgr, uint32_t id) {
    ui_data_src_t data_src;

    data_src = ui_data_src_find_by_id(ed_mgr->m_data_mgr, id);
    if (data_src == NULL) return NULL;

    return ui_ed_src_find_by_data(ed_mgr, data_src);
}

ui_ed_src_t ui_ed_src_find_by_path(ui_ed_mgr_t ed_mgr, const char * path, ui_data_src_type_t type) {
    ui_data_src_t data_src;

    data_src = ui_data_src_find_by_path(ed_mgr->m_data_mgr, path, type);
    if (data_src == NULL) {
        return NULL;
    }

    return ui_ed_src_find_by_data(ed_mgr, data_src);
}

ui_ed_src_t ui_ed_src_find_by_data(ui_ed_mgr_t ed_mgr, ui_data_src_t data_src) {
    ui_ed_src_t ed_src;
    struct ui_ed_src key;

    assert(data_src);

    key.m_data_src = data_src;

    ed_src = cpe_hash_table_find(&ed_mgr->m_ed_srcs, &key);
    if (ed_src == NULL) {
        ed_src = ui_ed_src_create_i(ed_mgr, data_src, ui_ed_src_state_normal);
        if (ed_src == NULL) {
            CPE_ERROR(ed_mgr->m_em, "create ed_src fail");
            return NULL;
        }

        if (ui_data_src_is_loaded(ed_src->m_data_src)) {
            int r = ui_ed_src_do_load(ed_mgr, ed_src);
            if (r) {
                CPE_ERROR(ed_mgr->m_em, "create ed_src: do load fail");
                ui_ed_src_free_i(ed_src);
                ed_src = NULL;
            }
        }
    }

    return ed_src;
}

static uint32_t ui_ed_src_gen_guid(uint32_t sd, const char * file_name) {
    struct timeval tv;
    struct timezone tz;
    char buf[128];
	const uint32_t m = 0x5bd1e995;
	const uint32_t r = 24;

    uint32_t len;
    char * data;
	uint32_t h;

    gettimeofday(&tv, &tz);

    len = snprintf(
        buf, sizeof(buf), "%s%d%d%d%d",
        file_name, (int)tv.tv_sec, (int)tv.tv_usec, (int)tz.tz_minuteswest, (int)tz.tz_dsttime);
    data = buf;
    h = sd ^ len;

	while (len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m; 
        k ^= k >> r; 
        k *= m; 

        h *= m; 
        h ^= k;

        data += 4;
        len  -= 4;
	}

	switch (len) {
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
        h *= m;
	};

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

ui_ed_src_t ui_ed_src_new(ui_ed_mgr_t ed_mgr, const char * path, ui_data_src_type_t type) {
    ui_data_src_t data_src;
    ui_ed_src_t ed_src;
    uint32_t i;
    uint32_t GEN_ID_RETRY_MAX = 256;
    ui_ed_obj_t obj;

    data_src = ui_data_src_create_relative(ed_mgr->m_data_mgr, type, path);
    if (data_src == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed create data src at %s fail", path);
        return NULL;
    }

    if (ui_data_src_init(data_src) != 0) {
        CPE_ERROR(ed_mgr->m_em, "ed init data src at %s fail", path);
        ui_data_src_free(data_src);
        return NULL;
    }

    for(i = 0; i < GEN_ID_RETRY_MAX; ++i) {
        uint32_t src_id = ui_ed_src_gen_guid(cpe_rand_dft(0), path);

        if (ui_data_src_set_id(data_src, src_id) == 0) {
            break;
        }
    }

    if (i >= GEN_ID_RETRY_MAX) {
        CPE_ERROR(ed_mgr->m_em, "ed init data src at %s, gen id fail", path);
        ui_data_src_free(data_src);
        return NULL;
    }

    ed_src = ui_ed_src_create_i(ed_mgr, data_src, ui_ed_src_state_new);
    if (ed_src == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed create ed src at %s fail", path);
        ui_data_src_free(data_src);
        return NULL;
    }

    if (ui_ed_src_do_load(ed_mgr, ed_src) != 0) {
        CPE_ERROR(ed_mgr->m_em, "ed create ed src at %s: do load fail", path);
        ui_ed_src_free_i(ed_src);
        ui_data_src_free(data_src);
        return NULL;
    }

    TAILQ_FOREACH(obj, &ed_src->m_objs, m_next_for_src) {
        if (obj == ed_src->m_root_obj) continue;
        dr_meta_set_defaults(obj->m_data, obj->m_data_capacity, obj->m_meta->m_data_meta, DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);
    }

    return ed_src;
}

void ui_ed_src_delete(ui_ed_src_t ed_src) {
    ed_src->m_state = ui_ed_src_state_removed;
}

void ui_ed_src_touch(ui_ed_src_t ed_src) {
    ed_src->m_state = ui_ed_src_state_changed;
}

int ui_ed_src_init(ui_ed_src_t ed_src) {
    ui_ed_obj_t obj;
    ui_ed_mgr_t ed_mgr = ed_src->m_ed_mgr;
    int rv;

    ui_ed_obj_free_childs_i(ed_src->m_root_obj);

    if ((rv = ui_data_src_init(ed_src->m_data_src)) != 0) {
        CPE_ERROR(ed_mgr->m_em, "ed src init: data init fail, rv=%d", rv);
        return -1;
    }

    if ((rv = ui_ed_src_do_load(ed_mgr, ed_src)) != 0) {
        CPE_ERROR(ed_mgr->m_em, "ed src init: do load fail, rv=%d", rv);
        return -1;
    }

    if (ed_src->m_state == ui_ed_src_state_normal) {
        ed_src->m_state = ui_ed_src_state_changed;
    }

    TAILQ_FOREACH(obj, &ed_src->m_objs, m_next_for_src) {
        if (obj == ed_src->m_root_obj) continue;
        dr_meta_set_defaults(obj->m_data, obj->m_data_capacity, obj->m_meta->m_data_meta, DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);
    }

    return 0;
}

int ui_ed_src_load(ui_ed_src_t ed_src, error_monitor_t em) {
    ui_ed_mgr_t ed_mgr = ed_src->m_ed_mgr;
    int r;

    assert(ed_src->m_data_src);

    if (ui_data_src_load_state(ed_src->m_data_src) == ui_data_src_state_loaded) return 0;

    if ((r = ui_data_src_load(ed_src->m_data_src, em))) {
        CPE_ERROR(
            em, "%s: ed load src: load data fail, rv=%d",
            ui_data_src_path_dump(&ed_mgr->m_dump_buffer, ed_src->m_data_src), r);
        return r;
    }

    return ui_ed_src_do_load(ed_mgr, ed_src);
}

void ui_ed_src_free_all(ui_ed_mgr_t ed_mgr) {
    struct cpe_hash_it src_it;
    ui_ed_src_t src;

    cpe_hash_it_init(&src_it, &ed_mgr->m_ed_srcs);

    src = cpe_hash_it_next(&src_it);
    while (src) {
        ui_ed_src_t next = cpe_hash_it_next(&src_it);
        ui_ed_src_free_i(src);
        src = next;
    }
}

void ui_ed_src_unload(ui_ed_src_t ed_src) {
    //ui_ed_mgr_t ed_mgr = ed_src->m_ed_mgr;

    ui_ed_src_free_i(ed_src);
}

ui_ed_obj_t ui_ed_src_root_obj(ui_ed_src_t ed_src) {
    return ed_src->m_root_obj;
}

void ui_ed_src_path_print(write_stream_t s, ui_ed_src_t ed_src) {
    ui_data_src_path_print_to(s, ed_src->m_data_src, ui_data_mgr_src_root(ed_src->m_ed_mgr->m_data_mgr));
}

const char * ui_ed_src_path_dump(mem_buffer_t buffer, ui_ed_src_t ed_src) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    ui_data_src_path_print_to((write_stream_t)&stream, ed_src->m_data_src, ui_data_mgr_src_root(ed_src->m_ed_mgr->m_data_mgr));

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

int ui_ed_src_save(ui_ed_src_t ed_src, const char * root, error_monitor_t em) {
    switch(ed_src->m_state) {
    case ui_ed_src_state_normal:
        return 0;
    case ui_ed_src_state_new:
    case ui_ed_src_state_changed: {
        int r = ui_data_src_save(ed_src->m_data_src, root, em);
        if (r) {
            CPE_ERROR(
                em, "ed save src %s fail, rv=%d",
                ui_data_src_path_dump(&ed_src->m_ed_mgr->m_dump_buffer, ed_src->m_data_src), r);
            return r;
        }
        else {
            ed_src->m_state = ui_ed_src_state_normal;
            return 0;
        }
    }
    case ui_ed_src_state_removed:
        CPE_ERROR(
            em, "ed save src %s fail, already removed",
            ui_data_src_path_dump(&ed_src->m_ed_mgr->m_dump_buffer, ed_src->m_data_src));
        return -1;
    default:
        CPE_INFO(
            em, "ed save src %s fail, unknown state %d",
            ui_data_src_path_dump(&ed_src->m_ed_mgr->m_dump_buffer, ed_src->m_data_src), ed_src->m_state);
        return 0;
    }
}

ui_ed_src_t ui_ed_src_check_create(ui_ed_mgr_t ed_mgr, const char * path, ui_data_src_type_t type) {
    ui_ed_src_t ed_src;

    ed_src = ui_ed_src_find_by_path(ed_mgr, path, type);
    if (ed_src == NULL) {
        ed_src = ui_ed_src_new(ed_mgr, path, type);
        if (ed_src == NULL) {
            CPE_ERROR(ed_mgr->m_em, "check create src at %s: create fail!", path);
            return NULL;
        }
    }
    else {
        assert(ui_ed_src_type(ed_src) == type);
        if (ui_ed_src_load(ed_src, ed_mgr->m_em) != 0) {
            CPE_INFO(ed_mgr->m_em, "check create src at %s: load fail reinit!", path);
            if (ui_ed_src_init(ed_src) != 0) {
                CPE_ERROR(ed_mgr->m_em, "check create src at %s: reinit fail!", path);
                return NULL;
            }
        }
    }

    assert(ed_src);
    return ed_src;
}

void ui_ed_src_strings_clear(ui_ed_src_t ed_src) {
    if (ed_src->m_state == ui_ed_src_state_normal) {
        ed_src->m_state = ui_ed_src_state_changed;
    }

    ui_data_src_strings_clear(ed_src->m_data_src);
}

uint32_t ui_ed_src_msg_alloc(ui_ed_src_t ed_src, const char * msg) {
    if (ed_src->m_state == ui_ed_src_state_normal) {
        ed_src->m_state = ui_ed_src_state_changed;
    }

    return ui_data_src_msg_alloc(ed_src->m_data_src, msg);
}

void ui_ed_src_msg_remove(ui_ed_src_t ed_src, uint32_t msg_id) {
    if (ed_src->m_state == ui_ed_src_state_normal) {
        ed_src->m_state = ui_ed_src_state_changed;
    }

    return ui_data_src_msg_remove(ed_src->m_data_src, msg_id);
}

uint32_t ui_ed_src_msg_update(ui_ed_src_t ed_src, uint32_t id, const char * msg) {
    if (ed_src->m_state == ui_ed_src_state_normal) {
        ed_src->m_state = ui_ed_src_state_changed;
    }

    return ui_data_src_msg_update(ed_src->m_data_src, id, msg);
}

const char * ui_ed_src_msg(ui_ed_src_t ed_src, uint32_t id) {
    return ui_data_src_msg(ed_src->m_data_src, id);
}

uint32_t ui_ed_src_hash(ui_ed_src_t src) {
    return (uint32_t)((ptr_int_t)src->m_data_src);
}

int ui_ed_src_eq(ui_ed_src_t l, ui_ed_src_t r) {
    return l->m_data_src == r->m_data_src;
}

static int ui_ed_src_do_load(ui_ed_mgr_t ed_mgr, ui_ed_src_t ed_src) {
    struct ui_ed_src_meta * src_meta;
    ui_data_src_type_t src_type = ui_data_src_type(ed_src->m_data_src);

    assert((uint8_t)src_type >= (uint8_t)UI_DATA_SRC_TYPE_MIN && (uint8_t)src_type < (uint8_t)UI_DATA_SRC_TYPE_MAX);

    src_meta = &ed_mgr->m_src_metas[src_type - UI_DATA_SRC_TYPE_MIN];

    if (src_meta->m_load_fun == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed load src: not support src type %d", src_type);
        ui_data_src_unload(ed_src->m_data_src);
        return -1;
    }

    if (src_meta->m_load_fun(ed_src) != 0) {
        CPE_ERROR(ed_mgr->m_em, "ed load src: load objs fail");
        ui_data_src_unload(ed_src->m_data_src);
        return -1;
    }

    return 0;
}
