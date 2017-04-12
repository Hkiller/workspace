#include "cpe/utils/math_ex.h"
#include "cpe/utils/binpack.h"
#include "gd/net_trans/net_trans_group.h"
#include "render/utils/ui_rect.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_remoteanim_group_i.h"
#include "plugin_remoteanim_block_i.h"

plugin_remoteanim_group_t
plugin_remoteanim_group_create(plugin_remoteanim_module_t module, const char * name, ui_vector_2_t capacity) {
    plugin_remoteanim_group_t group;
    size_t name_len = strlen(name) + 1;
    
    group = mem_alloc(module->m_alloc, sizeof(struct plugin_remoteanim_group) + name_len);
    if (group == NULL) {
        CPE_ERROR(module->m_em, "plugin_remoteanim_group_create: alloc fail!");
        return NULL;
    }

    group->m_module = module;
    memcpy(group + 1, name, name_len);
    group->m_name = (void *)(group + 1);
    group->m_capacity = *capacity;
    
    group->m_trans_group = net_trans_group_create(module->m_trans_mgr, NULL);
    if (group->m_trans_group == NULL) {
        CPE_ERROR(module->m_em, "plugin_remoteanim_group_create: create trans group fail!");
        mem_free(module->m_alloc, group);
        return NULL;
    }
    
    group->m_texture_alloc = binpack_maxrects_ctx_create(module->m_alloc, module->m_em);
    if (group->m_texture_alloc == NULL) {
        CPE_ERROR(module->m_em, "plugin_remoteanim_group_create: create bin pack fail!");
        net_trans_group_free(group->m_trans_group);
        mem_free(module->m_alloc, group);
        return NULL;
    }

    if (binpack_maxrects_ctx_init(group->m_texture_alloc, (uint32_t)capacity->x, (uint32_t)capacity->y) != 0) {
        CPE_ERROR(module->m_em, "plugin_remoteanim_group_create: bin pack init fail, capacity=(%f,%f)!", capacity->x, capacity->y);
        binpack_maxrects_ctx_free(group->m_texture_alloc);
        net_trans_group_free(group->m_trans_group);
        mem_free(module->m_alloc, group);
        return NULL;
    }
    
    binpack_maxrects_ctx_set_span(group->m_texture_alloc, 2);

    group->m_texture = ui_cache_res_create(ui_runtime_module_cache_mgr(module->m_runtime), ui_cache_res_type_texture);
    if (group->m_texture == NULL) {
        CPE_ERROR(module->m_em, "plugin_remoteanim_group_create: create cache fail!");
        binpack_maxrects_ctx_free(group->m_texture_alloc);
        net_trans_group_free(group->m_trans_group);
        mem_free(module->m_alloc, group);
        return NULL;
    }
    ui_cache_texture_set_summary(group->m_texture, capacity->x, capacity->y, ui_cache_pf_r8g8b8a8);
    ui_cache_texture_set_need_part_update(group->m_texture, 1);
    ui_cache_res_load_sync(group->m_texture, NULL);

    TAILQ_INIT(&group->m_blocks);
    
    cpe_hash_entry_init(&group->m_hh);
    if (cpe_hash_table_insert(&module->m_groups, group) != 0) {
        CPE_ERROR(module->m_em, "plugin_remoteanim_group_create: group %s insert fail!", name);
        ui_cache_res_free(group->m_texture);
        binpack_maxrects_ctx_free(group->m_texture_alloc);
        net_trans_group_free(group->m_trans_group);
        mem_free(module->m_alloc, group);
        return NULL;
    }
    
    return group;
}

void plugin_remoteanim_group_free(plugin_remoteanim_group_t group) {
    plugin_remoteanim_module_t module = group->m_module;

    while(!TAILQ_EMPTY(&group->m_blocks)) {
        plugin_remoteanim_block_free(TAILQ_FIRST(&group->m_blocks));
    }

    ui_cache_res_free(group->m_texture);
    
    binpack_maxrects_ctx_free(group->m_texture_alloc);

    net_trans_group_free(group->m_trans_group);

    cpe_hash_table_remove_by_ins(&module->m_groups, group);
    mem_free(module->m_alloc, group);
}

plugin_remoteanim_group_t
plugin_remoteanim_group_find(plugin_remoteanim_module_t module, const char * name) {
    struct plugin_remoteanim_group key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_groups, &key);
}

net_trans_group_t
plugin_remoteanim_group_trans_group(plugin_remoteanim_group_t group) {
    return group->m_trans_group;
}

ui_vector_2_t plugin_remoteanim_group_capacity(plugin_remoteanim_group_t group) {
    return &group->m_capacity;
}

uint32_t plugin_remoteanim_group_hash(plugin_remoteanim_group_t group) {
    return cpe_hash_str(group->m_name, strlen(group->m_name));
}

int plugin_remoteanim_group_eq(plugin_remoteanim_group_t l, plugin_remoteanim_group_t r) {
    return strcmp(l->m_name, r->m_name) == 0 ? 1 : 0;
}

