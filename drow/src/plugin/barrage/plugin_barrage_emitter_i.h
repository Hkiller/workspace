#ifndef PLUGIN_BARRAGE_EMITTER_I_H
#define PLUGIN_BARRAGE_EMITTER_I_H
#include "plugin/barrage/plugin_barrage_emitter.h"
#include "plugin_barrage_barrage_i.h"
#include "plugin_barrage_data_emitter_i.h"
#include "plugin_barrage_env_i.h"
#include "plugin_barrage_op_i.h"
#include "plugin_barrage_data_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_emitter {
    plugin_barrage_barrage_t m_barrage;
    plugin_barrage_data_emitter_t m_data_emitter;
    plugin_barrage_bullet_proto_t m_bullet_proto;
    plugin_particle_obj_emitter_t m_particle_emitter;
    struct plugin_barrage_bullet_list m_bullets;
    plugin_barrage_data_emitter_flip_type_t m_flip_type;
    TAILQ_ENTRY(plugin_barrage_emitter) m_next_for_barrage;
    struct plugin_barrage_trigger_op_list m_trigger_ops;
    struct plugin_barrage_op m_emitter_op;
    plugin_barrage_data_emitter_trigger_t m_next_trigger;

    BARRAGE_EMITTER m_data;
    uint8_t m_is_working;
};

void plugin_barrage_emitter_start(plugin_barrage_emitter_t emitter);
void plugin_barrage_emitter_stop(plugin_barrage_emitter_t emitter);

void plugin_barrage_emitter_reset(plugin_barrage_emitter_t emitter);
void plugin_barrage_emitter_emit(plugin_barrage_emitter_t emitter);    
void plugin_barrage_emitter_trigger_once(void * ctx);

void plugin_barrage_emitter_real_free(plugin_barrage_emitter_t emitter);

uint8_t plugin_barrage_emitter_trigger_check(plugin_barrage_emitter_t emitter, plugin_barrage_data_emitter_trigger_t trigger);
void plugin_barrage_emitter_trigger_do(plugin_barrage_emitter_t emitter, plugin_barrage_data_emitter_trigger_t trigger);

#ifdef __cplusplus
}
#endif

#endif
