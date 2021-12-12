#pragma once

#include "filesystemchangemonitorimpl.h"

#if FILEMONITOR_PLATFORM == FILEMONITOR_PLATFORM_LINUX
#include <map>
#include <sys/types.h>

namespace WRL
{
    class FileMonitorLinux : public FileMonitorImpl
    {
    public:
        // type for a map from MonitorID to MonitorStruct pointer
        typedef std::map<MonitorID, MonitorStruct *> MonitorMap;

    public:
        FileMonitorLinux();

        virtual ~FileMonitorLinux();

        MonitorID addMonitor(const String &directory, FileMonitorListener *Monitor);

        void removeMonitor(const String &directory);

        void removeMonitor(MonitorID monitorID);

        // Updates the Monitor. Must be called often.
        void update();

        // Handles the action
        void handleAction(MonitorStruct *Monitor, const String &filename, unsigned long action);

    private:
        // Map of MonitorID to MonitorStruct pointers
        MonitorMap mMonitores;
        // The last monitorID
        MonitorID mLastMonitorID;
        // inotify file descriptor
        int mFD;
        // time out data
        struct timeval mTimeOut;
        // File descriptor set
        fd_set mDescriptorSet;
    };
}

#endif
