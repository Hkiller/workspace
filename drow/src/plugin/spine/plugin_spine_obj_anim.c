#include <assert.h>
#include "spine/extension.h"
#include "cpe/utils/string_utils.h"
#include "plugin_spine_obj_anim_i.h"
#include "plugin_spine_track_listener_i.h"
#include "plugin_spine_obj_anim_group_i.h"
#include "plugin_spine_obj_anim_group_binding_i.h"
#include "plugin_spine_obj_part_i.h"

static plugin_spine_obj_anim_t plugin_spine_obj_anim_create_from_def_i(plugin_spine_obj_t obj, const char * anim_def, size_t anim_def_len);

plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_in_track(plugin_spine_obj_track_t track, const char * anim_name, uint16_t loop_count) {
	spAnimation * animation_data;

    animation_data = spSkeletonData_findAnimation(track->m_obj->m_skeleton->data, anim_name);
	if (!animation_data) {
        CPE_ERROR(track->m_obj->m_module->m_em, "plugin_spine_obj_anim_create: anim %s not exist!", anim_name);
		return NULL;
	}

    return plugin_spine_obj_anim_create_i(track, animation_data, loop_count);
}

plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_i(
    plugin_spine_obj_track_t track, spAnimation * animation_data, uint16_t loop_count)
{
    plugin_spine_obj_t obj = track->m_obj;
    plugin_spine_module_t module = obj->m_module;
    plugin_spine_obj_anim_t anim;
    
    anim = TAILQ_FIRST(&module->m_free_anims);
    if (anim) {
        TAILQ_REMOVE(&module->m_free_anims, anim, m_next);
    }
    else {
        anim = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_obj_anim));
        if (anim == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_anim_create: alloc fail!");
            return NULL;
        }
    }

    anim->m_track = track;
    anim->m_loop_count = loop_count;
    anim->m_listeners = NULL;
    TAILQ_INIT(&anim->m_groups);
    anim->m_track_entry =
        spAnimationState_addAnimation(
            obj->m_anim_state, track->m_track_index, animation_data, loop_count != 1, 0.0f);
    if (anim->m_track_entry == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_anim_create: alloc fail!");
        TAILQ_INSERT_TAIL(&module->m_free_anims, anim, m_next);        
        return NULL;
    }
    anim->m_track_entry->rendererObject = anim;
    anim->m_track_entry->listener = plugin_spine_obj_animation_callback;
    anim->m_track_entry->timeScale = track->m_time_scale;
    //printf("anim %p create: track=%p, anim=%s, track_entry=%p\n", anim, track, animation_data->name, anim->m_track_entry);
    
    TAILQ_INSERT_TAIL(&track->m_anims, anim, m_next);
    obj->m_need_update = 1;
    
    return anim;
}

plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_in_obj(plugin_spine_obj_t obj, const char * track_name, const char * anim_name, uint16_t loop_count) {
    plugin_spine_obj_track_t track;

    track = plugin_spine_obj_track_find(obj, track_name);
    if (track == NULL) {
        track = plugin_spine_obj_track_create(obj, track_name);
        if (track == NULL) return NULL;
    }

    return plugin_spine_obj_anim_create_in_track(track, anim_name, loop_count);
}

plugin_spine_obj_anim_t plugin_spine_obj_anim_create_from_def(plugin_spine_obj_t obj, const char * anim_def) {
    return plugin_spine_obj_anim_create_from_def_i(obj, anim_def, strlen(anim_def));
}

