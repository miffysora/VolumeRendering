
/*////////////////////////////////////////////////////////////////////
indexは２のべき乗しか対応してません！！！！
*////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>

using namespace std;

#define BLX 64
#define BLY 64
#define BLZ 64

#define NUMLEVEL 5

#define PATH "C:/takasao/Largedata_rendering/LargeData/"
#define VFILE "VertebralBody2048-2048-970-ooc-sgdf"
#define FTYPE ".dff"

static string volumefile = PATH VFILE FTYPE; /* ボリュームデータファイル名 */

int gx,gy,gz;
HANDLE File,IndexFile,IndexBlockFile;
DWORD dwWriteSize;
DWORD nb;

float loadData[4*BLZ+2][4*BLY+2][4*BLX+2];

LARGE_INTEGER getLaddress(long long address)
{
	address = address * sizeof(float);

	int lower; 
	int upper;
	memcpy(&lower,(unsigned char*)&address,   sizeof(int));
	memcpy(&upper,(unsigned char*)&address  + sizeof(int) ,   sizeof(int));

	LARGE_INTEGER laddr;
	laddr.LowPart = lower;
	laddr.HighPart = upper;

	return laddr;
}

void checkHandleCreate(HANDLE handle)
{
	if(handle == INVALID_HANDLE_VALUE)
	{


		LPVOID lpMsgBuf;
		//ここにチェックしたい処理を書く

		FormatMessage(				//エラー表示文字列作成
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);

		MessageBox(NULL, (LPCWSTR)lpMsgBuf, NULL, MB_OK);	//メッセージ表示

		LocalFree(lpMsgBuf);

		fprintf(stderr,"open failed");
	}
}

