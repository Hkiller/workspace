#ifndef PLUGIN_SPINE_UTILS_H
#define PLUGIN_SPINE_UTILS_H
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_spine_bone_calc_transform(struct spBone * bone, ui_transform_t transform);
int plugin_spine_slot_calc_transform(struct spSlot * bone, ui_transform_t transform);
int plugin_spine_ik_calc_transform(struct spIkConstraint * ik, ui_transform_t transform);
void plugin_spine_ik_set_transform(struct spIkConstraint * ik, ui_transform_t transform);
void plugin_spine_ik_set_pos(struct spIkConstraint * ik, ui_vector_2_t pos);
uint8_t plugin_spine_ik_no_modify(struct spIkConstraint * ik);
void plugin_spine_ik_restore(struct spIkConstraint * ik);

/*attachment*/
struct plugin_spine_attachment_it {
    plugin_spine_data_attachment_t (*next)(struct plugin_spine_attachment_it * it);
    char m_data[64];
};
#define plugin_spine_attachment_it_next(it) ((it)->next ? (it)->next(it) : NULL)

void plugin_spine_skin_attachments(plugin_spine_attachment_it_t it, struct spSkin * skin);

struct spAttachment * plugin_spine_data_attachment_attachment(plugin_spine_data_attachment_t attachment);
int plugin_spine_data_attachment_slot_index(plugin_spine_data_attachment_t attachment);
const char * plugin_spine_data_attachment_name(plugin_spine_data_attachment_t attachment);
    
#ifdef __cplusplus
}
#endif

#endif
