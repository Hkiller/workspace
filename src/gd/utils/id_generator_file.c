#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/range.h"
#include "cpe/utils/file.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/error.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/utils/id_generator.h"
#include "gd/utils/id_generator_file.h"

struct gd_id_file_generator {
    struct gd_id_generator m_gen;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    const char * m_save_dir;
    uint16_t m_global_id;
    uint32_t m_one_alloc_size;
    int m_debug;

    struct cpe_hash_table m_id_infos;
};

struct generator_info {
    char m_name[32]; 
    struct cpe_range m_range;
    struct cpe_hash_entry m_hh;
};

static uint32_t generator_info_hash(const struct generator_info * info) {
    return cpe_hash_str(info->m_name, strlen(info->m_name));
}

int generator_info_eq(const struct generator_info * l, const struct generator_info * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

static void gd_id_file_generator_clear(nm_node_t node);
static int gd_id_file_generator_write_back(gd_id_file_generator_t id_generator, const char * id_name, ptr_int_t new_begin);
static int gd_id_file_generator_gen(gd_id_t * r, gd_id_generator_t generator, const char * id_name);

struct nm_node_type s_nm_node_type_gd_id_file_generator = {
    "gd_id_file_generator",
    gd_id_file_generator_clear
};

gd_id_file_generator_t
gd_id_file_generator_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    gd_id_file_generator_t generator;
    nm_node_t generator_node;

    assert(app);

    generator_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct gd_id_file_generator));
    if (generator_node == NULL) return NULL;

    generator = (gd_id_file_generator_t)nm_node_data(generator_node);

    generator->m_gen.magic = (uint32_t)GD_ID_GENERATOR_MAGIC;
    generator->m_gen.gen_fun = gd_id_file_generator_gen;
    generator->m_gen.app = app;
    generator->m_alloc = alloc ? alloc : gd_app_alloc(app);
    generator->m_em = em ? em : gd_app_em(app);
    generator->m_one_alloc_size = 2046;
    generator->m_save_dir = NULL;
    generator->m_debug = 0;
    generator->m_global_id = 0;

    if (cpe_hash_table_init(
            &generator->m_id_infos,
            alloc,
            (cpe_hash_fun_t) generator_info_hash,
            (cpe_hash_eq_t) generator_info_eq,
            CPE_HASH_OBJ2ENTRY(generator_info, m_hh),
            -1) != 0)
    {
        nm_node_free(generator_node);
        return NULL;
    }

    nm_node_set_type(generator_node, &s_nm_node_type_gd_id_file_generator);

    return generator;
}

static void gd_id_file_generator_clear(nm_node_t node) {
    gd_id_file_generator_t generator;
    struct cpe_hash_it id_info_it;
    struct generator_info * id_info;

    generator = (gd_id_file_generator_t)nm_node_data(node);


    cpe_hash_it_init(&id_info_it, &generator->m_id_infos);

    id_info = cpe_hash_it_next(&id_info_it);
    while (id_info) {
        struct generator_info * next = cpe_hash_it_next(&id_info_it);
        cpe_hash_table_remove_by_ins(&generator->m_id_infos, id_info);
        mem_free(generator->m_alloc, id_info);
        id_info = next;
    }
    cpe_hash_table_fini(&generator->m_id_infos);

    if (generator->m_save_dir) {
        mem_free(generator->m_alloc, (void*)generator->m_save_dir);
        generator->m_save_dir = NULL;
    }
}

void gd_id_file_generator_free(gd_id_file_generator_t generator) {
    nm_node_t generator_node;
    assert(generator);

    generator_node = nm_node_from_data(generator);
    if (nm_node_type(generator_node) != &s_nm_node_type_gd_id_file_generator) return;
    nm_node_free(generator_node);
}

gd_id_file_generator_t
gd_id_file_generator_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gd_id_file_generator) return NULL;
    return (gd_id_file_generator_t)nm_node_data(node);
}

gd_id_file_generator_t
gd_id_file_generator_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return NULL;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gd_id_file_generator) return NULL;
    return (gd_id_file_generator_t)nm_node_data(node);
}

gd_app_context_t gd_id_file_generator_app(gd_id_file_generator_t generator) {
    return generator->m_gen.app;
}

