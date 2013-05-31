
#include "Cg.h"


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


int v1[24] = {0,0,1,4, 1,0,1,4, 0,0,2,5, 2,0,2,5, 0,0,3,6, 3,0,3,6};
int v2[24] = {0,1,4,7, 5,1,4,7, 0,2,5,7, 6,2,5,7, 0,3,6,7, 4,3,6,7};


int nSequence[64] = {0,1,2,3,4,5,6,7,              
1,4,5,0,3,7,2,6,
2,5,6,0,1,7,3,4,
3,0,6,4,1,2,7,5,
4,3,7,1,0,6,5,2,
5,7,2,1,4,6,0,3,
6,2,7,3,0,5,4,1,
7,6,4,5,2,3,1,0};

float vecVertices[8][3] = 
{ {1.0,1.0,1.0},
{1.0,1.0,0.0},
{1.0,0.0,1.0},
{ 0.0,1.0,1.0},
{0.0,1.0,0.0},
{1.0,0.0,0.0},
{0.0,0.0,1.0},
{0.0,0.0,0.0}};



Cg::Cg(){}


void Cg::Init(void)
{

  context = NULL;
	vertexProgram = NULL;
	vertexProfile = CG_PROFILE_ARBVP1;
	fragmentProgram = NULL;
	fragmentProfile = CG_PROFILE_ARBFP1;



	//vertex用
	BNUMParam = NULL;
	BRParam = NULL;
	INIBNParam = NULL;



	//fragment用
	vdecalParam   = NULL;
	fdecalParam   = NULL;
	globalAmbientParam = NULL;
	lightColorParam = NULL;
	lightPositionParam = NULL;
	eyePositionParam = NULL;
	KeParam = NULL;
	KaParam = NULL;
	KdParam = NULL;
	KsParam = NULL;
	shininessParam = NULL;
	matModelViewNormalParam = NULL;
	cameraParam = NULL;
	stepsizeParam = NULL;
	volExtentMinParam = NULL;
	volExtentMaxParam = NULL;
	rsclParam = NULL;
	resoColorParam = NULL;
	renderAlphaParam = NULL;
	renderBetaParam  = NULL;
	farSliceParam = NULL;
	rayLoopParam = NULL;
	srcDivParam  = NULL;
	BLParam      = NULL;
	transfer_functionParam = NULL;

	//スライス用
	vertex_sliceProgram = NULL;

	BR2Param = NULL;
	BNUM2Param = NULL;
	INIBN2Param = NULL;
	dPlaneStartParam  = NULL;
	frontIdxParam     = NULL;
	vecViewParam       = NULL;
	nSequenceTemp    = NULL;
	vecVerticesTemp = NULL;
	v1Temp = NULL;
	v2Temp = NULL;

	//occlusion fragment用
	fragment_occlusionProgram = NULL;
	fdecal2Param = NULL;


	vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	fprintf(stderr, "Video card supports : %d.\n", vertexProfile);
	CheckCgError();

	fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	fprintf(stderr, "Video card supports : %d.\n", fragmentProfile);
	CheckCgError();


	context = cgCreateContext();
	CheckCgError();


	vertexProgram= cgCreateProgramFromFile(context, CG_SOURCE, "shader.cg",vertexProfile, "vertex", NULL);//TODO:

	CheckCgError();

	if(vertexProgram != NULL) {
		cgGLLoadProgram(vertexProgram);CheckCgError();
		BNUMParam = cgGetNamedParameter(vertexProgram, "BNUM");CheckCgError();
		BRParam = cgGetNamedParameter(vertexProgram, "BR");CheckCgError();
		INIBNParam = cgGetNamedParameter(vertexProgram, "INIBN");CheckCgError();
	}

	fragmentProgram= cgCreateProgramFromFile(context, CG_SOURCE, "shader.cg",fragmentProfile, "fragment", NULL);
	CheckCgError();

	if(fragmentProgram != NULL)
	{
		cgGLLoadProgram(fragmentProgram);
		CheckCgError();


		vdecalParam = cgGetNamedParameter(fragmentProgram,"vdecal");CheckCgError();	  
		fdecalParam = cgGetNamedParameter(fragmentProgram,"fdecal");CheckCgError();	  

		globalAmbientParam = cgGetNamedParameter(fragmentProgram, "globalAmbient");CheckCgError();
		lightColorParam = cgGetNamedParameter(fragmentProgram, "lightColor");CheckCgError();
		lightPositionParam = cgGetNamedParameter(fragmentProgram, "lightPosition");CheckCgError();
		eyePositionParam = cgGetNamedParameter(fragmentProgram, "eyePosition");CheckCgError();
		KeParam = cgGetNamedParameter(fragmentProgram, "Ke");CheckCgError();
		KaParam = cgGetNamedParameter(fragmentProgram, "Ka");CheckCgError();
		KdParam = cgGetNamedParameter(fragmentProgram, "Kd");CheckCgError();
		KsParam = cgGetNamedParameter(fragmentProgram, "Ks");CheckCgError();
		shininessParam = cgGetNamedParameter(fragmentProgram, "shininess");CheckCgError();
		matModelViewNormalParam = cgGetNamedParameter(fragmentProgram, "matModelViewNormal");CheckCgError();

		cameraParam = cgGetNamedParameter(fragmentProgram, "camera");CheckCgError();
		stepsizeParam = cgGetNamedParameter(fragmentProgram, "stepsize");CheckCgError();
		volExtentMinParam = cgGetNamedParameter(fragmentProgram, "volExtentMin");CheckCgError();
		volExtentMaxParam = cgGetNamedParameter(fragmentProgram, "volExtentMax");CheckCgError();

		rsclParam = cgGetNamedParameter(fragmentProgram, "rscl");CheckCgError();
		resoColorParam = cgGetNamedParameter(fragmentProgram, "resoColor");CheckCgError();

		renderAlphaParam = cgGetNamedParameter(fragmentProgram, "renderAlpha");CheckCgError();
		renderBetaParam = cgGetNamedParameter(fragmentProgram, "renderBeta");CheckCgError();

		farSliceParam = cgGetNamedParameter(fragmentProgram, "farSlice");CheckCgError();

		rayLoopParam = cgGetNamedParameter(fragmentProgram, "rayLoop");CheckCgError();

		srcDivParam = cgGetNamedParameter(fragmentProgram, "srcDiv");CheckCgError();

		BLParam = cgGetNamedParameter(fragmentProgram, "BL");CheckCgError();
		transfer_functionParam = cgGetNamedParameter(fragmentProgram,"transfer_function");CheckCgError();
		//cgGLSetTextureParameter(transfer_functionParam,preintName);CheckCgError();
	}

	vertex_sliceProgram= cgCreateProgramFromFile(context, CG_SOURCE, "shader.cg",vertexProfile, "vertex_slice", NULL);CheckCgError();


	CheckCgError();
	if(vertex_sliceProgram != NULL) {
		cgGLLoadProgram(vertex_sliceProgram);
		CheckCgError();


		frontIdxParam = cgGetNamedParameter(vertex_sliceProgram, "frontIdx");
		CheckCgError();
		dPlaneStartParam = cgGetNamedParameter(vertex_sliceProgram, "dPlaneStart");
		CheckCgError();
		vecViewParam = cgGetNamedParameter(vertex_sliceProgram, "vecView");
		CheckCgError();


		nSequenceTemp = cgGetNamedParameter(vertex_sliceProgram, "nSequence");
		CheckCgError();
		for(int i=0;i< 64;i++)
			nSequenceParam[i] = cgGetArrayParameter(nSequenceTemp,i);
		CheckCgError();
		vecVerticesTemp = cgGetNamedParameter(vertex_sliceProgram, "vecVertices");
		CheckCgError();
		for(int i=0 ;i< 8;i++)
			vecVerticesParam[i] = cgGetArrayParameter(vecVerticesTemp,i);
		CheckCgError();
		v1Temp = cgGetNamedParameter(vertex_sliceProgram, "v1");
		CheckCgError();
		for(int i= 0;i<24;i++)
			v1Param[i] = cgGetArrayParameter(v1Temp,i);
		CheckCgError();
		v2Temp = cgGetNamedParameter(vertex_sliceProgram, "v2");
		CheckCgError();
		for(int i= 0;i<24;i++)
			v2Param[i] = cgGetArrayParameter(v2Temp,i);
		CheckCgError();

		BNUM2Param = cgGetNamedParameter(vertex_sliceProgram, "BNUM");
		CheckCgError();

		BR2Param = cgGetNamedParameter(vertex_sliceProgram, "BR");
		CheckCgError();



		INIBN2Param = cgGetNamedParameter(vertex_sliceProgram, "INIBN");
		CheckCgError();
	}



	//オクルージョンfragment用
	fragment_occlusionProgram= cgCreateProgramFromFile(context, CG_SOURCE, "shader.cg",fragmentProfile, "fragment_occlusion", NULL);CheckCgError();

	if(fragment_occlusionProgram != NULL)
	{
		cgGLLoadProgram(fragment_occlusionProgram);
		CheckCgError();


		fdecal2Param = cgGetNamedParameter(fragmentProgram,"fdecal");
		CheckCgError();	  

	}
	

	this->SetParameter(this->srcDivParam,SRCDIV);


	/*
	MAXBL計算
	*/
	int MAXBL;
	MAXBL = max(BLX,BLY);
	MAXBL = max(MAXBL,BLZ);
	int MAXINI;
	MAXINI = max(INIBLX,INIBLY);
	MAXINI = max(MAXINI,INIBLZ);

	Block::brX = (float)BLX/(float)MAXBL;
	Block::brY = (float)BLY/(float)MAXBL;
	Block::brZ = (float)BLZ/(float)MAXBL;

	
	Block::iniX = (float)INIBLX/(float)MAXINI;
	Block::iniY = (float)INIBLY/(float)MAXINI;
	Block::iniZ = (float)INIBLZ/(float)MAXINI;

	float bl[3] = {BLX,BLY,BLZ};//bl=(64,64,64) １ブロックあたりのボクセルの数を示すと思われる。

	this->SetParameter(BLParam,bl);

	float br[3] = {Block::brX,Block::brY,Block::brZ};//br=(1,1,1)
	this->SetParameter(BRParam,br);
	this->SetParameter(BR2Param,br);

	float ini[3] = {Block::iniX,Block::iniY,Block::iniZ};//ini=(1,1,1)
	//printf("ini(%.lf,%.lf,%.lf)\n",ini[0],ini[1],ini[2]);
	this->SetParameter(INIBNParam,ini);
	this->SetParameter(INIBN2Param,ini);

	float volExtentMin[3] = {0.0f*Block::brX,0.0f*Block::brY,0.0f*Block::brZ};
	//printf("br(%.lf,%.lf,%.lf)\n",Block::brX,Block::brY,Block::brZ);
	this->SetParameter(volExtentMinParam , volExtentMin);


	float volExtentMax[3] = {1.0f*Block::brX,1.0f*Block::brY,1.0f*Block::brZ};
	this->SetParameter(volExtentMaxParam , volExtentMax);


	for(int i = 0;i<24;i++)
	{
		this->SetParameter(v1Param[i],(float)v1[i]);
		this->SetParameter(v2Param[i],(float)v2[i]);

	}
	for(int i = 0;i<64;i++)
		this->SetParameter(nSequenceParam[i],(float)nSequence[i]);
	for(int i= 0;i<8;i++)
		this->SetParameter(vecVerticesParam[i],vecVertices[i]);
}

