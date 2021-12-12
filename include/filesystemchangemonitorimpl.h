#pragma once

#include "filesystemchangemonitor.h"

#define FILEMONITOR_PLATFORM_WINDOWS 1
#define FILEMONITOR_PLATFORM_LINUX 2

#if defined(_WIN32)
#define FILEMONITOR_PLATFORM FILEMONITOR_PLATFORM_WINDOWS
#elif defined(__linux__)
#define FILEMONITOR_PLATFORM FILEMONITOR_PLATFORM_LINUX
#endif

namespace WRL
{
    struct MonitorStruct;

    class FileMonitorImpl
    {
    public:
        FileMonitorImpl() {}

        virtual ~FileMonitorImpl() {}

        virtual MonitorID addMonitor(const String &directory, FileMonitorListener *Monitorer) = 0;

        virtual void removeMonitor(const String &directory) = 0;

        virtual void removeMonitor(MonitorID monitorID) = 0;

        // Updates the Monitorer. Must be called often.
        virtual void update() = 0;

        // Handles the action
        virtual void handleAction(MonitorStruct *Monitor, const String &filename, unsigned long action) = 0;
    };

}