const char * gd_id_file_generator_name(gd_id_file_generator_t generator) {
    return nm_node_name(nm_node_from_data(generator));
}

cpe_hash_string_t
gd_id_file_generator_name_hs(gd_id_file_generator_t generator) {
    return nm_node_name_hs(nm_node_from_data(generator));
}

const char * gd_id_file_generator_save_dir(gd_id_file_generator_t generator) {
    return generator->m_save_dir;
}

int gd_id_file_generator_set_save_dir(gd_id_file_generator_t generator, const char * file) {
    char * new_save_dir;

    new_save_dir = NULL;
    if (file) {
        size_t size;
        size = strlen(file) + 1;

        new_save_dir = mem_alloc(generator->m_alloc, size);
        if (new_save_dir == NULL) return -1;
        memcpy(new_save_dir, file, size);
    }

    if (generator->m_save_dir) {
        mem_free(generator->m_alloc, (void*)generator->m_save_dir);
    }

    generator->m_save_dir = new_save_dir;
    return 0;
}

uint32_t gd_id_file_generator_once_alloc_size(gd_id_file_generator_t generator) {
    return generator->m_one_alloc_size;
}

void gd_id_file_generator_set_once_alloc_size(gd_id_file_generator_t generator, uint32_t once_alloc_size) {
    generator->m_one_alloc_size = once_alloc_size;
}

static int gd_id_file_generator_read_range_start(ptr_int_t * result, gd_id_file_generator_t generator, const char * id_name) {
    char path[128];
    struct mem_buffer buffer;
    ssize_t size;
    char * read_end_pos;

    if (generator->m_save_dir == NULL) return -1;

    snprintf(path, sizeof(path), "%s/%s.id", generator->m_save_dir, id_name);

    if (!file_exist(path, generator->m_em)) {
        *result = 1;
        return 0;
    }

    mem_buffer_init(&buffer, NULL);

    size = file_load_to_buffer(&buffer, path, generator->m_em);
    if (size < 0) {
        CPE_ERROR(
            generator->m_em,
            "gd_id_file_generator %s: open %s to read id range fail!", 
            gd_id_file_generator_name(generator), path);
        mem_buffer_clear(&buffer);
        return -1;
    } 

    mem_buffer_append_char(&buffer, 0);

    read_end_pos = 0;
    *result = strtol((const char *)mem_buffer_make_continuous(&buffer, 0), &read_end_pos, 10);
    if (read_end_pos == 0 || *read_end_pos != 0) {
        CPE_ERROR(
            generator->m_em,
            "gd_id_file_generator %s: read next id from %s fail, input is '%s'!", 
            gd_id_file_generator_name(generator), path,
            (const char *)mem_buffer_make_continuous(&buffer, 0));
        mem_buffer_clear(&buffer);
        return -1;
    }

    mem_buffer_clear(&buffer);

    if (generator->m_debug) {
        CPE_INFO(
            generator->m_em, "gd_id_file_generator %s: load new range success: start at %d",
            gd_id_file_generator_name(generator), (int)*result);
    }

    return 0;
}

static int gd_id_file_generator_write_back(gd_id_file_generator_t id_generator, const char * id_name, ptr_int_t new_begin) {
    char path[128];
    char buf[20];

    snprintf(buf, sizeof(buf), "%d", (int)new_begin);
    snprintf(path, sizeof(path), "%s/%s.id", id_generator->m_save_dir, id_name);

    if (file_write_from_str(path, buf, id_generator->m_em) <= 0) {
        CPE_ERROR(
            id_generator->m_em, "gd_id_file_generator %s: write back to %s fail!", 
            gd_id_file_generator_name(id_generator), path);
        return -1;
    }

    if (id_generator->m_debug) {
        CPE_INFO(
            id_generator->m_em, "gd_id_file_generator %s: save new range success: value=%d, file=%s",
            gd_id_file_generator_name(id_generator), (int)new_begin, path);
    }

    return 0;
}

