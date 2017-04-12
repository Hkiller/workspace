#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "render/model_manip/model_manip_cocos.h"
#include "ops.h"

int do_cocos_module_import(
    gd_app_context_t app, ui_data_mgr_t data_mgr,
    const char * to_module, const char * plist, const char * pic)
{
    ui_ed_mgr_t ed_mgr = NULL;

    ed_mgr = ui_ed_mgr_find_nc(app, NULL);
    assert(ed_mgr);

    if (ui_model_import_cocos_module(ed_mgr, to_module, plist, pic, gd_app_em(app)) != 0) {
        APP_CTX_ERROR(app, "import cocos module fail!");
        return -1;
    }

    return 0;
}
