#include <iostream>
#include "CmdLineParser.hpp"
#include "im/include/im.h"
#include "im/include/im_complex.h"
#include "im/include/im_image.h"
#include "im/include/im_process.h"

const unsigned int major = 1;
const unsigned int minor = 0;

void xcorr(imImage* im1, imImage* im2)
{
	imImage* xcorr = imImageCreate(im1->width, im1->height, IM_RGB, IM_CFLOAT);

	imProcessCrossCorrelation(im1, im2, xcorr);

	imComplex<float>* xcorr_data = reinterpret_cast<imComplex<float>*>(xcorr->data[0]);
	float max_magn = 0.0f;
	int x = 0, y = 0;
	for (int i = 0; i < im1->height; ++i)
	{
		for (int j = 0; j < im1->width; ++j)
		{
			const auto xcorr_pixel = xcorr_data[i * im1->width + j];
			const float real = xcorr_pixel.real;
			const float imag = xcorr_pixel.imag;
			const float magn = sqrtf(real * real + imag * imag);
			if (magn > max_magn)
			{
				max_magn = magn;
				x = j;
				y = i;
			}
		}
	}
	imImageDestroy(xcorr);

	x = im1->width / 2 - x;
	y = im1->height / 2 - y;

	std::cout << x << ", " << y << std::endl;
}

int main(int argc, char** argv)
{
	int err = 0;
	darkroom::CmdLineParser cmdParser;
	cmdParser.addSingularOption("version", "Print tool version");

	auto& printUsage = [&]()
	{
		std::cout << "xcorr2 file1 file2" << std::endl;
		std::cout << "version " << major << "." << minor << std::endl;
		std::cout << "Estimate global motion vector based on two images of equal size" << std::endl;
		cmdParser.printUsage();
		exit(0);
	};

	if (!cmdParser.parse(argc, argv) || cmdParser.hasOption("version") || cmdParser.freeOptionsCount() != 2)
		printUsage();

	imFile* f1 = imFileOpen(cmdParser.getFreeOptionAs<std::string>(0).c_str(), &err);
	imFile* f2 = imFileOpen(cmdParser.getFreeOptionAs<std::string>(1).c_str(), &err);
	imImage* im1 = imFileLoadImage(f1, 0, &err);
	imImage* im2 = imFileLoadImage(f2, 0, &err);
	imFileClose(f1);
	imFileClose(f2);

	if (im1->width != im2->width || im1->height != im2->height)
	{
		std::cout << "Images are not of equal size. Exiting." << std::endl;
		imImageDestroy(im1);
		imImageDestroy(im2);
		imFileClose(f1);
		imFileClose(f2);
	}

	xcorr(im1, im2);
	imImageDestroy(im1);
	imImageDestroy(im2);
	return 0;
}