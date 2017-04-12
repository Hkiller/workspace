#include "cpe/pal/pal_external.h" 
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_socket.h"

#ifdef _MSC_VER

int g_wsaData_ref = 0;
WSADATA g_wsaData;

EXPORT_DIRECTIVE
int app_net_env_global_init(void) {
    if (g_wsaData_ref == 0) {
        if (WSAStartup(MAKEWORD(2,2), &g_wsaData) != 0) {
            printf("WSAStartup failed with error %d\n", WSAGetLastError());
            return -1;
        }
    }

    ++g_wsaData_ref;
    return 0;
}

EXPORT_DIRECTIVE
void app_net_env_global_fini() {
    --g_wsaData_ref;
    if (g_wsaData_ref == 0) {
        if (WSACleanup() == SOCKET_ERROR) {
            printf("WSACleanup failed with error %d\n", WSAGetLastError());
        }
    }
}

#else

EXPORT_DIRECTIVE
int app_net_env_global_init(void) {
    return 0;
}

EXPORT_DIRECTIVE
void app_net_env_global_fini() {
}

#endif
