#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

namespace WRL
{
    using String = std::string;
    using MonitorID = unsigned long;

    class FileMonitorImpl;
    class FileMonitorListener;

    class Exception : public std::runtime_error
    {
    public:
        Exception(const String &message) : std::runtime_error(message) {}
    };

    class FileNotFoundException : public Exception
    {
    public:
        FileNotFoundException() : Exception("File not found") {}

        FileNotFoundException(const String &filename)
            : Exception("File not found (" + filename + ")") {}
    };

    enum class Action
    {
        // Sent when a file is created or renamed
        Add = 1,
        // Sent when a file is deleted or renamed
        Delete = 2,
        // Sent when a file is modified
        Modified = 4
    };

    class FileMonitor
    {
    public:
        FileMonitor();

        virtual ~FileMonitor();

        MonitorID addMonitor(const String &directory, FileMonitorListener *Monitor);

        void removeMonitor(const String &directory);

        void removeMonitor(MonitorID monitorID);

        // Updates the Monitor. Must be called often.
        void update();

    private:
        // The implementation
        FileMonitorImpl *mImpl;
    };

    class FileMonitorListener
    {
    public:
        FileMonitorListener() {}

        virtual ~FileMonitorListener() {}

        virtual void handleFileAction(MonitorID monitorID, const String &dir,
                                      const String &filename, Action action) = 0;
    };

    std::ostream &operator<<(std::ostream &ostream, Action &action);

}
