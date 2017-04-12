#ifndef UIPP_APP_FPSCALCULATOR_H
#define UIPP_APP_FPSCALCULATOR_H
#include "System.hpp"

namespace UI { namespace App {

class FpsCalculator {
public:
    FpsCalculator();

    float fps(void) const {  return m_total_delta > 0.0f ? (float)m_total_render_times / m_total_delta : 0.0f;  }
	float freeFps(void) const {  return m_total_delta > 0.0f ? (float)m_total_free_tick_times / m_total_delta : 0.0f;  }

    void init(void);
    void updateRenderTick(float delta);
    void updateFreeTick(void) { m_w->m_free_tick_count++; }
    
private:
    struct UpdateInfo {
        float m_delta;
        int64_t m_free_tick_count;
    };

    float m_total_delta;
    int64_t m_total_render_times;
    int64_t m_total_free_tick_times;

    UpdateInfo m_infos[10];
    UpdateInfo * m_r;
    UpdateInfo * m_w;
};

}}

#endif
