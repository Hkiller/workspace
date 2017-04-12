#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_user.h"
#include "ui_runtime_render_obj_i.h"
#include "ui_runtime_render_obj_child_i.h"
#include "ui_runtime_render_obj_ref_i.h"
#include "ui_runtime_render_obj_meta_i.h"

int ui_runtime_render_obj_set_url(ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url) {
    if (obj->m_meta->m_obj_type_id != obj_url->type) {
        CPE_ERROR(
            obj->m_module->m_em, "%s: obj create: obj meta of type (%s)%d and %d mismatch",
            ui_runtime_module_name(obj->m_module), obj->m_meta->m_obj_type_name, obj->m_meta->m_obj_type_id, obj_url->type);
        return -1;
    }

    if (obj->m_meta->m_set_fun == NULL) {
        CPE_ERROR(
            obj->m_module->m_em, "%s: obj create: obj meta of type (%s)%d not support set",
            ui_runtime_module_name(obj->m_module), obj->m_meta->m_obj_type_name, obj->m_meta->m_obj_type_id);
        return -1;
    }
    
    return obj->m_meta->m_set_fun(obj->m_meta->m_ctx, obj, obj_url);
}

ui_runtime_render_obj_t
ui_runtime_render_obj_create_by_url(ui_runtime_module_t module, UI_OBJECT_URL const * obj_url, const char * name) {
    ui_runtime_render_obj_meta_t obj_meta;
    ui_runtime_render_obj_t obj;

    if (obj_url->type == UI_OBJECT_TYPE_NONE) {
        obj_meta = ui_runtime_render_obj_meta_find_by_name(module, obj_url->data.unknown_type.type_name);
        if (obj_meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: obj create: obj meta of type %s not exist",
                ui_runtime_module_name(module), obj_url->data.unknown_type.type_name);
            return NULL;
        }

        obj = ui_runtime_render_obj_create(module, name, obj_meta);
        if (obj == NULL) return NULL;
    }
    else {
        obj_meta = ui_runtime_render_obj_meta_find_by_id(module, obj_url->type);
        if (obj_meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: obj create: obj meta of type %d not exist",
                ui_runtime_module_name(module), obj_url->type);
            return NULL;
        }

        obj = ui_runtime_render_obj_create(module, name, obj_meta);
        if (obj == NULL) return NULL;

        if (ui_runtime_render_obj_set_url(obj, obj_url) != 0) {
            ui_runtime_render_obj_free(obj);
            return NULL;
        }
    }

    return obj;
}

ui_runtime_render_obj_t
ui_runtime_render_obj_create_by_res(ui_runtime_module_t module, const char * input_res, const char * name) {
    char * args = NULL;
    const char * res;
    UI_OBJECT_URL url_buf;
    UI_OBJECT_URL * url;
    ui_runtime_render_obj_t obj;

    if (input_res[0] == '[') {
        const char * args_end = strchr(input_res, ']');
        if (args_end == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_obj_create: parse args: args format error!");
            return NULL;
        }

        mem_buffer_clear_data(&module->m_dump_buffer);
                              
        args = mem_buffer_strdup_len(&module->m_dump_buffer, input_res + 1, args_end - input_res - 1);

        res = args_end + 1;
    }
    else {
        res = input_res;
    }

    if ((url = ui_object_ref_parse(res, &url_buf, module->m_em)) == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_obj_create: parse url %s fail!", input_res);
        return NULL;
    }

    if ((obj = ui_runtime_render_obj_create_by_url(module, url, name)) == NULL) return NULL;

    if (args && args[0]) ui_runtime_render_obj_setup(obj, args);
    
    return obj;
}

ui_runtime_render_obj_t
ui_runtime_render_obj_create_by_type(ui_runtime_module_t module, const char * name, const char * type_name) {
    ui_runtime_render_obj_meta_t obj_meta;

    obj_meta = ui_runtime_render_obj_meta_find_by_name(module, type_name);
    if (obj_meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: obj create: obj meta of type %s not exist",
            ui_runtime_module_name(module), type_name);
        return NULL;
    }

    return ui_runtime_render_obj_create(module, name, obj_meta);
}

