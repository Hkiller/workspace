#include <assert.h>
#include <pthread.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "cpe/vfs/vfs_manage.h"
#include "cpe/vfs/vfs_backend.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_entry_info.h"
#include "cpe/vfs/vfs_mount_point.h"
#include "gd/app/app_context.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "plugin_app_vfs_backend_i.h"

static pthread_key_t g_android_asset_mgr_key = 0;
static jobject g_asset_mgr = NULL;

static AAssetManager * android_asset_mgr(void) {
    AAssetManager * asset_mgr;
    
    assert(g_asset_mgr);
    assert(g_android_asset_mgr_key);
    asset_mgr = (AAssetManager*)pthread_getspecific(g_android_asset_mgr_key);

    if (asset_mgr == NULL) {
        JNIEnv * env = (JNIEnv*)android_jni_env();

        asset_mgr = AAssetManager_fromJava(env, g_asset_mgr);
        assert(asset_mgr);
		pthread_setspecific(g_android_asset_mgr_key, asset_mgr);
    }
    
    return asset_mgr;
}

extern "C"
int android_asset_set_mgr(JNIEnv* env, jobject assetManager) {
    g_asset_mgr = env->NewGlobalRef(assetManager);

    //assert(g_android_asset_mgr_key == 0);
    pthread_key_create(&g_android_asset_mgr_key, NULL);
    assert(g_android_asset_mgr_key != 0);

    return 0;
}

static int ui_app_android_vfs_file_open(void * ctx, void * env, vfs_file_t file, const char * path, const char * mode) {
    plugin_app_env_module_t module = (plugin_app_env_module_t)ctx;
    AAsset * asset = AAssetManager_open(android_asset_mgr(), path, AASSET_MODE_UNKNOWN);
    if (asset == NULL) {
        CPE_ERROR(module->m_em, "ui_app_android_vfs_file_open: file %s not exist in assets", path);
        return -1;
    }

    *(AAsset **)vfs_file_data(file) = asset;
        
    return 0;
}

static void ui_app_android_vfs_file_close(void * ctx, vfs_file_t file) {
    AAsset * asset = *(AAsset **)vfs_file_data(file);
    AAsset_close(asset);
}

static ssize_t ui_app_android_vfs_file_read(void * ctx, vfs_file_t file, void * buf, size_t size) {
    AAsset * asset = *(AAsset **)vfs_file_data(file);
    return (ssize_t) AAsset_read(asset, buf, size);  
}

static ssize_t ui_app_android_vfs_file_write(void * ctx, vfs_file_t file, const void * buf, size_t size) {
    plugin_app_env_module_t module = (plugin_app_env_module_t)ctx;
    CPE_ERROR(module->m_em, "ui_app_android_vfs_file_write: android asset not support write");
    return -1;
}

static int ui_app_android_vfs_file_seek(void * ctx, vfs_file_t file, ssize_t off, vfs_file_seek_op_t op) {
    AAsset * asset = *(AAsset **)vfs_file_data(file);

    switch(op) {
    case vfs_file_seek_set:
        return (int)AAsset_seek(asset, off, SEEK_SET);
    case vfs_file_seek_cur:
        return (int)AAsset_seek(asset, off, SEEK_CUR);
    case vfs_file_seek_end:
        return (int)AAsset_seek(asset, off, SEEK_END);
    default:
        return -1;
    }
}

static ssize_t ui_app_android_vfs_file_tell(void * ctx, vfs_file_t file) {
    AAsset * asset = *(AAsset **)vfs_file_data(file);

    return (ssize_t) AAsset_getLength(asset) - (ssize_t) AAsset_getRemainingLength(asset);
}

static uint8_t ui_app_android_vfs_file_eof(void * ctx, vfs_file_t file) {
    AAsset * asset = *(AAsset **)vfs_file_data(file);

    return AAsset_getRemainingLength(asset) == 0 ? 1 : 0;
}

static int ui_app_android_vfs_file_flush(void * ctx, vfs_file_t file) {
    plugin_app_env_module_t module = (plugin_app_env_module_t)ctx;
    CPE_ERROR(module->m_em, "ui_app_android_vfs_file_flush: android asset not support flush");
    return -1;
}

static ssize_t ui_app_android_vfs_file_size(void * ctx, vfs_file_t file) {
    AAsset * asset = *(AAsset **)vfs_file_data(file);
    return (ssize_t) AAsset_getLength(asset);
}

static ssize_t ui_app_android_vfs_file_size_by_path(void * ctx, void * env, const char * path) {
    AAsset * asset;
    
    asset = AAssetManager_open(android_asset_mgr(), path, AASSET_MODE_UNKNOWN);
    if (asset == NULL) return -1;

    ssize_t rv = AAsset_getLength(asset);

    AAsset_close(asset);

    return rv;
}

static uint8_t ui_app_android_vfs_file_exist(void * ctx, void * env, const char * path) {
    AAsset * asset;
    
    asset = AAssetManager_open(android_asset_mgr(), path, AASSET_MODE_UNKNOWN);
    if (asset == NULL) return 0;

    AAsset_close(asset);
    return 1;
}