Cg::~Cg() {}

void Cg::Enable(){
	/* テクスチャ1をアクティブにする*/
	cgGLEnableTextureParameter(vdecalParam);
	cgGLEnableTextureParameter(fdecalParam);
	//cg有効化
	cgGLEnableProfile(vertexProfile);CheckCgError();
	cgGLEnableProfile(fragmentProfile);CheckCgError();
}

void  Cg::Disable(void){
	cgGLDisableTextureParameter(vdecalParam);
	cgGLDisableTextureParameter(fdecalParam);
	cgGLDisableProfile(vertexProfile);CheckCgError();
	cgGLDisableProfile(fragmentProfile);CheckCgError();
}

void  Cg::Term(void){
	cgDestroyProgram(vertexProgram);
	cgDestroyProgram(fragmentProgram);
	cgDestroyProgram(vertex_sliceProgram);
	cgDestroyContext(context);

}

void  Cg::CheckCgError(void){
	CGerror err = cgGetError();

	if (err != CG_NO_ERROR) {
		printf("CG error: %s\n", cgGetErrorString(err));
		abort();
	}

}

void  Cg::SetMatrixParameter(CGparameter param,CGGLenum glenum,CGGLenum glenum2){
	if (param != NULL)
		cgGLSetStateMatrixParameter(param,glenum,glenum2);
	else
	{
		cout<<"CGparameter is null"<<endl;
		abort();
	}
	CheckCgError();
}


