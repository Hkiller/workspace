#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui_sprite_render_obj_creator_i.h"

ui_sprite_render_obj_creator_t
ui_sprite_render_obj_creator_create(
    ui_sprite_render_module_t module, const char * name, ui_sprite_render_obj_create_fun_t fun, void * ctx)
{
    ui_sprite_render_obj_creator_t creator;
    size_t name_len = strlen(name) + 1;
    
    creator = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_render_obj_creator) + name_len);
    if (creator == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_obj_creator_create: alloc fail");
        return NULL;
    }

    memcpy(creator + 1, name, name_len);
    
    creator->m_module = module;
    creator->m_name = (void*)(creator + 1);
    creator->m_fun = fun;
    creator->m_ctx = ctx;

    cpe_hash_entry_init(&creator->m_hh);
    if (cpe_hash_table_insert(&module->m_obj_creators, creator) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_render_obj_creator_create: creator %s duplicate!", name);
        mem_free(module->m_alloc, creator);
        return NULL;
    }
    
    return creator;
}

void ui_sprite_render_obj_creator_free(ui_sprite_render_obj_creator_t creator) {
    cpe_hash_table_remove_by_ins(&creator->m_module->m_obj_creators, creator);
    mem_free(creator->m_module->m_alloc, creator);
}

ui_sprite_render_obj_creator_t
ui_sprite_render_obj_creator_find(ui_sprite_render_module_t module, const char * name) {
    struct ui_sprite_render_obj_creator key;
    key.m_name = name;

    return cpe_hash_table_find(&module->m_obj_creators, &key);
}

void ui_sprite_render_obj_creator_free_all(ui_sprite_render_module_t module) {
    struct cpe_hash_it creator_it;
    ui_sprite_render_obj_creator_t creator;

    cpe_hash_it_init(&creator_it, &module->m_obj_creators);

    creator = cpe_hash_it_next(&creator_it);
    while (creator) {
        ui_sprite_render_obj_creator_t next = cpe_hash_it_next(&creator_it);
        ui_sprite_render_obj_creator_free(creator);
        creator = next;
    }
}

int ui_sprite_render_module_register_obj_creator(
    ui_sprite_render_module_t module, const char * name, ui_sprite_render_obj_create_fun_t fun, void * ctx)
{
    if (ui_sprite_render_obj_creator_create(module, name, fun, ctx) == NULL) {
        return -1;
    }
    return 0;
}

int ui_sprite_render_module_unregister_obj_creator(ui_sprite_render_module_t module, const char * name) {
    ui_sprite_render_obj_creator_t creator;

    creator = ui_sprite_render_obj_creator_find(module, name);
    if (creator == NULL) return -1;

    ui_sprite_render_obj_creator_free(creator);
    return 0;
}

ui_runtime_render_obj_ref_t
ui_sprite_render_module_create_obj(
    ui_sprite_render_module_t module, ui_sprite_world_t world, uint32_t entity_id, const char * res, char ** left_args)
{
    ui_runtime_render_obj_ref_t render_obj_ref = NULL;
    const char * protocol;
    const char * protocol_end;
    const char * args = NULL;
    const char * args_end = NULL;
    
    if (res[0] == '[') {
        args = res + 1;
        args_end = cpe_str_char_not_in_pair(args, ']', "{[(", ")]}");
        if (args_end == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_render_module_create_obj: res %s format error!", res);
            return NULL;
        }

        protocol = args_end + 1;
    }
    else {
        protocol = res;
    }

    if ((protocol_end = strchr(protocol, ':'))) {
        char protocol_buf[32];
        size_t len = protocol_end - protocol;
        ui_sprite_render_obj_creator_t obj_creator;

        if (len < CPE_ARRAY_SIZE(protocol_buf) && cpe_str_char_range(protocol, protocol_end, '.') == NULL) {
            memcpy(protocol_buf, protocol, len);
            protocol_buf[len] = 0;

            obj_creator = ui_sprite_render_obj_creator_find(module, protocol_buf);
            if (obj_creator) {
                char * args = NULL;
                
                if (protocol != res) {
                    mem_buffer_clear_data(&module->m_tmp_buffer);
                    args = mem_buffer_strdup_len(&module->m_tmp_buffer, res + 1, protocol - res - 2);
                }
                
                if (obj_creator->m_fun(&render_obj_ref, obj_creator->m_ctx, world, entity_id, protocol_end + 1) != 0) {
                    CPE_ERROR(
                        module->m_em, "ui_sprite_render_module_create_obj: create render obj ref protocol=%s, res=%s fail!",
                        protocol_buf, protocol_end + 1);
                    return NULL;
                }

                if (args) {
                    ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);
                    const char * str_value;
                    
                    if ((str_value = cpe_str_read_and_remove_arg(args, "control", ',', '='))) {
                        ui_runtime_render_obj_ref_set_is_updator(render_obj_ref, atoi(str_value));
                    }
                    else if (ui_runtime_render_obj_ref_count(render_obj) == 1) {
                        ui_runtime_render_obj_ref_set_is_updator(render_obj_ref, 1);
                    }

                    if (args[0]) {
                        ui_runtime_render_obj_setup(render_obj, args);
                    }
                    
                    if (args[0]) {
                        ui_runtime_render_obj_ref_set_args(render_obj_ref, args);
                    }
                }
                
                if (left_args) {
                    *left_args = args;
                }

                return render_obj_ref;
            }
        }
    }

    render_obj_ref = ui_runtime_render_obj_ref_create_by_res(module->m_runtime, res, left_args);
    if (render_obj_ref == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_start_anim: create render obj ref %s fail!", res);
        return NULL;
    }

    return render_obj_ref;
}

uint32_t ui_sprite_render_obj_creator_hash(ui_sprite_render_obj_creator_t creator) {
    return cpe_hash_str(creator->m_name, strlen(creator->m_name));
}

int ui_sprite_render_obj_creator_eq(ui_sprite_render_obj_creator_t l, ui_sprite_render_obj_creator_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
