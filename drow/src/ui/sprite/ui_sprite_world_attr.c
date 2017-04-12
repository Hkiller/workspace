#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_data_entry.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_world_data_i.h"
#include "ui_sprite_data_build_i.h"

dr_data_entry_t ui_sprite_world_find_attr(dr_data_entry_t buff, ui_sprite_world_t world, const char * path) {
    ui_sprite_world_data_t world_data;

    assert(buff);

    TAILQ_FOREACH(world_data, &world->m_datas, m_next) {
        int data_start_pos;

        buff->m_entry = dr_meta_find_entry_by_path_ex(world_data->m_data.m_meta, path, &data_start_pos);
        if (buff->m_entry == NULL) continue;

        buff->m_data = ((char*)world_data->m_data.m_data) + data_start_pos;
        buff->m_size = dr_entry_element_size(buff->m_entry);

        return buff;
    }

    return NULL;
}

int ui_sprite_world_set_attr(
    ui_sprite_world_t world, const char * path, const char * value, dr_data_source_t data_source)
{
    ui_sprite_repository_t repo = world->m_repo;
    struct dr_data_entry attr_buf;
    dr_data_entry_t attr;
    char * tmp_value;

    attr = ui_sprite_world_find_attr(&attr_buf, world, path);
    if (attr == NULL) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr %s: attr not exist!",
            world->m_name, path);
        return -1;
    }

    tmp_value = cpe_str_mem_dup(repo->m_alloc, value);
    if (tmp_value == NULL) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr %s: clone tmp str fail!",
            world->m_name, path);
        return -1;
    }

    if (ui_sprite_data_build(attr, tmp_value, world, NULL, data_source) != 0) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr %s: set from %s fail!",
            world->m_name, path, value);
        mem_free(repo->m_alloc, tmp_value);
        return -1;
    }

    mem_free(repo->m_alloc, tmp_value);

    if (world->m_repo->m_debug >= 2) {
        CPE_INFO(
            repo->m_em, "world %s: attr %s ==> %s",
            world->m_name, path,
            dr_entry_to_string(&repo->m_dump_buffer, attr->m_data, attr->m_entry));
    }

    return 0;
}

static int ui_sprite_world_set_attr_one(ui_sprite_repository_t repo, ui_sprite_world_t world, char * def, dr_data_source_t data_source) {
    char * p = strchr(def, '=');
    char * arg_name = def;
    char * arg_value;
    struct dr_data_entry to_buff;
    dr_data_entry_t to;

    if (p == NULL) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr one: def %s format error!",
            world->m_name, def);
        return -1;
    }

    *((char*)cpe_str_trim_tail(p, arg_name)) = 0;
    arg_value = (char*)cpe_str_trim_head(p + 1);

    to = ui_sprite_world_find_attr(&to_buff, world, arg_name);
    if (to == NULL) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr one: attr %s: attr not exist!",
            world->m_name, arg_name);
        return -1;
    }

    if (ui_sprite_data_build(to, arg_value, world, NULL, data_source) != 0) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr one: attr %s: set value fail!",
            world->m_name, arg_name);
        return -1;
    }

    if (repo->m_debug) {
        if (dr_entry_type(to->m_entry) > CPE_DR_TYPE_COMPOSITE) {
            CPE_INFO(
                repo->m_em, "world %s: attr %s ==> %s",
                world->m_name, arg_name,
                dr_entry_to_string(&repo->m_dump_buffer, to->m_data, to->m_entry));
        }
        else {
            CPE_INFO(
                repo->m_em, "world %s: attr %s ==> %s",
                world->m_name, arg_name,
                dr_json_dump_inline(&repo->m_dump_buffer, to->m_data, to->m_size, dr_entry_ref_meta(to->m_entry)));
        }
    }

    return 0;
}

