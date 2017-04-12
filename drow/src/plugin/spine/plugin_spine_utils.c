#include "spine/Bone.h"
#include "spine/IkConstraint.h"
#include "spine/RegionAttachment.h"
#include "spine/BoundingBoxAttachment.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/buffer.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin_spine_module_i.h"

extern float g_plugin_spine_world_vertices[1000];

int plugin_spine_bone_calc_transform(struct spBone * bone, ui_transform_t transform) {
    ui_vector_3 o_s;
    ui_vector_2 o_trans;
    ui_quaternion o_r;

    memcpy(transform, &UI_TRANSFORM_IDENTITY, sizeof(*transform));

    if (!(bone->worldX == bone->worldX)) {
        return -1;
    }
    
    o_s.x = spBone_getWorldScaleX(bone) * bone->worldSignX;
    o_s.y = spBone_getWorldScaleY(bone) * bone->worldSignY;
    o_s.z = 1.0f;

    if (cpe_float_cmp(o_s.x, 0.0f, UI_FLOAT_PRECISION) == 0
        || cpe_float_cmp(o_s.y, 0.0f, UI_FLOAT_PRECISION) == 0)
    {
        return -1;
    }

    spBone_localToWorld (bone, 0.0f, 0.0f, &o_trans.x, &o_trans.y);
    o_trans.y *= -1.0f;

    /* printf("xxxxx: bone %s: flip-x=%f, flip-y=%f, pos=(%f,%f), angle=%f, sign=(%f,%f)\n", */
    /*        bone->data->name, bone->worldSignX, bone->worldSignY, bone->worldX, bone->worldY, */
    /*        spBone_getWorldRotationX(bone), bone->worldSignX, bone->worldSignY); */
    
    ui_quaternion_set_z_radians(&o_r, cpe_math_angle_to_radians(- spBone_getWorldRotationX(bone)) );

    /* cpe_math_angle_to_radians(- bone->appliedRotation)); */
        
    ui_transform_set_pos_2(transform, &o_trans);
    ui_transform_set_quation_scale(transform, &o_r, &o_s);

    return 0;
}

int plugin_spine_slot_calc_transform(struct spSlot * slot, ui_transform_t transform) {
    if (plugin_spine_bone_calc_transform(slot->bone, transform) != 0) return -1;

    if (slot->attachment && slot->attachment->type ==  SP_ATTACHMENT_REGION) {
        ui_transform p = *transform;
        spRegionAttachment * attachment = (spRegionAttachment *)slot->attachment;
        ui_vector_3 o_s;
        ui_vector_2 o_trans;
        ui_quaternion o_r = UI_QUATERNION_IDENTITY;

        * transform = UI_TRANSFORM_IDENTITY;
    
        o_s.x = attachment->scaleX;
        o_s.y = attachment->scaleY;
        o_s.z = 1.0f;

        o_trans.x = attachment->x;
        o_trans.y = - attachment->y;

        ui_transform_set_pos_2(transform, &o_trans);
        ui_transform_set_quation_scale(transform, &o_r, &o_s);

        ui_transform_adj_by_parent(transform, &p);
    }
    else if (slot->attachment && slot->attachment->type ==  SP_ATTACHMENT_BOUNDING_BOX) {
        spBoundingBoxAttachment * attachment = (spBoundingBoxAttachment *)slot->attachment;
        ui_vector_2 trans;
        uint16_t center_pos;

        if (attachment->super.worldVerticesLength >= 6) {
            spBoundingBoxAttachment_computeWorldVertices(attachment, slot, g_plugin_spine_world_vertices);
            center_pos = (uint16_t)((attachment->super.worldVerticesLength / 2 + 1) / 2);

            trans.x = (g_plugin_spine_world_vertices[0] + g_plugin_spine_world_vertices[center_pos * 2]) / 2.0f;
            trans.y = - (g_plugin_spine_world_vertices[1] + g_plugin_spine_world_vertices[center_pos * 2 + 1]) / 2.0f;

            ui_transform_set_pos_2(transform, &trans);
        }
    }
    
    return 0;
}

int plugin_spine_ik_calc_transform(struct spIkConstraint * ik, ui_transform_t transform) {
    return plugin_spine_bone_calc_transform(ik->target, transform);
}

