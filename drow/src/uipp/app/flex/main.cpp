#include "FlexApp.hpp"

static FlexApp * g_app = NULL;

int main(int argc, char **argv) {
    printf("begin new FlexApp");
    g_app = new FlexApp();
    printf("end new FlexApp");
    int rv = g_app->main(argc, argv);
    if (rv != 0) {
        delete g_app;
        g_app = NULL;
    }
    return rv;
}

extern "C" void do_exit(void) {
    if (g_app == NULL) return;

    delete g_app;
    g_app = NULL;
}

