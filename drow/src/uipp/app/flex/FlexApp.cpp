#include "cpe/utils/error.h"
#include "render/runtime/ui_runtime_render.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin/ui/plugin_ui_mouse.h"
#include "AppContainer.hpp"
#include "FlexApp.hpp"
#include "DownloadTask.hpp"
#include "../EnvExt.hpp"
#include "../UICenterExt.hpp"
#include "../RuningExt.hpp"

using namespace UI::App;
extern "C" int init_symbols(void);
uint8_t is_symbols_inited = 0;

FlexApp::FlexApp()
    : ListenerWrapBase("FlexApp")
    , m_state(Init)
    , m_is_loading_closed(false)
{
    g_app_container = new AppContainer();

    dumpSystemInfo("app init");
    
    if (!is_symbols_inited) {
        if (init_symbols() != 0) {
            CPE_ERROR(g_app_container->em(), "FlexApp: init symbols fail.");
        }
        is_symbols_inited = 1;
    }

    m_stage = internal::get_Stage();
    m_stage->scaleMode = flash::display::StageScaleMode::NO_SCALE;
    m_stage->align = flash::display::StageAlign::TOP_LEFT;
    m_stage->frameRate = 30;
}

FlexApp::~FlexApp() {
    clearEventListener();

    if (g_app_container) {
        delete g_app_container;
        g_app_container = NULL;
    }
}

void FlexApp::enterFrame(var as3Args) {
    if (m_state == Init) {
        if (!g_app_container->isStarting()) {
            if (!g_app_container->isDownloadOk()) {
                if (g_app_container->downloadError()) {
                    CPE_ERROR(g_app_container->em(), "FlexApp: executing: download error!!!!!");
                    exit(1);
                }
                return;
            }

            if (!g_app_container->isStageOk()) {
                return;
            }

            if (g_app_container->pkgLoadingCount() > 0) {
                dumpSystemInfo("app init prepaire");
                return;
            }
        
            if (g_app_container->startApp() != 0) {
                CPE_ERROR(g_app_container->em(), "FlexApp: executing: startApp fail");
                exit(1);
                return;
            }
        }
        else {
            if (g_app_container->tickStartApp() != 0) {
                CPE_ERROR(g_app_container->em(), "FlexApp: executing: tickStartApp fail");
                exit(1);
                return;
            }

            if (g_app_container->isStarting()) {
                return;
            }
            
            g_app_container->resize(m_stage->stageWidth, m_stage->stageHeight);
            dumpSystemInfo("app started");

            m_state = MinimalDownloading;
        }
    }

    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();

    env.runing().update();
    plugin_ui_phase_node_t phase_node = plugin_ui_phase_node_current(ui_env);

    if (m_state == MinimalDownloading) {
        if (phase_node == NULL || plugin_ui_phase_node_state(phase_node) == plugin_ui_phase_node_state_prepare_loading) {
            dumpSystemInfo("app minimal downloading");
            return;
        }

        dumpSystemInfo("app minimal download complete");
        m_state = Started;
    }

    ui_runtime_render_t render = env.context();
    uint8_t have_pre_frame;
    ui_runtime_render_begin(render, &have_pre_frame);

    plugin_ui_env_render(ui_env, render);
    ui_runtime_render_done(render);

    plugin_ui_phase_t phase = phase_node ? plugin_ui_phase_node_current_phase(phase_node) : NULL;
    if (phase) {
        m_stage->frameRate = plugin_ui_phase_fps(phase);
    }

    if (!m_is_loading_closed) {
        m_is_loading_closed = true;
        guardCall(&FlexApp::closeLoading);
    }
}

void FlexApp::closeLoading(void) {
    if(flash::external::ExternalInterface::available){
        flash::external::ExternalInterface::call(internal::new_String("drow_close_loading"));
        CPE_INFO(g_app_container->em(), "FlexApp: close loading");
    }
    else{
        CPE_INFO(g_app_container->em(), "FlexApp: ExternalInterface not usable, skip close loading");
    }
}

