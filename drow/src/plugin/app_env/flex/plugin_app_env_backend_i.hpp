#ifndef PLUGIN_APP_ENV_BACKEND_I_H
#define PLUGIN_APP_ENV_BACKEND_I_H
#include "../plugin_app_env_module_i.h"

typedef struct plugin_app_env_socket * plugin_app_env_socket_t;

struct plugin_app_env_backend {
    plugin_app_env_module_t m_module;
    cpe_hash_table m_sockets;
};

#endif
