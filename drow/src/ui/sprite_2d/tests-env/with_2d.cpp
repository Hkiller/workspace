#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/cfg.h"
#include "gd/app/tests-env/with_app.hpp"
#include "render/utils/ui_vector_2.h"
#include "ui/sprite/tests-env/with_world.hpp"
#include "ui/sprite_fsm/tests-env/with_fsm.hpp"
#include "ui/sprite_2d/ui_sprite_2d_module.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui/sprite_2d/tests-env/with_2d.hpp"

namespace ui { namespace sprite { namespace testenv {

with_2d::with_2d() : m_module(NULL) {
}

void with_2d::SetUp() {
    Base::SetUp();

    envOf<with_world>()
        .t_s_component_builder_add(
            UI_SPRITE_2D_TRANSFORM_NAME, this, build_2d_transform);

    m_module = 
        ui_sprite_2d_module_create(
            envOf<gd::app::testenv::with_app>().t_app(),
            envOf<with_world>().t_s_repo(),
            envOf<with_fsm>().t_s_fsm_repo(),
            t_allocrator(),
            NULL,
            envOf<utils::testenv::with_em>().t_em());

    ASSERT_TRUE(m_module != NULL);
}

void with_2d::TearDown() {
    ui_sprite_2d_module_free(m_module);
    m_module = NULL;

    envOf<with_world>()
        .t_s_component_builder_remove(UI_SPRITE_2D_TRANSFORM_NAME);

    Base::TearDown();
}

void with_2d::build_2d_transform(void * ctx, void * component_data, cfg_t cfg) {
    //with_2d * env = (with_2d *)ctx;
    ui_sprite_2d_transform_t transform = (ui_sprite_2d_transform_t)component_data;

    if (cfg_t rect_cfg = cfg_find_cfg(cfg, "rect")) {
        ui_rect rect;
        rect.lt.x = cfg_get_float(rect_cfg, "lt.x", 0.0f);
        rect.lt.y = cfg_get_float(rect_cfg, "lt.y", 0.0f);
        rect.rb.x = cfg_get_float(rect_cfg, "rb.x", 0.0f);
        rect.rb.y = cfg_get_float(rect_cfg, "rb.y", 0.0f);

        ui_sprite_2d_transform_merge_rect(transform, &rect);
    }

    if (cfg_t pos_cfg = cfg_find_cfg(cfg, "pos")) {
        ui_vector_2 pos;
        pos.x = cfg_get_float(pos_cfg, "x", 0.0f);
        pos.y = cfg_get_float(pos_cfg, "y", 0.0f);

        ui_sprite_2d_transform_set_origin_pos(transform, pos);
    }
}

ui_rect with_2d::t_s_2d_rect_build(const char * def) {
    ui_rect r;
    sscanf(def, "(%f,%f)-(%f,%f)", &r.lt.x, &r.lt.y, &r.rb.x, &r.rb.y);
    return r;
}

const char * with_2d::t_s_2d_rect_dump(ui_rect const & rect) {
    char buf[64];

    snprintf(
        buf, sizeof(buf), "(%f,%f)-(%f,%f)",
        rect.lt.x, rect.lt.y, rect.rb.x, rect.rb.y);

    return t_tmp_strdup(buf);
}

bool with_2d::t_s_2d_rect_eq(ui_rect const & l, ui_rect const & r, float delta) {
    return ui_sprite_2d_rect_eq(&l, &r, delta) ? true : false;
}

bool with_2d::t_s_2d_pair_eq(ui_vector_2 const & l, ui_vector_2 const & r, float delta) {
    return ui_sprite_2d_pt_eq(l, r, delta) ? true : false;
}

}}}
