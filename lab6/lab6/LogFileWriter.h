#pragma once
#include "LinkedList.h"
#include <string>
#include <fstream>

class LogFileWriter
{
public: 
	LogFileWriter() 
	{
	}

	void WriteLogs(CLinkedList<int>& data)
	{
		for (auto item : data)
		{
			m_ostrm << item << "\n";
		}

		m_ostrm << "Buffer flushed\n";
		m_ostrm.flush();
		data.Clear();
	}

private:
	std::ofstream m_ostrm = std::ofstream("buffer-output.txt");
};