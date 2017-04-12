#ifndef UIPP_SPRITE_CFG_CONTEXT_H
#define UIPP_SPRITE_CFG_CONTEXT_H
#include "gdpp/app/System.hpp"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_cfg/ui_sprite_cfg_context.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Cfg {

class CfgContext {
public:
    CfgContext(ui_sprite_cfg_loader_t loader, cfg_t cfg) {
        m_context = ui_sprite_cfg_context_create(loader, cfg);
    }

    ~CfgContext() {
        ui_sprite_cfg_context_free(m_context);
    }

private:
    ui_sprite_cfg_context_t m_context;
};

}}}

#endif
