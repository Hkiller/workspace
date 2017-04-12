#ifndef DROW_PLUGIN_BARRAGE_DATA_EMITTER_H
#define DROW_PLUGIN_BARRAGE_DATA_EMITTER_H
#include "plugin_barrage_data_barrage.h"

#ifdef __cplusplus
extern "C" {
#endif

/*emitter ops*/
plugin_barrage_data_emitter_t plugin_barrage_data_emitter_create(plugin_barrage_data_barrage_t barrage);
void plugin_barrage_data_emitter_free(plugin_barrage_data_emitter_t data_emitter);

BARRAGE_EMITTER_INFO * plugin_barrage_data_emitter_data(plugin_barrage_data_emitter_t data_emitter);

/*emitter trigger ops*/
struct plugin_barrage_data_emitter_trigger_it {
    plugin_barrage_data_emitter_trigger_t (*next)(struct plugin_barrage_data_emitter_trigger_it * it);
    char m_data[64];
};

plugin_barrage_data_emitter_trigger_t
plugin_barrage_data_emitter_trigger_create(plugin_barrage_data_emitter_t emitter);

void plugin_barrage_data_emitter_trigger_free(plugin_barrage_data_emitter_trigger_t trigger);

BARRAGE_EMITTER_EMITTER_TRIGGER_INFO *
plugin_barrage_data_emitter_trigger_data(plugin_barrage_data_emitter_trigger_t trigger);

int plugin_barrage_data_emitter_trigger_update(plugin_barrage_data_emitter_trigger_t trigger);

void plugin_barrage_data_emitter_triggers(
    plugin_barrage_data_emitter_trigger_it_t trigger_it,
    plugin_barrage_data_emitter_t emitter);

#define plugin_barrage_data_emitter_trigger_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*bullet trigger ops*/
struct plugin_barrage_data_bullet_trigger_it {
    plugin_barrage_data_bullet_trigger_t (*next)(struct plugin_barrage_data_bullet_trigger_it * it);
    char m_data[64];
};

plugin_barrage_data_bullet_trigger_t
plugin_barrage_data_bullet_trigger_create(plugin_barrage_data_emitter_t emitter);

void plugin_barrage_data_bullet_trigger_free(plugin_barrage_data_bullet_trigger_t trigger);

BARRAGE_EMITTER_BULLET_TRIGGER_INFO *
plugin_barrage_data_bullet_trigger_data(plugin_barrage_data_bullet_trigger_t trigger);

int plugin_barrage_data_bullet_trigger_update(plugin_barrage_data_bullet_trigger_t trigger);

void plugin_barrage_data_bullet_triggers(
    plugin_barrage_data_bullet_trigger_it_t trigger_it,
    plugin_barrage_data_emitter_t emitter);

#define plugin_barrage_data_bullet_trigger_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

