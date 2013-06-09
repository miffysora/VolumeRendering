//ブロック化プログラム for 1byte/voxel data
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#define FILENAME ("bonsai.raw")
#define XMAX 256
#define YMAX 256
#define ZMAX 256
#define BLOCKX 8//１ブロックあたりのボクセルの個数
#define BLOCKY 8
#define BLOCKZ 8
int size = XMAX*YMAX*ZMAX;
#define FILETYPE "bonsai.raw"
unsigned char *volumePointer = new unsigned char[size];//sizeは256*256*256
using namespace std;


void loadTextures(){
  FILE *pFile = fopen(FILENAME,"rb");//rb=読み出し専用で、バイナリモードで読みだす。

	if (NULL == pFile) {
		printf("ファイルがみつかりません");
	}
	bool ok = (size == fread(volumePointer,sizeof(unsigned char), size,pFile));
	fclose(pFile);


}
void firstBlocking(){//rawデータを、32^3個のブロックに分ける。1ブロック8^3ボクセル
	stringstream filename;
	int blockNumX =XMAX/BLOCKX;
	int blockNumY =YMAX/BLOCKY;
	int blockNumZ =ZMAX/BLOCKX;

	for(int blockIndexZ=0;blockIndexZ<blockNumZ;blockIndexZ++){
		for(int blockIndexY=0;blockIndexY<blockNumY;blockIndexY++){
			for(int blockIndexX=0;blockIndexX<blockNumX;blockIndexX++){

				filename<<"level0/bonsai_Block_0_"<<blockIndexX<<"_"<<blockIndexY<<"_"<<blockIndexZ<<FILETYPE;
				string filenamestring = filename.str();
				ofstream fout(filenamestring.c_str());


				for(int z=0;z<BLOCKZ;z++){
					for(int y=0;y<BLOCKY;y++){
						for(int x=0;x<BLOCKX;x++){
							fout<<volumePointer[(z*YMAX+y)*XMAX+x];
						}}}

				filename.str("");
				fout.close();
			}}}
}
int main(){
	loadTextures();
	firstBlocking();
	delete [] volumePointer;
}
