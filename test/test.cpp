#include "../include/filesystemchangemonitor.h"
#include <iostream>

using WRL::operator<<;

// Process a file action
class UpdateListener : public WRL::FileMonitorListener
{
public:
	UpdateListener() {}
	void handleFileAction(WRL::MonitorID monitorID, const WRL::String &dir, const WRL::String &filename,
						  WRL::Action action)
	{
		std::cout << "DIR (" << dir + ") FILE (" + filename + ") has event " << action << std::endl;
	}
};

int main(int argc, char **argv)
{
	try
	{
		// create the file Monitor object
		WRL::FileMonitor fileMonitor;

		// add a Monitor to the system
		WRL::MonitorID mnitorID = fileMonitor.addMonitor("./test", new UpdateListener());

		std::cout << "Press ^C to exit demo" << std::endl;

		// loop until a key is pressed
		while (true)
		{
			fileMonitor.update();
		}
	}
	catch (std::exception &e)
	{
		fprintf(stderr, "An exception has occurred: %s\n", e.what());
	}

	return 0;
}