void Cg::SetParameter(CGparameter param,float* f){
	if (param != NULL)
		cgGLSetParameter3fv(param, f);
	else
	{
		cout<<"CGparameter is null"<<endl;
		abort();
	}
	CheckCgError();
}
void Cg::SetParameter(CGparameter param,double* f){
	if (param != NULL)
		cgGLSetParameter3f(param, (float)f[0],(float)f[1],(float)f[2]);
	else
	{
		cout<<"CGparameter is null"<<endl;
		abort();
	}
	CheckCgError();
}
void Cg::SetParameter(CGparameter param,float* f,int elenum){
	if (param != NULL)
	{
		if(elenum == 4)
			cgGLSetParameter4fv(param, f);

	}
	else
	{
		cout<<"CGparameter is null"<<endl;
		abort();
	}
	CheckCgError();
}

void Cg::SetParameter(CGparameter param,float f){
	if (param != NULL)
		cgGLSetParameter1f(param, f);
	else
	{
		cout<<"CGparameter is null"<<endl;
		abort();
	}
	CheckCgError();
}

void Cg::SetTextureParameter(CGparameter decal,int texname){
	if (decal != NULL)
		cgGLSetTextureParameter(decal,texname);
	else
	{
		cout<<"CGparameter is null"<<endl;
		abort();
	}
	CheckCgError();
}


void Cg::BindProgram(CGprogram program){
	if(program != NULL)
		cgGLBindProgram(program);
	else
	{
		cout<<"CGparameter is null"<<endl;
		abort();
	}
	CheckCgError();
}
