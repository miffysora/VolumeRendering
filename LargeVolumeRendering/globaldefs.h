using namespace std;
#define YELLOW   {1.0,1.0,0.0}
#define CYAN     {0.0,1.0,1.0}
#define MAGENTA  {1.0,0.0,1.0}
#define ORANGE   {1.0,0.5,0.0}
#define WHITE    {1.0,1.0,1.0}
const float BLUE[3]   = {0.0,0.0,1.0};
const float GREEN[3]  = {0.0,1.0,0.0};
const float RED[3]    = {1.0,0.0,0.0};
const float CLR[5][3] = {WHITE,YELLOW,CYAN,MAGENTA,ORANGE};


#define EPSILON 1.0e-6

#define FILTER 0.2


#define THRESHOLD 0.93f

#define BLX   64
#define BLY   64
#define BLZ   64
#define VOXELSIZE 4
//初期状態（つまりlevel4）での各辺のブロックの個数
#define INIBLX 2   
#define INIBLY 2
#define INIBLZ 1



#define SPX  1.0f
#define SPY  1.0f
#define SPZ  1.0f


#define NUMLEVEL 5
//複数ソースコードをまたいで変数やメソッドを使用するときに時に使います。
extern int MAXBNUM;  //メモリに格納できる最大ブロック数(一つはファイルロード用)　file.cppで使う。　１０２４
extern int QSIZE;
//extern int MAXTEXNUM;
//#define QSIZE 800
//#define MAXBNUM 800

/*
操作できそうパラメータ集！！
*/
//パラ1


#define SAMPLEINTERVAL  1   //時間計測を行う間隔
//パラ2
#define RESOCONTROL 1.0f  //解像度の度合いを表すパラメータ(0 〜 1:最高画質)

#define BFRUSTUMRESO 1    //backupfrustum解像度何段階上か?



#define FRUSTUMSIZEPRIORITY 0.8

//パラ5
#define INITNFRUSTUMSIZE  1.0f //normalfrustumのサイズはAPIビューイングフラスタムの何倍？(台形の体積で考えてます)





//#define STEPSIZE 0.005f
#define STEPSIZE 0.005f
#define RAYLOOP  800
#define SRCDIV   2.0




#define INITNEAR 7.0f
#define INITFAR  11.0f

#define TEXLOADPARAMETER 14  //欲しいフレームレート(fps)
#define MINTEXLOAD 4   //一回のフレームでテクスチャロードできる最少保障数

#define HDMAINALPHA 1.0//1.0
#define DATAPOOLALPHA 1.0//0.49//0.5ぐらいが安全に確保できる割合。(1300個ぐらい？)　1.0だと0x7ffffffff以上はnew出来ないのエラー（bad_allocとかにある）
//少なすぎてもダメ（３００個じゃ足りない）
#define DATAPOOLBORDER 1300
/*//////////////////////////////////////////////////////////////////////////
立方体関係
//////////////////////////////////////////////////////////////////////////*/
const int NoV  = 8;
const int NoF  = 6;
const int NoFV = 4;


const int faces[NoF * NoFV] =                  /* 立方体の面 (6個) 　　　　*/
{1, 2, 6, 5,      2, 3, 7, 6,       4, 5, 6, 7,
0, 4, 7, 3,      0, 1, 5, 4,       0, 3, 2, 1};
const float CUBE_VERTICES[NoV * 3] =//中心が0.5,0.5,0.5になるような立方体。
{0.0, 0.0, 0.0,   1.0, 0.0, 0.0,    1.0, 1.0, 0.0,    0.0, 1.0, 0.0,
0.0, 0.0, 1.0,   1.0, 0.0, 1.0,    1.0, 1.0, 1.0,    0.0, 1.0, 1.0};


const float texcoord[NoV][3] =                // 立方体の頂点 (8個) 　　　
{{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0},
{0.0, 0.0, 1.0},  {1.0, 0.0, 1.0},  {1.0, 1.0, 1.0},  {0.0, 1.0, 1.0}};

extern  float g_RayCastTime;/// これはmxRenderNumにかかわるから大事
extern  float g_OcculusionTime;/// これはmxRenderNumにかかわるから大事
extern unsigned int g_MaxTexNum;//テクスチャメモリに格納できる最大ブロック数　 PC環境に合わせて変わる

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