void FlexApp::handleKeyUp(var as3Args) {
    if (m_state != Started) return;
    
    flash::events::KeyboardEvent ke = var(as3Args[0]);
    //thegame.handleKeyUp(ke->keyCode);
    ke->stopPropagation();
}

void FlexApp::handleKeyDown(var as3Args) {
    if (m_state != Started) return;
    
    flash::events::KeyboardEvent ke = var(as3Args[0]);
    //thegame.handleKeyDown(ke->keyCode);
    ke->stopPropagation();
}

void FlexApp::handleMouthUp(var as3Args) {
    if (m_state != Started) return;
    
    flash::events::MouseEvent me = var(as3Args[0]);
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();

    plugin_ui_mouse_t mouse = plugin_ui_env_mouse(ui_env);
    if (mouse) {
        ui_vector_2 pt = UI_VECTOR_2_INITLIZER(me->stageX, me->stageY);
        
        plugin_ui_mouse_set_l_down(mouse, 0);
        plugin_ui_env_process_touch_rise(ui_env, 0, &pt);
    }
}

void FlexApp::handleMouthDown(var as3Args) {
    if (m_state != Started) return;
    flash::events::MouseEvent me = var(as3Args[0]);
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();

    plugin_ui_mouse_t mouse = plugin_ui_env_mouse_check_create(ui_env);
    if (mouse) plugin_ui_mouse_set_l_down(mouse, 1);

    ui_vector_2 pt = UI_VECTOR_2_INITLIZER(me->stageX, me->stageY);
    plugin_ui_env_process_touch_down(ui_env, 0, &pt);
}

void FlexApp::handleMouthMove(var as3Args) {
    if (m_state != Started) return;
    flash::events::MouseEvent me = var(as3Args[0]);
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();
    ui_vector_2 pt = UI_VECTOR_2_INITLIZER(me->stageX, me->stageY);

    plugin_ui_mouse_t mouse = plugin_ui_env_mouse(ui_env);
    if (mouse) {
        plugin_ui_env_mouse_set_pos(mouse, &pt);

        if (me->buttonDown) {
            plugin_ui_env_process_touch_move(ui_env, 0, &pt);
        }
    }
}

void FlexApp::onResize(var as3Args) {
    if (m_stage->stageWidth <= 0 || m_stage->stageHeight <= 0) {
        CPE_ERROR(
            g_app_container->em(), "FlexApp: onResize: stage size (%d-%d) error",
            (int)m_stage->stageWidth, (int)m_stage->stageHeight);
    }
    
    if (!g_app_container->isStageOk()) {
        g_app_container->setStageOk();
    }

    if (m_state == Started) {
        g_app_container->resize(m_stage->stageWidth, m_stage->stageHeight);
    }
}

void FlexApp::onFocusIn(var as3Args) {
    char *arg = internal::utf8_toString(var(as3Args[0]));
    CPE_ERROR(g_app_container->em(), "FlexApp: onFocusIn: %s!", arg);
    free(arg);

    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();
    plugin_ui_env_mouse_active(ui_env);
}

void FlexApp::onFocusOut(var as3Args) {
    char *arg = internal::utf8_toString(var(as3Args[0]));
    CPE_ERROR(g_app_container->em(), "FlexApp: onFocusOut: %s!", arg);
    free(arg);
    
    EnvExt & env = EnvExt::instance(Gd::App::Application::instance());
    plugin_ui_env_t ui_env = env.uiCenter().uiEnv();
    plugin_ui_env_mouse_deactive(ui_env);
}

void FlexApp::handleRightClick(var as3Args) {
    char *arg = internal::utf8_toString(var(as3Args[0]));
    CPE_ERROR(g_app_container->em(), "FlexApp: handleRightClick: %s!", arg);
    free(arg);
}

void FlexApp::handleFullScreen(var as3Args) {
    m_stage->width = m_stage->fullScreenHeight;
    m_stage->height = m_stage->fullScreenHeight;
}

void FlexApp::onContext3DError(var as3Args) {
    char *arg = internal::utf8_toString(var(as3Args[0]));
    CPE_ERROR(g_app_container->em(), "Stage3D: stage 3d context error: %s!", arg);
    free(arg);
}

