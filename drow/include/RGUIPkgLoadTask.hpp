#ifndef DROW_RGUI_PKGLOADTASK_H_INCLEDED
#define DROW_RGUI_PKGLOADTASK_H_INCLEDED
#include "cpepp/utils/ClassCategory.hpp"
#include "plugin/package/plugin_package_types.h"
#include "RGUI.h"

namespace Drow {

class PkgLoadTask : public Cpe::Utils::SimulateObject {
public:
    operator plugin_package_load_task_t () const { return (plugin_package_load_task_t)this; }

    typedef void (RGUIWindow::*on_progress_fun_t)(PkgLoadTask & task, float progress);
    
    PkgLoadTask & onProgress(on_progress_fun_t fun);
};

}

#endif
