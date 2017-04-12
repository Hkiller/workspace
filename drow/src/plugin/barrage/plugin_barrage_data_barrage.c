#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "render/model/ui_data_src_src.h"
#include "plugin_barrage_data_barrage_i.h"
#include "plugin_barrage_data_emitter_i.h"

plugin_barrage_data_barrage_t plugin_barrage_data_barrage_create(plugin_barrage_module_t module, ui_data_src_t src) {
    plugin_barrage_data_barrage_t barrage;

    if (ui_data_src_type(src) != ui_data_src_type_barrage) {
        CPE_ERROR(
            module->m_em, "create barrage at %s: src not barrage!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create barrage at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    barrage = (plugin_barrage_data_barrage_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_data_barrage));
    if (barrage == NULL) {
        CPE_ERROR(
            module->m_em, "create barrage at %s: alloc barrage fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    barrage->m_module = module;
    barrage->m_src = src;
    TAILQ_INIT(&barrage->m_emitters);

    ui_data_src_set_product(src, barrage);

    return barrage;
}

void plugin_barrage_data_barrage_free(plugin_barrage_data_barrage_t barrage) {
    while(!TAILQ_EMPTY(&barrage->m_emitters)) {
        plugin_barrage_data_emitter_free(TAILQ_FIRST(&barrage->m_emitters));
    }

    mem_free(barrage->m_module->m_alloc, barrage);
}

BARRAGE_BARRAGE_INFO *
plugin_barrage_data_barrage_data(plugin_barrage_data_barrage_t barrage) {
    return &barrage->m_data;
}

const char * plugin_barrage_data_barrage_dump(plugin_barrage_data_barrage_t barrage) {
    mem_buffer_t buffer = &barrage->m_module->m_dump_buffer;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    struct plugin_barrage_data_emitter_it emitters;
    plugin_barrage_data_emitter_t emitter;

    mem_buffer_clear_data(buffer);

    dr_json_print(
        (write_stream_t)&stream,
        &barrage->m_data, sizeof(barrage->m_data), barrage->m_module->m_meta_barrage_info,
        DR_JSON_PRINT_MINIMIZE, NULL);
    
    plugin_barrage_data_barrage_emitters(&emitters, barrage);
    while((emitter = plugin_barrage_data_emitter_it_next(&emitters))) {
        struct plugin_barrage_data_emitter_trigger_it emitter_triggers;
        plugin_barrage_data_emitter_trigger_t emitter_trigger;
        struct plugin_barrage_data_bullet_trigger_it bullet_triggers;
        plugin_barrage_data_bullet_trigger_t bullet_trigger;
        
        dr_json_print(
            (write_stream_t)&stream,
            &emitter->m_data, sizeof(emitter->m_data), barrage->m_module->m_meta_emitter_info,
            DR_JSON_PRINT_MINIMIZE, NULL);

        plugin_barrage_data_emitter_triggers(&emitter_triggers, emitter);
        while((emitter_trigger = plugin_barrage_data_emitter_trigger_it_next(&emitter_triggers))) {
            stream_printf((write_stream_t)&stream, "\n    emitter trigger: ");

            dr_json_print(
                (write_stream_t)&stream,
                &emitter_trigger->m_data, sizeof(emitter_trigger->m_data), barrage->m_module->m_meta_emitter_trigger_info,
                DR_JSON_PRINT_MINIMIZE, NULL);
        }

        plugin_barrage_data_bullet_triggers(&bullet_triggers, emitter);
        while((bullet_trigger = plugin_barrage_data_bullet_trigger_it_next(&bullet_triggers))) {
            stream_printf((write_stream_t)&stream, "\n    emitter trigger: ");

            dr_json_print(
                (write_stream_t)&stream,
                &bullet_trigger->m_data, sizeof(bullet_trigger->m_data), barrage->m_module->m_meta_bullet_trigger_info,
                DR_JSON_PRINT_MINIMIZE, NULL);
        }

        stream_putc((write_stream_t)&stream, '\n');
    }
    
    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

int plugin_barrage_data_barrage_update_using(ui_data_src_t src) {
    plugin_barrage_data_barrage_t barrage = (plugin_barrage_data_barrage_t)ui_data_src_product(src);
    plugin_barrage_data_emitter_t emitter;
    ui_data_src_t bullet_src;
    char buf[128];
    char * sep;

    TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next) {
        cpe_str_dup(buf, sizeof(buf), emitter->m_data.bullet.proto);
        sep = strrchr(buf, '#');
        if (sep) *sep = 0;
    
        bullet_src = ui_data_src_find_by_path(barrage->m_module->m_data_mgr, buf, ui_data_src_type_particle);
        if (bullet_src == NULL) {
            CPE_ERROR(barrage->m_module->m_em, "emitter use bullet %s not exist!", buf);
            return -1;
        }

        if (ui_data_src_src_create(src, bullet_src) == NULL) {
            CPE_ERROR(barrage->m_module->m_em, "emitter use bullet %s create src ref fail!", emitter->m_data.bullet.proto);
            return -1;
        }
    }

    return 0;
}

static plugin_barrage_data_emitter_t plugin_barrage_data_barrage_next_emitter(plugin_barrage_data_emitter_it_t it) {
    plugin_barrage_data_emitter_t * data = (plugin_barrage_data_emitter_t *)(it->m_data);
    plugin_barrage_data_emitter_t r = *data;
    if (*data) *data = TAILQ_NEXT(*data, m_next);
    return r;
}

void plugin_barrage_data_barrage_emitters(plugin_barrage_data_emitter_it_t it, plugin_barrage_data_barrage_t barrage) {
    *(plugin_barrage_data_emitter_t *)(it->m_data) = TAILQ_FIRST(&barrage->m_emitters);
    it->next = plugin_barrage_data_barrage_next_emitter;
}
