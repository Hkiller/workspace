#include "svr/conn/net_cli/conn_net_cli.h"
#include "conn_net_cli_internal_ops.h"

int conn_net_cli_monitor_add(conn_net_cli_t cli, conn_net_cli_state_process_fun_t process_fun, void * process_ctx) {
    conn_net_cli_monitor_t monitor = mem_alloc(cli->m_alloc, sizeof(struct conn_net_cli_monitor));
    if (monitor == NULL) return -1;

    monitor->m_cli = cli;
    monitor->m_process_fun = process_fun;
    monitor->m_process_ctx = process_ctx;

    TAILQ_INSERT_TAIL(&cli->m_monitors, monitor, m_next);

    return 0;
}

int conn_net_cli_monitor_remove(conn_net_cli_t cli, conn_net_cli_state_process_fun_t process_fun, void * process_ctx) {
    conn_net_cli_monitor_t monitor;

    TAILQ_FOREACH(monitor, &cli->m_monitors, m_next) {
        if (monitor->m_process_fun == process_fun && monitor->m_process_ctx == process_ctx) {
            conn_net_cli_monitor_free(monitor);
            return 0;
        }
    }

    return -1;
}

void conn_net_cli_monitor_process(fsm_machine_t fsm_ins, void * ctx) {
    conn_net_cli_t cli = ctx;
    conn_net_cli_monitor_t monitor;

    TAILQ_FOREACH(monitor, &cli->m_monitors, m_next) {
        monitor->m_process_fun(cli, monitor->m_process_ctx);
    }
}

void conn_net_cli_monitor_free(conn_net_cli_monitor_t monitor) {
    conn_net_cli_t cli = monitor->m_cli;

    TAILQ_REMOVE(&cli->m_monitors, monitor, m_next);

    mem_free(cli->m_alloc, monitor);
}

void conn_net_cli_monitor_free_all(conn_net_cli_t cli) {
    while(!TAILQ_EMPTY(&cli->m_monitors)) {
        conn_net_cli_monitor_free(TAILQ_FIRST(&cli->m_monitors));
    }
}