void plugin_spine_ik_set_pos(struct spIkConstraint * ik, ui_vector_2_t i_pos) {
    spBone * parent = ik->target->parent;
    ui_vector_2 pos = * i_pos;
    float pos_angle;
    float pos_distance;
    
    pos.y = -pos.y;

    if (parent->worldSignX < 0.0f) pos.x = -pos.x;
    if (parent->worldSignY < 0.0f) pos.y = -pos.y;

    pos_angle = cpe_math_angle(parent->worldX, parent->worldY, pos.x, pos.y) - spBone_getWorldRotationX(parent);
    pos_distance = cpe_math_distance(parent->worldX, parent->worldY, pos.x, pos.y);
    /* printf("(%f,%f) ==> (%f,%f), distance=%f, angle=%f\n", parent->worldX, parent->worldY, pos.x, pos.y, pos_distance, pos_angle); */
    
    ik->target->x = cpe_cos_angle(pos_angle) * pos_distance / parent->scaleX;
    ik->target->y = cpe_sin_angle(pos_angle) * pos_distance / parent->scaleY;
    /* printf("      pos-adj=(%f,%f), scale=(%f,%f), result=(%f,%f)\n", */
    /*        cpe_cos_angle(pos_angle) * pos_distance, */
    /*        cpe_sin_angle(pos_angle) * pos_distance, */
    /*        parent->scaleX, */
    /*        parent->scaleY, */
    /*        ik->target->x, ik->target->y); */

    /* spBone_updateWorldTransform(ik->target); */
    
    /* printf( */
    /*     "xxxx: input=(%f,%f), pos=(%f,%f), world-pos=(%f,%f)\n", */
    /*     transform->m_m4.m14, transform->m_m4.m24,  */
    /*     ik->target->x, ik->target->y, */
    /*     ik->target->worldX, ik->target->worldY); */
}

void plugin_spine_ik_set_transform(struct spIkConstraint * ik, ui_transform_t transform) {
    ui_vector_2 pos;
    float angle;
    spBone * parent = ik->target->parent;
    
    ui_transform_get_pos_2(transform, &pos);

    angle = - cpe_math_radians_to_angle(ui_transform_calc_angle_z_rad(transform));
    angle = cpe_math_angle_flip_deg(angle, parent->worldSignX < 0.0f ? 1 : 0, parent->worldSignY < 0.0f ? 1 : 0);

    if (ik->target->data->inheritRotation) {
        if (parent->worldSignX * parent->worldSignY < 0.0f) {
            ik->target->rotation = angle + parent->appliedRotation;
        }
        else {
            ik->target->rotation = angle - parent->appliedRotation;
        }
    }
    else {
        ik->target->rotation = angle;
    }
    
    if (ik->target->data->inheritScale) {
        ik->target->scaleX = fabs(transform->m_s.x) / spBone_getWorldScaleX(parent);
        ik->target->scaleY = fabs(transform->m_s.y) / spBone_getWorldScaleY(parent);
    }
    else {
        ik->target->scaleX = fabs(transform->m_s.x);
        ik->target->scaleY = fabs(transform->m_s.y);
    }
    
    *(float*)&ik->target->worldSignX = transform->m_s.x < 0.0f ? - parent->worldSignX : parent->worldSignX;
    *(float*)&ik->target->worldSignY = transform->m_s.y < 0.0f ? - parent->worldSignY : parent->worldSignY;
    
    plugin_spine_ik_set_pos(ik, &pos);
}

uint8_t plugin_spine_ik_no_modify(struct spIkConstraint * ik) {
    if (ik->target->x != ik->data->target->x) return 0;
    if (ik->target->y != ik->data->target->y) return 0;
    if (ik->target->rotation != ik->data->target->rotation) return 0;
    if (ik->target->scaleX != ik->data->target->scaleX) return 0;
    if (ik->target->scaleY != ik->data->target->scaleY) return 0;

    return 1;
}

void plugin_spine_ik_restore(struct spIkConstraint * ik) {
    ik->target->x = ik->data->target->x;
    ik->target->y = ik->data->target->y;
    ik->target->rotation = ik->data->target->rotation;
    ik->target->scaleX = ik->data->target->scaleX;
    ik->target->scaleY = ik->data->target->scaleY;
}
