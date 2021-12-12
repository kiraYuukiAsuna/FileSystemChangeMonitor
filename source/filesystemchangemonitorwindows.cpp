#include "../include/filesystemchangemonitorwindows.h"

#if FILEMONITOR_PLATFORM == FILEMONITOR_PLATFORM_WINDOWS

#define _WIN32_WINNT 0x0550
#include <windows.h>

#if defined(_MSC_VER)
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")

// disable secure warnings
#pragma warning(disable : 4996)
#endif

namespace WRL
{
    // Internal Monitor data
    struct MonitorStruct
    {
        OVERLAPPED mOverlapped;
        HANDLE mDirHandle;
        BYTE mBuffer[32 * 1024];
        LPARAM lParam;
        DWORD mNotifyFilter;
        bool mStopNow;
        FileMonitorImpl *mFileMonitor;
        FileMonitorListener *mFileMonitorListener;
        char *mDirName;
        MonitorID mMonitorID;
    };

#pragma region Internal Functions

    // forward decl
    bool RefreshMonitor(MonitorStruct *pMonitor, bool _clear = false);

    // Unpacks events and passes them to a user defined callback.
    void CALLBACK MonitorCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
    {
        TCHAR szFile[MAX_PATH];
        PFILE_NOTIFY_INFORMATION pNotify;
        MonitorStruct *pMonitor = (MonitorStruct *)lpOverlapped;
        size_t offset = 0;

        if (dwNumberOfBytesTransfered == 0)
            return;

        if (dwErrorCode == ERROR_SUCCESS)
        {
            do
            {
                pNotify = (PFILE_NOTIFY_INFORMATION)&pMonitor->mBuffer[offset];
                offset += pNotify->NextEntryOffset;

#if defined(UNICODE)
                {
                    lstrcpynW(szFile, pNotify->FileName,
                              min(MAX_PATH, pNotify->FileNameLength / sizeof(WCHAR) + 1));
                }
#else
                {
                    int count = WideCharToMultiByte(CP_ACP, 0, pNotify->FileName,
                                                    pNotify->FileNameLength / sizeof(WCHAR),
                                                    szFile, MAX_PATH - 1, NULL, NULL);
                    szFile[count] = TEXT('\0');
                }
#endif

                pMonitor->mFileMonitor->handleAction(pMonitor, szFile, pNotify->Action);

            } while (pNotify->NextEntryOffset != 0);
        }

        if (!pMonitor->mStopNow)
        {
            RefreshMonitor(pMonitor);
        }
    }

    // Refreshes the directory monitoring.
    bool RefreshMonitor(MonitorStruct *pMonitor, bool _clear)
    {
        return ReadDirectoryChangesW(
                   pMonitor->mDirHandle, pMonitor->mBuffer, sizeof(pMonitor->mBuffer), FALSE,
                   pMonitor->mNotifyFilter, NULL, &pMonitor->mOverlapped, _clear ? 0 : MonitorCallback) != 0;
    }

    // Stops monitoring a directory.
    void DestroyMonitor(MonitorStruct *pMonitor)
    {
        if (pMonitor)
        {
            pMonitor->mStopNow = TRUE;

            CancelIo(pMonitor->mDirHandle);

            RefreshMonitor(pMonitor, true);

            if (!HasOverlappedIoCompleted(&pMonitor->mOverlapped))
            {
                SleepEx(5, TRUE);
            }

            CloseHandle(pMonitor->mOverlapped.hEvent);
            CloseHandle(pMonitor->mDirHandle);
            delete pMonitor->mDirName;
            HeapFree(GetProcessHeap(), 0, pMonitor);
        }
    }

    // Starts monitoring a directory.
    MonitorStruct *CreateMonitor(LPCTSTR szDirectory, DWORD mNotifyFilter)
    {
        MonitorStruct *pMonitor;
        size_t ptrsize = sizeof(*pMonitor);
        pMonitor = static_cast<MonitorStruct *>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ptrsize));

        pMonitor->mDirHandle = CreateFile(szDirectory, FILE_LIST_DIRECTORY,
                                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                                          OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

        if (pMonitor->mDirHandle != INVALID_HANDLE_VALUE)
        {
            pMonitor->mOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            pMonitor->mNotifyFilter = mNotifyFilter;

            if (RefreshMonitor(pMonitor))
            {
                return pMonitor;
            }
            else
            {
                CloseHandle(pMonitor->mOverlapped.hEvent);
                CloseHandle(pMonitor->mDirHandle);
            }
        }

        HeapFree(GetProcessHeap(), 0, pMonitor);
        return NULL;
    }

#pragma endregion

    FileMonitorWindows::FileMonitorWindows()
        : mLastMonitorID(0)
    {
    }

    FileMonitorWindows::~FileMonitorWindows()
    {
        MonitorMap::iterator iter = mMonitores.begin();
        MonitorMap::iterator end = mMonitores.end();
        for (; iter != end; ++iter)
        {
            DestroyMonitor(iter->second);
        }
        mMonitores.clear();
    }

    MonitorID FileMonitorWindows::addMonitor(const String &directory, FileMonitorListener *monitor)
    {
        MonitorID monitorID = ++mLastMonitorID;

        MonitorStruct *Monitor = CreateMonitor(directory.c_str(),
                                               FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME);

        if (!Monitor)
            throw FileNotFoundException(directory);

        Monitor->mMonitorID = monitorID;
        Monitor->mFileMonitor = this;
        Monitor->mFileMonitorListener = monitor;
        Monitor->mDirName = new char[directory.length() + 1];
        strcpy(Monitor->mDirName, directory.c_str());

        mMonitores.insert(std::make_pair(monitorID, Monitor));

        return monitorID;
    }

    void FileMonitorWindows::removeMonitor(const String &directory)
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

    void FileMonitorWindows::removeMonitor(MonitorID monitorID)
    {
        MonitorMap::iterator iter = mMonitores.find(monitorID);

        if (iter == mMonitores.end())
            return;

        MonitorStruct *Monitor = iter->second;
        mMonitores.erase(iter);

        DestroyMonitor(Monitor);
    }

    void FileMonitorWindows::update()
    {
        MsgWaitForMultipleObjectsEx(0, NULL, 0, QS_ALLINPUT, MWMO_ALERTABLE);
    }

    void FileMonitorWindows::handleAction(MonitorStruct *Monitor, const String &filename, unsigned long action)
    {
        Action fwAction;

        switch (action)
        {
        case FILE_ACTION_RENAMED_NEW_NAME:
        case FILE_ACTION_ADDED:
            fwAction = Action::Add;
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
        case FILE_ACTION_REMOVED:
            fwAction = Action::Delete;
            break;
        case FILE_ACTION_MODIFIED:
            fwAction = Action::Modified;
            break;
        };

        Monitor->mFileMonitorListener->handleFileAction(Monitor->mMonitorID, Monitor->mDirName, filename, fwAction);
    }
}

#endif
