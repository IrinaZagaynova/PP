#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

CRITICAL_SECTION CriticalSection;

DWORD WINAPI ThreadProc(CONST LPVOID workingVariable)
{
    int* variable = (int*)workingVariable;
    EnterCriticalSection(&CriticalSection);

    int value = *variable;
    std::srand(std::time(nullptr));
    int delta = rand() % 10 + 1;

    value += delta;
    *variable = value;

    std::cout << "workingVariable = " + std::to_string(*variable) + " localValue = " + std::to_string(value) + "\n";
    if (*variable != value)
    {
        std::cout << "Error. workingVariable = " + std::to_string(*variable) + " localValue = " + std::to_string(value) + "\n";
    }

    LeaveCriticalSection(&CriticalSection);

    ExitThread(0);
}

HANDLE* CreateThreads(unsigned threadCount, int* args)
{
    HANDLE* handles = new HANDLE[threadCount];

    for (unsigned i = 0; i < threadCount; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, args, CREATE_SUSPENDED, NULL);
    }

    return handles;
}

void LaunchThreads(HANDLE* const& handles, unsigned count) {

    for (size_t i = 0; i < count; i++)
    {
        ResumeThread(handles[i]);
    }
}

int main()
{
    //std::cin.get();

    if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400))
        return 1;

    int* variable = new int(1);
    auto handles = CreateThreads(2, variable);
    LaunchThreads(handles, 2);

    WaitForMultipleObjects(2, handles, true, INFINITE);

    DeleteCriticalSection(&CriticalSection);

    return 0;
}
