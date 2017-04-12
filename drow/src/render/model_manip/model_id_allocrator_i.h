#ifndef UI_MODEL_MANIP_ID_ALLOCRATOR_I_H
#define UI_MODEL_MANIP_ID_ALLOCRATOR_I_H
#include "render/model_manip/model_id_allocrator.h"

struct ui_model_id_allocrator {
    mem_allocrator_t m_alloc;
    uint32_t m_allocked_capacity;
    uint32_t m_allocked_count;
    uint32_t * m_allocked_ids;
    uint32_t m_next_id;
};

#endif
