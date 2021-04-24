#include "../bmp-library/EasyBMP.h"
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <fstream>
#include <optional>
#define _USE_MATH_DEFINES 
#include <math.h>

struct Args
{
	char* inputFileName;
	char* outputFileName;
	int threadsNumber = 0;
	int coresNumber = 0;
	std::vector<int> priorities;
};

std::optional<Args> ParseArgs(int argc, char* argv[])
{
	if (argc < 6)
	{
		std::cout << "Usage: {lab4.exe} {input file name} {output file name} {threads number} {cores number} {array of priorities}\n";
		return std::nullopt;
	}

	Args args;
	args.inputFileName = argv[1];
	args.outputFileName = argv[2];
	args.threadsNumber = atoi(argv[3]);
	args.coresNumber = atoi(argv[4]);

	if (argc < 5 + args.threadsNumber)
	{
		std::cout << "Usage: {lab4.exe} {input file name} {output file name} {threads number} {cores number} {array of priorities}\n";
		return std::nullopt;
	}

	for (int i = 0; i < args.threadsNumber; ++i)
	{
		args.priorities.push_back(atoi(argv[5 + i]));
	}

	return args;
}

struct BlurParams
{
	BMP* bmp;
	std::ofstream* ostrm;
	int stripeStart;
	int stripeEnd;
};

void BlurHorizonStripe(BlurParams* blurParams, int blurRadius)
{
	float rs = ceil(blurRadius * 2.57);
	for (int i = 0; i < blurParams->bmp->TellHeight(); i++)
	{
		for (int j = blurParams->stripeStart; j < blurParams->stripeEnd; j++)
		{
			double r = 0, g = 0, b = 0;
			double count = 0;

			for (int iy = i - rs; iy < i + rs + 1; iy++)
			{
				for (int ix = j - rs; ix < j + rs + 1; ix++)
				{
					auto x = std::min<int>(blurParams->bmp->TellWidth() - 1, std::max<int>(0, ix));
					auto y = std::min<int>(blurParams->bmp->TellHeight() - 1, std::max<int>(0, iy));

					auto dsq = ((ix - j) * (ix - j)) + ((iy - i) * (iy - i));
					auto wght = std::exp(-dsq / (2.0 * blurRadius * blurRadius)) / (M_PI * 2.0 * blurRadius * blurRadius);

					RGBApixel pixel = blurParams->bmp->GetPixel(x, y);

					r += pixel.Red * wght;
					g += pixel.Green * wght;
					b += pixel.Blue * wght;
					count += wght;
				}
			}

			RGBApixel pixel = blurParams->bmp->GetPixel(j, i);
			pixel.Red = std::round(r / count);
			pixel.Green = std::round(g / count);
			pixel.Blue = std::round(b / count);
			blurParams->bmp->SetPixel(j, i, pixel);

			*blurParams->ostrm << clock() << "\n";
		}
	}
}

DWORD WINAPI ThreadProc(LPVOID params)
{
	BlurParams* blurParams = (BlurParams*)params;
	BlurHorizonStripe(blurParams, 5);
	ExitThread(0);
}

std::ofstream* GetOutputFiles(int threadsNumber)
{
	std::ofstream* outputFiles = new std::ofstream[threadsNumber];

	for (int i = 0; i < threadsNumber; i++)
	{
		outputFiles[i] = std::ofstream("output" + std::to_string(i) + ".txt");
	}

	return outputFiles;
}

BlurParams* GetBlurParams(BMP& bmp, int threadsNumber, std::ofstream* outputFiles)
{
	BlurParams* blurParams = new BlurParams[threadsNumber];

	int stripeWidth = bmp.TellWidth() / threadsNumber;
	int lastStripeWidth = stripeWidth + bmp.TellWidth() % threadsNumber;
	int currentWidth = 0;

	for (int i = 0; i < threadsNumber - 1; i++)
	{
		int nextWidth = currentWidth + stripeWidth;
		BlurParams stripeParams = { &bmp, &outputFiles[i], currentWidth, nextWidth };
		blurParams[i] = stripeParams;
		currentWidth = nextWidth;
	}

	BlurParams stripeParams = { &bmp, &outputFiles[threadsNumber - 1], currentWidth, currentWidth + lastStripeWidth };
	blurParams[threadsNumber - 1] = stripeParams;

	return blurParams;
}

void RunThreads(BMP& bmp, int threadsNumber, int coresNumber, const std::vector<int>& priorities)
{
	std::ofstream* outputFiles = GetOutputFiles(threadsNumber);
	BlurParams* blurParams = GetBlurParams(bmp, threadsNumber, outputFiles);

	HANDLE* handles = new HANDLE[threadsNumber];

	for (int i = 0; i < threadsNumber; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, (BlurParams*)&blurParams[i], CREATE_SUSPENDED, NULL);
		SetThreadAffinityMask(handles[i], (1 << coresNumber) - 1);
		SetThreadPriority(handles[i], priorities[i]);
	}

	for (int i = 0; i < threadsNumber; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(threadsNumber, handles, true, INFINITE);

	delete[] outputFiles;
}

int main(int argc, char* argv[])
{
	srand(time(0));

	auto args = ParseArgs(argc, argv);

	if (!args)
	{
		return 1;
	}

	BMP bmp;
	bmp.ReadFromFile(args->inputFileName);

	RunThreads(bmp, args->threadsNumber, args->coresNumber, args->priorities);

	bmp.WriteToFile(args->outputFileName);

	return 0;
}