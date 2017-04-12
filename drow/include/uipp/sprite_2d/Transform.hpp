#ifndef UIPP_SPRITE_2D_TRANSFORM_H
#define UIPP_SPRITE_2D_TRANSFORM_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite/System.hpp"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace P2D {

class Transform : Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_2d_transform_t () const { return (ui_sprite_2d_transform_t)this; }

    Pair localPos(PosPolicy policy, uint8_t adj_type) const { return ui_sprite_2d_transform_local_pos(*this, (uint8_t)policy, adj_type); }
    Pair worldPos(PosPolicy policy, uint8_t adj_type) const { return ui_sprite_2d_transform_world_pos(*this, (uint8_t)policy, adj_type); }
    Pair originPos(void) const { return ui_sprite_2d_transform_world_pos(*this, Origin, 0); }
    void setOriginPos(Pair const & pos) { ui_sprite_2d_transform_set_origin_pos(*this, pos); }

    Pair scalePair(void) const { return ui_sprite_2d_transform_scale_pair(*this); }
    void setScale(float scale) { ui_sprite_2d_transform_set_scale(*this, scale); }
    void setScalePair(Pair const & scale) { ui_sprite_2d_transform_set_scale_pair(*this, scale); }

    float angle(void) const { return ui_sprite_2d_transform_angle(*this); }
    void setAngle(float angle) { ui_sprite_2d_transform_set_angle(*this, angle); }

    Rect rect(void) const { return ui_sprite_2d_transform_rect(*this); }
    void mergeRect(Rect const & rect) { ui_sprite_2d_transform_merge_rect(*this, &rect); }
    void setRect(Rect const & rect) { ui_sprite_2d_transform_set_rect(*this, &rect); }

    uint8_t flipX(void) const { return ui_sprite_2d_transform_flip_x(*this); }
    uint8_t flipY(void) const { return ui_sprite_2d_transform_flip_y(*this); }
    void setFlip(uint8_t flip_x, uint8_t flip_y) { ui_sprite_2d_transform_set_flip(*this, flip_x, flip_y); }

    float adjAngleByFlip(float angle) const {
        return ui_sprite_2d_transform_adj_angle_by_flip(*this, angle);
    }

    Pair adjWorldPos(Pair const & pt, uint8_t adj_type) const {
        return ui_sprite_2d_transform_adj_world_pos(*this, pt, adj_type);
    }

    Pair adjLocalPos(Pair const & pt, uint8_t adj_type) const {
        return ui_sprite_2d_transform_adj_local_pos(*this, pt, adj_type);
    }

    static const char * NAME;
};

PosPolicy posPolicyFromStr(const char * str_policy);

}}}

#endif
