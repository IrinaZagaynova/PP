#pragma once
#include "ITask.h"

class IWorker
{
public:
	virtual void Execute() = 0;
	virtual ~IWorker() {};
};