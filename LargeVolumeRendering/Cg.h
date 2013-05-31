#pragma once
#ifndef _CG_
#define _CG_
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _BLOCK_
#include "Block.h"
#endif

class Cg 
{

public:

  CGcontext context;
	CGprogram vertexProgram;
	CGprofile vertexProfile;
	CGprogram fragmentProgram;
	CGprofile fragmentProfile;

	CGparameter vdecalParam;
	CGparameter fdecalParam;
	
	
	CGparameter matModelViewNormalParam;
	CGparameter globalAmbientParam;
	CGparameter lightColorParam;
	CGparameter lightPositionParam;
	CGparameter eyePositionParam;
	CGparameter KeParam;
	CGparameter KaParam;
	CGparameter KdParam;
	CGparameter KsParam;
	CGparameter shininessParam;
	CGparameter cameraParam;
	CGparameter stepsizeParam;
	CGparameter volExtentMinParam;
	CGparameter volExtentMaxParam;
	CGparameter BNUMParam;
	CGparameter BRParam;
	CGparameter INIBNParam;
	CGparameter rsclParam;
	CGparameter resoColorParam;
	CGparameter farSliceParam;
	CGparameter rayLoopParam;
	CGparameter srcDivParam;
	CGparameter BLParam;
	CGparameter transfer_functionParam;
	
	//高→低描画用
	CGparameter renderAlphaParam;
	CGparameter renderBetaParam;


	


	//スライス用
	CGprogram vertex_sliceProgram;
	CGparameter BR2Param;
	CGparameter BNUM2Param;
	CGparameter dPlaneStartParam;
	CGparameter frontIdxParam;
	CGparameter vecViewParam;
	CGparameter nSequenceTemp;
	CGparameter nSequenceParam[64];
	CGparameter vecVerticesTemp;
	CGparameter vecVerticesParam[8];
	CGparameter v1Temp;
	CGparameter v1Param[24];
	CGparameter v2Temp;
	CGparameter v2Param[24];
	CGparameter INIBN2Param;


	//オクルージョンfragment用
	CGprogram fragment_occlusionProgram;
	CGparameter fdecal2Param;

	
	Cg::Cg(void);
	Cg::~Cg();
	
	void  Cg::Init(void);
	void  Cg::Enable(void);
	void  Cg::Disable(void);
	void  Cg::Term(void);
	void  Cg::CheckCgError(void);
	void  Cg::SetMatrixParameter(CGparameter param,CGGLenum glenum,CGGLenum glenum2);
	void  Cg::SetParameter(CGparameter param,float* f);
	void  Cg::SetParameter(CGparameter param,double* f);
	void  Cg::SetParameter(CGparameter param,float* f,int elenum);
	void  Cg::SetParameter(CGparameter param,float f);
	void  Cg::SetTextureParameter(CGparameter decal,int texname);
	void  Cg::BindProgram(CGprogram program);

};


#endif