void plugin_spine_obj_anim_free(plugin_spine_obj_anim_t anim) {
    plugin_spine_obj_track_t track = anim->m_track;
    plugin_spine_module_t module = track->m_obj->m_module;

    while(!TAILQ_EMPTY(&anim->m_groups)) {
        plugin_spine_obj_anim_group_binding_free(TAILQ_FIRST(&anim->m_groups));
    }
    
    if (anim->m_track_entry) {
        spAnimationState * anim_state = track->m_obj->m_anim_state;
        spTrackEntry** p;
        
        assert(track->m_track_index < anim_state->tracksCount);
        
        p = &anim_state->tracks[track->m_track_index];
        while (*p) {
            if (*p == anim->m_track_entry) {
                //printf("anim %p free: dispose track_entry=%p\n", anim, anim->m_track_entry);
                (*p) = anim->m_track_entry->next;
                anim->m_track_entry->next = NULL;
                _spTrackEntry_dispose(anim->m_track_entry);
                anim->m_track_entry = NULL;
                break;
            }
            else {
                p = &(*p)->next;
            }
        }

        assert(anim->m_track_entry == NULL);
        TAILQ_REMOVE(&track->m_anims, anim, m_next);
    }
    else {
        //printf("anim %p free: not dispose track_entry\n", anim);
        TAILQ_REMOVE(&track->m_done_anims, anim, m_next);
    }

    plugin_spine_track_listener_free_list(module, &anim->m_listeners);
    
    TAILQ_INSERT_TAIL(&module->m_free_anims, anim, m_next);
}

void plugin_spine_obj_anim_real_free_all(plugin_spine_module_t module) {
    while(!TAILQ_EMPTY(&module->m_free_anims)) {
        plugin_spine_obj_anim_t anim = TAILQ_FIRST(&module->m_free_anims);
        TAILQ_REMOVE(&module->m_free_anims, anim, m_next);
        mem_free(module->m_alloc, anim);
    }
}

plugin_spine_obj_track_t plugin_spine_obj_anim_track(plugin_spine_obj_anim_t anim) {
    return anim->m_track;
}

int plugin_spine_obj_anim_add_track_listener(plugin_spine_obj_anim_t anim, plugin_spine_anim_event_fun_t fun, void * ctx) {
    plugin_spine_module_t module = anim->m_track->m_obj->m_module;
    plugin_spine_track_listener_t listener;

    listener = plugin_spine_track_listener_create(module, fun, ctx);
    if (listener == NULL) {
        return -1;
    }

    listener->m_next = anim->m_listeners;
    anim->m_listeners = listener;
    
    return 0;
}

void plugin_spine_obj_anim_remove_track_listener(plugin_spine_obj_anim_t anim, void * ctx) {
    plugin_spine_module_t module = anim->m_track->m_obj->m_module;

    plugin_spine_track_listener_free_list_by_ctx(module, &anim->m_listeners, ctx);
}

void plugin_spine_obj_anim_dispose(spTrackEntry* entry) {
    plugin_spine_obj_anim_t anim = entry->rendererObject;
    plugin_spine_obj_track_t track = anim->m_track;

    assert(anim->m_track_entry == entry);

    //printf("anim %p on anim dispose: track=%p, dispose track_entry %p\n", anim, track, entry);

    anim->m_track_entry = NULL;
    TAILQ_REMOVE(&track->m_anims, anim, m_next);
    TAILQ_INSERT_TAIL(&track->m_done_anims, anim, m_next);
    
	_spTrackEntry_dispose(entry);
}

int plugin_spine_obj_add_listener(plugin_spine_obj_t obj, plugin_spine_anim_event_fun_t fun, void * ctx) {
    plugin_spine_track_listener_t listener;

    listener = plugin_spine_track_listener_create(obj->m_module, fun, ctx);
    if (listener == NULL) return -1;

    listener->m_next = obj->m_listeners;
    obj->m_listeners = listener;

    return 0;
}

plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_from_def_i(plugin_spine_obj_t obj, const char * anim_def, size_t res_len) {
    char anim_def_buf[64];
    char * loop_count = NULL;
    char * track = NULL;
        
    if ((res_len + 1) > CPE_ARRAY_SIZE(anim_def_buf)) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_play_anim_by_def_one: anim def %s(len=%d) overflow!", anim_def, (int)res_len);
        return NULL;
    }

    memcpy(anim_def_buf, anim_def, res_len);
    anim_def_buf[res_len] = 0;

    loop_count = strchr(anim_def_buf, '*');
    if (loop_count) {
        *loop_count = 0;
        loop_count = loop_count + 1;
    }

    track = strchr(loop_count ? loop_count : anim_def_buf, '@');
    if (track) {
        *track = 0;
        track = track + 1;
    }

    return plugin_spine_obj_anim_create_in_obj(obj, track ? track : "", anim_def_buf, loop_count ? atoi(loop_count) : 0);
}

int plugin_spine_obj_play_anims(plugin_spine_obj_t obj, const char * anim_def, plugin_spine_obj_anim_group_t group) {
    const char * anim_sep;
            
    while((anim_sep = strchr(anim_def, '+'))) {
        plugin_spine_obj_anim_t anim = plugin_spine_obj_anim_create_from_def_i(obj, anim_def, anim_sep - anim_def);
        if (anim == NULL) return -1;

        if (group) {
            if (plugin_spine_obj_anim_group_add_anim(group,  anim) != 0) return -1;
        }
        
        anim_def = anim_sep + 1;
    }

    if (anim_def[0]) {
        plugin_spine_obj_anim_t anim = plugin_spine_obj_anim_create_from_def(obj, anim_def);
        if (anim == NULL) return -1;

        if (group) {
            if (plugin_spine_obj_anim_group_add_anim(group,  anim) != 0) return -1;
        }
    }

    return 0;
}

uint8_t plugin_spine_obj_anim_is_playing(plugin_spine_obj_anim_t anim) {
    return anim->m_track_entry ? 1 : 0;
}

void plugin_spine_obj_remove_listener(plugin_spine_obj_t obj, void * ctx) {
    plugin_spine_track_listener_free_list_by_ctx(obj->m_module, &obj->m_listeners, ctx);
}

static void plugin_spine_obj_animation_do_set_state(void * ctx, const char * value) {
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ctx;
    const char * sep;
    char buf[64];
    const char * part_name;
    const char * transition_name;
    plugin_spine_obj_part_t part;

    sep = strchr(value, '=');
    if (sep == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-part arg %s format error!", value);
        return;
    }

    part_name = cpe_str_dup_range(buf, sizeof(buf), value, sep);
    if (part_name == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-part arg %s part name overflow!", value);
        return;
    }

    part = plugin_spine_obj_part_find(obj, part_name);
    if (part == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-part part %s not exist!", part_name);
        return;
    }

    transition_name = cpe_str_dup_range(buf, sizeof(buf), sep + 1, cpe_str_trim_tail((char*)sep + strlen(sep), sep + 1));
    if (transition_name == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-part arg %s transition name overflow!", value);
        return;
    }

    plugin_spine_obj_part_apply_transition_force(part, transition_name);
}

static void plugin_spine_obj_animation_do_set_timescale(void * ctx, const char * value) {
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ctx;
    const char * sep;
    char buf[64];
    const char * part_name;
    plugin_spine_obj_part_t part;

    sep = strchr(value, '=');
    if (sep == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-timescale arg %s format error!", value);
        return;
    }

    part_name = cpe_str_dup_range(buf, sizeof(buf), value, sep);
    if (part_name == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-timescale arg %s part name overflow!", value);
        return;
    }

    part = plugin_spine_obj_part_find(obj, part_name);
    if (part == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-timescale part %s not exist!", part_name);
        return;
    }

    plugin_spine_obj_track_set_time_scale(part->m_track, atof(sep + 1));
}

