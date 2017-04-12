#include "render/model/ui_data_src.h"
#include "render/runtime/ui_runtime_module.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_render_def_i.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_module_i.h"

ui_sprite_render_def_t ui_sprite_render_def_create(ui_sprite_render_sch_t render_sch, const char * anim_name, const char * res, uint8_t auto_start) {
    ui_sprite_render_module_t module = render_sch->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(render_sch));
    ui_sprite_render_def_t render_def;
    size_t name_len = strlen(anim_name) + 1;
    size_t res_len = strlen(res) + 1;
    char * p;

    if (anim_name[0] == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): add render %s(%s): name is empty!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            anim_name, res);
        return NULL;
    }

    if (ui_sprite_render_def_find(render_sch, anim_name) != NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): add render %s(%s): name duplicate!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            anim_name, res);
        return NULL;
    }

    render_def = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_render_def) + name_len + res_len);
    if (render_def == NULL) {
        CPE_ERROR(module->m_em, "crate render def %s: alloc fail!", anim_name);
        return NULL;
    }

    p = (char *)(render_def + 1);
    memcpy(p, anim_name, name_len);
    memcpy(p + name_len, res, res_len);

    render_def->m_sch = render_sch;
    render_def->m_auto_start = auto_start;
    render_def->m_anim_name = p;
    render_def->m_anim_res = p + name_len;

    TAILQ_INSERT_TAIL(&render_sch->m_defs, render_def, m_next_for_sch);

    return render_def;
}

void ui_sprite_render_def_free(ui_sprite_render_def_t render_def) {
    ui_sprite_render_sch_t render_sch = render_def->m_sch;
    ui_sprite_render_module_t module = render_sch->m_module;

    TAILQ_REMOVE(&render_sch->m_defs, render_def, m_next_for_sch);
    mem_free(module->m_alloc, render_def);
}

ui_sprite_render_def_t ui_sprite_render_def_find(ui_sprite_render_sch_t render_sch, const char * name) {
    ui_sprite_render_def_t render_def;

    TAILQ_FOREACH(render_def, &render_sch->m_defs, m_next_for_sch) {
        if (strcmp(render_def->m_anim_name, name) == 0) return render_def;
    }

    return NULL;
}

uint8_t ui_sprite_render_def_auto_start(ui_sprite_render_def_t render_def) {
    return render_def->m_auto_start;
}

const char * ui_sprite_render_def_anim_name(ui_sprite_render_def_t render_def) {
    return render_def->m_anim_name;
}

const char * ui_sprite_render_def_anim_res(ui_sprite_render_def_t render_def) {
    return render_def->m_anim_res;
}

ui_data_src_t ui_sprite_render_def_anim_src(ui_sprite_render_def_t render_def) {
    return ui_data_src_find_by_res(
        ui_runtime_module_data_mgr(render_def->m_sch->m_module->m_runtime),
        render_def->m_anim_res);
}

static ui_sprite_render_def_t ui_sprite_render_sch_defs_it_next(struct ui_sprite_render_def_it * it) {
    ui_sprite_render_def_t * data = (ui_sprite_render_def_t *)(it->m_data);
    ui_sprite_render_def_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_sch);

    return r;
}

void ui_sprite_render_sch_defs(ui_sprite_render_def_it_t it, ui_sprite_render_sch_t render_sch) {
    *(ui_sprite_render_def_t *)(it->m_data) = TAILQ_FIRST(&render_sch->m_defs);
    it->next = ui_sprite_render_sch_defs_it_next;
}

