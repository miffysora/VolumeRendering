#pragma once
#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>
using namespace std;

#define VOXELX 2048
#define VOXELY 2048
#define VOXELZ 970

#define PATH "C:/takasao/Largedata_rendering/LargeData/"
#define VFILE "bonsai"
#define FTYPE ".dff"
static string volumefile = PATH VFILE FTYPE; /* ボリュームデータファイル名 */


DWORD dwReadSize;
DWORD dwWriteSize;
HANDLE SeedFile;
HANDLE wFile;
void makeLowerResolutionFile();

LARGE_INTEGER seedladdr;
LARGE_INTEGER writeladdr;

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
