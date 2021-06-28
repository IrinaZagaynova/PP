#pragma once
#pragma once
#include "IWorker.h"
#include "windows.h"
#include <iostream>

class CWorker : public IWorker
{
public:

	~CWorker()
	{
		WaitForSingleObject(thread, INFINITE);
	}

	bool ExecuteTask(ITask* task)
	{
		if (!IsBusy())
		{
			thread = CreateThread(NULL, 0, &ThreadProc, task, 0, NULL);
			return true;
		}
		return false;
	}

	bool IsBusy()
	{
		DWORD exitcode;
		GetExitCodeThread(thread, &exitcode);
		return exitcode == STILL_ACTIVE;
	}

private:
	static DWORD WINAPI ThreadProc(LPVOID index)
	{
		ITask* task = (ITask*)index;
		task->Execute();
		ExitThread(0);
	}

private:
	bool m_isBusy = false;
	HANDLE thread;
};