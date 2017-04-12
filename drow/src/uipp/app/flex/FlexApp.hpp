#ifndef UIPP_APP_ENV_FLEX_APP_H
#define UIPP_APP_ENV_FLEX_APP_H
#include "ListenerWrap.hpp"

class FlexApp : public ListenerWrap<FlexApp> {
public:
    enum State { Init, MinimalDownloading, Started };
    
    FlexApp();
    ~FlexApp();
    int main(int argc, char **argv);

private:
    void enterFrame(var as3Args);
    void handleKeyUp(var as3Args);
    void handleKeyDown(var as3Args);
    void handleRightClick(var as3Args);

    void handleMouthUp(var as3Args);
    void handleMouthDown(var as3Args);
    void handleMouthMove(var as3Args);

    void handleFullScreen(var as3Args);
    void dumpSystemInfo(const char * location);

    void onResize(var as3Args);

    void onFocusIn(var as3Args);
    void onFocusOut(var as3Args);
    
    void onContext3DError(var as3Args);
    void onContext3DCreated(var as3Args);

    void closeLoading(void);
    
    flash::display::Stage m_stage;
    State m_state;
    bool m_is_loading_closed;
};
    
#endif
