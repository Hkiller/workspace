#ifndef UI_SPRITE_2D_TESTENV_WITHWORLD_H
#define UI_SPRITE_2D_TESTENV_WITHWORLD_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_sprite_2d_types.h"

namespace ui { namespace sprite { namespace testenv {

class with_2d : public ::testenv::env<> {
public:
    with_2d();

    void SetUp();
    void TearDown();

    ui_sprite_2d_module_t t_s_2d_module(void) { return m_module; }

    ui_rect t_s_2d_rect_build(const char * def);
    const char * t_s_2d_rect_dump(ui_rect const & rect);

    bool t_s_2d_rect_eq(ui_rect const & l, ui_rect const & r, float delta = 0.1f);
    bool t_s_2d_pair_eq(ui_vector_2 const & l, ui_vector_2 const & r, float delta = 0.1f);

private:
    static void build_2d_transform(void * ctx, void * component_data, cfg_t cfg);

    ui_sprite_2d_module_t m_module;
};

}}}

#endif
