#ifndef DROW_PLUGIN_BARRAGE_BARRAGE_H
#define DROW_PLUGIN_BARRAGE_BARRAGE_H
#include "plugin_barrage_types.h"
#include "protocol/plugin/barrage/barrage_ins.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_barrage_barrage_t
plugin_barrage_barrage_create(plugin_barrage_group_t group);

void plugin_barrage_barrage_free(plugin_barrage_barrage_t barrage);

void plugin_barrage_barrage_set_collision_info(
    plugin_barrage_barrage_t barrage, 
    uint32_t collition_categories, uint32_t collition_mask, uint32_t collition_group);
void plugin_barrage_barrage_set_show_dead_anim_mask(plugin_barrage_barrage_t barrage, uint32_t mask);

void plugin_barrage_barrage_set_transform(plugin_barrage_barrage_t barrage, ui_transform_t trans);

void plugin_barrage_barrage_set_target_fun(plugin_barrage_barrage_t barrage, void * ctx, plugin_barrage_target_fun_t fun);

dr_data_t plugin_barrage_barrage_carray_data(plugin_barrage_barrage_t barrage);
int plugin_barrage_barrage_set_carray_data(plugin_barrage_barrage_t barrage, dr_data_t data);

float plugin_barrage_barrage_speed_adj(plugin_barrage_barrage_t barrage);
void plugin_barrage_barrage_set_speed_adj(plugin_barrage_barrage_t barrage, float speed_adj);

float plugin_barrage_barrage_emitter_adj(plugin_barrage_barrage_t barrage);
void plugin_barrage_barrage_set_emitter_adj(plugin_barrage_barrage_t barrage, float emitter_adj);

uint8_t plugin_barrage_barrage_is_enable(plugin_barrage_barrage_t barrage);
uint32_t plugin_barrage_barrage_loop_count(plugin_barrage_barrage_t barrage);
    
int plugin_barrage_barrage_enable(plugin_barrage_barrage_t barrage, uint32_t loop_count);
void plugin_barrage_barrage_disable(plugin_barrage_barrage_t barrage, uint8_t clear_bullets);

void plugin_barrage_barrage_clear_bullets(plugin_barrage_barrage_t barrage);
    
/*iterator*/    
struct plugin_barrage_emitter_it {
    plugin_barrage_emitter_t (*next)(struct plugin_barrage_emitter_it * it);
    char m_data[64];
};

void plugin_barrage_barrage_emitters(
    plugin_barrage_emitter_it_t trigger_it,
    plugin_barrage_barrage_t barrage);

#define plugin_barrage_emitter_it_next(it) ((it)->next ? (it)->next(it) : NULL)

    
#ifdef __cplusplus
}
#endif

#endif

