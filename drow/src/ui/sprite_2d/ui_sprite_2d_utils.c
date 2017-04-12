#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"

uint8_t ui_sprite_2d_pt_in_rect(ui_vector_2 pt, ui_rect const * rect) {
    return pt.x >= rect->lt.x && pt.x <= rect->rb.x && pt.y >= rect->lt.y && pt.y <= rect->rb.y;
}

uint8_t ui_sprite_2d_pt_in_circle(ui_vector_2 check_pt, ui_vector_2 const * center, float radius) {
    return cpe_math_distance(center->x, center->y, check_pt.x, check_pt.y) <= radius;
}

uint8_t ui_sprite_2d_rect_in_rect(ui_rect const * check, ui_rect const * rect) {
    return check->lt.x >= rect->lt.x && check->rb.x <= rect->rb.x && check->lt.y >= rect->lt.y && check->rb.y <= rect->rb.y;
}

uint8_t ui_sprite_2d_pt_eq(ui_vector_2 l1, ui_vector_2 l2, float delta) {
    return fabs(l1.x - l2.x) <= delta && fabs(l1.y - l2.y) <= delta;
}

uint8_t ui_sprite_2d_rect_eq(ui_rect const * r1, ui_rect const * r2, float delta) {
    return fabs(r1->lt.x - r2->lt.x) <= delta
        && fabs(r1->lt.y - r2->lt.y) <= delta
        && fabs(r1->rb.x - r2->rb.x) <= delta
        && fabs(r1->rb.y - r2->rb.y) <= delta;
}

uint8_t ui_sprite_2d_rect_merge(ui_rect * target, ui_rect const * input) {
    uint8_t changed = 0;

    assert(input->lt.x <= input->rb.x);
    assert(input->lt.y <= input->rb.y);

    if (target->lt.x == target->lt.x) {
        target->lt.x = input->lt.x;
        target->rb.x = input->rb.x;
        changed = 1;
    }
    else {
        if (input->lt.x < target->lt.x) {
            target->lt.x = input->lt.x;
            changed = 1;
        }

        if (input->rb.x > target->rb.x) {
            target->rb.x = input->rb.x;
            changed = 1;
        }
    }

    if (target->lt.y == target->lt.y) {
        target->lt.y = input->lt.y;
        target->rb.y = input->rb.y;
        changed = 1;
    }
    else {
        if (input->lt.y < target->lt.y) {
            target->lt.y = input->lt.y;
            changed = 1;
        }

        if (input->rb.y > target->rb.y) {
            target->rb.y = input->rb.y;
            changed = 1;
        }
    }

    return changed;
}

int ui_sprite_2d_merge_contain_rect_group(ui_vector_2 * lt, ui_vector_2 * rb, ui_sprite_group_t group) {
    int collect_count = 0;
    ui_sprite_entity_it_t entity_it;
	ui_sprite_entity_t entity;

    entity_it = ui_sprite_group_entities(gd_app_alloc(ui_sprite_world_app(ui_sprite_group_world(group))), group);
    if (entity_it == NULL) {
        return -1;
    }

    while((entity = ui_sprite_entity_it_next(entity_it))) {
        if (ui_sprite_2d_merge_contain_rect_entity(lt, rb, entity) == 0) {
            ++collect_count;
        }
    }

    return collect_count;
}

int ui_sprite_2d_merge_contain_rect_entity(ui_vector_2 * r_lt, ui_vector_2 * r_rb, ui_sprite_entity_t entity) {
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    ui_vector_2 lt;
    ui_vector_2 rb;

    if (transform == NULL) return -1;

    lt = ui_sprite_2d_transform_world_pos(
        transform,
        UI_SPRITE_2D_TRANSFORM_POS_F_TOP_LEFT, 
        UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE);

    rb = ui_sprite_2d_transform_world_pos(
        transform,
        UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_RIGHT, 
        UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE);

    assert(lt.x < rb.x);
    assert(lt.y < rb.y);

    if (r_lt->x == r_rb->x) {
        *r_lt = lt;
        *r_rb = rb;
    }
    else {
        if (lt.x < r_lt->x) r_lt->x = lt.x;
        if (lt.y < r_lt->y) r_lt->y = lt.y;
        if (rb.x > r_rb->x) r_rb->x = rb.x;
        if (rb.y > r_rb->y) r_rb->y = rb.y;
    }

    return 0;
}
