#ifndef _FILE_
#define _FILE_
#include <GL/glew.h>
#include <GL/freeglut.h>
#ifndef MIFFY_FRUSTUM
#include <miffy/math/collisiondetection/frustum.h>
#endif
#ifndef _CG_
#include "Cg.h"
#endif
class Cg;
#ifndef _BLOCK_
#include "Block.h"
#endif
class Block;
using namespace std;
using namespace miffy;
const int ULONGLONGSIZE=10;///< 優先度スコアを拡張する数だと思う
/// メインブロックリクエストキュー管理構造体
typedef struct _RequestQueue{
  ULONGLONG needflag[ULONGLONGSIZE]; /// そのブロックが必要か否か　優先度スコア
	Block* block;
}RequestQueue;
static const ULONGLONG LASTFIVEBITS=32;
/// メインブロック管理構造体　ブロックの数だけ作る
typedef struct _MainMemManage{
	ULONGLONG needflag[ULONGLONGSIZE];/// 優先度スコア
	Block* block;
	int FromLoadTime;
	float* data;                     //texSubImage3Dに渡す、GPUに渡すデータのポインタ。実際の場所を指すアドレス
	bool replaceable;                   //上書きを禁止するかフラグ
	bool loadStartFlag;       //そのブロックをロードしはじめたか否か
	LPOVERLAPPED ol;//winAPI 
	HANDLE Fl;
}MainMemManage;


/// 全メインブロック管理用構造体
typedef struct _bstruct{
	MainMemManage* dataIndex;  //メインメモリ上のどこに存在するかのアドレス
	RequestQueue*  queIndex;
}bstruct;

//// テクスチャブロック管理用構造体
typedef struct _TexMemManage{
    ULONGLONG needflag[ULONGLONGSIZE];           //そのブロックが必要か否か
	GLuint* texdata;                     //実際の場所を指すアドレス
	Block* block;
	bool loadStartFlag;///< 私が加えた。blockRockを使う代わりにこれにしたんだっけかな。
}TexMemManage;

/// 全テクスチャブロック管理用構造体
typedef struct _btexstruct{
	TexMemManage* texdataIndex;/// テクスチャメモリ上のどこに存在するかのアドレス
}btexstruct;

class File 
{

private:
	/*! @name プロファイリング用の変数*/
	//@{
	int HDblockLoadCount[NUMLEVEL][32][32][16];///< HDからメインメモリへロードされたブロックを数える
	//int texmemlog[g_MaxTexNum];
	
	//@}
	/*visible index(全階層分持つ)*/
   unsigned int* indexblock[NUMLEVEL];
   unsigned int* texCompressInfo[NUMLEVEL-1];
   string m_Path;//多解像度ボリュームデータの保存してあるディレクトリ
   string m_DataName;//ボリュームデータセットの名前
   /*メインメモリ制御用*/
	bstruct**** mainMemMap;///< MainMemManageなどを入れておく4次元配列   私はblockRockをやめてこれをつかってる。なんでだっけ？                
	MainMemManage*  memblock;
	float*    datapool;
	//リクエストキュー
	RequestQueue* reqque;

	/*テクスチャメモリ制御用*/
	btexstruct**** texMemMap;
	TexMemManage* texblock;
	GLuint*    texName;

	float loadTime;
	float texLoadTime;
	bool  texLoadflag;
	/*! @name 私が加えたかもしんない謎の変数*/
	//@{
	bool texmemfullflag;
	bool mainmemfullflag;
	//@}
	/*ハードディスク読込み時間測定用*/
	LARGE_INTEGER* mTimeStart;
	LARGE_INTEGER* mTimeEnd;
	LARGE_INTEGER freq;
	float temp_sum;
	int temp_count;

	/*ハードディスク読込み時間測定用*/
	LARGE_INTEGER fTimeprev,before,after,freq2;
	LARGE_INTEGER fTime;
	float fileThreadTime;
	double maintotextime;

public:
	 
	File();
	~File();
	int getCompressInfo(Block testBlock);
	void Init(string path,string dataname);
	void deleteMemory(btexstruct* bstex);
	void deleteMemory(bstruct* _mainMemMap);
	LARGE_INTEGER getLaddress(long long address);
	void loadFile(string _file,int _header,int _size,MainMemManage* _mmm);
	void loadIndexFile(string file,unsigned int* data,int header,int fx, int fy, int fz);
	GLuint* getTexaddress(int index);
	
	bool loadHDToMain(int mostWantedIndex);
	void loadMainToTex(Block bl,Cg* Cg,CGparameter decal,ULONGLONG _reqvalue);///< 私は最後の引数を加えた
	void setTexBlock(Cg* Cg,CGparameter decal,Block block);

	void countMainLRU(void);
	void countTexLRU(void);

	int getIndexblock(Block bl);
		
	bool blockExist(Block bl);
	Block lowBlockExist(Block bl);
	Block lowBlockExistFromRoot(Block bl);
	bool highBlockExist(Block bl);
	int getTexCompressInfo(Block bl);
	bool texBlockExist(Block bl);
	Block texLowBlockExist(Block bl);
	/*! @name 私が加えたかもしんない謎の関数*/
	//@{
	int getTexLoadCompressInfo(Block testBlock);
	void updateNeedScoreInMain(double* modelmatrix,double* projmatrix,Block bl,frustum<double> fr,frustum<double>::FrustumName fname);
	
	void initTexLoad(int _level);///< 部分的詳細度制御かもしんない
	void initMainLoad(int _level);///< 部分的詳細度制御かもしんない
	//void initMainLoad();
	//void initTexLoad();
	void getMainNeedFlagChar(int _i,int _j);
	void outPutfile();
	int getMinIndexInTex(ULONGLONG *reqquevalue);
	int getMinIndexInMain(ULONGLONG *reqquevalue);
	int getMinIndexInReq();
	int returnHDInfo(Block bl);
	void printBits(ULONGLONG _value[ULONGLONGSIZE]){
		for(int i=ULONGLONGSIZE-1;i>=0;i--){
			char c[64];
			sprintf(c,"%016I64x",_value[i]);//16進数で表示したいとき	
			//	sprintf(c,"%I64u",_value);//10進数で表示したいとき
			printf("%s,",c);
		}
		printf("\n");
	}
	int min_yoko_find(ULONGLONG miffy[ULONGLONGSIZE]){
		for(int i=ULONGLONGSIZE-1;i>=0;i--){
			if(miffy[i]!=0) return i;
		}
		return -1;
	}
	//@}
	GLuint getTexName(int i);
	void blockProhibitReplaceInMain(Block bl);
	void blockPermitReplaceInMain(Block bl);
	/*引数blがまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。blがもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
	void mainBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<double> _frustum,frustum<double>::FrustumName fname);
	void texBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<double> _frustum,frustum<double>::FrustumName fname);
	int getMaxIndexInReq();

	ULONGLONG getReqbit(double* modelmatrix,double* projmatrix,Block bl,frustum<double> _frustum,frustum<double>::FrustumName fname);

	float getTexLoadTime();
	bool  getTexLoadflag();

	float getMainLoadTime();
	
	void countThreadTime();/*ハードディスク読込み時間測定用*/
	float getThreadTime();
	double getMainToTexTime();
	void deleteMemory();
	
	int returnTexMemInfo(int _i);
	int returnMainMemInfo(int _i);
	
};


#endif
