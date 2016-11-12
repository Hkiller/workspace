#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/pal/pal_unistd.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "app_internal_ops.h"

#ifdef GD_APP_MULTI_THREAD

uint8_t g_app_ins_key_init = 0;
pthread_key_t g_app_ins_key;

int gd_app_ms_global_init(void) {
    assert(g_app_ins_key_init == 0);
    if (pthread_key_create(&g_app_ins_key, NULL) != 0) {
        assert(0 && "pthread key create fail!");
        return -1;
    }

    g_app_ins_key_init = 1;

    return 0;
}

void gd_app_ms_global_fini(void) {
    assert(g_app_ins_key_init == 1);
    pthread_key_delete(g_app_ins_key);
    g_app_ins_key_init = 0;
}

gd_app_context_t gd_app_ins(void) {
    if (!g_app_ins_key_init) return NULL;
    return (gd_app_context_t)pthread_getspecific(g_app_ins_key);
}

void gd_app_ins_set(gd_app_context_t context) {
    int r;

    assert(g_app_ins_key_init);

    r = pthread_setspecific(g_app_ins_key, context);

    assert(r == 0);
}

#else

gd_app_context_t g_app_context_ins = NULL;

gd_app_context_t gd_app_ins(void) {
    return g_app_context_ins;
}

void gd_app_ins_set(gd_app_context_t context) {
    g_app_context_ins = context;
}

#endif
