#pragma once
#include "../bmp-library/EasyBMP.h"
#include "ITask.h"
#define _USE_MATH_DEFINES 
#include <math.h>
#include <thread>

const int BLUR_RADIUS = 5;

struct BlurParams
{
    BMP* bmp;
    int stripeStart;
    int stripeEnd;
};

class BlurTask : public ITask
{
public:

    BlurTask(BlurParams* blurParams)
    {
        m_blurParams = blurParams;
    }

    void Execute() override
    {
		float rs = ceil(BLUR_RADIUS * 2.57);
		for (int i = 0; i < m_blurParams->bmp->TellHeight(); i++)
		{
			for (int j = m_blurParams->stripeStart; j < m_blurParams->stripeEnd; j++)
			{
				double r = 0, g = 0, b = 0;
				double count = 0;

				for (int iy = i - rs; iy < i + rs + 1; iy++)
				{
					for (int ix = j - rs; ix < j + rs + 1; ix++)
					{
						auto x = std::min<int>(m_blurParams->bmp->TellWidth() - 1, std::max<int>(0, ix));
						auto y = std::min<int>(m_blurParams->bmp->TellHeight() - 1, std::max<int>(0, iy));

						auto dsq = ((ix - j) * (ix - j)) + ((iy - i) * (iy - i));
						auto wght = std::exp(-dsq / (2.0 * BLUR_RADIUS * BLUR_RADIUS)) / (M_PI * 2.0 * BLUR_RADIUS * BLUR_RADIUS);

						RGBApixel pixel = m_blurParams->bmp->GetPixel(x, y);

						r += pixel.Red * wght;
						g += pixel.Green * wght;
						b += pixel.Blue * wght;
						count += wght;
					}
				}

				RGBApixel pixel = m_blurParams->bmp->GetPixel(j, i);
				pixel.Red = std::round(r / count);
				pixel.Green = std::round(g / count);
				pixel.Blue = std::round(b / count);
				m_blurParams->bmp->SetPixel(j, i, pixel);
			}
		}
    }

private:
    BlurParams* m_blurParams;
};