static int gd_id_file_generator_load_next_range(gd_id_file_generator_t generator, struct generator_info * id_info) {
    ptr_int_t new_begin;

    if (generator->m_save_dir == 0) {
        id_info->m_range.m_start = id_info->m_range.m_end;
        id_info->m_range.m_end += generator->m_one_alloc_size;
        return 0;
    }

    if (gd_id_file_generator_read_range_start(&new_begin, generator, id_info->m_name) != 0) return -1;

    if(new_begin < 0) {
        CPE_ERROR(
            generator->m_em, "gd_id_file_generator %s: id overflow!", 
            gd_id_file_generator_name(generator));
        return -1;
    }

    if (gd_id_file_generator_write_back(generator, id_info->m_name, new_begin + generator->m_one_alloc_size) != 0) return -1;

    id_info->m_range.m_start = new_begin;
    id_info->m_range.m_end = new_begin + generator->m_one_alloc_size;

    return 0;
}

static int gd_id_file_generator_gen(gd_id_t * r, gd_id_generator_t gen, const char * id_name) {
    gd_id_file_generator_t generator = (gd_id_file_generator_t)gen;
    struct generator_info key;
    struct generator_info * id_info;

    assert(r);
    assert(generator);
    assert(generator->m_gen.gen_fun == gd_id_file_generator_gen);

    cpe_str_dup(key.m_name, sizeof(key.m_name), id_name);
    id_info = (struct generator_info *)cpe_hash_table_find(&generator->m_id_infos, &key);
    if (id_info == NULL) {
        id_info = mem_alloc(generator->m_alloc, sizeof(struct generator_info));
        if (id_info == NULL) {
            CPE_ERROR(
                generator->m_em, "%s: %s: alloc id_info fail!",
                gd_id_file_generator_name(generator), id_name);
            return -1;
        }

        cpe_str_dup(id_info->m_name, sizeof(id_info->m_name), id_name);
        id_info->m_range.m_start = 1;
        id_info->m_range.m_end = 1;
        cpe_hash_entry_init(&id_info->m_hh);

        if (cpe_hash_table_insert_unique(&generator->m_id_infos, id_info) != 0) {
            CPE_ERROR(
                generator->m_em, "%s: %s: insert id_info fail!",
                gd_id_file_generator_name(generator), id_name);
            mem_free(generator->m_alloc, id_info);
            return -1;
        }
    }

    assert(id_info);
    if (cpe_range_size(id_info->m_range) <= 0) {
        if (gd_id_file_generator_load_next_range(generator, id_info) != 0) {
            return -1;
        }
    }

    *r =
        ((gd_id_t)generator->m_global_id << 48)
        | (((gd_id_t)id_info->m_range.m_start) & 0xFFFFFFFFFFFFLL);

    ++id_info->m_range.m_start;

    return 0;
}

EXPORT_DIRECTIVE
int gd_id_file_generator_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    gd_id_file_generator_t gd_id_file_generator;

    gd_id_file_generator = gd_id_file_generator_create(app, gd_app_module_name(module), NULL, NULL);
    if (gd_id_file_generator == NULL) return -1;

    if (gd_id_file_generator_set_save_dir(gd_id_file_generator, cfg_get_string(cfg, "save-dir", NULL)) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: set save-dir fail!", gd_app_module_name(module));
        gd_id_file_generator_free(gd_id_file_generator);
        return -1;
    }

    gd_id_file_generator_set_once_alloc_size(
        gd_id_file_generator,
        cfg_get_uint32(cfg, "once-alloc-size", gd_id_file_generator->m_one_alloc_size));

    gd_id_file_generator->m_debug = cfg_get_uint32(cfg, "debug", 0);

    if (gd_id_file_generator->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: save-dir=%s, once-alloc-size=%d!",
            gd_app_module_name(module), gd_id_file_generator->m_save_dir, gd_id_file_generator->m_one_alloc_size);
    }

    return 0;
}

EXPORT_DIRECTIVE
void gd_id_file_generator_app_fini(gd_app_context_t app, gd_app_module_t module) {
    gd_id_file_generator_t gd_id_file_generator;

    gd_id_file_generator = gd_id_file_generator_find_nc(app, gd_app_module_name(module));
    if (gd_id_file_generator) {
        gd_id_file_generator_free(gd_id_file_generator);
    }
}

