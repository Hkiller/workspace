#include "spine/Skin.h"
#include "cpe/utils/math_ex.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin_spine_module_i.h"

//typedef struct _Entry _Entry;

static plugin_spine_data_attachment_t plugin_spine_attachment_next(struct plugin_spine_attachment_it * it) {
    _Entry * * data = (_Entry * *)(it->m_data);
    _Entry * r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = r->next;

    return (plugin_spine_data_attachment_t)r;
}

void plugin_spine_skin_attachments(plugin_spine_attachment_it_t it, struct spSkin * skin) {
    *(_Entry * *)(it->m_data) = ((_spSkin*)skin)->entries;
    it->next = plugin_spine_attachment_next;
}

struct spAttachment * plugin_spine_data_attachment_attachment(plugin_spine_data_attachment_t attachment) {
    return ((_Entry*)attachment)->attachment;
}

int plugin_spine_data_attachment_slot_index(plugin_spine_data_attachment_t attachment) {
    return ((_Entry*)attachment)->slotIndex;
}

const char * plugin_spine_data_attachment_slot_name(plugin_spine_data_attachment_t attachment) {
    return ((_Entry*)attachment)->name;
}
