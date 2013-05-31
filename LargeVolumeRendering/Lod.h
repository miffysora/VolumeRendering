#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <list>//<queue>ヘッダの内部で<deque>ヘッダを呼んでいる。dequeはdouble-ended-queue（両頭キュー）の略。
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <miffy/math/color.h>
#include <miffy/math/colorpalette.h>
#ifndef _CG_
#include "Cg.h"
#endif
#ifndef _FILE_
#include "File.h"
#endif
#ifndef _BLOCK_
#include "Block.h"
#endif
class Block;
/*レンダリング時間計測構造体*/
typedef struct _TotalRendering {
  float totalrendertime;
	bool flag;
} TotalRendering;

/*!
LODといいつつも、とりあえずはビューフラスタムカリングも入れる。おおざっぱにね。
*/
class CLod
{
public:
	CLod(int _w,int _h);
	~CLod(void);
	void InitBflag();
	void DDAblockRender(vec3<double>& _viewvec, vec3<double>& _abview,Block bl,int* normalreso,int* existblcount,list<Block>* blockQue,int _levelLimit);
	void DDAblockFile(vec3<double> _viewvec, vec3<double> _abview,Block _bl,int* _reso,int* _exist_in_main_count,unsigned int** _blockstate,bool ****needCountMap,bool **** existInMainMap,list<Block>* blockQue);
	void Exit();
	void FileInit(string _path, string _dataname);
	void FileTexInit();//最初にテクスチャメモリにデータをいっぱい載せるか？
	void FileRun();
	void initCountflag(int diff,bool **** needCountflag,bool **** existCountflag);
	void Init();
	void Idle();
	void createTF();
	void exit_func(void);
	vec3<double> getViewvector(vec3<double>* view_z);
	void ProcessNextResoRender(Block bl,int* countA,int* countC,list<Block>* blockQue,int _levelLimit);//1段階高解像度のブロックのノードを探索
	void display();
	void setFrontSlice(Cg& _cg);
	void renderFrustum(vec3<double> _viewvec, vec3<double> _abview,Block _rootBlock,int _levelLimit);//これがなくなると、ボリュームとブロックの線がなくなる。
	void renderBlock(Block block,bool real);//Blockクラスにいるべきでは？
	void testRenderFrustum(Block block,int* Preso,int* NeedInTex_ExistInMainBlcount,list<Block>* _blockQue,int _levelLimit);
	void renderDebugWindow();
	/*!
	@name ファイルスレッドで使う関数
	*/
	//@{
	void requestNormalFrustum(vec3<double> viewvec,vec3<double> abview,Block block);
	void ProcessNextResoLoad(Block bl,int* countA,int* countB,unsigned int** rblock,bool **** needCountflag,bool **** existCountflag,list<Block>* blockQue);//NExt->次のより高解像度のやつって意味。
	void testLoadFrustum(Block block,int* octreeReso,int* existblcount,unsigned int** renderblock,bool **** needCountflag,bool **** existCountflag, list<Block>* blockQue);
	
	//@}
	void SetClippingPlane();
	void PrintInfoOnWindow();
	void Reshape(int _w,int _h);
	void Translate(float _shift);
	void destroyFBO();
	void createFBO();
	void SetLighting(void);
	void SetNextFrustum();
	void SetRenderFrustum();
	void PasteFBOTexture();
	void idleFileRequest(vec3<double> viewvec,vec3<double> abview,Block block);
private:
	int backupreso;
	int practicalRenderBlockNum;
	int FPSMAX;//スライダー調整で決まるFPSの上限
	int s_normalreso;//現解像度
	int s_backupreso;//バックアップ用の解像度。現解像度よりも必ず１個粗い。
	/*!
	@name 先回り視錐台関連
	*/
	//@{
	frustum<double> m_localNextFrustum;/// 先回りしてファイルを読んでおく用の視錐台 予測は、現在の回転ベクトル、へいこういどう　ベクトルから予測する。しかもローカル座標での位置がほしい
	mat4<double> Next_modelViewMatrix;
	mat4<double> Next_projMatrix;
	//@}
	mat4<double> m_InvModelView;//いろいろなとこで使う setFrontSlice getViewvector renderDebugWindow
	frustum<double> m_localRenderFrustum;/// 今レンダリング中の視錐台 
	
	float aspect;
	float m_FovY;
	float m_Bclipresio;
	float Bclipeyeresio;//ズーム値が変わるたびに代わるクリッピング面。
	
	
	/*!
	@nameプロファイリング用
	*/
	//@{
	TotalRendering totalrender;
	double bltime_sum;//1フレームごとの足し算
	double blcount;//1フレームごと
	double bltime_perframe;//1フレームの、ブロックの各処理にかかった時間の平均
	double maintotextime;
	
