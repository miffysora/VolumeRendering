#ifndef _BLOCK_
#define _BLOCK_
#include <GL/glew.h>
#include <GL/freeglut.h>

#ifndef MIFFY_VEC2
#include <miffy/math/vec2.h>
#endif
#ifndef MIFFY_VEC3
#include <miffy/math/vec3.h>
#endif
#ifndef MIFFY_VEC4
#include <miffy/math/vec4.h>
#endif
#ifndef MIFFY_MATRIX
#include <miffy/math/matrix.h>
#endif
#ifndef MIFFY_FRUSTUM
#include <miffy/math/collisiondetection/frustum.h>
#endif
#include <miffy/math/colorpalette.h>


#include "globaldefs.h"
#ifndef _CG_
#include "Cg.h"
#endif
class Cg;
#ifndef _FILE_
#include "File.h"
#endif
class File;



using namespace miffy;

class Block 
{


public:
  int x;//x方向のブロックインデックス（level0だと0〜32の値、level4だと0か1）
	int y;//y方向のブロックインデックス（level0だと0〜32の値、level4だと0か1）
	int z;//z方向のブロックインデックス（level0だと0〜16の値、level4だと0）
	short int level;
	int bnumx;///< x方向のブロックの個数（levelによって一意に決まる）
	int bnumy;
	int bnumz;

	bool countflag;
	

	static unsigned int* nRequestBlock[NUMLEVEL]; //描画するブロックを格納（前回層分持つ）
	static unsigned int* bRequestBlock[NUMLEVEL];
	static unsigned int* rRequestBlock[NUMLEVEL];
	//static bool**** occludedIndex;//[NUMLEVEL][32][32][16];

	//比率をそろえるための何かよね。
    static float brX,brY,brZ;
	static float iniX,iniY,iniZ;
	static bool resocolorflag;
	//static enum renderReso{low,present,high};
	//                     0         1             2         3        4         5                  6                     7                  8                    9               
	static enum blockState{notexist,renderpreblock,frameout,nextreso,waitblock,renderfirstlowblock,rendersecondlowblock,renderthirdlowblock,renderfourthlowblock,occlusionculling};//10個
	//0notexist,//empty space skippingされたもの（データがないからレンダリングする価値なし）（testRenderFrustum,testLoadFrustumで使われる）
	//1renderpreblock,//現在の解像度がレンダリングされた。一番望ましい状態（renderBlock()で使われる）
	//2frameout,//視錐台テストでアウトだったもの（testRenderFrustum,testLoadFrustumで使われる）
	//3nextreso,より細かいブロック（子ノード）によって置き換えられたブロックかと思われる(processNextResoRender ot processNextResoLoadで使われる)
	//4waitblock,テクスチャメモリのロード待ち（renderBlock()で使われる）
	//5renderfirstlowblock,実際よりも1段階低解像度が使われた（renderBlock()で使われる）
	//6rendersecondlowblock,実際よりも2段階低解像度が使われた（renderBlock()で使われる）
	//7renderthirdlowblock,実際よりも3段階低解像度が使われた（renderBlock()で使われる）
	//8renderfourthlowblock,実際よりも4段階低解像度が使われた（renderBlock()で使われる）
	//9occlusionculling　オクルージョンカリングされたもの（renderBlock()で使われる）

	static float  CLR[NUMLEVEL][3];
	bool isSameBlock(Block testBlock);
	int IsInFrustum(const frustum<double>& _frustum);
	Block(int x,int y,int z,short int level,int bnumx,int bnumy,int bnumz);
	Block(void);
	~Block();
	Block returnParent();
	void setBlock(int x,int y,int z,short int level,int bnumx,int bnumy,int bnumz);
	static void renderSubblock(unsigned int** rblock,Block bl,float alpha);

	bool  resoChange(const mat4<double>& _modelmatrix,const mat4<double>& _projmatrix,const vec2<int>& _winsize,int _levelLimit);
	float calcDistanceFromcamera(double* modelmatrix);

	void RayCast(Cg* Cg,int dif);
	void DDAblock(vec3<float> viewvec,vec3<float> abview,File File,Cg Cg,frustum<float> frustum,double* modelmatrix);
	Block getLowblock();
	Block getMultiLowblock(int stage);
	Block getThisBlock();

	Block getHighblock();
	static Block GetRootBlock();
	void setBlockState(blockState state,unsigned int** renderblock);
	vec3<double> getBlockVec(const mat4<double>& _modelViewMatrix);
	bool testOcclusion(Cg* Cg);//もしアルファ値がthresholdよりも小さいフラグメントが存在するならtrue,存在しないならflaseが変える
	blockState getLowState(Block lowblock);
	void earlyRayTermination(Cg* Cg);

	static void mMtx(double matrix[], float org[], float dst[]);
	void renderNumber(int _num,float r,float g,float b);
	void setOccludeState(bool _state);
	void Info(const char* message,float _num);
	void renderBlockLines(float r,float g,float b,float alpha);
	vec3<float>* convertTexToLocal();
};


#endif
