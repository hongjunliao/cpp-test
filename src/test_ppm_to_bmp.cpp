#include "bd_test.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iomanip>
using namespace std;

int test_ppm_to_bmp_main(int argc, char ** argv) {

	if(argc < 3){
		fprintf(stderr, "%s from.ppm to.bmp\n", argv[0]);
		return 0;
	}
	char* pFilename = argv[1], * outFile = argv[2];
	ifstream ifile; //input ppm file
	ofstream ofile; //output bmp file
	ifile.open(pFilename, ios::binary);
	if (!ifile) {
		cout << "open error!" << endl;
	}
	ofile.open(outFile, ios::binary);

	string FileType; //file type-p6
	int Width, Height, Count; //count is the number of pixels
	int DataSize, HeadSize, FileSize;
	ifile >> FileType >> Width >> Height >> Count; //get the para of the ppm file

	cout << FileType << "!" << Width << "!" << Height << endl;

	DataSize = Width * Height * 3; //every pixel need 3 byte to store
	HeadSize = 0x36; //54 Bype
	FileSize = HeadSize + DataSize;
	Count = Width * Height;

	cout << Count << endl;

	unsigned char BmpHead[54];

	for (int i = 0; i < 53; i++) {
		BmpHead[i] = 0;
	}

	BmpHead[0] = 0x42;
	BmpHead[1] = 0x4D; //type
	cout << FileSize << endl;

	int SizeNum = 2; //the size of bmp file
	while (FileSize != 0) {
		BmpHead[SizeNum++] = FileSize % 256;
		FileSize = FileSize / 256;
	}

	cout << BmpHead[2] << BmpHead[3] << BmpHead[4] << BmpHead[5] << endl;

	BmpHead[0x0A] = 0x36; //data begin here
	BmpHead[0x0E] = 0x28; //size of bitmap information head

	int FileWidthCount = 0x12; //width
	while (Width != 0) {

		BmpHead[FileWidthCount++] = Width % 256;
		Width = Width / 256;

	}
	int FileHeightCount = 0x16; //height
	while (Height != 0) {

		BmpHead[FileHeightCount++] = Height % 256;
		Height = Height / 256;
	}

	BmpHead[0x1A] = 0x1; //device
	BmpHead[0x1C] = 0x18; //every pixel need 3 byte

	int FileSizeCount = 0x22;
	while (DataSize != 0) {
		BmpHead[FileSizeCount++] = DataSize % 256;
		DataSize = DataSize / 256;
	}

	int i, j;

	for (i = 0; i < 54; i++) {
		ofile << BmpHead[i];
		cout << BmpHead[i] << endl;
	}

	char blue, green, red;

	ifile.ignore();
	cout << "ok" << endl;
	for (int j = Count; j >= 1; j--) {
		//cout<<"begin"<<endl;
		ifile.get(blue);
		ifile.get(green);
		ifile.get(red);

		ofile << hex;
		ofile.put(red);
		ofile.put(green);
		ofile.put(blue);
		//cout<<j<<endl;
	}

	ifile.close();
	ofile.close();
	return 0;
}
