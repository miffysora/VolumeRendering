// RayCasting.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
#include <math.h>
#include <fstream>
#include <limits>
#include <filesystem>//カレントディレクトリを知るのに必要
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <miffy/math/vec2.h>
#include <miffy/math/vec3.h>
#include <miffy/math/vec4.h>
#include <miffy/math/matrix.h>
#include <miffy/math/collisiondetection/aabox.h>
#include <miffy/math/collisiondetection/frustum.h>
#include "Lod.h"
#include "File.h"
#include "Block.h"
using namespace std;
using namespace miffy;
const string INIFILE="./miffysettings.ini";
int MAXBNUM;  //メモリに格納できる最大ブロック数(一つはファイルロード用) raycasting.cpp
int QSIZE;
unsigned int g_MaxTexNum;
float g_RayCastTime=0.0f;
float g_OcculusionTime=0.0f;
int g_BlockStateCount[10];//miffy original
stringstream g_TexLoadTime;
stringstream g_FileLoadTime;
float g_MemRequestTime;
float g_TexRequestTime;
GLuint preintName;
const double TranslateEpsilon=1.0e-6;      /* 平行移動の閾値 すべって逆転してしまうのを防ぐ。          */
const double IdleTranslateRatio=0.25;
bool zoomflag= false;
int winwidth=720;
int winheight=576;
CLod lod(winwidth,winheight);
/*///////////////////////////////////////////////////////////////////////////////
ファイル管理
//////////////////////////////////////////////////////////////////////////////*/
static string path =  "../../";
static string dataname = "VertebralBody_Brick";
/*///////////////////////////////////////////////////////////////////////////////
マルチスレッド関係
////////////////////////////////////////////////////////////////////////////////*/
/*スレッド関数の引数データ構造体*/
typedef struct _thread_arg {
  int thread_no;
	int *data;
} thread_arg_t;
//時間測定用ファイル
LARGE_INTEGER before,after,freq;

/*//////////////////////////////////////////////////////////////////////////////////
その他
///////////////////////////////////////////////////////////////////////////////*/
const double AngleRatio = 0.25;          /* マウスの移動と回転角の比 */
const double IdleAngleRatio = 0.25;      /* idle時の回転角の比　*/
const double AngleEpsilon = 1.0e-6;      /* 回転の閾値          */
int buttondown = -1;                     /* ボタン状態 up(-1)/left(0) */
int beginX, beginY;                      /* マウスカーソルの位置 */
double originX, originY, originZ;        /* 原点位置　　　　　　*/

void multMatrix(double matrix[], float org[], float dst[]);

/*////////////////////////////////////////////////////////////////////////////////////
計測
////////////////////////////////////////////////////////////////////////////////////*/

bool subwindowflag = true;

HANDLE sem;


/*! utility*/
void multMatrix(double matrix[], float org[], float dst[]) {
	float tmp[4];
	tmp[0] = org[0]; tmp[1] = org[1]; tmp[2] = org[2]; tmp[3] = org[3];
	for (int i = 0; i < 4; i++) {
		dst[i] = 0.0;
		for (int j = 0; j < 4; j++) {
			dst[i] += (float)matrix[j*4+i] * tmp[j];
		}
	}
}
void printMatrix(double matrix[16],const char* message){
	printf("%s\n",message);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[0],matrix[1],matrix[2],matrix[3]);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[4],matrix[5],matrix[6],matrix[7]);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[8],matrix[9],matrix[10],matrix[11]);
	printf("[%.3f][%.3f][%.3f][%.3f]\n",matrix[12],matrix[13],matrix[14],matrix[15]);
	puts("");
}

void createTF(){//table for 256*4

	FILE *tffile;
	char strFromIni[200];
	unsigned long readnum= GetPrivateProfileString("Files","sTransferFunction",NULL,strFromIni,200,INIFILE.c_str());
	DWORD dwGle = GetLastError();
	if(readnum<=0){
		cerr<<"読めてないよ！iniファイルの場所変えた？"<<endl;abort();
	}
	tffile=fopen(strFromIni,"rb");
				if(tffile==NULL){
					
					tr2::sys::path mypath;
					cout<<"現在のディレクトリ"<<tr2::sys::current_path<tr2::sys::path>().string()<<endl;
					cout<<"ファイルが見つかりません";abort();
				}
				int tfsize=64;
				GLubyte *TransferFunction = new GLubyte[tfsize*4];
				size_t num = fread(TransferFunction,sizeof(GLubyte),tfsize*4,tffile);
				printf("data num=%d\n",num);
				glGenTextures(1,&preintName);
				glBindTexture(GL_TEXTURE_1D, preintName);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,tfsize,0,GL_RGBA,GL_UNSIGNED_BYTE,TransferFunction);
}

