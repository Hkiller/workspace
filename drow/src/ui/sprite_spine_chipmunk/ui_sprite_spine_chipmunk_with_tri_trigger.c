#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_tri/ui_sprite_tri_trigger.h"
#include "ui_sprite_spine_chipmunk_with_tri_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_spine_chipmunk_with_tri_build_trigger_event(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, 
    ui_sprite_tri_rule_t rule, char * cfg)
{
    ui_sprite_tri_trigger_t trigger;
    char * sep;
    const char * event = cfg;
    const char * condition = NULL;
    
    if ((sep = strchr(cfg, ':'))) {
        *sep = 0;
        condition = sep + 1;
    }

    trigger = ui_sprite_tri_trigger_create_on_event(rule, event, condition);
    if (trigger == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: on event %s[%s] create fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), event, condition);
        return -1;
    }

    return 0;
}

int ui_sprite_spine_chipmunk_with_tri_build_trigger_attr(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, 
    ui_sprite_tri_rule_t rule, char * cfg)
{
    ui_sprite_tri_trigger_t trigger;

    trigger = ui_sprite_tri_trigger_create_on_attr(rule, cfg);
    if (trigger == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: on attr %s create fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), cfg);
        return -1;
    }

    return 0;
}
    
#ifdef __cplusplus
}
#endif
