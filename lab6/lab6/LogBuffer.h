#pragma once
#include "LinkedList.h"
#include "LogFileWriter.h"
#include <Windows.h>

const int MAX_BUFFER_SIZE = 100;

class LogBuffer
{
public:
	LogBuffer()
	{
		InitializeCriticalSectionAndSpinCount(&m_criticalSection, 0x00000400);
		m_flushBufferEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("FlushBufferEvent"));
		m_logWriterThread = CreateThread(NULL, 0, &LogSizeMonitoring, (void*)this, 0, NULL);
	}

	void Log(int value)
	{
		EnterCriticalSection(&m_criticalSection);

		if (m_logs.GetSize() == MAX_BUFFER_SIZE)
		{
			SetEvent(m_flushBufferEvent);

			if (WaitForSingleObject(m_logWriterThread, INFINITE) == WAIT_OBJECT_0)
			{
				m_logWriterThread = CreateThread(NULL, 0, &LogSizeMonitoring, (void*)this, 0, NULL);
				ResetEvent(m_flushBufferEvent);
			}
		}

		m_logs.PushBack(value);

		LeaveCriticalSection(&m_criticalSection);
	}

	~LogBuffer()
	{
		DeleteCriticalSection(&m_criticalSection);
	}

private:
	static DWORD WINAPI LogSizeMonitoring(CONST LPVOID lpParam)
	{
		auto logBuffer = (LogBuffer*)lpParam;
		if (WaitForSingleObject(logBuffer->m_flushBufferEvent, INFINITE) == WAIT_OBJECT_0)
		{
			logBuffer->m_fileWriter.WriteLogs(logBuffer->m_logs);
		}

		ExitThread(0);
	}

private:
	CRITICAL_SECTION m_criticalSection;
	CLinkedList<int> m_logs;
	LogFileWriter m_fileWriter;

	HANDLE m_flushBufferEvent;
	HANDLE m_logWriterThread;
};