int ui_sprite_world_bulk_set_attrs(
    ui_sprite_world_t world, const char * defs, dr_data_source_t data_source)
{
    ui_sprite_repository_t repo = world->m_repo;
    char * allocked_tmp_args;
    char * tmp_args;
    char * p;

    allocked_tmp_args = cpe_str_mem_dup(repo->m_alloc, defs);
    if (allocked_tmp_args == NULL) {
        CPE_ERROR(
            repo->m_em, "world %s: bulk set attrs: clone tmp str fail!",
            world->m_name);
        return -1;
    }

    tmp_args = allocked_tmp_args;
    while((p = (char*)cpe_str_char_not_in_pair(tmp_args, ',', "{[(", ")]}"))) {
        char * arg_begin = tmp_args;
        char * arg_end = (char*)cpe_str_trim_tail(p, arg_begin);

        *arg_end = 0;
        tmp_args = (char*)cpe_str_trim_head(p + 1);

        if (ui_sprite_world_set_attr_one(repo, world, arg_begin, data_source) != 0) {
            mem_free(repo->m_alloc, allocked_tmp_args);
            return -1;
        }
    }

    if (*tmp_args != 0) {
        *(char *)cpe_str_trim_tail(tmp_args + strlen(tmp_args), tmp_args) = 0;


        if (ui_sprite_world_set_attr_one(repo, world, tmp_args, data_source) != 0) {
            mem_free(repo->m_alloc, allocked_tmp_args);
            return -1;
        }
    }

    mem_free(repo->m_alloc, allocked_tmp_args);
    return 0;
}

dr_data_source_t ui_sprite_world_build_data_source(ui_sprite_world_t world, dr_data_source_t data_source_buf, uint16_t buf_capacity) {
    int i = -1;
    ui_sprite_world_data_t world_data;

    TAILQ_FOREACH(world_data, &world->m_datas, m_next) {
        dr_data_source_t cur_node;

        if (i + 1 >= buf_capacity) break;
        
        cur_node = data_source_buf + i + 1;
        cur_node->m_next = NULL;
        cur_node->m_data = world_data->m_data;
        
        if (i >= 0) {
            (data_source_buf + i)->m_next = cur_node;
        }

        ++i;
    }

    return i >= 0 ? data_source_buf : NULL;
}

#define UI_SPRITE_WORLD_IMPL_ATTR_FUN(__type_name, __type, __fmt)            \
__type ui_sprite_world_get_attr_ ## __type_name(ui_sprite_world_t world, const char * path, __type dft) {\
    struct dr_data_entry buff;\
    dr_data_entry_t attr = ui_sprite_world_find_attr(&buff, world, path);\
    if (attr == NULL) {\
        CPE_ERROR(\
            world->m_repo->m_em, "world %s: get attr: attr %s: attr not exist!",\
            world->m_name, path);\
        return dft;\
    }\
    return dr_entry_read_with_dft_ ## __type_name(attr->m_data, attr->m_entry, dft);\
}\
int ui_sprite_world_set_attr_ ## __type_name(ui_sprite_world_t world, const char * path, __type v) { \
    struct dr_data_entry buff;\
    dr_data_entry_t attr = ui_sprite_world_find_attr(&buff, world, path);\
    int r;                                                              \
    if (attr == NULL) {\
        CPE_ERROR(\
            world->m_repo->m_em, "world %s: set attr: attr %s: attr not exist!",\
            world->m_name, path);\
        return -1;\
    }\
    if ((*(__type *)attr->m_data) == v) return 0;\
    r = dr_entry_set_from_ ## __type_name(attr->m_data, v, attr->m_entry, world->m_repo->m_em); \
    if (r != 0) {                                                   \
        CPE_ERROR(\
            world->m_repo->m_em, "world %s: set attr: attr %s: set attr from " __fmt " fail!",\
            world->m_name, path, v);                       \
        return -1;\
    }                                                               \
    if (world->m_repo->m_debug >= 2) {\
        CPE_INFO(\
            world->m_repo->m_em, "world %s: attr %s ==> %s",    \
            world->m_name, path,\
            dr_entry_to_string(&world->m_repo->m_dump_buffer, attr->m_data, attr->m_entry));\
    }\
    return 0;\
}

UI_SPRITE_WORLD_IMPL_ATTR_FUN(int8, int8_t, "%d")
UI_SPRITE_WORLD_IMPL_ATTR_FUN(uint8, uint8_t, "%u")

UI_SPRITE_WORLD_IMPL_ATTR_FUN(int16, int16_t, "%d")
UI_SPRITE_WORLD_IMPL_ATTR_FUN(uint16, uint16_t, "%u")

UI_SPRITE_WORLD_IMPL_ATTR_FUN(int32, int32_t, FMT_INT32_T)
UI_SPRITE_WORLD_IMPL_ATTR_FUN(uint32, uint32_t, FMT_UINT32_T)

UI_SPRITE_WORLD_IMPL_ATTR_FUN(int64, int64_t, FMT_INT64_T)
UI_SPRITE_WORLD_IMPL_ATTR_FUN(uint64, uint64_t, FMT_UINT64_T)

