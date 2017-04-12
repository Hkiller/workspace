#ifndef DROW_PLUGIN_BARRAGE_BULLET_H
#define DROW_PLUGIN_BARRAGE_BULLET_H
#include "plugin_barrage_types.h"
#include "protocol/plugin/barrage/barrage_ins.h"
#include "protocol/plugin/barrage/barrage_info.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_bullet_it {
    plugin_barrage_bullet_t (*next)(struct plugin_barrage_bullet_it * it);
    char m_data[64];
};

void plugin_barrage_bullet_free(plugin_barrage_bullet_t bullet);

plugin_barrage_bullet_state_t plugin_barrage_bullet_state(plugin_barrage_bullet_t bullet);
plugin_barrage_data_emitter_flip_type_t plugin_barrage_bullet_flip_type(plugin_barrage_bullet_t bullet);

dr_data_t plugin_barrage_bullet_carray_data(plugin_barrage_bullet_t bullet);

ui_vector_2_t plugin_barrage_bullet_pos(plugin_barrage_bullet_t bullet);

BARRAGE_BULLET const * plugin_barrage_bullet_data(plugin_barrage_bullet_t bullet);

int plugin_barrage_bullet_show_dead_anim(plugin_barrage_bullet_t bullet);

#define plugin_barrage_bullet_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

