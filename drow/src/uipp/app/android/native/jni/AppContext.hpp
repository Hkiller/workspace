#ifndef UIPP_APP_APPCONTEXT_H
#define UIPP_APP_APPCONTEXT_H
#include <jni.h>
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"

namespace Ui { namespace App {

class AppContext {
public:
    AppContext();

    int init(jobject activity);
    void fini(void);

    void update(void);

    static AppContext * s_ins;
    
    gd_app_context_t m_app;
    struct error_monitor m_em;
    jclass m_activity_cls;
    jmethodID m_render_commit_begin;
    jmethodID m_render_commit_done;
};

}}

#endif
