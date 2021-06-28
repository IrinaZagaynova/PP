#include "Worker.h"
#include "Task.h"
#include <string>

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		return 1;
	}

	int threadsCount = std::stoi(argv[1]);
	CWorker worker;

	for (size_t i = 0; i < threadsCount; i++)
	{
		ITask* task = new CTask(i + 1);		
		Sleep(100);
		worker.ExecuteTask(task);
	}
}

