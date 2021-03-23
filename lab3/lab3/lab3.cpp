#include <tchar.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>

struct ThreadParams
{
    size_t threadNumber;
    std::ofstream* ostrm;
    std::chrono::steady_clock::time_point startTime;
};

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    ThreadParams* threadParams = (ThreadParams*)lpParam;

    for (size_t i = 0; i < 1000000; i++)
    {
        std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
        const auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - threadParams->startTime).count();
        *threadParams->ostrm << std::to_string((size_t)threadParams->threadNumber) + "|" + std::to_string(runtime) + "\n";   
    }

    delete threadParams;

    ExitThread(0);
}

int main(int argc, char* argv[])
{
    const std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    size_t threadCount = 2;

    HANDLE* handles = new HANDLE[threadCount];
    std::ofstream* files = new std::ofstream[threadCount];

    files[0] = std::ofstream("output1.txt");
    files[1] = std::ofstream("output2.txt");

    for (size_t i = 0; i < threadCount; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, new ThreadParams { i, &files[i], startTime }, CREATE_SUSPENDED, NULL);
    }

    for (size_t i = 0; i < threadCount; i++)
    {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(threadCount, handles, true, INFINITE);

    delete[] files;

    return 0;
}