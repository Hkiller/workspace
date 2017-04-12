#ifndef UIPP_APP_RUNING_EXT_H
#define UIPP_APP_RUNING_EXT_H
#include "uipp/app/Runing.hpp"
#include "FpsCalculator.hpp"

namespace UI { namespace App {

class DebugInfo;
class RuningExt : public Runing {
public:
    enum TouchAction {
        TouchBegin = 0,
        TouchMove = 1,
        TouchEnd = 2,
    };

    RuningExt(EnvExt & env);
    ~RuningExt();

    void init(void);
    void setSize(int32_t w, int32_t h);
    void update(void);
    int64_t lastUpdateTime(void) const { return m_lastUpdateTime; }

    static const char * state_name(ui_runtime_runing_level_t state);
    ui_runtime_runing_level_t state(void) const;
    void setState(ui_runtime_runing_level_t state);
    void stop(void);

    void processInput(TouchAction _action, uint32_t _id, int16_t _x, int16_t _y);
    
	virtual float runingFps(void) const { return m_fpsCalc.fps(); }
	virtual float runingFreeFps(void) const { return m_fpsCalc.freeFps(); }
    
private:
    void doUpdate(float deltaTime);

private:
    EnvExt & m_env;
    int64_t m_lastUpdateTime;

    FpsCalculator m_fpsCalc;
};

}}

#endif
