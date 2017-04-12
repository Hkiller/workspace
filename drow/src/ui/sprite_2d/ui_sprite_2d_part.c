#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_2d_part_i.h"
#include "ui_sprite_2d_part_binding_i.h"
#include "ui_sprite_2d_part_attr_i.h"

static ui_sprite_2d_part_t ui_sprite_2d_part_create_i(ui_sprite_2d_transform_t transform, const char * name) {
    ui_sprite_2d_module_t module = transform->m_module;
    ui_sprite_2d_part_t part;

    part = TAILQ_FIRST(&module->m_free_parts);
    if (part == NULL) {
        part = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_2d_part));
        if (part == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_2d_part_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&module->m_free_parts, part, m_next);
    }

    part->m_transform = transform;
    cpe_str_dup(part->m_name, sizeof(part->m_name), name);
    part->m_trans = UI_TRANSFORM_IDENTITY;
    part->m_attr_updated = 0;
    part->m_trans_updated = 0;    
    
    TAILQ_INIT(&part->m_bindings);
    TAILQ_INIT(&part->m_attrs);
    
    TAILQ_INSERT_TAIL(&transform->m_parts, part, m_next);
    
    return part;
}

ui_sprite_2d_part_t ui_sprite_2d_part_create(ui_sprite_2d_transform_t transform, const char * name) {
    if (ui_sprite_2d_part_find(transform, name) != NULL) {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(transform));
        CPE_ERROR(
            transform->m_module->m_em, "entity %d(%s): create part: part %s already exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
        return NULL;
    }

    return ui_sprite_2d_part_create_i(transform, name);
}

void ui_sprite_2d_part_free(ui_sprite_2d_part_t part) {
    ui_sprite_2d_transform_t transform = part->m_transform;
    ui_sprite_2d_module_t module = transform->m_module;
    
    assert(transform);
    
    while(!TAILQ_EMPTY(&part->m_bindings)) {
        ui_sprite_2d_part_binding_free(TAILQ_FIRST(&part->m_bindings));
    }

    while(!TAILQ_EMPTY(&part->m_attrs)) {
        ui_sprite_2d_part_attr_free(TAILQ_FIRST(&part->m_attrs));
    }
    
    TAILQ_REMOVE(&transform->m_parts, part, m_next);
    TAILQ_INSERT_TAIL(&module->m_free_parts, part, m_next);
}

void ui_sprite_2d_part_real_free(ui_sprite_2d_module_t module, ui_sprite_2d_part_t part) {
    TAILQ_REMOVE(&module->m_free_parts, part, m_next);
    mem_free(module->m_alloc, part);
}

ui_sprite_2d_part_t ui_sprite_2d_part_find(ui_sprite_2d_transform_t transform, const char * name) {
    ui_sprite_2d_part_t part;

    if (name[0] == 0) return NULL;

    TAILQ_FOREACH(part, &transform->m_parts, m_next) {
        if (strcmp(part->m_name, name) == 0) return part;
    }

    return NULL;
}

ui_sprite_2d_part_t ui_sprite_2d_part_check_create(ui_sprite_2d_transform_t transform, const char * name) {
    ui_sprite_2d_part_t part = ui_sprite_2d_part_find(transform, name);

    if (part == NULL) {
        part = ui_sprite_2d_part_create_i(transform, name);
    }
    
    return part;
}

const char * ui_sprite_2d_part_name(ui_sprite_2d_part_t part) {
    return part->m_name;
}

ui_sprite_2d_transform_t ui_sprite_2d_part_transform(ui_sprite_2d_part_t part) {
    return part->m_transform;
}

ui_transform_t ui_sprite_2d_part_trans(ui_sprite_2d_part_t part) {
    return &part->m_trans;
}

void ui_sprite_2d_part_set_trans(ui_sprite_2d_part_t part, ui_transform_t trans) {
    ui_transform_assert_sane(trans);
    ui_transform_assert_sane(&part->m_trans);
    
    if (ui_transform_cmp(&part->m_trans, trans) != 0) {
        part->m_trans = *trans;
        part->m_trans_updated = 1;
    }
}

int ui_sprite_2d_part_set_world_trans(ui_sprite_2d_part_t part, ui_transform_t world_trans) {
    ui_transform entity_trans;
    ui_transform part_trans;
    
    if (ui_sprite_2d_transform_calc_trans(part->m_transform, &entity_trans) != 0) return -1;

    ui_transform_inline_reverse(&entity_trans);

    part_trans = *world_trans;
    ui_transform_adj_by_parent(&part_trans, &entity_trans);

    /* printf("xxxx: entity %d(%s): part %s set pos (%f,%f) ==>  (%f,%f)\n", */
    /*        ui_sprite_entity_id(ui_sprite_component_entity(ui_sprite_component_from_data(part->m_transform))), */
    /*        ui_sprite_entity_name(ui_sprite_component_entity(ui_sprite_component_from_data(part->m_transform))), */
    /*        ui_sprite_2d_part_name(part), */
    /*        world_trans->m_m4.m14, world_trans->m_m4.m24, */
    /*        part_trans.m_m4.m14, part_trans.m_m4.m24); */
    
    ui_sprite_2d_part_set_trans(part, &part_trans);

    return 0;
}

int ui_sprite_2d_part_calc_world_trans(ui_sprite_2d_part_t part, ui_transform_t world_trans) {
    ui_transform entity_trans;

    if (ui_sprite_2d_transform_calc_trans(part->m_transform, &entity_trans) != 0) return -1;

    *world_trans = part->m_trans;

    ui_transform_adj_by_parent(world_trans, &entity_trans);

    return 0;
}

static ui_sprite_2d_part_t ui_sprite_2d_part_next(struct ui_sprite_2d_part_it * it) {
    ui_sprite_2d_part_t * data = (ui_sprite_2d_part_t *)(it->m_data);
    ui_sprite_2d_part_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_sprite_2d_transform_parts(ui_sprite_2d_part_it_t emitter_it, ui_sprite_2d_transform_t transform) {
    *(ui_sprite_2d_part_t *)(emitter_it->m_data) = TAILQ_FIRST(&transform->m_parts);
    emitter_it->next = ui_sprite_2d_part_next;
}

uint8_t ui_sprite_2d_part_attr_updated(ui_sprite_2d_part_t part) {
    return part->m_attr_updated;
}

uint8_t ui_sprite_2d_part_trans_updated(ui_sprite_2d_part_t part) {
    return part->m_trans_updated;
}

void ui_sprite_2d_part_dispatch_event(ui_sprite_2d_part_t part) {
    ui_sprite_2d_part_binding_t binding, next;

    if (!part->m_attr_updated && !part->m_trans_updated) return;

    for(binding = TAILQ_FIRST(&part->m_bindings); binding; binding = next) {
        next = TAILQ_NEXT(binding, m_next);
        binding->m_on_updated(part, binding->m_ctx);
    }

    if (part->m_attr_updated) {
        ui_sprite_2d_part_attr_t attr;

        TAILQ_FOREACH(attr, &part->m_attrs, m_next) {
            attr->m_value_changed = 0;
        }
    }

    part->m_attr_updated = 0;
    part->m_trans_updated = 0;
}