ui_runtime_render_obj_t
ui_runtime_render_obj_create(ui_runtime_module_t module, const char * name, ui_runtime_render_obj_meta_t obj_meta) {
    ui_runtime_render_obj_t obj;

    obj = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_obj) + obj_meta->m_data_capacity);
    if (obj == NULL) {
        CPE_ERROR(module->m_em, "%s: obj alloc fail", ui_runtime_module_name(module));
        return NULL;
    }

    if (name == NULL) name = "";

    obj->m_module = module;
    obj->m_src = NULL;
    obj->m_meta = obj_meta;
    obj->m_ref_count = 0;
    obj->m_auto_release = 1;
    obj->m_suspend = 0;
    obj->m_keep_update = 0;
    obj->m_size = UI_VECTOR_2_ZERO;
    obj->m_time_scale = 1.0f;
    obj->m_in_module_update = 0;
    obj->m_updator = NULL;
    obj->m_transform = UI_TRANSFORM_IDENTITY;
    obj->m_evt_fun = NULL;
    obj->m_evt_ctx = NULL;
    obj->m_touch_process_fun = NULL;
    obj->m_touch_process_ctx = NULL;
    TAILQ_INIT(&obj->m_refs);
    TAILQ_INIT(&obj->m_childs);

    cpe_str_dup(obj->m_name, sizeof(obj->m_name), name);

    if (obj->m_name[0]) {
        cpe_hash_entry_init(&obj->m_hh_for_module);
        if (cpe_hash_table_insert(&module->m_named_objs, obj) != 0) {
            CPE_ERROR(
                module->m_em, "%s: obj %s: hash table insert fail!",
                ui_runtime_module_name(module), name);
            mem_free(module->m_alloc, obj);
            return NULL;
        }
    }

    if (obj_meta->m_init_fun(obj_meta->m_ctx, obj) != 0) {
        CPE_ERROR(
            module->m_em, "%s: obj %s: type %s(%d) init fail!",
            ui_runtime_module_name(module), name, obj_meta->m_obj_type_name, obj_meta->m_obj_type_id);
        if (obj->m_name[0]) cpe_hash_table_remove_by_ins(&obj->m_module->m_named_objs, obj);
        mem_free(module->m_alloc, obj);
        return NULL;
    }
    
    module->m_obj_count++;
    TAILQ_INSERT_TAIL(&module->m_objs, obj, m_next_for_module);
    obj_meta->m_obj_count++;
    TAILQ_INSERT_TAIL(&obj_meta->m_objs, obj, m_next_for_meta);

    ui_runtime_render_obj_sync_module_update(obj);

    return obj;
}

void ui_runtime_render_obj_free(ui_runtime_render_obj_t obj) {
    ui_runtime_module_t module = obj->m_module;

    while(!TAILQ_EMPTY(&obj->m_childs)) {
        ui_runtime_render_obj_child_free(TAILQ_FIRST(&obj->m_childs));
    }

    assert(module->m_obj_count > 0);
    module->m_obj_count--;
    TAILQ_REMOVE(&module->m_objs, obj, m_next_for_module);
    
    assert(obj->m_meta->m_obj_count > 0);
    obj->m_meta->m_obj_count--;
    TAILQ_REMOVE(&obj->m_meta->m_objs, obj, m_next_for_meta);

    if (obj->m_meta->m_free_fun) {
        obj->m_meta->m_free_fun(obj->m_meta->m_ctx, obj);
    }
    
    if (obj->m_name[0]) {
        cpe_hash_table_remove_by_ins(&module->m_named_objs, obj);
    }

    while(!TAILQ_EMPTY(&obj->m_refs)) {
        ui_runtime_render_obj_ref_disconnect(TAILQ_FIRST(&obj->m_refs));
    }
    assert(obj->m_ref_count == 0);
    assert(TAILQ_EMPTY(&obj->m_refs));
    assert(obj->m_updator == NULL);
    
    if (obj->m_in_module_update) {
        TAILQ_REMOVE(&module->m_update_objs, obj, m_next_for_update);
        obj->m_in_module_update = 0;
    }

    if (obj->m_src) {
        ui_data_src_remove_user(obj->m_src, obj);
        obj->m_src = NULL;
    }

    mem_free(module->m_alloc, obj);
}

int ui_runtime_render_obj_add_child(ui_runtime_render_obj_t obj, ui_runtime_render_obj_ref_t child_obj_ref, void const * tag, uint8_t auto_render) {
    ui_runtime_render_obj_child_t child;
    child = ui_runtime_render_obj_child_create(obj, child_obj_ref, tag, auto_render);
    return child ? 0 : -1;
}

int ui_runtime_render_obj_setup(ui_runtime_render_obj_t obj, char * args) {
    int rv = 0;
    char * p;

    if ((p = cpe_str_read_and_remove_arg(args, "time-scale", ',', '='))) {
        obj->m_time_scale = atof(p);
    }
    
    if (obj->m_meta->m_setup_fun) {
        if (obj->m_meta->m_setup_fun(obj->m_meta->m_ctx, obj, args) != 0) rv = -1;
    }

    return rv;
}

