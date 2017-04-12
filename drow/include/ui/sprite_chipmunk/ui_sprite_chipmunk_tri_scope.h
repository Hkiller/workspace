#ifndef UI_SPRITE_CHIPMUNK_TRI_SCOPE_H
#define UI_SPRITE_CHIPMUNK_TRI_SCOPE_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_chipmunk_tri_scope_update_result {
    ui_sprite_chipmunk_tri_scope_update_success,
    ui_sprite_chipmunk_tri_scope_update_fail,
    ui_sprite_chipmunk_tri_scope_update_skip,
} ui_sprite_chipmunk_tri_scope_update_result_t;
    
typedef ui_sprite_chipmunk_tri_scope_update_result_t (*ui_sprite_chipmunk_tri_scope_update_fun_t)(void * ctx, void * shape, void * body);

ui_sprite_chipmunk_tri_scope_t
ui_sprite_chipmunk_tri_scope_create(ui_sprite_chipmunk_obj_t obj);
    
void ui_sprite_chipmunk_tri_scope_free(ui_sprite_chipmunk_tri_scope_t scope);

uint32_t ui_sprite_chipmunk_tri_scope_group(ui_sprite_chipmunk_tri_scope_t scope);
void ui_sprite_chipmunk_tri_scope_set_group(ui_sprite_chipmunk_tri_scope_t scope, uint32_t group);

uint32_t ui_sprite_chipmunk_tri_scope_mask(ui_sprite_chipmunk_tri_scope_t scope);
void ui_sprite_chipmunk_tri_scope_set_mask(ui_sprite_chipmunk_tri_scope_t scope, uint32_t mask);
    
int ui_sprite_chipmunk_tri_scope_set_dynamic(
    ui_sprite_chipmunk_tri_scope_t scope,
    ui_sprite_chipmunk_tri_scope_update_fun_t fun, void * ctx, size_t ctx_size);

/* int ui_sprite_chipmunk_tri_scope_set_fix( */
/*     ui_sprite_chipmunk_tri_scope_t scope, */
/*     ui_sprite_2d_part_t part); */
    
typedef void (*ui_sprite_chipmunk_tri_scope_visit_fun_t)(ui_sprite_chipmunk_tri_scope_t scope, ui_sprite_chipmunk_obj_body_t body, void * ctx);
int ui_sprite_chipmunk_tri_scope_query(ui_sprite_chipmunk_tri_scope_t scope, ui_sprite_chipmunk_tri_scope_visit_fun_t visit, void * ctx);

int ui_sprite_chipmunk_tri_scope_query_entities(ui_sprite_chipmunk_tri_scope_t scope, ui_sprite_group_t group);

#ifdef __cplusplus
}
#endif

#endif
