#include "../include/filesystemchangemonitor.h"
#include "../include/filesystemchangemonitorimpl.h"

#if FILEMONITOR_PLATFORM == FILEMONITOR_PLATFORM_WINDOWS
#include "../include/filesystemchangemonitorwindows.h"
#define FILEMONITOR_IMPL FileMonitorWindows
#elif FILEMONITOR_PLATFORM == FILEMONITOR_PLATFORM_LINUX
#include "../include/filesystemchangemonitorlinux.h"
#define FILEMONITOR_IMPL FileMonitorLinux
#endif

namespace WRL
{
    FileMonitor::FileMonitor()
    {
        mImpl = new FILEMONITOR_IMPL();
    }

    FileMonitor::~FileMonitor()
    {
        delete mImpl;
        mImpl = 0;
    }

    MonitorID FileMonitor::addMonitor(const String &directory, FileMonitorListener *Monitorer)
    {
        return mImpl->addMonitor(directory, Monitorer);
    }

    void FileMonitor::removeMonitor(const String &directory)
    {
        mImpl->removeMonitor(directory);
    }

    void FileMonitor::removeMonitor(MonitorID monitorID)
    {
        mImpl->removeMonitor(monitorID);
    }

    void FileMonitor::update()
    {
        mImpl->update();
    }
}
