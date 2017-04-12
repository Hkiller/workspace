#ifndef UI_SPRITE_WORLD_RES_I_H
#define UI_SPRITE_WORLD_RES_I_H
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui_sprite_repository_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_world_res {
    ui_sprite_world_t m_world;
    const char * m_name;
    uint16_t m_size;
    struct cpe_hash_entry m_hh_for_world;
    TAILQ_ENTRY(ui_sprite_world_res) m_next_for_world;
    void * m_free_fun_ctx;
    ui_sprite_world_res_free_fun_t m_free_fun;
};

void ui_sprite_world_res_free_all(const ui_sprite_world_t world);

uint32_t ui_sprite_world_res_hash(const ui_sprite_world_res_t world_res);
int ui_sprite_world_res_eq(const ui_sprite_world_res_t l, const ui_sprite_world_res_t r);

#ifdef __cplusplus
}
#endif

#endif
