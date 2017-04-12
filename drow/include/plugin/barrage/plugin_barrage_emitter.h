#ifndef DROW_PLUGIN_BARRAGE_EMITTER_H
#define DROW_PLUGIN_BARRAGE_EMITTER_H
#include "plugin_barrage_types.h"
#include "protocol/plugin/barrage/barrage_ins.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_barrage_emitter_t
plugin_barrage_emitter_create(
    plugin_barrage_barrage_t barrage,
    plugin_barrage_data_emitter_t emitter,
    plugin_barrage_data_emitter_flip_type_t flip_type);

void plugin_barrage_emitter_free(plugin_barrage_emitter_t emitter);

BARRAGE_EMITTER const * plugin_barrage_emitter_data(plugin_barrage_emitter_t emitter);

uint8_t plugin_barrage_emitter_is_working(plugin_barrage_emitter_t emitter);

void plugin_barrage_emitter_clear_bullets(plugin_barrage_emitter_t emitter);
    
void plugin_barrage_emitter_bullets(plugin_barrage_bullet_it_t bullet_it, plugin_barrage_emitter_t emitter);

void plugin_barrage_emitter_pos(plugin_barrage_emitter_t emitter, ui_vector_2_t result);

#ifdef __cplusplus
}
#endif

#endif

