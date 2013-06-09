#include "header.h"

void makeLowerResolutionFile(){
wchar_t *wc = new wchar_t[volumefile.size()+1];
mbstowcs(wc,volumefile.data(),volumefile.size()+1);
SeedFile =CreateFile((LPCWSTR)(wc),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
checkHandleCreate(SeedFile); //エラーハンドル
SetFilePointer(SeedFile,0,NULL,FILE_BEGIN);

wFile =CreateFile((LPCWSTR)(wc),GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

int seedPlaneSize=VOXELX*VOXELY;
int lowerPlaneSize=VOXELX*VOXELY*0.125;
for(int z=0;z<VOXELZ;z++){
//xy平面2面ずつ読み込む(2面じゃないと低解像ファイルが作れない)

float *writeData = new float[lowerPlaneSize];
float *loadData=new float[seedPlaneSize*2];

ReadFile(SeedFile,&loadData,/*sizeof(float)**/(VOXELX*VOXELY)*2,&dwReadSize, NULL);
printf("read %d bytes\n",dwReadSize);
float sum=0.0;
//writeData 1面ずつ
for(int wy=0;wy<(int)VOXELY*0.5;wy++){
for(int wx=0;wx<(int)VOXELX*0.5;wx++){
for(int z=0;z<2;z++){
for(int y=0;y<2;y++){
for(int x=0;x<2;x++){
sum += loadData[(z*VOXELY+y)*VOXELX+x];
}}}
int LOWVOXELX=(int)VOXELX*0.5;
writeData[wy*LOWVOXELX+wx]=sum*0.125;//平均値をwriteDataに入れる
//１面読み込んだら１面書き込む
}}
WriteFile(wFile,writeData,lowerPlaneSize,&dwWriteSize,NULL);
//そしてこの辺でファイルポインタを動かす操作が必要
writeladdr = getLaddress(lowerPlaneSize);
SetFilePointer(wFile,writeladdr,NULL,FILE_CURRENT);
seedladdr=getLaddress(seedPlaneSize);
SetFilePointer(SeedFile,seedladdr,NULL,FILE_CURRENT);
}//ｚインデックスの終わり、１面の終わり
CloseHandle(SeedFile);
CloseHandle(WriteFile);
  }
void main(){
	makeLowerResolutionFile();

}