const char * ui_runtime_render_obj_name(ui_runtime_render_obj_t obj) {
    return obj->m_name;
}

ui_runtime_render_obj_t
ui_runtime_render_obj_find(ui_runtime_module_t module, const char * name) {
    struct ui_runtime_render_obj key;
    cpe_str_dup(key.m_name, sizeof(key.m_name), name);
    return cpe_hash_table_find(&module->m_named_objs, &key);
}

ui_runtime_module_t ui_runtime_render_obj_module(ui_runtime_render_obj_t obj) {
    return obj->m_module;
}

uint8_t ui_runtime_render_obj_type_id(ui_runtime_render_obj_t obj) {
    return obj->m_meta->m_obj_type_id;
}

const char * ui_runtime_render_obj_type_name(ui_runtime_render_obj_t obj) {
    return obj->m_meta->m_obj_type_name;
}

ui_data_src_t ui_runtime_render_obj_src(ui_runtime_render_obj_t obj) {
    return obj->m_src;
}

void ui_runtime_render_obj_set_evt_processor(ui_runtime_render_obj_t obj, ui_runtime_render_obj_evt_fun_t fun, void * ctx) {
    obj->m_evt_fun = fun;
    obj->m_evt_ctx = ctx;
}

static void ui_runtime_render_obj_on_src_unload(void * ctx, ui_data_src_t src) {
    ui_runtime_render_obj_t obj = ctx;
    assert(obj->m_src == src);

    ui_runtime_render_obj_free(obj);
}

void ui_runtime_render_obj_set_src(ui_runtime_render_obj_t obj, ui_data_src_t src) {
    if (obj->m_src) ui_data_src_remove_user(obj->m_src, obj);

    obj->m_src = src;

    if (obj->m_src) {
        ui_data_src_user_create(obj->m_src, obj, ui_runtime_render_obj_on_src_unload);
    }
}

ui_runtime_render_obj_meta_t ui_runtime_render_obj_meta(ui_runtime_render_obj_t obj) {
    return obj->m_meta;
}

void * ui_runtime_render_obj_data(ui_runtime_render_obj_t obj) {
    return obj + 1;
}

ui_runtime_render_obj_t ui_runtime_render_obj_from_data(void * data) {
    return ((ui_runtime_render_obj_t)data) - 1;
}

int ui_runtime_render_obj_get_bounding(ui_runtime_render_obj_t obj, ui_rect_t bounding) {
    if (obj->m_meta->m_bounding_fun) {
        obj->m_meta->m_bounding_fun(obj->m_meta->m_ctx, obj, bounding);
        return 0;
    }
    else {
        return -1;
    }
}

uint8_t ui_runtime_render_obj_support_update(ui_runtime_render_obj_t obj) {
    return obj->m_meta->m_update_fun ? 1 : 0; 
}

uint8_t ui_runtime_render_obj_is_playing(ui_runtime_render_obj_t obj) {
    if (obj->m_meta->m_is_playing_fun) {
        return obj->m_meta->m_is_playing_fun(obj->m_meta->m_ctx, obj);
    }
    else {
        return obj->m_meta->m_update_fun ? 1 : 0;
    }
}

int ui_runtime_render_obj_render(ui_runtime_render_obj_t obj, ui_runtime_render_t ctx, ui_rect_t clip_rect, ui_transform_t t) {
    ui_runtime_render_obj_child_t child;
    int rv = 0;

    if (obj->m_meta->m_render_fun(
            obj->m_meta->m_ctx, obj, ctx, clip_rect, NULL, t) != 0) rv = -1;

    TAILQ_FOREACH(child, &obj->m_childs, m_next_for_obj) {
        if (child->m_auto_render) {
            if (ui_runtime_render_obj_ref_render(child->m_child_obj_ref, ctx, clip_rect, t) != 0) rv = -1;
        }
    }

    return rv;
}

void ui_runtime_render_obj_update(ui_runtime_render_obj_t obj, float delta) {
    ui_runtime_render_obj_child_t child, next_child;
    
    if (obj->m_suspend) return;

    delta *= obj->m_time_scale;
    
    if(obj->m_meta->m_update_fun) {
        obj->m_meta->m_update_fun(obj->m_meta->m_ctx, obj, delta);
    }

    for(child = TAILQ_FIRST(&obj->m_childs); child; child = next_child) {
        next_child =  TAILQ_NEXT(child, m_next_for_obj);
        
        ui_runtime_render_obj_ref_update(child->m_child_obj_ref, delta);
        if (!ui_runtime_render_obj_is_playing(child->m_child_obj_ref->m_obj)) {
            ui_runtime_render_obj_child_free(child);
        }
    }
}

