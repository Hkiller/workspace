#include "cpe/utils/stream_mem.h"
#include "cpe/utils/error.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/tests-env/with_entity.hpp"
#include "ui/sprite/tests-env/with_world.hpp"

namespace ui { namespace sprite { namespace testenv {

with_entity::with_entity()
    : m_entity_id(0)
{
}

void with_entity::SetUp() {
    Base::SetUp();

    ui_sprite_entity_t entity = ui_sprite_entity_create(envOf<with_world>().t_s_world(), "", NULL);
    ASSERT_TRUE(entity != NULL);

    envOf<with_world>().t_s_entity_enter(entity);

    m_entity_id = ui_sprite_entity_id(entity);
}

void with_entity::TearDown() {
    ui_sprite_entity_t entity = ui_sprite_entity_find_by_id(envOf<with_world>().t_s_world(), m_entity_id);
    if (entity) ui_sprite_entity_free(entity);
    m_entity_id = 0;

    Base::TearDown();
}

void with_entity::t_s_entity_send_event(LPDRMETA meta, void const * data, size_t data_size) {
    ui_sprite_entity_send_event(t_s_entity(), meta, data, data_size);
}

void with_entity::t_s_entity_send_event(const char * event_def) {
    ui_sprite_entity_build_and_send_event(t_s_entity(), event_def, NULL);
}

ui_sprite_entity_t with_entity::t_s_entity(void) {
    ui_sprite_entity_t entity = ui_sprite_entity_find_by_id(envOf<with_world>().t_s_world(), m_entity_id);
    EXPECT_TRUE(entity != NULL) << "with_entity:: entity " << m_entity_id << " not exist";
    return entity;
}

void with_entity::t_s_entity_debug(uint8_t debug) {
    ui_sprite_entity_set_debug(t_s_entity(), debug);
}

int8_t with_entity::t_s_entity_calc_bool(const char * def, dr_data_source_t data_source) {
    uint8_t r = 0;
    EXPECT_TRUE(ui_sprite_entity_try_calc_bool(&r, def, t_s_entity(), data_source, NULL) == 0);
    return r;
}

int64_t with_entity::t_s_entity_calc_int64(const char * def, dr_data_source_t data_source) {
    int64_t r = 0;
    EXPECT_TRUE(ui_sprite_entity_try_calc_int64(&r, def, t_s_entity(), data_source, NULL) == 0);
    return r;
}

double with_entity::t_s_entity_calc_double(const char * def, dr_data_source_t data_source) {
    double r = 0;
    EXPECT_TRUE(ui_sprite_entity_try_calc_double(&r, def, t_s_entity(), data_source, NULL) == 0);
    return r;
}

const char * with_entity::t_s_entity_calc_str(const char * def, dr_data_source_t data_source) {
    struct mem_buffer buff;
    mem_buffer_init(&buff, NULL);

    const char * r = ui_sprite_entity_try_calc_str(&buff, def, t_s_entity(), data_source, NULL);

    if (r) {
        r = t_tmp_strdup(r);
    }

    mem_buffer_clear(&buff);

    EXPECT_TRUE(r) << "calc str \"" << def << "\" fail!";

    return r;
}


}}}
