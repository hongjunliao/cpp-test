/*!
 * test libfreeimageplus-dev
 * PPM图片裁剪
 */
#include "bd_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <FreeImagePlus.h>
static char const * ARGS = "soucr-file target-file left top right bottom";
int test_libfreeimage_crop_main(int argc, char ** argv);

int test_libfreeimageplus_crop_main(int argc, char ** argv)
{
	return test_libfreeimage_crop_main(argc, argv);

	if(argc < 2){
		printf("%s %s %s\n", argv[0], TEST_LIB_FREEIMAGE,  ARGS);
		return 0;
	}
	fipImage img;
	BOOL r = img.load(argv[1]);
	if(!r){
		fprintf(stderr, "load image failed\n");
		return 0;
	}
	r = img.crop(400, 350, 780, 580);
	if(!r){
		fprintf(stderr, "fipImage.crop failed\n");
	}
	img.save(argv[1]);
	return 0;
}

int test_libfreeimage_crop_main(int argc, char ** argv)
{
	if(argc < 7){
		printf("%s %s %s\n", argv[0], TEST_LIB_FREEIMAGE,  ARGS);
		return 0;
	}
	FIBITMAP * dib = FreeImage_Load(FIF_PPM, argv[1]);
	if(!dib){
		fprintf(stderr, "load image failed\n");
		return 0;
	}

	// ppm info
	FREE_IMAGE_FORMAT fif = FIF_PPM;
	FREE_IMAGE_TYPE type = FreeImage_GetImageType(dib);
	unsigned width = FreeImage_GetWidth(dib);
	unsigned height = FreeImage_GetHeight(dib);
	unsigned pitch = FreeImage_GetPitch(dib);
	unsigned bpp = FreeImage_GetBPP(dib);
	BYTE *bits = FreeImage_GetBits(dib);

	fprintf(stderr, "ppm file info: format = %u, type = %u, width = %u, height = %u, pitch = %u, bpp = %u, bits = 0x%0x\n",
			fif, type, width, height, pitch, bpp, bits
	);

	int left = atoi(argv[3]), top = atoi(argv[4]), right = atoi(argv[5]), bottom = atoi(argv[6]);
	FIBITMAP * sub_dlib = FreeImage_Copy(dib, left, top, right, bottom);
	if(sub_dlib){
		FreeImage_Save(FIF_PPM, sub_dlib, argv[2]);
		FreeImage_Unload(sub_dlib);
	}
	else
		fprintf(stderr, "FreeImage_Copy failed\n");
	FreeImage_Unload(dib);
	return 0;
}