uint8_t ui_runtime_render_obj_auto_release(ui_runtime_render_obj_t obj) {
    return obj->m_auto_release;
}

void ui_runtime_render_obj_set_auto_release(ui_runtime_render_obj_t obj, uint8_t auto_release) {
    obj->m_auto_release = auto_release;
}

uint8_t ui_runtime_render_obj_keep_update(ui_runtime_render_obj_t obj) {
    return obj->m_keep_update;
}

void ui_runtime_render_obj_set_keep_update(ui_runtime_render_obj_t obj, uint8_t keep_update) {
    obj->m_keep_update = keep_update;
}

float ui_runtime_render_obj_time_scale(ui_runtime_render_obj_t obj) {
    return obj->m_time_scale;
}

void ui_runtime_render_obj_set_time_scale(ui_runtime_render_obj_t obj, float time_scale) {
    obj->m_time_scale = time_scale;
}

uint16_t ui_runtime_render_obj_ref_count(ui_runtime_render_obj_t obj) {
    return obj->m_ref_count;
}

ui_runtime_render_obj_ref_t ui_runtime_render_obj_updator(ui_runtime_render_obj_t obj) {
    return obj->m_updator;
}

uint8_t ui_runtime_render_obj_is_suspend(ui_runtime_render_obj_t obj) {
    return obj->m_suspend;
}

void ui_runtime_render_obj_set_suspend(ui_runtime_render_obj_t obj, uint8_t suspend) {
    if (suspend) suspend = 1;

    if (obj->m_suspend != suspend) {
        obj->m_suspend = suspend;
        ui_runtime_render_obj_sync_module_update(obj);
    }
}

void ui_runtime_render_obj_sync_module_update(ui_runtime_render_obj_t obj) {
    uint8_t need_module_update = 1;

    if (obj->m_meta->m_update_fun == NULL) return;
    
    if (obj->m_suspend
        || (obj->m_ref_count == 0 && !obj->m_keep_update)
        || obj->m_updator
        )
    {
        need_module_update = 0;
    }

    if (need_module_update != obj->m_in_module_update) {
        if (need_module_update) {
            TAILQ_INSERT_TAIL(&obj->m_module->m_update_objs, obj, m_next_for_update);
        }
        else {
            TAILQ_REMOVE(&obj->m_module->m_update_objs, obj, m_next_for_update);
        }

        obj->m_in_module_update = need_module_update;
    }
}

ui_transform_t ui_runtime_render_obj_transform(ui_runtime_render_obj_t obj) {
    return &obj->m_transform;
}

void ui_runtime_render_obj_send_event(ui_runtime_render_obj_t obj, const char * value) {
    ui_runtime_render_obj_ref_t obj_ref;

    if (obj->m_evt_fun) {
        obj->m_evt_fun(obj->m_evt_ctx, obj, value);
    }
    
    TAILQ_FOREACH(obj_ref, &obj->m_refs, m_next_for_obj) {
        if (obj_ref->m_evt_fun) {
            obj_ref->m_evt_fun(obj_ref->m_evt_ctx, obj, value);
        }
    }
}

uint8_t ui_runtime_render_obj_touch_is_support(ui_runtime_render_obj_t obj) {
    return obj->m_touch_process_fun ? 1 : 0;
}

void ui_runtime_render_obj_touch_set_processor(
    ui_runtime_render_obj_t obj, ui_runtime_touch_process_fun_t process_fun, void * process_ctx)
{
    obj->m_touch_process_fun = process_fun;
    obj->m_touch_process_ctx = process_ctx;
}

ui_vector_2_t ui_runtime_render_obj_size(ui_runtime_render_obj_t obj) {
    return &obj->m_size;
}

void ui_runtime_render_obj_set_size(ui_runtime_render_obj_t obj, ui_vector_2_t size) {
    if (ui_vector_2_cmp(&obj->m_size, size) != 0) {
        obj->m_size = *size;
        if (obj->m_meta->m_resize_fun) {
            obj->m_meta->m_resize_fun(obj->m_meta->m_ctx, obj, size);
        }
    }
}

uint32_t ui_runtime_render_obj_name_hash(ui_runtime_render_obj_t render_obj) {
    return cpe_hash_str(render_obj->m_name, strlen(render_obj->m_name));
}

int ui_runtime_render_obj_name_eq(ui_runtime_render_obj_t l, ui_runtime_render_obj_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

