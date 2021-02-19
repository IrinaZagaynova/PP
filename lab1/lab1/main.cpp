#include <tchar.h>
#include <windows.h>
#include <string>
#include <iostream>

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    std::cout << "Поток № " + std::to_string((int)lpParam) + " выполняет свою работу\n";;
    ExitThread(0); 
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        return 1;
    }

    setlocale(LC_ALL, "ru");

    auto threadCount = atoi(argv[1]);

    HANDLE* handles = new HANDLE[threadCount];

    for (auto i = 0; i < threadCount; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, (int*)(i + 1), CREATE_SUSPENDED, NULL);
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(threadCount, handles, true, INFINITE);

    return 0;
}

