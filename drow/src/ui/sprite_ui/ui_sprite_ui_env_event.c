#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_ui_env_i.h"

void ui_sprite_ui_env_send_event(void * ctx, plugin_ui_env_t b_env, LPDRMETA meta, void * data, uint32_t data_size, dr_data_t overwrite) {
    ui_sprite_ui_env_t env = ctx;

    if (overwrite) {
        if (dr_meta_copy_same_entry_part(
                data, data_size, meta,
                overwrite->m_data, overwrite->m_size, overwrite->m_meta,
                NULL, 0, env->m_module->m_em)
            < 0)
        {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_send_event: send event %s: copy page data from %s fail",
                dr_meta_name(meta), dr_meta_name(overwrite->m_meta));
            return;
        }
    }
    
    ui_sprite_entity_send_event(env->m_entity, meta, data, data_size);
}

void ui_sprite_ui_env_build_and_send_event(void * ctx, plugin_ui_env_t b_env, const char * def, dr_data_source_t data_source, dr_data_t overwrite) {
    ui_sprite_ui_env_t env = ctx;
    
    if (strchr(def, ':')) {
        ui_sprite_entity_build_and_send_event(env->m_entity, def, data_source);
    }
    else {
        ui_sprite_world_t world = ui_sprite_entity_world(env->m_entity);
        ui_sprite_repository_t repo = ui_sprite_world_repository(world);
        LPDRMETA meta;
        size_t event_size;
        char event_buf[128];
        
        meta = ui_sprite_repository_find_event(repo, def);
        if (meta == NULL) {
            CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_send_event: %s: event not exist", def);
            return;
        }

         event_size = dr_meta_size(meta);
         if (event_size > CPE_ARRAY_SIZE(event_buf)) {
             CPE_ERROR(
                 env->m_module->m_em, "ui_sprite_ui_env_send_event: %s: event size %d overflow!",
                 def, (int)event_size);
             return;
         }

         bzero(event_buf, event_size);

         ui_sprite_ui_env_send_event(ctx, b_env, meta, event_buf, event_size, overwrite);
    }
}
    
