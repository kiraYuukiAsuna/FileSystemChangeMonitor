#pragma once

#include "filesystemchangemonitorimpl.h"

#if FILEMonitorER_PLATFORM == FILEMonitorER_PLATFORM_WINDOWS

#include <map>

namespace WRL
{
    // Implementation for Win32 based on ReadDirectoryChangesW.
    class FileMonitorWindows : public FileMonitorImpl
    {
    public:
        // type for a map from MonitorID to MonitorStruct pointer
        typedef std::map<MonitorID, MonitorStruct *> MonitorMap;

    public:
        FileMonitorWindows();

        virtual ~FileMonitorWindows();

        MonitorID addMonitor(const String &directory, FileMonitorListener *Monitorer);

        void removeMonitor(const String &directory);

        void removeMonitor(MonitorID monitorID);

        // Updates the Monitorer. Must be called often.
        void update();

        // Handles the action
        void handleAction(MonitorStruct *Monitor, const String &filename, unsigned long action);

    private:
        // Map of MonitorID to MonitorStruct pointers
        MonitorMap mMonitores;
        // The last monitorID
        MonitorID mLastMonitorID;
    };

}

#endif