UI_SPRITE_WORLD_IMPL_ATTR_FUN(float, float, "%f")
UI_SPRITE_WORLD_IMPL_ATTR_FUN(double, double, "%f")

const char * ui_sprite_world_get_attr_string(ui_sprite_world_t world, const char * path, const char * dft) {
    struct dr_data_entry buff;
    dr_data_entry_t attr = ui_sprite_world_find_attr(&buff, world, path);
    if (attr == NULL) {
        CPE_ERROR(
            world->m_repo->m_em, "world %s: get attr: attr %s: attr not exist!",
            world->m_name, path);
        return dft;
    }
    return dr_entry_read_with_dft_string(attr->m_data, attr->m_entry, dft);
}

int ui_sprite_world_set_attr_string(ui_sprite_world_t world, const char * path, const char * v) {
    struct dr_data_entry buff;
    dr_data_entry_t attr = ui_sprite_world_find_attr(&buff, world, path);
    int r;
    if (attr == NULL) {
        CPE_ERROR(
            world->m_repo->m_em, "world %s: set attr: attr %s: attr not exist!",
            world->m_name, path);
        return -1;
    }

    if (strcmp((const char *)attr->m_data, v) == 0) return 0;

    r = dr_entry_set_from_string(attr->m_data, v, attr->m_entry, world->m_repo->m_em);
    if (r != 0) {
        CPE_ERROR(
            world->m_repo->m_em, "world %s: set attr: attr %s: set attr from %s fail!",
            world->m_name, path, v);
        return -1;
    }

    if (world->m_repo->m_debug >= 2) {
        CPE_INFO(
            world->m_repo->m_em, "world %s: attr %s ==> %s",
            world->m_name, path,
            dr_entry_to_string(&world->m_repo->m_dump_buffer, attr->m_data, attr->m_entry));
    }

    return 0;
}

int ui_sprite_world_set_attr_bin(ui_sprite_world_t world, const char * path, const void * v, LPDRMETA meta) {
    struct dr_data_entry buff;
    LPDRMETA des_meta;
    dr_data_entry_t attr = ui_sprite_world_find_attr(&buff, world, path);
    if (attr == NULL) {
        CPE_ERROR(
            world->m_repo->m_em, "world %s: set attr: attr %s: attr not exist!",
            world->m_name, path);
        return -1;
    }

    des_meta = dr_entry_ref_meta(attr->m_entry);
    if (des_meta == NULL) {
        CPE_ERROR(
            world->m_repo->m_em, "world %s: set attr: attr %s: attr is not composide, can`t set from bin!",
            world->m_name, path);
        return -1;
    }

    if (dr_meta_copy_same_entry(attr->m_data, attr->m_size, des_meta, v, dr_meta_size(meta), meta, 0, world->m_repo->m_em) <= 0) {
        CPE_ERROR(
            world->m_repo->m_em, "world %s: set attr: attr %s: copy same entry fail!",
            world->m_name, path);
        return -1;
    }

    if (world->m_repo->m_debug >= 2) {
        CPE_INFO(
            world->m_repo->m_em, "world %s: attr %s ==> %s",
            world->m_name, path,
            dr_json_dump_inline(&world->m_repo->m_dump_buffer, v, dr_meta_size(meta), meta));
    }

    return 0;
}

int ui_sprite_world_set_attr_value(ui_sprite_world_t world, const char * path, dr_value_t value) {
    ui_sprite_repository_t repo = world->m_repo;
    struct dr_data_entry buff;

    dr_data_entry_t attr = ui_sprite_world_find_attr(&buff, world, path);
    if (attr == NULL) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr: attr %s: attr not exist!",
            world->m_name, path);
        return -1;
    }

    if (dr_data_entry_set_from_value(attr, value, repo->m_em) != 0) {
        CPE_ERROR(
            repo->m_em, "world %s: set attr: attr %s: set from value fail!",
            world->m_name, path);
        return -1;
    }

    if (world->m_repo->m_debug >= 2) {
        if (value->m_type > CPE_DR_TYPE_COMPOSITE) {
            CPE_INFO(
                repo->m_em, "world %s: attr %s ==> %s",
                world->m_name, path,
                dr_ctype_to_string(&repo->m_dump_buffer, value->m_data, value->m_type));
        }
        else {
            CPE_INFO(
                repo->m_em, "world %s: attr %s ==> %s",
                world->m_name, path,
                dr_json_dump_inline(&repo->m_dump_buffer, value->m_data, value->m_size, value->m_meta));
        }
    }

    return 0;
}
