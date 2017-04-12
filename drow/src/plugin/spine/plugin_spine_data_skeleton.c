#include <assert.h>
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_src.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_spine_data_skeleton_i.h"
#include "plugin_spine_atlas_i.h"

plugin_spine_data_skeleton_t plugin_spine_data_skeleton_create(plugin_spine_module_t module, ui_data_src_t src) {
    plugin_spine_data_skeleton_t spine;

    if (ui_data_src_type(src) != ui_data_src_type_spine_skeleton) {
        CPE_ERROR(
            module->m_em, "create spine at %s: src not spine!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create spine at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    spine = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_data_skeleton));
    if (spine == NULL) {
        CPE_ERROR(
            module->m_em, "create spine at %s: alloc fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    spine->m_module = module;
    spine->m_src = src;
    spine->m_atlas = NULL;
    spine->m_skeleton_data = NULL;

    ui_data_src_set_product(src, spine);

    return spine;
}

void plugin_spine_data_skeleton_free(plugin_spine_data_skeleton_t spine) {
    plugin_spine_module_t module = spine->m_module;
    
    if (spine->m_skeleton_data) {
        spSkeletonData_dispose(spine->m_skeleton_data);
        spine->m_skeleton_data = NULL;
    }

    if (spine->m_atlas) {
        plugin_spine_atlas_free(spine->m_atlas);
        spine->m_atlas = NULL;
    }

    mem_free(module->m_alloc, spine);
}

spAtlas * plugin_spine_data_skeleton_atlas(plugin_spine_data_skeleton_t spine) {
    return spine->m_atlas;
}

spSkeletonData * plugin_spine_data_skeleton_data(plugin_spine_data_skeleton_t spine) {
    return spine->m_skeleton_data;
}

void plugin_spine_data_skeleton_set_data(plugin_spine_data_skeleton_t spine, spSkeletonData * skeleton_data, spAtlas * atlas) {
    if (spine->m_skeleton_data) {
        spSkeletonData_dispose(spine->m_skeleton_data);
    }
    
    if (spine->m_atlas) {
        plugin_spine_atlas_free(spine->m_atlas);
    }

    spine->m_atlas = atlas;
    spine->m_skeleton_data = skeleton_data;
}

int plugin_spine_data_skeleton_update_usings(ui_data_src_t src) {
    plugin_spine_data_skeleton_t spine = ui_data_src_product(src);
    ui_data_src_t using_src;
    struct mem_buffer path_buff;
    const char * path;
    int i;
    
    mem_buffer_init(&path_buff, NULL);

    path = ui_data_src_path_dump(&path_buff, spine->m_src);

    using_src = ui_data_src_find_by_path(spine->m_module->m_data_mgr, path, ui_data_src_type_sprite);
    if (using_src == NULL) {
        using_src = ui_data_src_find_by_path(spine->m_module->m_data_mgr, path, ui_data_src_type_module);
    }
    
    if (using_src == NULL) {
        CPE_ERROR(spine->m_module->m_em, "plugin_spine_data_skeleton_update_refs: no module or sprite %s", path);
        mem_buffer_clear(&path_buff);
        return -1;
    }

    if (ui_data_src_src_create(spine->m_src, using_src) == NULL) {
        CPE_ERROR(spine->m_module->m_em, "plugin_spine_data_skeleton_update_refs: create src ref fail");
        mem_buffer_clear(&path_buff);
        return -1;
    }

    for(i = 0; i < spine->m_skeleton_data->animationsCount; ++i) {
        spAnimation* animation =  spine->m_skeleton_data->animations[i];
        int j;

        for(j = 0; j < animation->timelinesCount; ++j) {
            spTimeline* tl = animation->timelines[j];
            spEventTimeline * event_tl;
            int k;
            
            if (tl->type != SP_TIMELINE_EVENT) continue;

            event_tl = (spEventTimeline *)tl;

            for(k = 0; k < event_tl->framesCount; ++k) {
                spEvent * event = event_tl->events[k];
                const char * event_name = event->data->name;
                const char * event_value = event->stringValue ? event->stringValue : event->data->stringValue;
                if (event_value == NULL) continue;
                if (event_name[0] == ':') continue;

                if (ui_data_src_collect_res_from_event(src, event_name, event_value) != 0) {
                    struct mem_buffer buffer;
                    mem_buffer_init(&buffer, NULL);
                    CPE_ERROR(
                        spine->m_module->m_em, "spine %s: animation %s: collect: process %s:%s fail!",
                        path, animation->name, event_name, event_value);
                    mem_buffer_clear(&buffer);
                    return -1;
                }
            }
        }
    }
    
    mem_buffer_clear(&path_buff);
    return 0;
}
