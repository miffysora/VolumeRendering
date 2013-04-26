#pragma once
#include <cstdio>
#include <cstdlib>   /* for exit */
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
using namespace std;
#include <windows.h>
#include <GL/glew.h> 
#include <GL/glfw.h>
#include <GL/freeglut.h>
#pragma comment(lib,"GLFW.lib")
#pragma comment(lib,"glew32.lib")
#include <miffy/gl/glutility.h>
#include <miffy/math/vec2.h>
#include <miffy/math/vec3.h>
#include <miffy/math/matrix.h>
#include <miffy/math/quaternion.h>
#include <miffy/math/cube.h>
using namespace miffy;
#include "Block.h"

class DDARender
{
public:
  DDARender(int _voxel);
	~DDARender(void);
	void init(mat4<float>& _modelview);
	void DDA();
	void geometryInit();
	void display( );
	cube<float> boundingCube;
	Block *manyblock;//レンダリングする用の元のデータとする。
	float blockLength;
	const int nVoxel;

	Block ***planeList ;//面リスト　２次元配列へのポインタの配列
	int *unRenderedVoxelIt;//未描画のボクセルを示す
	float dPlane;//1.0以上だったら次の面を挿入する
	float *dLine;//1.0以上だったら次の列を挿入する
	float slopePlane;
	float slopeLine;
	int *columnbegin;
	int *insertedColumn;

	int planePos;
	int insertedPlane;
	
	
	bool inverseFlag[3];//trueな進める方向ら逆にする
	int m_first_orientation;
	int m_second_orientation;
	int startIndex;//視線から一番遠い頂点
	int ini[3];
	
	mat4<float> modelViewMatrix;
	mat4<float> inv_modelView;

	double projMatrix[16];
	int viewport[4];
	mat4<float> m_modelview;	
	
	mat4<float> m_inv_modelview;

	
};

