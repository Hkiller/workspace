#include <assert.h>
#include "ui_runtime_render_obj_meta_i.h"
#include "ui_runtime_render_obj_i.h"

ui_runtime_render_obj_meta_t
ui_runtime_render_obj_meta_create(
    ui_runtime_module_t module,
    const char * obj_type_name,
    uint8_t obj_type_id,
    size_t data_capacity,
    void * ctx,
    ui_runtime_render_obj_init_fun_t init_fun,
    ui_runtime_render_obj_set_fun_t set_fun,
    ui_runtime_render_obj_setup_fun_t setup_fun,
    ui_runtime_render_obj_update_fun_t update_fun,
    ui_runtime_render_obj_free_fun_t free_fun,
    ui_runtime_render_obj_render_fun_t render_fun,
    ui_runtime_render_obj_is_playing_fun_t is_playing_fun,
    ui_runtime_render_obj_bounding_fun_t bounding_fun,
    ui_runtime_render_obj_resize_fun_t resize_fun)
{
    ui_runtime_render_obj_meta_t obj_meta;
    size_t name_len = strlen(obj_type_name) + 1;

    if (obj_type_id > 0) {
        if (obj_type_id < UI_OBJECT_TYPE_MIN || obj_type_id >= UI_OBJECT_TYPE_MAX) {
            CPE_ERROR(module->m_em, "%s: create obj meta %d: type is unknown", ui_runtime_module_name(module), obj_type_id);
            return NULL;
        }

        if (module->m_obj_metas_by_id[obj_type_id - UI_OBJECT_TYPE_MIN] != NULL) {
            CPE_ERROR(module->m_em, "%s: create obj meta %d: already registerted", ui_runtime_module_name(module), obj_type_id);
            return NULL;
        }
    }
    
    obj_meta = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_obj_meta) + name_len);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create obj meta (%s)%d: alloc fail", ui_runtime_module_name(module), obj_type_name, obj_type_id);
        return NULL;
    }

    memcpy(obj_meta + 1, obj_type_name, name_len);
    
    obj_meta->m_module = module;
    obj_meta->m_obj_type_name = (void*)(obj_meta + 1);
    obj_meta->m_obj_type_id = obj_type_id;
    obj_meta->m_data_capacity = data_capacity;
    obj_meta->m_ctx = ctx;
    obj_meta->m_init_fun = init_fun;
    obj_meta->m_set_fun = set_fun;
    obj_meta->m_setup_fun = setup_fun;
    obj_meta->m_update_fun = update_fun;
    obj_meta->m_free_fun = free_fun;
    obj_meta->m_render_fun = render_fun;
    obj_meta->m_is_playing_fun = is_playing_fun;
    obj_meta->m_bounding_fun = bounding_fun;
    obj_meta->m_resize_fun = resize_fun;
    obj_meta->m_obj_count = 0;
    TAILQ_INIT(&obj_meta->m_objs);

    cpe_hash_entry_init(&obj_meta->m_hh);
    if (cpe_hash_table_insert(&module->m_obj_metas, obj_meta) != 0) {
        CPE_ERROR(module->m_em, "%s: create obj meta %s(%d): name duplicate!", ui_runtime_module_name(module), obj_type_name, obj_type_id);
        mem_free(module->m_alloc, obj_meta);
        return NULL;
    }

    if (obj_type_id) {
        module->m_obj_metas_by_id[obj_type_id - UI_OBJECT_TYPE_MIN] = obj_meta;
    }
        
    return obj_meta;
}

void ui_runtime_render_obj_meta_free(ui_runtime_render_obj_meta_t obj_meta) {
    ui_runtime_module_t module = obj_meta->m_module;

    while(!TAILQ_EMPTY(&obj_meta->m_objs)) {
        ui_runtime_render_obj_free(TAILQ_FIRST(&obj_meta->m_objs));
    }
    assert(obj_meta->m_obj_count == 0);
    
    cpe_hash_table_remove_by_ins(&module->m_obj_metas, obj_meta);
        
    if (obj_meta->m_obj_type_id) {
        assert(obj_meta->m_obj_type_id >= UI_OBJECT_TYPE_MIN && obj_meta->m_obj_type_id < UI_OBJECT_TYPE_MAX);
        assert(module->m_obj_metas_by_id[obj_meta->m_obj_type_id - UI_OBJECT_TYPE_MIN] == obj_meta);
        module->m_obj_metas_by_id[obj_meta->m_obj_type_id - UI_OBJECT_TYPE_MIN] = NULL;
    }

    mem_free(module->m_alloc, obj_meta);
}

ui_runtime_render_obj_meta_t
ui_runtime_render_obj_meta_find_by_id(ui_runtime_module_t module, uint8_t obj_type_id) {
    return
        (obj_type_id >= UI_OBJECT_TYPE_MIN && obj_type_id < UI_OBJECT_TYPE_MAX)
        ? module->m_obj_metas_by_id[obj_type_id - UI_OBJECT_TYPE_MIN]
        : NULL;
}

ui_runtime_render_obj_meta_t
ui_runtime_render_obj_meta_find_by_name(ui_runtime_module_t module, const char * name) {
    struct ui_runtime_render_obj_meta key;
    key.m_obj_type_name = name;
    return cpe_hash_table_find(&module->m_obj_metas, &key);
}

void ui_runtime_render_obj_meta_free_all(ui_runtime_module_t module) {
    struct cpe_hash_it obj_meta_it;
    ui_runtime_render_obj_meta_t obj_meta;

    cpe_hash_it_init(&obj_meta_it, &module->m_obj_metas);

    obj_meta = cpe_hash_it_next(&obj_meta_it);
    while (obj_meta) {
        ui_runtime_render_obj_meta_t next = cpe_hash_it_next(&obj_meta_it);
        ui_runtime_render_obj_meta_free(obj_meta);
        obj_meta = next;
    }
}

void * ui_runtime_render_obj_meta_ctx(ui_runtime_render_obj_meta_t meta) {
    return meta->m_ctx;
}

const char * ui_runtime_render_obj_meta_type_name(ui_runtime_render_obj_meta_t meta) {
    return meta->m_obj_type_name;
}

uint32_t ui_runtime_render_obj_meta_obj_count(ui_runtime_render_obj_meta_t meta) {
    return meta->m_obj_count;
}

static ui_runtime_render_obj_meta_t ui_runtime_render_obj_meta_next(struct ui_runtime_render_obj_meta_it * it) {
    cpe_hash_it_t * hs_it = (cpe_hash_it_t *)it->m_data;
    return cpe_hash_it_next(hs_it);
}

void ui_runtime_render_obj_metas(ui_runtime_module_t module, ui_runtime_render_obj_meta_it_t it) {
    cpe_hash_it_init((cpe_hash_it_t *)(it->m_data), &module->m_obj_metas);
    it->next = ui_runtime_render_obj_meta_next;
}

uint32_t ui_runtime_render_obj_meta_hash(ui_runtime_render_obj_meta_t obj) {
    return cpe_hash_str(obj->m_obj_type_name, strlen(obj->m_obj_type_name));
}

int ui_runtime_render_obj_meta_eq(ui_runtime_render_obj_meta_t l, ui_runtime_render_obj_meta_t r) {
    return strcmp(l->m_obj_type_name, r->m_obj_type_name) == 0;
}
