#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_facebook_share_backend.hpp"
#include "appsvr/share/appsvr_share_request_block.h"
#include "appsvr/share/appsvr_share_request.h"

static struct {
    const char * name; 
    int (*init)(appsvr_facebook_share_backend_t backend);
    void (*fini)(appsvr_facebook_share_backend_t backend);
} s_auto_reg_products[] = {
    { "jni", appsvr_facebook_share_jni_init, appsvr_facebook_share_jni_fini }
    , { "delegate", appsvr_facebook_share_delegate_init, appsvr_facebook_share_delegate_fini }
};

int appsvr_facebook_share_backend_init(appsvr_facebook_share_module_t module) {
    appsvr_facebook_share_backend_t backend;
    uint16_t component_pos;

    backend = (appsvr_facebook_share_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_facebook_share_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_facebook_share_backend_create: alloc fail!");
        return -1;
    }

    bzero(backend, sizeof(*backend));
    backend->m_module = module;

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(backend) != 0) {
            CPE_ERROR(module->m_em, "appsvr_umeng_backend_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(backend);
            }

            mem_free(module->m_alloc, backend);
            return -1;
        }
    }

    CPE_INFO(module->m_em, "appsvr_facebook_share_backend_create: success!");

    module->m_backend = backend;
    return 0;
}

void appsvr_facebook_share_backend_fini(appsvr_facebook_share_module_t module) {
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module->m_backend);
    }

    mem_free(module->m_alloc, module->m_backend);
}

int appsvr_facebook_share_backend_commit(appsvr_facebook_share_module_t module, appsvr_share_request_t req){
    JNIEnv *env = (JNIEnv *)android_jni_env();
    const char * contentURL =appsvr_share_request_block_get_str(req, appsvr_share_request_block_navigation, 0, "");
    const char * contentTitle =appsvr_share_request_block_get_str(req, appsvr_share_request_block_title, 0, "");
    const char * imageURL =appsvr_share_request_block_get_str(req, appsvr_share_request_block_remote_picture, 0, "");
    env->CallStaticVoidMethod(
        module->m_backend->m_manip_cls, module->m_backend->m_onShare,
        env->NewStringUTF(contentURL),
        env->NewStringUTF(contentTitle),
        env->NewStringUTF(imageURL));

    if (env->ExceptionCheck()) {
        CPE_ERROR(module->m_em, "appsvr_facebook_share_dosdkpay: call loing fail!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        appsvr_share_request_set_done(req, 0);
        return -1;
    }
    appsvr_share_request_set_done(req, 1);
    return 0;
}

extern "C" {
//     JNIEXPORT void JNICALL Java_com_drowgames_facebook_share_facebook_shareOffLineListener_nativeNotifyProductInfoResult(
//         JNIEnv *env, jobject obj, jlong ptr, jstring product_id, jstring price)
//     {
//         appsvr_facebook_share_module_t facebook_share = (appsvr_facebook_share_module_t)(ptr);
//         char * str_product_id = (char*)env->GetStringUTFChars(product_id, NULL);
//         char * str_price= (char*)env->GetStringUTFChars(price, NULL);
//         if(str_product_id==NULL || str_price==NULL)
//         {
//             appsvr_payment_adapter_notify_product_sync_done(facebook_share->m_payment_adapter);
//             return;
//         }
// 
//         appsvr_payment_product_create(facebook_share->m_payment_module,facebook_share->m_payment_adapter,str_product_id,str_price);
// 
//         env->ReleaseStringUTFChars(price, str_price);
//         env->ReleaseStringUTFChars(product_id, str_product_id);
//     }

}

