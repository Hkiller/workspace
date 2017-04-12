#ifndef UIPP_SPRITE_2D_SYSTEM_H
#define UIPP_SPRITE_2D_SYSTEM_H
#include "uipp/sprite/System.hpp"
#include "uipp/sprite_fsm/System.hpp"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_data.h"

namespace UI { namespace Sprite { namespace P2D {

typedef ui_vector_2 Pair;
typedef ui_rect Rect;
class Transform;

enum PosPolicy {
    Origin = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN
    , TopLeft = UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT
    , TopCenter = UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER
    , TopRight = UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT
    , CenterLeft = UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT
    , Center = UI_SPRITE_2D_TRANSFORM_POS_CENTER
    , CenterRight = UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT
    , BottomLeft = UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT
    , BottomCenter = UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER
    , BottomRight = UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT
};

enum PosAdjType {
    PosAdjByFlip = UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP
    , PosAdjByScale = UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE
    , PosAdjByAngle = UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_ANGLE
    , PosAdjAll = UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL
};

}}}

#endif