/*
** 初期化
*/
void myInit()
{	
	lod.cg.Init();
	cgGLSetTextureParameter(lod.cg.transfer_functionParam,preintName);lod.cg.CheckCgError();
	
	lod.Init();
	//PCのスペックを調べる。
	const GLubyte *i_vendor, *i_renderer, *i_version;
	i_vendor = glGetString(GL_VENDOR);
	i_renderer = glGetString(GL_RENDERER);
	i_version = glGetString(GL_VERSION);
	cout <<"OpenGL information : "<<endl;
	cout <<"  vendor   : "<< i_vendor << endl;
	cout <<"  renderer : "<< i_renderer<<endl;
	cout <<"  version  : "<< i_version<<endl;

	/* 初期設定 */
	//glClearColor(1.0,1.0,1.0,1.0);//印刷用
	glClearColor(0.0, 0.0, 0.0, 0.0);//フレームバッファを使ってオクルージョンテストするなら背景は黒にすべき
	glClearDepth(1.0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	lod.FileTexInit();

}
void display(void)
{	

	lod.display();
	
	lod.PasteFBOTexture();
	if(subwindowflag){
		lod.renderDebugWindow();
	}

	glDisable(GL_BLEND);
	
	lod.PrintInfoOnWindow();
	/* ダブルバッファリング */
	glutSwapBuffers();

}
void myReshape(int w, int h)
{

	lod.Reshape(w,h);
	lod.SetRenderFrustum();
	lod.SetNextFrustum();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void idle(void){                               /* Idleイベント関数    */
	if(zoomflag)
	{
		if(abs(lod.m_Translate.z) > TranslateEpsilon){//あまりにも小さい値の時はズームインしない。
			lod.m_Translate.z *= IdleTranslateRatio;}
		else{
			lod.m_Translate.z = 0.0;
		}
	
		zoomflag = false;
	}
	lod.Idle();
}
void mouse(int button ,int state, int x, int y){  /* Mouseイベント関数 */
	if (state == GLUT_DOWN){                   /* ボタンプレスの場合  */
		buttondown = button;                   /* ボタン状態の変更　　*/
		beginX = x;                            /* カーソル位置の記録(更新) */
		beginY = y;
		if (button == GLUT_LEFT_BUTTON) {      /* 左ボタンのイベントの場合 */
			lod.angleDelta = 0.0;   
			lod.m_Translate.x = 0.0;
			lod.m_Translate.y = 0.0;
			lod.m_Translate.z = 0.0;                  /* 回転角のクリア(無回転)   */
			glutIdleFunc(NULL);                /* Idleイベント関数のクリア */
		}

		if(button == GLUT_MIDDLE_BUTTON){
			lod.angleDelta = 0.0;   
			lod.m_Translate.x = 0.0;
			lod.m_Translate.y = 0.0;
			lod.m_Translate.z = 0.0;
			glutIdleFunc(NULL);
		}
	}
	else if (state == GLUT_UP){                /* ボタンリリースの場合　*/
		buttondown = -1;                       /* ボタン状態の変更 up(-1)  */
		if (button == GLUT_LEFT_BUTTON) {      /* 左ボタンのイベントの場合 */
			if ( lod.angleDelta > AngleEpsilon ){  /* 回転角がゼロでない場合　 */
				lod.angleDelta *= IdleAngleRatio;
				
			}
			else { 
				lod.angleDelta = 0.0;
			}		
		}
		
		if(button == GLUT_MIDDLE_BUTTON){
			if(abs(lod.m_Translate.x) > TranslateEpsilon)
				lod.m_Translate.x *= IdleTranslateRatio;
			else
				lod.m_Translate.x = 0.0;

			if(abs(lod.m_Translate.y) > TranslateEpsilon)
				lod.m_Translate.y *= IdleTranslateRatio;
			else
				lod.m_Translate.y = 0.0;

		}

		glutIdleFunc(idle);            /* Idleイベント関数の設定   */
	}
}
void motion(int x, int y){                     /* Motionイベント関数      */
	double deltaX, deltaY,deltaZ;                     /* マウスカーソルの移動量　*/
	double X, Y, Z;
	double identMatrix[16] =              /* 単位モデル変換行列 　　　*/
	{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 
	0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}; 



	if (buttondown >= 0) {                /* ボタン状態down(非負)の場合 */
		double modelview[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		
		/* モデル変換行列の取得    */

		switch(buttondown){
		case GLUT_LEFT_BUTTON:
			deltaX = x - beginX;                   /* カーソル移動量(x方向)   */
			deltaY = y - beginY;                   /* カーソル移動量(y方向)   */
			lod.angleDelta = sqrt(deltaX*deltaX + deltaY*deltaY) * AngleRatio;
			/* 回転角の計算 = カーソル移動距離*AngleRatio */
			if (lod.angleDelta > AngleEpsilon) {       /* 回転角がゼロでない場合 */
				gluUnProject(originX+deltaY, originY+deltaX, originZ,modelview, lod.projMatrix.m, lod.m_ViewPort,&lod.axisX, &lod.axisY, &lod.axisZ);       /* 回転軸の計算（点)       */
				gluUnProject(originX, originY, originZ, modelview,lod.projMatrix.m,lod.m_ViewPort, &X, &Y, &Z);         /* 回転軸の計算 (原点)     */
				lod.axisX -= X;  lod.axisY -= Y;  lod.axisZ -= Z; /* 回転軸の計算(ベクトル) */
				if(lod.axisX==std::numeric_limits<double>::has_denorm){abort();}
				glRotated(lod.angleDelta, lod.axisX, lod.axisY, lod.axisZ); /* 回転           */
			}
			else { 
				lod.angleDelta = 0.0;
			}
			break;
		case GLUT_MIDDLE_BUTTON:
		case GLUT_RIGHT_BUTTON:
			glMatrixMode(GL_PROJECTION);      /* 投影変換の設定 　　　　　*/
			gluUnProject((double)x, (double)y, originZ,identMatrix, lod.projMatrix.m, lod.m_ViewPort,&X, &Y, &Z);         /* 平行移動の後 　　　　　　*/
			gluUnProject((double)beginX, (double)beginY, originZ,identMatrix, lod.projMatrix.m, lod.m_ViewPort,&deltaX, &deltaY, &deltaZ); /* 平行移動の後　　　*/
			X -= deltaX;  Y -= deltaY;        /* 平行移動量の計算 　　　　*/
			if(buttondown == GLUT_RIGHT_BUTTON)
			{
				lod.Translate((float)Y);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
			}
			else
			{
				lod.m_Translate.x = X;
				lod.m_Translate.y = -Y;
				glGetDoublev(GL_MODELVIEW_MATRIX, lod.modelMatrix.m);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glTranslated(lod.m_Translate.x,lod.m_Translate.y, 0.0);     /* 平行移動(奥行き方向) 　　*/
				glMultMatrixd(lod.modelMatrix.m);
			}
			glGetDoublev(GL_PROJECTION_MATRIX, lod.projMatrix.m);
			/* 射影変換行列の取得 　　　*/
			gluProject(0.0, 0.0, 0.0, lod.modelMatrix.m, lod.projMatrix.m, lod.m_ViewPort,&originX, &originY, &originZ); /* モデル座標原点の変換　*/
			/* 原点がスクリーン座標に写された位置(回転軸の計算に利用)　*/
			glMatrixMode(GL_MODELVIEW);       /* モデル変換の設定 　　　　*/
			break;
		}

		glutPostRedisplay();                   /* Displayイベント発生     */
		beginX = x;                            /* カーソル位置の記録(x方向) */
		beginY = y;                            /* カーソル位置の記録(y方向  */
	}
}
void wheel(int wheel_number,int direction,int x,int y)
{
	lod.m_Translate.z -= direction*0.01*lod.eyeDis;
	glGetDoublev(GL_MODELVIEW_MATRIX, lod.modelMatrix.m);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0.0,0.0,lod.m_Translate.z);
	glMultMatrixd(lod.modelMatrix.m);
	
	lod.eyeDis -= lod.m_Translate.z;
	lod.SetClippingPlane();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	zoomflag = true;
	glutPostRedisplay();                   // Displayイベント発生     

}
static void keyboard(unsigned char c, int x, int y)
{
	if(c=='r'){
		//if(!nowprocessingflag){}

	//	mainmemlog<<"バックアップ領域"<<BBLoadMargin
		}
	if(c==27){
		exit(0);
	}
	if(c == 'f')
	{
		if(subwindowflag == false)
			subwindowflag = true;
		else 
			subwindowflag = false;
	}

	if(c=='g'){
		lod.showGridFlag=!lod.showGridFlag;
	}
	if(c == 's')
	{
		if(lod.frustumstate == frustum<double>::render)
			lod.frustumstate = frustum<double>::normal;
		else if(lod.frustumstate == frustum<double>::normal)
			lod.frustumstate = frustum<double>::back;
		else
			lod.frustumstate = frustum<double>::render;
	}

	if(c == 'd')
	{
		if(Block::resocolorflag == false)
			Block::resocolorflag = true;
		else 
			Block::resocolorflag = false;
	}

	glutPostRedisplay();

}

/* スレッド関数*/
void fileget_func(void *arg){	
//printf("セマフォ待ち\n");
	WaitForSingleObject(sem,INFINITE);//セマフォ開始（セマフォカウンタを１減らす）ここからReleaseSemaphore(sem)までが排他制御した処理
	//もしセマフォカウンタが０だったら、このスレッドはこのカウンタが他のスレッドによって１以上になるのを待って処理がブロックされる。
	//double sum=0;double	counter=0; double average;
	while(1)
	{
		lod.FileRun();
		lod.nowprocessingflag=false;
		
	}
}
void render_func(void *arg){	

	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(winwidth,winheight);
	glutCreateWindow("OpenGL");
	glewInit();
	createTF();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutReshapeFunc(myReshape);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(wheel);
	

	myInit();
	
	//printf("もすぐセマフォ解放\n");
	//セマファ解放！！
	ReleaseSemaphore(sem,1,NULL);//セマフォ終了（排他処理終了）セマフォカウンタを１増やす
	//printf("セマフォ解放 sem+1\n");
	glutMainLoop();
	lod.cg.Term();
}

void exit_func(void){
	
	
	lod.Exit();
	printf("メモリ解放\n");
	printf("exit\n");
}
/*! 
PCのスペックを調べて、環境にあったパラメータを計算する関数
*/
void InspectPCSpec(){
	//メインメモリ空き容量取得
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	DWORD dwPh = (ms.dwAvailPhys);
	MAXBNUM = (int)(DATAPOOLALPHA*(float)(dwPh/((BLX+2)*(BLY+2)*(BLZ+2)*VOXELSIZE)));//1150,1173 実行するたびに変わる！！！
	//↑で割り出した数が最適とは限らない。なんでた？？？
	g_MaxTexNum=699;//800程度がいいんだけど、その根拠は。。。
	if(MAXBNUM>DATAPOOLBORDER){MAXBNUM=700;}//1000ならいけた。doubleにしたら800もだめになった。。。
	printf("MAXBNUM=%d\n",MAXBNUM);
	//MAXBNUM = 800;
	QSIZE = (int)(DATAPOOLALPHA*(float)(dwPh/((BLX+2)*(BLY+2)*(BLZ+2)*VOXELSIZE)));
	
}
/* メインプログラム*/
int main(int argc, char *argv[])
{
	//	mainmemParameterLog<<"BBLoadMargin,バックアップ解像度,必要なバックアップブロック,既に存在しているバックアップブロックの数,現解像度,必要な現解像度ブロック,既に存在している現解像度ブロック\n";
	atexit( exit_func );//プログラム終了時に呼び出される関数 escape押したときだけ
	InspectPCSpec();
	glutInit(&argc, argv);
	lod.InitBflag();

	thread_arg_t ftarg, rtarg;//スレッド関数の引数（どっちもなしかな）

	sem = CreateSemaphore(NULL,0,1,NULL);//セマフォの初期値＝0,セマフォの最大数=1　ここでエラー
	
	//ファイルインスタント初期化
	lod.nowprocessingflag=true;
	char pathFromIni[200];
	char datanameFromIni[200];
	unsigned long readnum= GetPrivateProfileString("Files","sVolumeDataDir",NULL,pathFromIni,200,INIFILE.c_str());
	if(readnum<=0){cerr<<"iniファイルの場所変えた？"<<endl;abort();	}
	readnum= GetPrivateProfileString("Files","sVolumeDataName",NULL,datanameFromIni,200,INIFILE.c_str());
	lod.FileInit(pathFromIni,datanameFromIni);
	lod.nowprocessingflag=false;
	/*filegetスレッドの作成*/
	/*スレッド関数の引数データの初期化*/
	ftarg.thread_no = 0;

	/*スレッドの生成*/
	HANDLE filegetHandle =CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)fileget_func,(void *)&ftarg,0,NULL);
	SetThreadIdealProcessor(filegetHandle,0);
	

	/*renderスレッドの作成*/
	/*スレッド関数の引数データの初期化*/
	rtarg.thread_no = 0;

	/*スレッドの生成*/
	HANDLE renderHandle= CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)render_func,(void *)&rtarg,0,NULL);
	SetThreadIdealProcessor(renderHandle,1);//生成したスレッドrenderの優先プロセッサを１番に指定。

	WaitForSingleObject(renderHandle,INFINITE);//セマフォ開始（セマフォカウンタを１減らす）
	return 0;
}
