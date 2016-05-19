#include "bd_test.h"
#include <iostream>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>

#include <iostream>

#pragma pack(2)

using namespace std;

//下面两个结构是位图的结构
typedef struct BITMAPFILEHEADER {
	u_int16_t bfType;
	u_int32_t bfSize;
	u_int16_t bfReserved1;
	u_int16_t bfReserved2;
	u_int32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER {
	u_int32_t biSize;
	u_int32_t biWidth;
	u_int32_t biHeight;
	u_int16_t biPlanes;
	u_int16_t biBitCount;
	u_int32_t biCompression;
	u_int32_t biSizeImage;
	u_int32_t biXPelsPerMeter;
	u_int32_t biYPelsPerMeter;
	u_int32_t biClrUsed;
	u_int32_t biClrImportant;
} BITMAPINFODEADER;

void showBmpHead(BITMAPFILEHEADER const * pBmpHead) {

	printf("位图文件头: \n");
	printf("文件头类型: %u\n",  pBmpHead->bfType );
	printf("文件大小: %u\n",  pBmpHead->bfSize );
	printf("保留字_1: %u\n",  pBmpHead->bfReserved1 );
	printf("保留字_2: %u\n",  pBmpHead->bfReserved2 );
	printf("实际位图数据的偏移字节数: %u\n",  pBmpHead->bfOffBits );
}

void showBmpInforHead(BITMAPINFODEADER const * pBmpInforHead)
{
	printf("位图信息头: \n");
	printf("结构体的长度: %u\n",  pBmpInforHead->biSize );
	printf("位图宽: %u\n",  pBmpInforHead->biWidth );
	printf("位图高: %u\n",  pBmpInforHead->biHeight );
	printf("biPlanes平面数: %u\n",  pBmpInforHead->biPlanes );
	printf("biBitCount采用颜色位数: %u\n",  pBmpInforHead->biBitCount );
	printf("压缩方式: %u\n",  pBmpInforHead->biCompression );
	printf("biSizeImage实际位图数据占用的字节数: %u\n",  pBmpInforHead->biSizeImage );
	printf("X方向分辨率: %u\n",  pBmpInforHead->biXPelsPerMeter );
	printf("Y方向分辨率: %u\n",  pBmpInforHead->biYPelsPerMeter );
	printf("使用的颜色数: %u\n",  pBmpInforHead->biClrUsed );
	printf("重要颜色数: %u\n",  pBmpInforHead->biClrImportant );
}

int read_bitmap(uint8_t ** data, int * bw, int * bh)
{
	BITMAPFILEHEADER head;
	BITMAPINFODEADER info;
	FILE *fp = fopen("/home/jun/Pictures/qemu-nsis.bmp", "rb");
	fread(&head, 1, sizeof(BITMAPFILEHEADER), fp);
	fread(&info, 1, sizeof(BITMAPINFODEADER), fp);

	showBmpHead(&head);
	showBmpInforHead(&info);

	*data = (uint8_t *)calloc(info.biSizeImage, sizeof(uint8_t));
	fread(*data, info.biSizeImage, sizeof(uint8_t), fp);
	*bw = info.biWidth;
	*bh = info.biHeight;
	fclose(fp);
	return 0;
}


int test_bmp_io_main(int argc, char **argv)
{
	static uint8_t *bitmap_file = 0;
	static int bwidth, bheight;
    if(!bitmap_file){
    	 read_bitmap(&bitmap_file, &bwidth, &bheight);
    }

	return 0;
}

