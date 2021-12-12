#include "../include/filesystemchangemonitorlinux.h"

#if FILEMONITOR_PLATFORM == FILEMONITOR_PLATFORM_LINUX

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFF_SIZE ((sizeof(struct inotify_event) + FILENAME_MAX) * 1024)

namespace WRL
{

    struct MonitorStruct
    {
        MonitorID mMonitorID;
        String mDirName;
        FileMonitorListener *mListener;
    };

    FileMonitorLinux::FileMonitorLinux()
    {
        mFD = inotify_init();
        if (mFD < 0)
            fprintf(stderr, "Error: %s\n", strerror(errno));

        mTimeOut.tv_sec = 0;
        mTimeOut.tv_usec = 0;

        FD_ZERO(&mDescriptorSet);
    }

    FileMonitorLinux::~FileMonitorLinux()
    {
        MonitorMap::iterator iter = mMonitores.begin();
        MonitorMap::iterator end = mMonitores.end();
        for (; iter != end; ++iter)
        {
            delete iter->second;
        }
        mMonitores.clear();
    }

    MonitorID FileMonitorLinux::addMonitor(const String &directory, FileMonitorListener *monitor)
    {
        int wd = inotify_add_watch(mFD, directory.c_str(),
                                   IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_MOVED_FROM | IN_DELETE);
        if (wd < 0)
        {
            if (errno == ENOENT)
                throw FileNotFoundException(directory);
            else
                throw Exception(strerror(errno));

            //			fprintf (stderr, "Error: %s\n", strerror(errno));
            //			return -1;
        }

        MonitorStruct *pMonitor = new MonitorStruct();
        pMonitor->mListener = monitor;
        pMonitor->mMonitorID = wd;
        pMonitor->mDirName = directory;

        mMonitores.insert(std::make_pair(wd, pMonitor));

        return wd;
    }

    void FileMonitorLinux::removeMonitor(const String &directory)
    {
        MonitorMap::iterator iter = mMonitores.begin();
        MonitorMap::iterator end = mMonitores.end();
        for (; iter != end; ++iter)
        {
            if (directory == iter->second->mDirName)
            {
                removeMonitor(iter->first);
                return;
            }
        }
    }

    void FileMonitorLinux::removeMonitor(MonitorID monitorID)
    {
        MonitorMap::iterator iter = mMonitores.find(monitorID);

        if (iter == mMonitores.end())
            return;

        MonitorStruct *Monitor = iter->second;
        mMonitores.erase(iter);

        inotify_rm_watch(mFD, monitorID);

        delete Monitor;
        Monitor = 0;
    }

    void FileMonitorLinux::update()
    {
        FD_SET(mFD, &mDescriptorSet);

        int ret = select(mFD + 1, &mDescriptorSet, NULL, NULL, &mTimeOut);
        if (ret < 0)
        {
            perror("select");
        }
        else if (FD_ISSET(mFD, &mDescriptorSet))
        {
            ssize_t len, i = 0;
            char action[81 + FILENAME_MAX] = {0};
            char buff[BUFF_SIZE] = {0};

            len = read(mFD, buff, BUFF_SIZE);

            while (i < len)
            {
                struct inotify_event *pevent = (struct inotify_event *)&buff[i];

                MonitorStruct *Monitor = mMonitores[pevent->wd];
                handleAction(Monitor, pevent->name, pevent->mask);
                i += sizeof(struct inotify_event) + pevent->len;
            }
        }
    }

    void FileMonitorLinux::handleAction(MonitorStruct *Monitor, const String &filename, unsigned long action)
    {
        if (!Monitor->mListener)
            return;

        if (IN_CLOSE_WRITE & action)
        {
            Monitor->mListener->handleFileAction(Monitor->mMonitorID, Monitor->mDirName, filename,
                                                 Action::Modified);
        }
        if (IN_MOVED_TO & action || IN_CREATE & action)
        {
            Monitor->mListener->handleFileAction(Monitor->mMonitorID, Monitor->mDirName, filename,
                                                 Action::Add);
        }
        if (IN_MOVED_FROM & action || IN_DELETE & action)
        {
            Monitor->mListener->handleFileAction(Monitor->mMonitorID, Monitor->mDirName, filename,
                                                 Action::Delete);
        }
    }

}

#endif
