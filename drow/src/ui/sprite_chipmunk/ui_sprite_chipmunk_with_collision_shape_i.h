#ifndef UI_SPRITE_CHIPMUNK_WITH_COLLISION_SHAPE_I_H
#define UI_SPRITE_CHIPMUNK_WITH_COLLISION_SHAPE_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_collision.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_chipmunk_with_collision_shape * ui_sprite_chipmunk_with_collision_shape_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_with_collision_shape_list, ui_sprite_chipmunk_with_collision_shape) ui_sprite_chipmunk_with_collision_shape_list_t;
    
struct ui_sprite_chipmunk_with_collision_shape {
    ui_sprite_chipmunk_with_collision_src_t m_src;
    TAILQ_ENTRY(ui_sprite_chipmunk_with_collision_shape) m_next;
    CHIPMUNK_FIXTURE m_data;
    char * m_mass;
    char * m_density;
    char * m_elasticity;
    char * m_friction;
    char * m_collision_group;
    char * m_shape_def;
};

ui_sprite_chipmunk_with_collision_shape_t
ui_sprite_chipmunk_with_collision_shape_create(
    ui_sprite_chipmunk_with_collision_src_t src);

ui_sprite_chipmunk_with_collision_shape_t
ui_sprite_chipmunk_with_collision_shape_clone(
    ui_sprite_chipmunk_with_collision_src_t src, ui_sprite_chipmunk_with_collision_shape_t from);
    
void ui_sprite_chipmunk_with_collision_shape_free(
    ui_sprite_chipmunk_with_collision_shape_t shape);

int ui_sprite_chipmunk_with_collision_shape_calc_data(
    ui_sprite_chipmunk_with_collision_shape_t shape, CHIPMUNK_FIXTURE * data);

int ui_sprite_chipmunk_with_collision_shape_set_density(ui_sprite_chipmunk_with_collision_shape_t shape, const char * density);
int ui_sprite_chipmunk_with_collision_shape_set_mass(ui_sprite_chipmunk_with_collision_shape_t shape, const char * mass);
int ui_sprite_chipmunk_with_collision_shape_set_elasticity(ui_sprite_chipmunk_with_collision_shape_t shape, const char * elasticity);
int ui_sprite_chipmunk_with_collision_shape_set_friction(ui_sprite_chipmunk_with_collision_shape_t shape, const char * friction);
int ui_sprite_chipmunk_with_collision_shape_set_collision_group(ui_sprite_chipmunk_with_collision_shape_t shape, const char * collision_group);
int ui_sprite_chipmunk_with_collision_shape_set_shape_def(ui_sprite_chipmunk_with_collision_shape_t shape, const char * shape_def);
    
#ifdef __cplusplus
}
#endif

#endif
