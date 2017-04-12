#ifndef UI_RENDER_S3D_UTILS_H
#define UI_RENDER_S3D_UTILS_H
#include "render/runtime/ui_runtime_module.h"

#ifdef __cplusplus
extern "C" {
#endif

#define S3D_REPORT_EXCEPTION(__msg, __op)                           \
    catch(AS3::ui::var e) {                                         \
        char *err = AS3::ui::internal::utf8_toString(e);            \
        CPE_ERROR(module->m_em, "%s: " __msg ": exception %s",      \
                  plugin_render_s3d_module_name(module),            \
                  err);                                             \
        free(err);                                                  \
        __op;                                                       \
    }                                                               \
    catch(::std::exception const & e) {                             \
        CPE_ERROR(module->m_em, "%s: " __msg ": exception %s",      \
                  plugin_render_s3d_module_name(module),            \
                  e.what());                                        \
        __op;                                                       \
    }                                                               \
    catch(...) {                                                    \
        CPE_ERROR(module->m_em, "%s: " __msg ": unknown exception", \
                  plugin_render_s3d_module_name(module));           \
        __op;                                                       \
    }

    
#define S3D_REPORT_EXCEPTION_1(__msg, __msg_arg, __op)              \
    catch(AS3::ui::var e) {                                         \
        char *err = AS3::ui::internal::utf8_toString(e);            \
        CPE_ERROR(module->m_em, "%s: " __msg ": exception %s",      \
                  plugin_render_s3d_module_name(module),            \
                  (__msg_arg),                                      \
                  err);                                             \
        free(err);                                                  \
        __op;                                                       \
    }                                                               \
    catch(::std::exception const & e) {                             \
        CPE_ERROR(module->m_em, "%s: " __msg ": exception %s",      \
                  plugin_render_s3d_module_name(module),            \
                  (__msg_arg),                                      \
                  e.what());                                        \
        __op;                                                       \
    }                                                               \
    catch(...) {                                                    \
        CPE_ERROR(module->m_em, "%s: " __msg ": unknown exception", \
                  plugin_render_s3d_module_name(module),            \
                  (__msg_arg));                                     \
        __op;                                                       \
    }

#define S3D_REPORT_EXCEPTION_2(__msg, __msg_arg1, __msg_arg2, __op) \
    catch(AS3::ui::var e) {                                         \
        char *err = AS3::ui::internal::utf8_toString(e);            \
        CPE_ERROR(module->m_em, "%s: " __msg ": exception %s",      \
                  plugin_render_s3d_module_name(module),            \
                  (__msg_arg1), (__msg_arg2),                       \
                  err);                                             \
        free(err);                                                  \
        __op;                                                       \
    }                                                               \
    catch(::std::exception const & e) {                             \
        CPE_ERROR(module->m_em, "%s: " __msg ": exception %s",      \
                  plugin_render_s3d_module_name(module),            \
                  (__msg_arg1), (__msg_arg2),                       \
                  e.what());                                        \
        __op;                                                       \
    }                                                               \
    catch(...) {                                                    \
        CPE_ERROR(module->m_em, "%s: " __msg ": unknown exception", \
                  plugin_render_s3d_module_name(module),            \
                  (__msg_arg1), (__msg_arg2));                      \
        __op;                                                       \
    }
    
#ifdef __cplusplus
}
#endif
    
#endif
