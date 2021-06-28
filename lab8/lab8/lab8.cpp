#include "BlurTask.h"
#include "Pool.h"
#include "../bmp-library/EasyBMP.h"
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

struct Args
{
    std::string mode;
    int threadsCount;
    int blockCount;
    std::string inputDirectory;
    std::string outputDirectory;
};

Args ParseArgs(int argc, const char* argv[])
{
    Args args;

    args.mode = argv[1];
    args.blockCount = std::atoi(argv[2]);
    args.inputDirectory = argv[3];
    args.outputDirectory = argv[4];
    args.threadsCount = std::atoi(argv[5]);

    if (argc != 6)
    {
        throw std::invalid_argument("Invalid argument count");
    }

    return args;
}

void LaunchThreads(HANDLE* const& handles, unsigned count) {

    for (size_t i = 0; i < count; i++)
    {
        ResumeThread(handles[i]);
    }
}

BlurParams* GetBlurParams(BMP& bmp, int blocksCount)
{
    BlurParams* blurParams = new BlurParams[blocksCount];

    int stripeWidth = bmp.TellWidth() / blocksCount;
    int lastStripeWidth = stripeWidth + bmp.TellWidth() % blocksCount;
    int currentWidth = 0;

    for (int i = 0; i < blocksCount - 1; i++)
    {
        int nextWidth = currentWidth + stripeWidth;
        BlurParams stripeParams = { &bmp, currentWidth, nextWidth };
        blurParams[i] = stripeParams;
        currentWidth = nextWidth;
    }

    BlurParams stripeParams = { &bmp, currentWidth, currentWidth + lastStripeWidth };
    blurParams[blocksCount - 1] = stripeParams;

    return blurParams;
}

std::vector<ITask*> CreateTasks(BMP* bitmap, size_t blocksCount)
{
    BlurParams* blurParams = GetBlurParams(*bitmap, blocksCount);
    std::vector<ITask*> tasks;

    for (size_t i = 0; i < blocksCount; i++)
    {
        auto task = new BlurTask(&blurParams[i]);
        tasks.push_back(task);
    }

    return tasks;
}

std::vector<std::filesystem::path> GetFiles(std::string directoryName)
{
    std::vector<std::filesystem::path> fileNames;
    for (auto& p : std::filesystem::directory_iterator(directoryName))
    {
        if (p.path().extension().string() == ".bmp")
        {
            fileNames.push_back(p.path());
        }
    }

    return fileNames;
}

int main(int argc, const char* argv[])
{
    auto startTime = std::chrono::high_resolution_clock::now();
    try
    {
        auto args = ParseArgs(argc, argv);
        auto files = GetFiles(args.inputDirectory);

        for (auto& file : files)
        {
            BMP bmp;
            bmp.ReadFromFile(file.string().c_str());
            std::vector<ITask*> tasks = CreateTasks(&bmp, args.blockCount);

            Pool pool(tasks, args.threadsCount);

            if (args.mode == "1")
            {
                pool.Execute();
            }

            else if (args.mode == "2")
            {
                HANDLE* handles = new HANDLE[args.blockCount];
                for (int i = 0; i < args.blockCount; i++)
                {
                    handles[i] = CreateThread(NULL, 0, &ThreadProc, tasks[i], 0, NULL);
                }

                WaitForMultipleObjects(args.blockCount, handles, true, INFINITE);
            }
            else
            {
                std::cout << "Invalid mode number\n";
            }

            std::string output = args.outputDirectory + "/" + file.filename().string();
            bmp.WriteToFile(output.c_str());
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n";
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    const auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "runtime = " << runtime << std::endl;

    return 0;
}