int createBlock(int level, int bx,int by,int bz,int vx,int vy,int vz)
{
	float* writeData = new float[(BLX+2)*(BLY+2)*(BLZ+2)];
	
	/*writeData初期化*/
	for(int i=0;i<(BLX+2)*(BLY+2)*(BLZ+2);i++)
		writeData[i] = 0.0;

	
	for(int i=0;i<4*BLX+2;i++)
	{
		for(int j=0;j<4*BLY+2;j++)
		{
			for(int k=0;k<4*BLZ+2;k++)
				loadData[i][j][k] = 0.0;
		}
	}


	if(level != 0)
	{
		

		for(int i=0 ; i<4;i++)
		{
			for(int j=0; j<4;j++)
			{
				for(int k=0;k<4;k++)
				{
					int fx = 2*bx+i-1;
					int fy = 2*by+j-1;
					int fz = 2*bz+k-1;
					
					if(fx <0 || fy <0 || fz <0 || fx > 2*vx-1 || fy > 2*vy-1 || fz > 2*vz-1) 
						;
					else
					{
						
						
						
						/*///////////////////////////////////////////////////////////////////////////////////////////////////////
						**ファイル読み込み（1高解像度)
						///////////////////////////////////////////////////////////////////////////////////////////////////////*/
						HANDLE rFile;

						stringstream prevfile;
						prevfile<<PATH<<"VertebralBody_Brick_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<level-1<<"/"<<"VertebralBody-"<<level-1<<"_"<<fx<<"_"<<fy<<"_"<<fz<<".dff";


						wchar_t *wc2 = new wchar_t[prevfile.str().size()+1];
						mbstowcs(wc2,prevfile.str().data(),prevfile.str().size()+1);

						rFile = CreateFile((LPCWSTR)wc2 , GENERIC_READ , 0 , NULL  ,OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
						checkHandleCreate(rFile);




						LARGE_INTEGER laddr;
						laddr = getLaddress(8);
						SetFilePointerEx(rFile,laddr,NULL,FILE_BEGIN);



						long  n =  i*BLX;
						long  m =  j*BLY;
						long  l =  k*BLZ;

					

						for(int c= 0;c< BLZ+2;c++)
						{
							for(int b= 0;b< BLY+2;b++)
							{
								ReadFile(rFile,&loadData[l+c][m+b][n],sizeof(float)*(BLX+2), &dwWriteSize, NULL);                      	          
							}

						}

						delete[] wc2;
						CloseHandle(rFile);

					}
				}
			}
		}

		/*/////////////////////////////////////////////////////////////////////////////////////
		低解像度volume作成（loadData->outData）
		//////////////////////////////////////////////////////////////////////////////////////*/
		



		for(int k=0;k < BLZ+2; k++)
		{
			for(int j=0;j < BLY+2; j++)
			{
				for(int i=0;i < BLX+2; i++)
				{
					float sum = 0.0;


					for(int c=0;c <2 ; c++)
					{
						for(int b=0;b <2; b++)
						{
							for(int a=0;a <2; a++)
							{
								sum += loadData[2*k+c+BLZ-1][2*j+b+BLY-1][2*i+a+BLX-1];
							}
						}
					}
					writeData[(k*(BLY+2)+j)*(BLX+2)+i] = sum / 8.0;
				}
			}
		}
	
		
	}
	else
	{

		LARGE_INTEGER laddr;
		//初期化
		laddr = getLaddress(5);
		SetFilePointerEx(File,laddr,NULL,FILE_BEGIN);

		//初期位置指定
		int offsetx = 0,offsety = 0,offsetz = 0;

		int ox = BLX*bx-1;
		if(ox < 0){
			ox = 0;
			offsetx = 1;
		}

		int oy = BLY*by-1;
		if(oy < 0){
			oy = 0;
			offsety = 1;
		}

		int oz = BLZ*bz-1;
		if(oz < 0){
			oz = 0;
			offsetz = 1;
		}
		laddr = getLaddress(((oz*gy+oy)*gx+ox));
		SetFilePointerEx(File,laddr,NULL,FILE_CURRENT);


		int dx;
		int dy;
		int dz;
		if((ox-offsetx) + (BLX+2) > gx)
			dx = gx - ox;
		else
			dx = BLX+2-offsetx;
		if((oy-offsety) + (BLY+2) > gy)
			dy = gy - oy;
		else
			dy = BLY+2-offsety;
		if((oz-offsetz) + (BLZ+2) > gz)
			dz = gz - oz;
		else
			dz = BLZ+2-offsetz;


		long long n= (offsetz*(BLY+2)+offsety)*(BLX+2)+offsetx;
		for(int k= 0;k< dz;k++)
		{
			for(int j= 0;j< dy;j++)
			{
				ReadFile(File,&writeData[n],sizeof(float)*dx, &dwWriteSize, NULL);
				n += BLX+2-offsetx;
				laddr = getLaddress(gx-dx);
				SetFilePointerEx(File,laddr,NULL,FILE_CURRENT);                              	          
			}
			laddr = getLaddress((gy-dy)*gx);
			SetFilePointerEx(File,laddr,NULL,FILE_CURRENT);
			n += ((BLY+2-offsety)-dy)*(BLX+2-offsetx);
		}

	}


	/*////////////////////////////////////////////////////////////////////////////
	**index書き込み
	//////////////////////////////////////////////////////////////////////////////*/
	//unsigned int pi;

	//int vx = (int)(ceil(BLX/32.0));
	//unsigned int* index = new unsigned int [vx*BLY*BLZ];

	int ibflag = 0;
	for(int i=0;i < (BLX+2)*(BLY+2)*(BLZ+2);i++){
		if(writeData[i] != 0.0)
			ibflag = 1;
	}
	
	/*
	for(int i=0; i< vx*BLY*BLZ;i++){
		index[i] = 0;

		int endl = 32;
		for(int l =0 ;l < endl;l++){
			if(writeData[32*i+l] != 0.0){
				pi = 1;
				ibflag = 1;
			}
			else{
				pi = 0;
			}
			pi = pi << l;

			index[i] = index[i]^pi;	
		}
	}


	WriteFile(IndexFile,index,sizeof(unsigned int)*vx*BLY*BLZ,&nb,NULL);
	*/




	//ファイル書き込み
	HANDLE wFile;
	stringstream filename;
	filename<<PATH<<"VertebralBody_Brick_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<level<<"/"<<"VertebralBody"<<"-"<<level<<"_"<<bx<<"_"<<by<<"_"<<bz<<".dff";
	

	wchar_t *wc = new wchar_t[filename.str().size()+1];
	mbstowcs(wc,filename.str().data(),filename.str().size()+1); 
	wFile = CreateFile((LPCWSTR)wc , GENERIC_WRITE , 0 , NULL  ,OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);
	checkHandleCreate(wFile);
	
	SetFilePointer(wFile , 0,NULL , FILE_BEGIN);


	//ヘッダー作成
	int hd;
	//ヘッダーサイズ
	hd = 32;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);
	//ボリュームデータXサイズ
	hd = gx;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);
	//ボリュームデータYサイズ
	hd = gy;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);
	//ボリュームデータZサイズ
	hd = gz;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);
	//ブロックXサイズ
	hd = BLX;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);
	//ブロックYサイズ
	hd = BLY;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);
	//ブロックZサイズ
	hd = BLZ;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);
	//ボクセルのバイト数
	hd = 4;
	WriteFile(wFile,&hd,sizeof(int),&nb,NULL);



	WriteFile(wFile,writeData,sizeof(float)*(BLX+2)*(BLY+2)*(BLZ+2),&nb,NULL);

	CloseHandle(wFile);

	delete[] wc;	

	


	delete[] writeData;

	return ibflag;
}