static int ui_app_android_vfs_dir_open(void * ctx, void * env, vfs_dir_t dir, const char * path) {
    plugin_app_env_module_t module = (plugin_app_env_module_t)ctx;
    AAssetDir * dirp = AAssetManager_openDir(android_asset_mgr(), path);
    if (dirp == NULL) {
        CPE_ERROR(module->m_em, "ui_app_android_vfs_dir_open: dir %s not exist in assets", path);
        return -1;
    }

    *(AAssetDir **)vfs_dir_data(dir) = dirp;

    return 0;
}

static void ui_app_android_vfs_dir_close(void * ctx, vfs_dir_t dir) {
    AAssetDir * dirp = *(AAssetDir **)vfs_dir_data(dir);
    AAssetDir_close(dirp);
}

struct ui_app_android_vfs_dir_it_data {
    AAssetDir * m_asset_dir;
    struct vfs_entry_info m_entry;
};

static vfs_entry_info_t ui_app_android_vfs_dir_it_next(struct vfs_entry_info_it * it) {
    struct ui_app_android_vfs_dir_it_data * it_data = (struct ui_app_android_vfs_dir_it_data *)(void*)it->m_data;

    const char * file_name = AAssetDir_getNextFileName(it_data->m_asset_dir);
    if (file_name == NULL) return NULL;


    it_data->m_entry.m_name = file_name;
    it_data->m_entry.m_type = vfs_entry_file;
    return &it_data->m_entry;
}

static void ui_app_android_vfs_dir_read(void * ctx, vfs_dir_t dir, vfs_entry_info_it_t it) {
    struct ui_app_android_vfs_dir_it_data * it_data = (struct ui_app_android_vfs_dir_it_data *)(void*)it->m_data;
    
    it->next = ui_app_android_vfs_dir_it_next;
    it_data->m_asset_dir = *(AAssetDir **)vfs_dir_data(dir);
}

static uint8_t ui_app_android_vfs_dir_exist(void * ctx, void * env, const char * path) {
    AAssetDir * dirp = AAssetManager_openDir(android_asset_mgr(), path);
    if (dirp == NULL) return 0;

    AAssetDir_close(dirp);
    return 1;
}

int ui_app_android_vfs_backend_init(plugin_app_env_backend_t ab) {
    return
        vfs_backend_create(
            gd_app_vfs_mgr(ab->m_module->m_app), "android", ab->m_module,
            /*env*/
            NULL,
            /*file*/
            sizeof(AAsset*), ui_app_android_vfs_file_open, ui_app_android_vfs_file_close,
            ui_app_android_vfs_file_read, ui_app_android_vfs_file_write, ui_app_android_vfs_file_flush, 
            ui_app_android_vfs_file_seek, ui_app_android_vfs_file_tell, ui_app_android_vfs_file_eof,
            ui_app_android_vfs_file_size,
            ui_app_android_vfs_file_size_by_path,
            ui_app_android_vfs_file_exist,
            NULL,
            /*dir*/
            sizeof(AAssetDir *), ui_app_android_vfs_dir_open, ui_app_android_vfs_dir_close, ui_app_android_vfs_dir_read,
            ui_app_android_vfs_dir_exist,
            NULL,
            NULL,
            NULL)
        == NULL
        ? -1
        : 0;
}

void ui_app_android_vfs_backend_fini(plugin_app_env_backend_t ab) {
    vfs_backend_t backend = vfs_backend_find_by_name(gd_app_vfs_mgr(ab->m_module->m_app), "android");
    if (backend) {
        vfs_backend_free(backend);
    }
}

int ui_app_android_vfs_backend_mount(plugin_app_env_backend_t ab) {
    vfs_backend_t backend = vfs_backend_find_by_name(gd_app_vfs_mgr(ab->m_module->m_app), "android");
    if (backend == NULL) {
        CPE_ERROR(ab->m_module->m_em, "ui_app_android_vfs_backend_mount: backend not exist!");
        return -1;
    }

    vfs_mgr_t vfs = gd_app_vfs_mgr(ab->m_module->m_app);
    vfs_mount_point_t mp = vfs_mgr_current_point(vfs);
    if (mp == NULL) {
        CPE_ERROR(ab->m_module->m_em, "ui_app_android_vfs_backend_mount: no current point!");
        return -1;
    }

    if (vfs_mount_point_mount(mp, "", NULL, backend) == 0) {
        CPE_ERROR(ab->m_module->m_em, "ui_app_android_vfs_backend_mount: mount current point!");
        return -1;
    }
    
    return 0;
}

void ui_app_android_vfs_backend_unmount(plugin_app_env_backend_t backend) {
    vfs_mgr_t vfs = gd_app_vfs_mgr(backend->m_module->m_app);
    vfs_mount_point_t mp = vfs_mgr_current_point(vfs);
    if (mp) {
        vfs_mount_point_unmount(mp, "");
    }
}
