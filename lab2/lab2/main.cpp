#include "../bmp-library/EasyBMP.h"
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <iostream>
#include <ctime>
#define _USE_MATH_DEFINES 
#include <math.h>

using namespace std;

struct Args
{
	char* inputFileName;
	char* outputFileName;
	size_t threadsNumber = 0;
	size_t coresNumber = 0;
};

Args ParseArgs(int argc, char* argv[])
{
	if (argc != 5)
	{
		throw std::invalid_argument("Invalid arguments count\nUsage: lab2.exe <input file name> <output file name> <threads number> <cores number>\n");
	}

	Args args;
	args.inputFileName = argv[1];
	args.outputFileName = argv[2];
	args.threadsNumber = atoi(argv[3]);
	args.coresNumber = atoi(argv[4]);

	return args;
}

struct HorizonStripeParams
{
	BMP& bmp;
	int stripeStart;
	int stripeEnd;
};

void BlurHorizonStripe(HorizonStripeParams* blurParams, int blurRadius)
{
	float rs = ceil(blurRadius * 2.57);
	for (int i = 0; i < blurParams->bmp.TellHeight(); i++)
	{
		for (int j = blurParams->stripeStart; j < blurParams->stripeEnd; j++)
		{
			double r = 0, g = 0, b = 0;
			double count = 0;

			for (int iy = i - rs; iy < i + rs + 1; iy++)
			{
				for (int ix = j - rs; ix < j + rs + 1; ix++)
				{
					auto x = std::min<int>(blurParams->bmp.TellWidth() - 1, std::max<int>(0, ix));
					auto y = std::min<int>(blurParams->bmp.TellHeight() - 1, std::max<int>(0, iy));

					auto dsq = ((ix - j) * (ix - j)) + ((iy - i) * (iy - i));
					auto wght = std::exp(-dsq / (2.0 * blurRadius * blurRadius)) / (M_PI * 2.0 * blurRadius * blurRadius);

					RGBApixel pixel = blurParams->bmp.GetPixel(x, y);

					r += pixel.Red * wght;
					g += pixel.Green * wght;
					b += pixel.Blue * wght;
					count += wght;
				}
			}

			RGBApixel pixel = blurParams->bmp.GetPixel(j, i);
			pixel.Red = std::round(r / count);
			pixel.Green = std::round(g / count);
			pixel.Blue = std::round(b / count);
			blurParams->bmp.SetPixel(j, i, pixel);
		}
	}
}

DWORD WINAPI ThreadProc(LPVOID params)
{
	HorizonStripeParams* blurParams = (HorizonStripeParams*)params;
	BlurHorizonStripe(blurParams, 5);
	ExitThread(0);
}

vector<HorizonStripeParams> GetHorizonStripesParams(BMP bmp, int stripesNumber)
{
	vector<HorizonStripeParams> blurParams;
	int stripeWidth = bmp.TellWidth() / stripesNumber;
	int lastStripeWidth = stripeWidth + bmp.TellWidth() % stripesNumber;
	int currentWidth = 0;

	for (int i = 0; i < stripesNumber - 1; i++)
	{
		int nextWidth = currentWidth + stripeWidth;
		HorizonStripeParams stripeParams = { bmp, currentWidth, nextWidth };
		blurParams.push_back(stripeParams);
		currentWidth = nextWidth;
	}

	HorizonStripeParams stripeParams = { bmp, currentWidth, lastStripeWidth };
	blurParams.push_back(stripeParams);

	return blurParams;
}

void Blur(BMP& bmp, int threadsNumber, int coresNumber)
{
	HANDLE* handles = new HANDLE[threadsNumber];

	vector<HorizonStripeParams> blurParams = GetHorizonStripesParams(bmp, threadsNumber);

	for (int i = 0; i < threadsNumber; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, (HorizonStripeParams*)&blurParams[i], CREATE_SUSPENDED, NULL);
		SetThreadAffinityMask(handles[i], (1 << coresNumber) - 1);
	}

	for (int i = 0; i < threadsNumber; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(threadsNumber, handles, true, INFINITE);
}

int main(int argc, char* argv[])
{
	auto args = ParseArgs(argc, argv);

	BMP bmp;
	bmp.ReadFromFile(args.inputFileName);

	Blur(bmp, args.threadsNumber, args.coresNumber);

	bmp.WriteToFile(args.outputFileName);

	cout << "runtime = " << clock() / 1000.0 << endl;

	return 0;
}
