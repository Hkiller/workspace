#include "FpsCalculator.hpp"

namespace UI { namespace App {

FpsCalculator::FpsCalculator() {
    init();
}

void FpsCalculator::init(void) {
    m_total_delta = 0.0f;
    m_total_render_times = 0;
    m_total_free_tick_times = 0;

    m_r = m_infos;
    m_w = m_infos;

    m_w->m_delta = 0.0f;
    m_w->m_free_tick_count = 0;
}

void FpsCalculator::updateRenderTick(float delta) {
    m_w->m_delta = delta;
    m_total_delta += delta;
    m_total_render_times++;
    m_total_free_tick_times += m_w->m_free_tick_count;
        
    UpdateInfo * end = m_infos + CPE_ARRAY_SIZE(m_infos);

    m_w++;
    if (m_w >= end) m_w = m_infos;

    if (m_w == m_r) {
        m_total_delta -= m_r->m_delta;
        m_total_render_times--;
        m_total_free_tick_times -= m_r->m_free_tick_count;

        m_r++;
        if (m_r >= end) m_r = m_infos;
    }

    m_w->m_delta = 0.0f;
    m_w->m_free_tick_count = 0;
}

}}

