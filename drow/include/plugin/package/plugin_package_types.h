#ifndef UI_PLUGIN_PACKAGE_TYPES_H
#define UI_PLUGIN_PACKAGE_TYPES_H
#include "render/model/ui_model_types.h"
#include "render/cache/ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_package_package_state {
    plugin_package_package_empty,       /*空包，用于代位 */ 
    plugin_package_package_downloading, /*下载中 */
    plugin_package_package_installed,   /*已经安装 */
    plugin_package_package_loading,     /*加载中 */
    plugin_package_package_loaded,      /*加载完成 */
} plugin_package_package_state_t;

typedef enum plugin_package_queue_policy {
    plugin_package_queue_policy_manual,   /*手动管理 */ 
    plugin_package_queue_policy_lru,      /*保持一定个数，溢出淘汰最先加入的package */
} plugin_package_queue_policy_t;

typedef enum plugin_package_package_using_state {
    plugin_package_package_using_state_free = 0,
    plugin_package_package_using_state_ref_count = 1,
} plugin_package_package_using_state_t;
    
typedef struct plugin_package_module * plugin_package_module_t;
typedef struct plugin_package_installer * plugin_package_installer_t;
typedef struct plugin_package_package * plugin_package_package_t;
typedef struct plugin_package_package_it * plugin_package_package_it_t; 
typedef struct plugin_package_depend * plugin_package_depend_t;
typedef struct plugin_package_language * plugin_package_language_t;
typedef struct plugin_package_language_it * plugin_package_language_it_t; 
typedef struct plugin_package_queue * plugin_package_queue_t;
typedef struct plugin_package_queue_it * plugin_package_queue_it_t; 
typedef struct plugin_package_group * plugin_package_group_t;
typedef struct plugin_package_region * plugin_package_region_t;
typedef struct plugin_package_region_it * plugin_package_region_it_t;
typedef struct plugin_package_load_task * plugin_package_load_task_t;

#ifdef __cplusplus
}
#endif

#endif
