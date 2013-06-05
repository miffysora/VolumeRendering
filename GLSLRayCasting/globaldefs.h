#include <GL/glew.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <miffy/gl/glutility.h>
#include <miffy/gl/shaderutility.h>
#include <miffy/volren/glvolren.h>
const int nVoxel=256;
const int nVoxelZ=256;
#define FILENAME "../../../data/raw/uchar/bonsai.raw"
#define TFFILE "../../../data/tfdata/float/icygrass-256.tf"
#include <miffy/math/vec4.h>
#include <miffy/math/vec3.h>
#include <miffy/math/matrix.h>
#include <miffy/math/quaternion.h>

GLuint gWidth=800;
GLuint gHeight=800;//1176;

miffy::mat4<float> rotation;
miffy::mat4<float> gProj;
miffy::mat4<float> gModelView;
miffy::mat4<float> gInvModelView;
//miffy::mat4 gModelViewProj;
  
miffy::quat<float> current_quaternion(1.0,0.0,0.0,0.0);
miffy::quat<float> target_quaternion;
float previous_x=0;
float previous_y=0;
enum TOUCHSTATE{DOWN,UP,MOVE,TWO_DOWN=261,ONE_UP=262};
float gfWinWidth;
float gfWinHeight;
float gZoom=-5;
GLuint gProgram;
GLuint gSimpleProgram;
GLuint gTexSamplerLoc;
GLuint gTFTexSamplerLoc;
GLuint gCamUniLoc;
GLint projectionUniform;
GLuint modelviewUniform;
GLuint gUniLocVolMax;
GLuint gUniLocVolMin;
miffy::vec4<float> gIniCam(0,0,gZoom,1.0);
miffy::vec4<float> gLocalCoordCam(0,0,gZoom,1.0);//座標変換後のカメラ座標
float gFovy=55;
float gRatio;
// インデックス
enum {
    ATTRIB_VERTEX,
    ATTRIB_COLOR,
	ATTRIB_TEXCOORD,
    NUM_ATTRIBUTES
};
char* gFragmentShader;
const char gSimpleVertexShader[] = 
    "attribute vec4 Position;      \n"
    "varying vec4 DestinationColor;\n"
    "uniform mat4 Projection;      \n"
    "uniform mat4 Modelview;       \n"
    "void main() {\n"
        "   gl_Position = Projection * Modelview * Position; \n"
        "   DestinationColor =gl_Color; \n"
    "}\n";
// フラグメントシェーダ
const char gSimpleFragmentShader[] = 
    "varying  vec4 DestinationColor; \n"//ここがlowpのエラーだった
    "void main() {  \n"
	"  gl_FragColor = DestinationColor;  \n"
    "}\n";
// 頂点シェーダ
 const char gVertexShader[] = 
    "attribute vec4 Position;      \n"
    "uniform mat4 Projection;      \n"
    "uniform mat4 Modelview;      \n"
	"varying vec3 vpos;\n"
    "void main() {\n"
		"gl_Position = Projection * Modelview * Position; \n"
		"vpos=(Position.xyz+1.0)/2.0;\n"//0.0-1.0に直した
    "}\n";


GLuint texId[2];
unsigned char *voldata;
float* tfdata;
float* points_vertex;
float* volTexture;
float threshold=0.1;
float second_thld=0.3;
float  verts[3*24];