void createMutiresoBlock(int level)
{
	cout<<"create resoluton level :"<<level<<endl;

	
	/*/////////////////////////////////////////////////////////////////////////
	**indexfile作成
	////////////////////////////////////////////////////////////////////////////*/
	/*
	stringstream indexfile;
	indexfile<<PATH<<"VerteblalBody_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<level<<"/"<<"VertebralBodyIndex-"<<level<<".dff";
	wchar_t *wc1 = new wchar_t[indexfile.str().size()+1];
	mbstowcs(wc1,indexfile.str().data(),indexfile.str().size()+1); 
	IndexFile = CreateFile((LPCWSTR)wc1 , GENERIC_WRITE , 0 , NULL  ,OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);
	checkHandleCreate(IndexFile);

	SetFilePointer(IndexFile , 0,NULL , FILE_BEGIN);
	//ヘッダー作成
	int hd;
	//ヘッダーサイズ
	hd = 8;
	WriteFile(IndexFile,&hd,sizeof(int),&nb,NULL);
	//ボクセルのバイト数
	hd = 4;
	WriteFile(IndexFile,&hd,sizeof(int),&nb,NULL);
	*/




	/*///////////////////////////////////////////////////////////////////////////////////////////////////////
	**indexblockfile作成
	///////////////////////////////////////////////////////////////////////////////////////////////////////*/
	stringstream indexblockfile;
	indexblockfile<<PATH<<"VertebralBody_Brick_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<level<<"/"<<"VertebralBodyIndexBlock-"<<level<<".dff";
	wchar_t *wc2 = new wchar_t[indexblockfile.str().size()+1];
	mbstowcs(wc2,indexblockfile.str().data(),indexblockfile.str().size()+1);

	mbstowcs(wc2,indexblockfile.str().data(),indexblockfile.str().size()+1); 
	IndexBlockFile = CreateFile((LPCWSTR)wc2 , GENERIC_WRITE , 0 , NULL  ,OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);
	checkHandleCreate(IndexBlockFile);
	

	SetFilePointer(IndexBlockFile , 0,NULL , FILE_BEGIN);
	//ヘッダー作成
	int hd;
	//ヘッダーサイズ
	hd = 8;
	WriteFile(IndexBlockFile,&hd,sizeof(int),&nb,NULL);
	//ボクセルのバイト数
	hd = 4;
	WriteFile(IndexBlockFile,&hd,sizeof(int),&nb,NULL);





	//block書き込み(indexblock書き込みもかねる）
	int mlx = gx/pow(2.0,(double)level);
	int mly = gy/pow(2.0,(double)level);
	int mlz = gz/pow(2.0,(double)level);


	int vx = (int)(ceil((double)mlx/BLX));
	int vy = (int)(ceil((double)mly/BLY));
	int vz = (int)(ceil((double)mlz/BLZ));


	unsigned int pi;
	
	for(int i=0;i <vx;i++)
	{
		for(int j= 0;j < vy;j++)
		{
			for(int k= 0;k < vz;k++)
			{
				cout<<"create Block :"<<level<<","<<i<<","<<j<<","<<k<<endl;
				pi = createBlock(level,i,j,k,vx,vy,vz);
				WriteFile(IndexBlockFile,&pi,sizeof(int),&nb,NULL);
			}

		}
	}
	

	WriteFile(IndexBlockFile,&pi,sizeof(int),&nb,NULL);


//	delete[] wc1;
	delete[] wc2;
	CloseHandle(IndexBlockFile);
	CloseHandle(IndexFile);



}


int _tmain(int argc, _TCHAR* argv[])
{
	int a;

	wchar_t *wc = new wchar_t[volumefile.size()+1];
	mbstowcs(wc,volumefile.data(),volumefile.size()+1);

	File = CreateFile((LPCWSTR)(wc),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	checkHandleCreate(File);
	SetFilePointer(File,0,NULL,FILE_BEGIN);

	dwWriteSize;
	int headersize;
	int sod;
	unsigned char buf[20];

	if(ReadFile(File,&headersize, sizeof(int),  &dwWriteSize, NULL) == false)
	{
		cerr<<"failed to read"<<endl;
	}
	if(ReadFile(File,&gx, sizeof(int),  &dwWriteSize, NULL) == false)
	{
		cerr<<"failed to read"<<endl;
	}
	if(ReadFile(File,&gy, sizeof(int),  &dwWriteSize, NULL) == false)
	{
		cerr<<"failed to read"<<endl;
	}
	if(ReadFile(File,&gz, sizeof(int),  &dwWriteSize, NULL) == false)
	{
		cerr<<"failed to read"<<endl;
	}
	if(ReadFile(File,&sod, sizeof(int),  &dwWriteSize, NULL) == false)
	{
		cerr<<"failed to read"<<endl;
	}
	cout<<"headersize:"<<headersize<<endl;
	cout<<"gx,gy,gz:"<<gx<<","<<gy<<","<<gz<<endl;
	cout<<"sod:"<<sod<<endl;

	
	


	for(int i=1;i < NUMLEVEL;i++)
		createMutiresoBlock(i);	


	CloseHandle(File);

	
	delete[] wc;

	return 0;
}