void plugin_spine_obj_animation_callback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) {
	spTrackEntry* entry = spAnimationState_getCurrent(state, trackIndex);
    plugin_spine_obj_anim_t anim = entry->rendererObject;
    plugin_spine_obj_t obj = anim->m_track->m_obj;
    plugin_spine_track_listener_t listener;

    if ((plugin_spine_anim_event_type_t)type == plugin_spine_anim_event_event && event->data->name[0] == ':') {
        const char * internal_evt_type = event->data->name + 1;
        
        if (strcmp(internal_evt_type, "set-part") == 0) {
            if (event->stringValue == NULL) {
                CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-part no string arg!");
                return;
            }

            cpe_str_list_for_each(event->stringValue, ',', plugin_spine_obj_animation_do_set_state, obj);
            return;
        }
        else if (strcmp(internal_evt_type, "set-timescale") == 0) {
            if (event->stringValue == NULL) {
                CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_anim_callback: set-timescale no string arg!");
                return;
            }

            cpe_str_list_for_each(event->stringValue, ',', plugin_spine_obj_animation_do_set_timescale, obj);
            return;
        }
        else {
            CPE_ERROR(
                obj->m_module->m_em, "plugin_spine_obj_anim_callback: unknown internal event %s!",
                internal_evt_type);
            return;
        }
    }
    else if ((plugin_spine_anim_event_type_t)type == plugin_spine_anim_event_complete) {
        if (anim->m_loop_count > 0 && loopCount >= anim->m_loop_count) {
            entry->loop = 0;
        }

        if (entry->loop) {
            type = (spEventType)plugin_spine_anim_event_loop;
        }
    }    

    listener = anim->m_listeners;
    while(listener) {
        plugin_spine_track_listener_t cur = listener;
        listener = listener->m_next;
        cur->m_func(cur->m_func_ctx, anim, (plugin_spine_anim_event_type_t)type, event);
    }
    
    listener = obj->m_listeners;
    while(listener) {
        plugin_spine_track_listener_t cur = listener;
        listener = listener->m_next;
        cur->m_func(cur->m_func_ctx, anim, (plugin_spine_anim_event_type_t)type, event);
    }
}

static plugin_spine_obj_anim_t
plugin_spine_obj_track_anim_create_from_def_i(plugin_spine_obj_track_t track, const char * anim_def, size_t res_len) {
    char anim_def_buf[64];
    char * loop_count = NULL;
        
    if ((res_len + 1) > CPE_ARRAY_SIZE(anim_def_buf)) {
        CPE_ERROR(track->m_obj->m_module->m_em, "plugin_spine_obj_play_anim_by_def_one: anim def %s(len=%d) overflow!", anim_def, (int)res_len);
        return NULL;
    }

    memcpy(anim_def_buf, anim_def, res_len);
    anim_def_buf[res_len] = 0;

    loop_count = strchr(anim_def_buf, '*');
    if (loop_count) {
        *loop_count = 0;
        loop_count = loop_count + 1;
    }

    return plugin_spine_obj_anim_create_in_track(track, anim_def_buf, loop_count ? atoi(loop_count) : 0);
}

plugin_spine_obj_anim_t
plugin_spine_obj_track_anim_create_from_def(plugin_spine_obj_track_t track, const char * anim_def) {
    return plugin_spine_obj_track_anim_create_from_def_i(track, anim_def, strlen(anim_def));
}

int plugin_spine_obj_track_play_anims(plugin_spine_obj_track_t track, const char * anim_def, plugin_spine_obj_anim_group_t group) {
    const char * anim_sep;
            
    while((anim_sep = strchr(anim_def, '+'))) {
        plugin_spine_obj_anim_t anim = plugin_spine_obj_track_anim_create_from_def_i(track, anim_def, anim_sep - anim_def);
        if (anim == NULL) return -1;

        if (group) {
            if (plugin_spine_obj_anim_group_add_anim(group,  anim) != 0) return -1;
        }
        
        anim_def = anim_sep + 1;
    }

    if (anim_def[0]) {
        plugin_spine_obj_anim_t anim = plugin_spine_obj_track_anim_create_from_def(track, anim_def);
        if (anim == NULL) return -1;

        if (group) {
            if (plugin_spine_obj_anim_group_add_anim(group,  anim) != 0) return -1;
        }
    }

    return 0;
}
