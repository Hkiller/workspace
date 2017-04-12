#include <assert.h>
#include "cpe/utils/buffer.h"
#include "DownloadTask.hpp"
#include "DownloadMonitor.hpp"

DownloadTask::DownloadTask(DownloadMonitor & monitor, gd_app_context_t app, const char * url)
    : ListenerWrapBase(::std::string("download: ") + url)
    , m_monitor(monitor)
    , m_app(app)
    , m_url(url)
    , m_state(Runing)
    , m_complete_called(false)
{
    m_stream = flash::net::URLStream::_new();
    addEventListener(m_stream, flash::events::SecurityErrorEvent::SECURITY_ERROR, &DownloadTask::on_security_error);
    addEventListener(m_stream, flash::events::IOErrorEvent::IO_ERROR, &DownloadTask::on_io_error);
    addEventListener(m_stream, flash::events::Event::COMPLETE, &DownloadTask::on_complete);
    addEventListener(m_stream, flash::events::ProgressEvent::PROGRESS, &DownloadTask::on_progress);
    start();
}

DownloadTask::~DownloadTask() {
    try {
        m_stream->stop();
    }
    catch(var e) {
        char *err = internal::utf8_toString(e);
        APP_CTX_ERROR(m_app, "download: %s: stop: %s", m_url.c_str(), err);
        free(err);
    }
    
    //APP_CTX_INFO(m_app, "download: %s: task destory", m_url.c_str());
}

void DownloadTask::start(void) {
    try {
        m_stream->load(flash::net::URLRequest::_new(m_url.c_str()));
        m_state = Runing;
        APP_CTX_INFO(m_app, "download: %s: started", m_url.c_str());
    }
    catch (var e) {
        m_state = Error;
        char *err = internal::utf8_toString(e);
        APP_CTX_ERROR(m_app, "download: %s: start fail: %s", m_url.c_str(), err);
        free(err);
    }
}

void DownloadTask::on_io_error(var as3Args) {
    char *err = internal::utf8_toString(var(as3Args[0]));
    APP_CTX_ERROR(m_app, "download: %s: on_io_error: %s", m_url.c_str(), err);
    free(err);

    m_monitor.onRestart(*this);
    start();
}

void DownloadTask::on_security_error(var as3Args) {
    if (m_state == Runing) {
        m_state = Error;
        if (!m_complete_called) {
            m_complete_called = true;
            m_monitor.onComplete(*this);
        }
    }

    char *err = internal::utf8_toString(var(as3Args[0]));
    APP_CTX_ERROR(m_app, "download: %s: on_security_error: %s", m_url.c_str(), err);
    free(err);
}

void DownloadTask::on_complete(var as3Args) {
    if (m_state == Runing) {
        //APP_CTX_INFO(m_app, "download: %s: complete", m_url.c_str());
        m_state = Complete;
        m_complete_called = true;
        m_monitor.onComplete(*this);
    }
    else {
        APP_CTX_INFO(m_app, "download: %s: complete with state %s", m_url.c_str(), state(m_state));
    }
}

void DownloadTask::on_progress(var as3Args) {
    if (m_state != Runing) {
        APP_CTX_INFO(m_app, "download: %s: progress with state %s", m_url.c_str(), state(m_state));
        return;
    }

    if (m_stream->bytesAvailable) {
        flash::events::ProgressEvent evt = var(as3Args[0]);
        m_monitor.onProgress(*this, evt->bytesTotal, evt->bytesLoaded);
    }
}

void * DownloadTask::readData(mem_buffer_t buffer) {
    mem_buffer_clear_data(buffer);

    uint32_t sz = m_stream->bytesAvailable;
    void * buf = mem_buffer_alloc(buffer, sz);
    if (buf == NULL) return NULL;

    m_stream->readBytes(internal::get_ram(), (unsigned)buf, sz, buf);

    return buf;
}

void DownloadTask::setError(void) {
    try {
        if (m_state == Runing) {
            m_state = Error;
            m_stream->stop();
        }
        else {
            m_state = Error;
        }

        if (!m_complete_called) {
            m_complete_called = true;
            m_monitor.onComplete(*this);
        }
    }
    catch(var e) {
        char *err = internal::utf8_toString(e);
        APP_CTX_ERROR(m_app, "download: %s: setError: %s", m_url.c_str(), err);
        free(err);
    }

}

const char * DownloadTask::state(State s) {
    switch(s) {
    case Runing:
        return "Runing";
    case Error:
        return "Error";
    case Complete:
        return "Complete";
    default:
        return "Unknown";
    }
}

/*DownloadMonitor*/
DownloadMonitor::~DownloadMonitor() {
}
