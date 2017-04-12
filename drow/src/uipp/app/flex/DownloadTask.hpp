#ifndef UIPP_APP_ENV_FLEX_DOWNLOAD_TASK_H
#define UIPP_APP_ENV_FLEX_DOWNLOAD_TASK_H
#include "ListenerWrap.hpp"

class DownloadTask : public ListenerWrap<DownloadTask> {
public:
    enum State { Runing, Error, Complete };
    
    DownloadTask(DownloadMonitor & monitor, gd_app_context_t app, const char * url);
    ~DownloadTask();

    State state(void) const { return m_state; }
    float progress(void) const { return m_progress; }
    void setError(void);

    void * readData(mem_buffer_t buffer);
    
private:
    void on_io_error(var as3Args);
    void on_security_error(var as3Args);
    void on_complete(var as3Args);
    void on_progress(var as3Args);
    static const char * state(State s);
    void start(void);

    DownloadMonitor & m_monitor;
    gd_app_context_t m_app;
    ::std::string m_url;
    flash::net::URLStream m_stream;
    State m_state;
    float m_progress;
    bool m_complete_called;
};

#endif
