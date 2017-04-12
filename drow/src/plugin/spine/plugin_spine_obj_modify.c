#include <assert.h>
#include "spine/extension.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin_spine_obj_i.h"
#include "plugin_spine_track_listener_i.h"

int plugin_spine_obj_modify_replace_atlas(plugin_spine_obj_t obj, const char * name, const char * model_url) {
    int i;
    
    for(i = 0; i < obj->m_skeleton->slotsCount; ++i) {
        spSlot * slot = obj->m_skeleton->slots[i];
        spAtlasRegion * atlas_region = NULL;
        
        if (slot->attachment == NULL) continue;

		switch (slot->attachment->type) {
		case SP_ATTACHMENT_REGION:
			atlas_region = (spAtlasRegion *)((spRegionAttachment*)slot->attachment)->rendererObject;
            break;
        case SP_ATTACHMENT_MESH:
			atlas_region = (spAtlasRegion *)((spMeshAttachment*)slot->attachment)->rendererObject;
			break;
        default:
            break;
        }

        if (atlas_region == NULL) continue;
        if (strcmp(atlas_region->name, name) != 0) continue;
    }

    return 0;
}

int plugin_spine_obj_modify(plugin_spine_obj_t obj, const char * modification) {
    plugin_spine_module_t module = obj->m_module;

    modification = cpe_str_trim_head((char*)modification);

    if (cpe_str_start_with(modification, "replace-atlas:")) {
        const char * source = cpe_str_trim_head((char*)modification + strlen("replace-atlas:"));
        const char * sep = strstr(source, "==>");
        char source_buf[64];
        char const * source_end;
        size_t len;
        
        if (sep == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_modify: replace format error");
            return -1;
        }

        /*parse source */
        source_end = cpe_str_trim_tail((char*)sep, source);
        len = source_end - source;
        if (len + 1 > CPE_ARRAY_SIZE(source_buf)) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_modify: replace: source len %d overflow", (int)len);
            return -1;
        }

        memcpy(source_buf, source, len);
        source_buf[len] = 0;
        
        return plugin_spine_obj_modify_replace_atlas(obj, source_buf, cpe_str_trim_head((char*)sep + strlen("==>")));
    }
    else {
        CPE_ERROR(module->m_em, "plugin_spine_obj_modify: not support modification %s", modification);
        return -1;
    }

    return 0;
}
