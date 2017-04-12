#include <stdexcept>
#include <sstream>
#include "uipp/sprite_2d/Transform.hpp"

namespace UI { namespace Sprite { namespace P2D {

const char * Transform::NAME = UI_SPRITE_2D_TRANSFORM_NAME;

PosPolicy posPolicyFromStr(const char * str_policy) {
    uint8_t r = ui_sprite_2d_transform_pos_policy_from_str(str_policy);
    if (r == 0) {
        ::std::ostringstream os;
        os << "pos policy " << str_policy << " is unknown!";
        throw ::std::runtime_error(os.str());
    }

    return (PosPolicy)r;
}

}}}
