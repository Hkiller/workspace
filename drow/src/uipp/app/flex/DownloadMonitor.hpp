#ifndef UIPP_APP_ENV_FLEX_DOWNLOAD_MONITOR_H
#define UIPP_APP_ENV_FLEX_DOWNLOAD_MONITOR_H
#include "System.hpp"

class DownloadMonitor {
public:
    virtual void onRestart(DownloadTask & task) = 0;
    virtual void onComplete(DownloadTask & task) = 0;
    virtual void onProgress(DownloadTask & task, int32_t bytesTotal, int32_t bytesLoaded) = 0;
    virtual ~DownloadMonitor();
};

#endif