	int frame_tex_pro;
	int tex_req_frame;
	float rbn;/// これはmaxRendernumにかかわるから大事だ。
	float obn;/// これはmaxRendernumにかかわるから大事だ。
	float ltn;/// これはMAXTEXLOADにかかわるから大事だ。loadtexblockcountと同義かもしれない。

	float totalrendertime_sum;
	
	int loadtexblockcount;/// MAXTEXLOADにかかわるから大事
	//@}
	/*!
	@brief レンダリング調節に必要な時間測定変数*/
	//{@
	int backupNum;
	double raycasttime_lastframe;
	double loadtime_lastframe;
	double render_loadTimeLimit;
	double renderTimeLimit;//1フレームにかけてもう良い時間
	int practicalnum;
	double renderEachBlockTime;//TF Shadeありなら0.5msec
	int realResolutionNum;//最適解像度ブロックがレンダリングされた数
	int realResolutionNum_perFrame;
	double realResolutionNumCount;//1フレームごと
	int renderblockcount;//レンダリングしてるブロックの数をカウント
	//`}
	int normalnbn;//描画に必要な現解像度ブロックの数
	int normalebn;//既に存在している現解像度ブロックの数
	int backnbn;
	int backebn;
	int idealBlockNum;///< 最適解像度ブロックの数(ファイルスレッド、かな？)
	int realResoNumInRender;///< 最適解像度ブロックの数(レンダリングスレッド)
	bool backupOrNot;/// 今、レンダリングが間に合わなくてバックアップ解像度状態になっているのか、そうでないのか。あれ、私は部分的制御したはずだけど。
	int renderBlockNum;/// 実際にレンダリングしているブロックの数
	float nearSlice;
	float farSlice;
	File m_File;
	//パラ5
	float NFrustumSize; //normalfrustumのサイズはAPIビューイングフラスタムの何倍？(台形の体積で考えてます)
	bool**** n_needInMain;
	bool**** n_existInMain;
	bool**** b_needInMain;
	bool**** b_existInMain;
	list<Block> bRequestQueue;/// requestNormalFrustumで使う
	list<Block> nRequestQueue;/// requestNormalFrustumで使う DDABlockFileで値が詰め込まれる。
	list<Block> rnRequestQueue;/// バックアップと現解像度の中から選ばれたやつ（だと思う）
	list<Block> rbRequestQueue;/// バックアップと現解像度の中から選ばれたやつ（だと思う）MAXTEXLOADを超えたときはこっちを使う。（しかし、このマシンのスペックならこのループに入ることはまずないっぽい）
	list<Block> sheedBlock;
	list<Block> miffybackupBlock;
	bool**** render_normal_blocks;/// どのメモリにあるなし関係なく今描画に必要なブロックリスト
	int render_normal_num;
	bool**** r_existInMainMemNotInTexMem;///テクスチャメモリにはないけどメインメモリにはある
	unsigned int **** blockQueueMap;///キューの何番目に何のブロックが入ってるかマップ
	float dPlaneStart;     //スライス開始位置
	vec4<double> m_wBasicCamPos;///これは、ワールド座標系だよね？ glulookatに使う。

	const vec4<double> ini_cam;///0,0,0つまりただの原点。だけど何回も使う。
	const vec3<double> ini_l;//0.1,0
	const vec3<double> ini_u;//0.0,0
	GLuint pbo;//使ってない。
	GLuint fb;
	GLuint fbTex;
	GLuint fbDep;
	int MAXTEXLOAD;
	int maxRenderNum;/// 1度に描画してもよい数
	vec4<double> eyePosition;
	int BACKUPTHRE; /// ファイルのwhileループ1回あたりに読み込んでもよいブロックの最大数
	int BBLoadMargin;
	
public:
	mat4<double>  modelMatrix;//gluUnprojectがdoubleしか対応してないから仕方なく。
	mat4<double> projMatrix;
	int m_ViewPort[4];/// gluUnprojectで使う。Reshapeで更新する。
	mat4<double> modelViewMatrix;
	vec4<double> m_Translate;//ただの平行移動値っぽいんだが
	double axisX;
	double axisY;
	double axisZ;              /* idle時の回転角(方向ベクトル) */
	double angleDelta;                 /* idle時の回転角(角速度)   */
	float eyeDis;
	//詳細度制御に必要
	vec2<int> m_WinSize;
	/*FPS測定・表示*/
	
	int miffytime;
	int timebase;
	double frameTime;
	int order;//描かれた順
	bool nowprocessingflag;//結果をファイル出力するための文字列作成のため
	Cg cg;
	bool showGridFlag;
	/*! @name デバッグウィンドウ制御用の変数
    */
    //@{
	frustum<double>::FrustumName frustumstate;/// サブウィンドウで表示する視錐台を切り替えるためのもの
	//@}

};

