#ifndef PLUGIN_APP_ENV_ANDROID_VFS_BACKEND_I_H
#define PLUGIN_APP_ENV_ANDROID_VFS_BACKEND_I_H
#include "plugin_app_env_backend_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_app_android_vfs_backend_init(plugin_app_env_backend_t backend);
void ui_app_android_vfs_backend_fini(plugin_app_env_backend_t backend);
int ui_app_android_vfs_backend_mount(plugin_app_env_backend_t backend);
void ui_app_android_vfs_backend_unmount(plugin_app_env_backend_t backend);

#ifdef __cplusplus
}
#endif
    
#endif
