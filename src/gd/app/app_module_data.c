#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "app_internal_ops.h"


static char g_module_name_prefix[] = "module::";

static struct nm_node_type g_module_root_group = {
    "app_module_root",
    nm_group_free_members
};

static struct nm_node_type g_module_group = {
    "app_module",
    nm_group_free_members
};

nm_node_t
gd_app_module_data_load(
    gd_app_context_t context,
    const char * moduleName)
{
    nm_mgr_t nmm;
    nm_node_t rootGroup;
    nm_node_t moduleGroup;

    size_t nameLen = strlen(moduleName);
    char groupNameBuf[CPE_STACK_BUF_LEN((sizeof(g_module_name_prefix) - 1) + (nameLen + 1))];

    nmm = gd_app_nm_mgr(context);

    rootGroup = nm_mgr_find_node(nmm, gd_app_module_type_root_group_name);
    if (rootGroup == NULL) {
        rootGroup = nm_group_create(nmm, cpe_hs_data(gd_app_module_type_root_group_name), 0);
        if (rootGroup == NULL) {
            APP_CTX_ERROR(context, "%s: create module: data root group fail!", moduleName);
            return NULL;
        }
        nm_node_set_type(rootGroup, &g_module_root_group);
    }

    //TODO: wangjians check sizes
    memcpy(groupNameBuf, g_module_name_prefix, sizeof(g_module_name_prefix) - 1);
    memcpy(groupNameBuf + sizeof(g_module_name_prefix) - 1, moduleName, nameLen + 1);

    moduleGroup = nm_group_create(gd_app_nm_mgr(context), groupNameBuf, 0);
    if (moduleGroup == NULL) {
        APP_CTX_ERROR(context, "%s: create module: data group fail!", groupNameBuf);
        gd_app_module_data_free(context, moduleName);
        return NULL;
    }

    nm_node_set_type(moduleGroup, &g_module_group);

    if (nm_group_add_member(rootGroup, moduleGroup) != 0) {
        APP_CTX_ERROR(context, "%s: create module: add to root group fail!", groupNameBuf);
        nm_node_free(moduleGroup);
        gd_app_module_data_free(context, moduleName);
        return NULL;
    }

    return moduleGroup;
}

void gd_app_module_data_free(gd_app_context_t context, const char * moduleName) {
    nm_mgr_t nmm;
    nm_node_t rootGroup;
    nm_node_t moduleGroup;

    size_t bufCapacity = cpe_hs_len_to_binary_len((sizeof(g_module_name_prefix) - 1) + strlen(moduleName)) ;
    char groupNameBuf[CPE_STACK_BUF_LEN(bufCapacity)];

    nmm = gd_app_nm_mgr(context);

    rootGroup = nm_mgr_find_node(nmm, gd_app_module_type_root_group_name);
    if (rootGroup == NULL) return;

    cpe_hs_init((cpe_hash_string_t)groupNameBuf, sizeof(groupNameBuf), g_module_name_prefix);
    cpe_hs_strcat((cpe_hash_string_t)groupNameBuf, sizeof(groupNameBuf), moduleName);

    moduleGroup = nm_group_find_member(rootGroup, (cpe_hash_string_t)groupNameBuf);
    if (moduleGroup) {
        nm_node_free(moduleGroup);
    }

    if (nm_group_member_count(rootGroup) == 0) {
        nm_node_free(rootGroup);
    }
}

nm_node_t
gd_app_module_data(gd_app_context_t context, const char * moduleName) {
    nm_mgr_t nmm;
    nm_node_t rootGroup;

    size_t bufCapacity = cpe_hs_len_to_binary_len((sizeof(g_module_name_prefix) - 1) + strlen(moduleName));
    char groupNameBuf[CPE_STACK_BUF_LEN(bufCapacity)];
    cpe_hs_init((cpe_hash_string_t)groupNameBuf, sizeof(groupNameBuf), g_module_name_prefix);
    cpe_hs_strcat((cpe_hash_string_t)groupNameBuf, sizeof(groupNameBuf), moduleName);

    nmm = gd_app_nm_mgr(context);

    rootGroup = nm_mgr_find_node(nmm, gd_app_module_type_root_group_name);

    return rootGroup ?
        nm_group_find_member(rootGroup, (cpe_hash_string_t)groupNameBuf)
        : NULL;
}


CPE_HS_DEF_VAR(gd_app_module_type_root_group_name, "app_modules");