void FlexApp::onContext3DCreated(var as3Args) {
    flash::display::Stage3D s3d = var(var(m_stage->stage3Ds)[0]);
    flash::display3D::Context3D ctx3d = s3d->context3D;
    String driverInfo = ctx3d->driverInfo;

    dumpSystemInfo("context 3d created");

    if(driverInfo->indexOf("Software") != -1) {
        // For various reasons your 3D context might actually
        // end up using software rendering instead of being GPU
        // accelerated. This would be a a good place to handle
        // that situation if you want to reduce the graphical
        // complexity or simply prevent the game from running.
        CPE_ERROR(g_app_container->em(), "Stage3D is running Software mode...");
    }

    clearEventListener(m_stage);
    addEventListener(m_stage, flash::events::Event::ENTER_FRAME, &FlexApp::enterFrame);
    addEventListener(m_stage, flash::events::KeyboardEvent::KEY_DOWN, &FlexApp::handleKeyDown);
    addEventListener(m_stage, flash::events::KeyboardEvent::KEY_UP, &FlexApp::handleKeyUp);
    addEventListener(m_stage, flash::events::FullScreenEvent::FULL_SCREEN, &FlexApp::handleFullScreen);
    addEventListener(m_stage, flash::events::MouseEvent::RIGHT_CLICK, &FlexApp::handleRightClick);
    addEventListener(m_stage, flash::events::MouseEvent::MOUSE_UP, &FlexApp::handleMouthUp);
    addEventListener(m_stage, flash::events::MouseEvent::MOUSE_DOWN, &FlexApp::handleMouthDown);
    addEventListener(m_stage, flash::events::MouseEvent::MOUSE_MOVE, &FlexApp::handleMouthMove);
    addEventListener(m_stage, flash::events::Event::RESIZE, &FlexApp::onResize);

    addEventListener(m_stage, flash::events::FocusEvent::FOCUS_IN, &FlexApp::onFocusIn);
    addEventListener(m_stage, flash::events::FocusEvent::FOCUS_OUT, &FlexApp::onFocusOut);

    // stage.addEventListener(IMEEvent.IME_COMPOSITION, this.imeCompositionHandler);

    if (!g_app_container->isStageOk()) {
        if (m_stage->stageWidth <= 0 || m_stage->stageHeight <= 0) {
            CPE_ERROR(
                g_app_container->em(), "context 3d created, but stage size (%d-%d) error",
                (int)m_stage->stageWidth, (int)m_stage->stageHeight);
        }
        else {
            g_app_container->setStageOk();
        }
    }
    else {
        g_app_container->reinit();
    }
}

int FlexApp::main(int argc, char **argv) {
    try {
        flash::display::Stage3D s3d = var(var(m_stage->stage3Ds)[0]);
        addEventListener(s3d, flash::events::Event::CONTEXT3D_CREATE, &FlexApp::onContext3DCreated);
        addEventListener(s3d, flash::events::ErrorEvent::ERROR, &FlexApp::onContext3DError);
        
        s3d->requestContext3D(
            flash::display3D::Context3DRenderMode::AUTO,
            flash::display3D::Context3DProfile::BASELINE_CONSTRAINED);
    
        /*init app*/
        if (g_app_container->createApp() != 0) return -1;

        AS3_GoAsync();
    }
    catch(var e) {
        char *err = internal::utf8_toString(e);
        CPE_ERROR(g_app_container->em(), "FlexApp: main: %s", err);
        free(err);
        return -1;
    }

    return 0;
}

void FlexApp::dumpSystemInfo(const char * location) {
    CPE_INFO(
        g_app_container->em(), "FlexApp: SystemInfo: CPU[%.2f], MEMORY[free=%.2fG, private=%.2fG, total=%.2fG ] @ (%s)",
        (float)flash::system::System::processCPUUsage,
        (float)flash::system::System::freeMemory / 1024.0f / 1024.0f / 1024.0f,
        (float)flash::system::System::privateMemory / 1024.0f / 1024.0f / 1024.0f,
        ((float)flash::system::System::totalMemory / 1024.0f / 1024.0f / 1024.0f),
        location);
}
