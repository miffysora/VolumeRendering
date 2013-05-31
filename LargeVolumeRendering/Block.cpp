#include "Block.h"
#include <miffy/math/cube.h>
#define min(a,b)            (((a) < (b)) ? (a) : (b))


float Block::brX = 0;//いつも1ぽいわ
float Block::brY = 0;
float Block::brZ = 0;
float Block::iniX = 0;
float Block::iniY = 0;
float Block::iniZ = 0;


unsigned int* Block::nRequestBlock[NUMLEVEL]; //requestNormalFrustum,testLoadFrustumで使われる　現解像度ブロックのステート達
unsigned int* Block::bRequestBlock[NUMLEVEL];//requestNormalFrustumtestLoadFrustumで使われる バックアップ用ブロックのステートたち
unsigned int* Block::rRequestBlock[NUMLEVEL];//様々なブロックステートを保持する。主にrenderBlock()の中にあるブロックについて　レンダー視錐台にあるブロック

bool  Block::resocolorflag = false;

float  Block::CLR[NUMLEVEL][3];



Block::Block(int x,int y,int z,short int level,int bnumx,int bnumy,int bnumz) :
x(x),y(y),z(z),level(level),bnumx(bnumx),bnumy(bnumy),bnumz(bnumz)
{//x,y,zはブロックのインデックス bnumは各辺のブロックの数。

//  setBlock(x,y,z,level,bnumx,bnumy,bnumz);
}



Block::Block(void)
:x(0),y(0),z(0),level(0),bnumx(0),bnumy(0),bnumz(0) {

}


Block::~Block() {}

	

void Block::setBlock(int x,int y,int z,short int level,int bnumx,int bnumy, int bnumz) {


	this->x = x;
	this->y = y;
	this->z = z;
	this->level = level;
	this->bnumx = bnumx;
	this->bnumy = bnumy;
	this->bnumz = bnumz;

}

Block Block::getThisBlock(){
	return Block(this->x,this->y,this->z,this->level,this->bnumx,this->bnumy,this->bnumz);
}
void Block::setOccludeState(bool _state){
//	this->occludedIndex[this->level][this->x][this->y][this->z]=_state;
}

Block Block::returnParent(){
	if(this->level!=4){
	return Block(this->x/2,this->y/2,this->z/2,this->level+1,this->bnumx/2,this->bnumy/2,this->bnumz/2);
	}else{printf("これ以上無理です\n");return Block(this->x,this->y,this->z,this->level,this->bnumx,this->bnumy,this->bnumz);}
}
void Block::renderSubblock(unsigned int** rblock,Block bl,float alpha)//サブウィンドウのブロックをレンダリングする。
{
	int rb = rblock[bl.level][(bl.x*bl.bnumy+bl.y)*bl.bnumz+bl.z];//blockstateを格納する。
	if( rb != Block::notexist)
	{
		if(rb != Block::nextreso)
		{//level4=オレンジ level3=ピンク　level2=シアン　level1=黄色　level0=白
			if(rb == Block::renderpreblock)
				glColor4f(CLR[bl.level][0],CLR[bl.level][1],CLR[bl.level][2],alpha);
			else if(rb == Block::renderfirstlowblock)
				glColor4f(CLR[bl.level+1][0],CLR[bl.level+1][1],CLR[bl.level+1][2],alpha);
			else if(rb == Block::rendersecondlowblock)
				glColor4f(CLR[bl.level+2][0],CLR[bl.level+2][1],CLR[bl.level+2][2],alpha);
			else if(rb == Block::renderthirdlowblock)
				glColor4f(CLR[bl.level+3][0],CLR[bl.level+3][1],CLR[bl.level+3][2],alpha);
			else if(rb == Block::renderfourthlowblock)
				glColor4f(CLR[bl.level+4][0],CLR[bl.level+4][1],CLR[bl.level+4][2],alpha);
			else if(rb == Block::frameout)
				glColor4f(BLUE[0],BLUE[1],BLUE[2],alpha);
			else if(rb == Block::waitblock)
				glColor4f(GREEN[0],GREEN[1],GREEN[2],alpha);
			else		if(rb==Block::occlusionculling)
				glColor4f(0.8,0.73,0.12,1.0);
			else
				glColor4f(1.0,1.0,1.0,0.125);//glColor4f(RED[0],RED[1],RED[2],0.1);
		
			//if(	occludedIndex[bl.level][bl.x][bl.y][bl.z]){}

			float x = (2.0f*bl.x-bl.bnumx)*brX*iniX/bl.bnumx;//ブロックの左下隅の座標
			float y = (2.0f*bl.y-bl.bnumy)*brY*iniY/bl.bnumy;
			float z = (2.0f*bl.z-bl.bnumz)*brZ*iniZ/bl.bnumz;
			//printf("[%.2f][%.2f][%.2f]\n",iniX,iniY,iniZ);//bl.x,bl.y,bl.zはブロックインデックス　0〜32の整数値。
		
			for (int a = 0; a < NoF; a++) {  //原点は画面の真ん中。
				glBegin(GL_LINE_LOOP);//ブロックの線を描画してるところ。         
				for (int b = 0; b < NoFV; b++)
				{//printf("[%.1f][%.1f][%.1f]\n",x,y,z);//x,y,zは0.5,0.25,-0.75など色々変わる。
					float dx = x + 2.0f*CUBE_VERTICES[3*faces[4*a+b]+0]*brX*iniX/bl.bnumx;//bl.bnumは、１辺あたりのブロックの個数（int）
					float dy = y + 2.0f*CUBE_VERTICES[3*faces[4*a+b]+1]*brY*iniY/bl.bnumy;
					float dz = z + 2.0f*CUBE_VERTICES[3*faces[4*a+b]+2]*brZ*iniZ/bl.bnumz;
					glVertex3f(dx,dy,dz); //br?もini?もずっと１
					//printf("[%.3f][%.3f][%.3f]\n",dx,dy,dz);
				}
				glEnd();                           
			}
		
		}//nextresoの場合
		else
		{//再帰的に呼び出す
			renderSubblock(rblock,Block(2*bl.x  ,2*bl.y  ,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
			renderSubblock(rblock,Block(2*bl.x+1,2*bl.y  ,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
			renderSubblock(rblock,Block(2*bl.x  ,2*bl.y+1,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
			renderSubblock(rblock,Block(2*bl.x  ,2*bl.y  ,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
			renderSubblock(rblock,Block(2*bl.x+1,2*bl.y+1,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
			renderSubblock(rblock,Block(2*bl.x  ,2*bl.y+1,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
			renderSubblock(rblock,Block(2*bl.x+1,2*bl.y  ,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
			renderSubblock(rblock,Block(2*bl.x+1,2*bl.y+1,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),alpha);
		}
	}//if block not exist or not
}
/*!
@brief dst=matrix*org
*/
void Block::mMtx(double matrix[], float org[], float dst[]) {
	float tmp[4];
	tmp[0] = org[0]; tmp[1] = org[1]; tmp[2] = org[2]; tmp[3] = org[3];
	for (int i = 0; i < 4; i++) {
		dst[i] = 0.0;
		for (int j = 0; j < 4; j++) {
			dst[i] += (float)matrix[j*4+i] * tmp[j];
		}
	}
}

float distance(float bp[],float vp[])
{
	float dis[3] = {bp[0]-vp[0],bp[1]-vp[1],bp[2]-vp[2]};
	
	return sqrt(dis[0]*dis[0]+dis[1]*dis[1]+dis[2]*dis[2]);
}


const float blsign[8 * 3] =
{0.0f, 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,    -1.0f, -1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
0.0f, 0.0f, -1.0f,   -1.0f, 0.0f, -1.0f,    -1.0f, -1.0f, -1.0f,    0.0f, -1.0f, -1.0f};

float Block::calcDistanceFromcamera(double* modelmatrix){
	float blockAPosition[4];//far blockpositionの位置になる
	float blockPosition[4];//far blockpositionの位置にモデルビューマトリックスをかけたもの
	float farblock[4];
	int  farId;
	float maxfarblockz = -100.0f;
	float farblockz;
	for(int i = 0;i  < 8;i++)
	{//convert tex coord to world coord
		float x = (2.0f*this->x-this->bnumx)*Block::brX*Block::iniX/this->bnumx + 2.0f*CUBE_VERTICES[3*i + 0]*Block::brX*Block::iniX/this->bnumx;
		float y = (2.0f*this->y-this->bnumy)*Block::brY*Block::iniY/this->bnumy + 2.0f*CUBE_VERTICES[3*i + 1]*Block::brY*Block::iniY/this->bnumy;
		float z = (2.0f*this->z-this->bnumz)*Block::brZ*Block::iniZ/this->bnumz + 2.0f*CUBE_VERTICES[3*i + 2]*Block::brZ*Block::iniZ/this->bnumz;

		blockAPosition[0] = x;
		blockAPosition[1] = y;
		blockAPosition[2] = z;
		blockAPosition[3] = 1.0;

		mMtx(modelmatrix, blockAPosition, blockPosition);
//カメラ位置を原点とした座標になるはず
	
		farblockz = sqrt(blockPosition[0]*blockPosition[0]+blockPosition[1]*blockPosition[1]+blockPosition[2]*blockPosition[2]);//視点から最も遠いブロックの距離。

		
//１ブロックの８つの頂点の中で視点から最も遠いブロックの頂点を求める
		if(farblockz > maxfarblockz)
		{
			maxfarblockz = farblockz;

			memcpy(farblock,blockAPosition,sizeof(float)*4);//blockAPositionの先頭からsizeof(float)*4個分farblockへコピーする。

			farId = i;//８回繰り返す（各頂点をチェック）
		}
		
	}//end for i=8
	return maxfarblockz;
}
//スクリーンの解像度を考慮した探索判定法　論文p27
bool  Block::resoChange(const mat4<double>& _modelmatrix,const mat4<double>& _projmatrix,const vec2<int>& _winsize,int _levelLimit){

	
	/*１ブロックの８つの頂点の中で視点から最も遠いブロックの頂点を求める*/
	vec4<double> local_block_pos;//far blockpositionの位置になる
	vec4<double> view_block_pos;//視点が原点な座標系
	vec4<double> farblock;
	int  farId;
	double maxfarblockz = -DBL_MAX;
	double farblockz;
	for(int i = 0;i  < 8;i++)
	{//convert tex coord to world coord
		local_block_pos.x = (2.0f*this->x-bnumx)/bnumx + 2.0f*CUBE_VERTICES[3*i + 0]/bnumx;
		local_block_pos.y = (2.0f*this->y-bnumy)/bnumy + 2.0f*CUBE_VERTICES[3*i + 1]/bnumy;
		local_block_pos.z  = (2.0f*this->z-bnumz)/bnumz + 2.0f*CUBE_VERTICES[3*i + 2]/bnumz;

		local_block_pos.w = 1.0;
		view_block_pos=_modelmatrix*local_block_pos;
		//カメラ位置を原点とした座標になるはず
		farblockz=view_block_pos.length();//視点から最も遠いブロックの距離。

		//１ブロックの８つの頂点の中で視点から最も遠いブロックの頂点を求める
		//なぜ１番遠いのを探すのか？１番遠いのが一番小さく映るからだ。
		if(farblockz > maxfarblockz)
		{
			maxfarblockz = farblockz;
			memcpy(&farblock.x,&local_block_pos.x,sizeof(vec4<double>));//blockAPositionの先頭からsizeof(float)*4個分farblockへコピーする。
			farId = i;//８回繰り返す（各頂点をチェック）
		}

	}//end for i=8
	//選ばれし１番遠いボクセル投影ピクセルを計算する
	/*1ボクセルの大きさを測る これは画面上の大きさだ*/
	double minX = DBL_MAX ,maxX = -DBL_MAX;
	double minY = DBL_MAX ,maxY = -DBL_MAX;
	//blsignは（0,0,0）-(-1,-1,-1)を範囲とする立方体　１回だけ足す
	//たぶん、内側になるように工夫してるんだね
	farblock.x += blsign[3*farId+0]/(bnumx*BLX);
	farblock.y += blsign[3*farId+1]/(bnumy*BLY);
	farblock.z += blsign[3*farId+2]/(bnumz*BLZ);
	cube<double> farthest_voxel;
	farthest_voxel.setFromCorner(farblock.toVec3(),1.0/(bnumx*BLX));
	
	vec2<double> pixel_size=farthest_voxel.projectedsize(_modelmatrix,_projmatrix,_winsize);
	
	bool is_need_more_resolution = true;
	
	if(this->level == _levelLimit || pixel_size.x < 2.0/_winsize.x || pixel_size.y < 2.0/_winsize.y){//最も高解像度 or スクリーン上のボクセルの大きさが1ピクセルよりも小さい場合。
		is_need_more_resolution = false;//ボクセルの大きさは、ピクセルの大きさよりも小さいため、これ以上高解像度にする必要はない。
		return is_need_more_resolution;
	}	//これがtrueならProcessNextResoLoad関数が呼び出される。
	else {return true;}
	return is_need_more_resolution;
}


/*!
@brief レイキャスティング＋ブロック化での不自然なクリッピング面問題を解決するためにスライス面を実装している
*/
void Block::earlyRayTermination(Cg* Cg)//RayCast()メソッドの中で使う
{
	glDepthMask(GL_TRUE);//write depth
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);//don't write colors
	//test alpha channel of the framebuffer
	glAlphaFunc(GL_GEQUAL,THRESHOLD);//フラグメントのアルファがTHRESHOLDと同じ値以上でなければフラグメントを受け入れる
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
	//The depth test is set to GL_LESS for back-to-front
	glDepthFunc(GL_LESS);//z値がデプスバッファに既に保存されている値よりも小さい場合に、入力されるフラグメントがデプステストに合格する
	

	glEnable(GL_CULL_FACE);
	Cg->Enable();
	glColor3f((float)this->x,(float)this->y,(float)this->z);
	Cg->BindProgram(Cg->vertexProgram);//cgGLBindProgram(program);をしてる。
	Cg->BindProgram(Cg->fragment_occlusionProgram);//Textured with frame buffer

	//立方体の各面を描画　メインのレンダリングプリミティブ
	for (int i = 0; i < NoF; i++) {       //立方体の面 (6個) 　　　　
		glBegin(GL_POLYGON);   //塗潰し多角形の描画開始  LINE_LOOPにすると変になる。これがCgと連動してる部分かも。
		for (int j = 0; j < NoFV; j++)
		{
			glVertex3fv(texcoord[faces[NoFV*i + j]]);
			}
		glEnd();                            //塗潰し多角形の描画終了
	}
/*頂点の正体。ブロックインデックスぽい頂点群をCgに渡している。計算を楽にするためと思われる。これはブロック一個一個の頂点だ。
//（100,110,111,101）,(110,010,011,111),(001,101,111,011),(000,001,011,010),(000,100,101,001),(000,010,110,100)*/
	glDisable(GL_CULL_FACE);
	//汚いのが見えないように、スライス面を上からかぶせて描いてるのかな
	//プログラムをスライス用に切り替え
	Cg->BindProgram(Cg->vertex_sliceProgram);

	//頂点配列有効 レイキャスティング+ブロック化による不自然なクリッピング面問題を解決するための部分
	glBegin(GL_POLYGON);
		glVertex2f(0.0,0.0);
		glVertex2f(1.0,0.0);
		glVertex2f(2.0,0.0);
		glVertex2f(3.0,0.0);
		glVertex2f(4.0,0.0);
		glVertex2f(5.0,0.0);
	glEnd();

	glFlush();
	Cg->Disable();

	//disable alpha test
	glDisable(GL_ALPHA_TEST);
	//continue integration
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glDepthMask(GL_FALSE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
}


/*!
@brief ここがレンダリングのコアだったかな。
*/
void Block::RayCast(Cg* Cg,int dif)//使ってる
{

	float bn[3] = {(float)this->bnumx,(float)this->bnumy,(float)this->bnumz};
	
	Cg->SetParameter(Cg->BNUMParam,bn);
	Cg->SetParameter(Cg->BNUM2Param,bn);

	float rscl = 1.0f/pow(2.0f,(NUMLEVEL-1.0f)-this->level);

	Cg->SetParameter(Cg->rsclParam,rscl);

	float resocolor[4];

	float renderAlpha;
	float renderBeta[3];

	
	float stepsize  = STEPSIZE;
	float rayLoop = RAYLOOP;

	float ptd = pow(2.0f,dif);
	renderAlpha   = 1.0f/ptd;
	renderBeta[0] = (this->x%(int)ptd);
	renderBeta[1] = (this->y%(int)ptd);
	renderBeta[2] = (this->z%(int)ptd);
	memcpy(resocolor,CLR[this->level+dif],sizeof(float)*4);

	stepsize *= ptd;//ptd=2のdif乗 stepsizeは、0.005か、0.01のどちらか。
	
	rayLoop  /= ptd;//rayLoopは800か400

	if(Block::resocolorflag == false)
	{
		resocolor[0] = 1.0;
		resocolor[1] = 1.0;
		resocolor[2] = 1.0;
		resocolor[3] = 1.0;
	}	
	
	Cg->SetParameter(Cg->rayLoopParam,rayLoop);
	Cg->SetParameter(Cg->stepsizeParam,stepsize);
	Cg->SetParameter(Cg->renderAlphaParam,renderAlpha);
	Cg->SetParameter(Cg->renderBetaParam,renderBeta);
	Cg->SetParameter(Cg->resoColorParam,resocolor,4);
	
	earlyRayTermination(Cg);//これがないと、オクルージョンカリングがうまく働かないっぽい 見た目も変になる

	glEnable(GL_CULL_FACE);//片面表示効果を有効にする
	Cg->Enable();
	//cgGLEnableProfile(Cg->vertexProfile);
	glColor3f((float)this->x,(float)this->y,(float)this->z);//最大0〜32でブロックの位置を表している。
	
	Cg->BindProgram(Cg->vertexProgram);
	Cg->BindProgram(Cg->fragmentProgram);
	for (int i = 0; i < NoF; i++) {       //立方体の面 (6個) 　　　　
		glBegin(GL_POLYGON);                //塗潰し多角形の描画開始 
		for (int j = 0; j < NoFV; j++)
		{
			glVertex3fv(texcoord[faces[NoFV*i + j]]);
		}
		glEnd();                            //塗潰し多角形の描画終了
	}

	glDisable(GL_CULL_FACE);
	////プログラムをスライス用に切り替え
	Cg->BindProgram(Cg->vertex_sliceProgram);
	//頂点配列有効
	glBegin(GL_POLYGON);
		glVertex2f(0.0,0.0);
		glVertex2f(1.0,0.0);
		glVertex2f(2.0,0.0);
		glVertex2f(3.0,0.0);
		glVertex2f(4.0,0.0);
		glVertex2f(5.0,0.0);
	glEnd();


	Cg->Disable();
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

}
//もしアルファ値がthresholdよりも小さいフラグメントが存在するならtrue,存在しないならflaseが変える
bool Block::testOcclusion(Cg* _cg)
{
	
	GLuint query;
	GLint sampleCount;
	GLint available;
	
	
	float bn[3] = {(float)this->bnumx,(float)this->bnumy,(float)this->bnumz};//１辺あたりのブロックの数２〜３２
	_cg->SetParameter(_cg->BNUMParam,bn);
	_cg->SetParameter(_cg->BNUM2Param,bn);


	glGenQueriesARB(1,&query);

//フレームバッファへの書き込みを無効化し転送率を抑える
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glDepthMask(GL_FALSE);

	glBeginQueryARB(GL_SAMPLES_PASSED_ARB,query);//render the geometry for the occlusion test
	//フレームバッファのアルファ値がthresholdよりも小さいときのみ採用
	glAlphaFunc(GL_LESS,THRESHOLD);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
	_cg->Enable();
	glColor3f((float)this->x,(float)this->y,(float)this->z);
	_cg->BindProgram(_cg->vertexProgram);
	_cg->BindProgram(_cg->fragment_occlusionProgram);//前フレームの結果を使うってことか？
//バウンディングボックス描画？

	for (int i = 0; i < NoF; i++) {       //立方体の面 (6個) 　　　　
		glBegin(GL_POLYGON);                //塗潰し多角形の描画開始 
		for (int j = 0; j < NoFV; j++)
		{
			glVertex3fv(texcoord[faces[NoFV*i + j]]);
		}
		glEnd();                            //塗潰し多角形の描画終了
	}

	
	glDisable(GL_CULL_FACE);
	////プログラムをスライス用に切り替え
	_cg->BindProgram(_cg->vertex_sliceProgram);

	//頂点配列有効 スライス？
	glBegin(GL_POLYGON);
		glVertex2f(0.0,0.0);
		glVertex2f(1.0,0.0);
		glVertex2f(2.0,0.0);
		glVertex2f(3.0,0.0);
		glVertex2f(4.0,0.0);
		glVertex2f(5.0,0.0);
	glEnd();

	_cg->Disable();
	

	glFlush();
	//specify that you completed the occlusion query
	
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	glDisable(GL_ALPHA_TEST);

	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glDepthMask(GL_TRUE);

	
	//判定結果を待つ
	do{glGetQueryObjectivARB(query,GL_QUERY_RESULT_AVAILABLE,&available);//retrieve the occlusion results(BeginQuery()-endquery()でやったテストの結果をretrieve)
	}while(!available);
	glGetQueryObjectivARB(query,GL_QUERY_RESULT,&sampleCount);



//もしアルファ値がthresholdよりも小さいフラグメントが存在するなら
	if(sampleCount > 0)
		return true;//フレームバッファへの書き込みを有効にする
	else
		return false;
	
}


Block Block::getLowblock()//解像度をLowにする
{
	Block lowblock;
	if(this->level != NUMLEVEL-1)
		lowblock.setBlock(this->x/2,this->y/2,this->z/2,this->level+1,this->bnumx/2,this->bnumy/2,this->bnumz/2);
	else
		lowblock.setBlock(this->x,this->y,this->z,this->level,this->bnumx,this->bnumy,this->bnumz);

	return lowblock;
}

Block Block::getMultiLowblock(int stage)
{//stageは０から４の値をとる。stageは詳細度レベルを表す。
	Block lowblock = this->getThisBlock(); 


	for(int i= 0;i < stage;i++){//繰り返しは多くてせいぜい3回のようだ。if(i>1)printf("繰り返し%d回目\n",i);
		lowblock = lowblock.getLowblock();}	

	return lowblock;
}


Block Block::getHighblock()
{
	Block highblock;
	if(this->level != 0)
		highblock.setBlock(this->x*2,this->y*2,this->z*2,this->level-1,this->bnumx*2,this->bnumy*2,this->bnumz*2);
	else
		highblock.setBlock(this->x,this->y,this->z,this->level,this->bnumx,this->bnumy,this->bnumz);

	return highblock;
}
Block Block::GetRootBlock()//一解像度の粗いブロック
{
	Block rootblock(0,0,0,NUMLEVEL-1,INIBLX,INIBLY,INIBLZ);

	return rootblock;
}

void Block::setBlockState(blockState state,unsigned int** renderblock )
{renderblock[this->level][(this->x*this->bnumy+this->y)*this->bnumz+this->z] = state;
}



vec3<double> Block::getBlockVec(const mat4<double>& _modelViewMatrix)
{
	vec4<double> blockpos;
	
	float offx,offy,offz;
	if(this->bnumx == 1)
		offx = 1.0f;
	else
		offx = 2.0f/this->bnumx;
	if(this->bnumy == 1)
		offy = 1.0f;
	else
		offy = 2.0f/this->bnumy;

	if(this->bnumz == 1)
		offz = 1.0f;
	else
		offz = 2.0f/this->bnumz;
		
	blockpos.x = (2.0f*this->x-this->bnumx)*Block::brX*Block::iniX/this->bnumx+offx*Block::brX*Block::iniX;
	blockpos.y = (2.0f*this->y-this->bnumy)*Block::brY*Block::iniY/this->bnumy+offy*Block::brY*Block::iniY;
	blockpos.z = (2.0f*this->z-this->bnumz)*Block::brZ*Block::iniZ/this->bnumz+offz*Block::brZ*Block::iniZ;
	blockpos.w = 1.0;


	blockpos = _modelViewMatrix*blockpos;

	vec3<double> blockvec;

	blockvec = - blockpos.toVec3();
	


	//正規化
	blockvec.normalize();

	return blockvec;//root blockならviewvecになる。
}

Block::blockState Block::getLowState(Block lowblock){

	int dif =  lowblock.level - this->level;
	Block::blockState bstate;

	if(dif == 0)
		bstate = Block::renderpreblock;
	else if(dif == 1)
		bstate = Block::renderfirstlowblock;
	else if(dif == 2)
		bstate = Block::rendersecondlowblock;
	else if(dif == 3)
		bstate = Block::renderthirdlowblock;
	else
		bstate = Block::renderfourthlowblock;


	return bstate;
}
bool Block::isSameBlock(Block testBlock){
	if(this->level!=testBlock.level){return false;}
	else if(this->x!=testBlock.x){return false;}
	else if(this->y!=testBlock.y){return false;}
	else if(this->z!=testBlock.z){return false;}
	else{return true;}
}
/*!
@brief frustum<T>の汎用性を保持したかったのでBlockのほうに判定を作った。
*/
int Block::IsInFrustum(const frustum<double>& _frustum){
	//もしビューボリュームにblockが入っていれば
        vec3<double> corner;//cornerの成分は0か-1だ。
        corner.x = (2.0f*x-bnumx)*Block::brX*Block::iniX/bnumx;
        corner.y = (2.0f*y-bnumy)*Block::brY*Block::iniY/bnumy;
        corner.z = (2.0f*z-bnumz)*Block::brZ*Block::iniZ/bnumz;
 
        aabox<double> abox(corner,2.0f*Block::brX*Block::iniX/bnumx,2.0f*Block::brY*Block::iniY/bnumy,2.0f*Block::brZ*Block::iniZ/bnumz);//ブロックの情報をAxis Aligned Boxで表現している。AABoxのx,y,zはブロックの辺の長さ。
        return _frustum.boxInFrustum(abox);
}
void Block::renderNumber(int _num,float r,float g,float b){
	void* fontType= GLUT_BITMAP_HELVETICA_18;
	char vertNumber[10]={'0','1','2','3','4','5','6','7','8','9'};
	vec4<float> blockpos;
	vec3<float> offset=vec3<float>(1.0,1.0,1.0);
	blockpos.x = (2.0f*this->x-this->bnumx)*Block::brX*Block::iniX/this->bnumx+Block::brX*Block::iniX -offset.x;
	blockpos.y = (2.0f*this->y-this->bnumy)*Block::brY*Block::iniY/this->bnumy+Block::brY*Block::iniY -offset.y;
	blockpos.z = (2.0f*this->z-this->bnumz)*Block::brZ*Block::iniZ/this->bnumz+Block::brZ*Block::iniZ -offset.z+0.5;
	blockpos.w = 1.0;
	vec3<float> _position=blockpos.toVec3();//this->blockCenterPoint;
	if(_num<10){
		glColor4f(r,g,b,1.0f);
		//	glColor4f(1.0f-r,1.0f-g,1.0f-b,1.0f);
		_position.glRasterPos();
		glutBitmapCharacter(fontType,vertNumber[_num]);
	}else{//10以上
		if(_num<100){//10-99
			int digit=(int)_num/10;
			glColor4f(r,g,b,1.0f);

			_position=_position-vec3<float>(0.1,0.0,0.0);
			_position.glRasterPos();
			glutBitmapCharacter(fontType,vertNumber[digit]);

			glColor4f(1.0f-r,1.0f-g,1.0f-b,1.0f);
			_num=_num-digit*10;
			_position=_position+vec3<float>(0.1,0.0,0.0);
			_position.glRasterPos();

			glutBitmapCharacter(fontType,vertNumber[_num]);
		}else{//100以上
			int hundred=(int)_num/100;
			glColor4f(r,g,b,1.0f);

			_position=_position-vec3<float>(0.2,0.0,0.0);
			_position.glRasterPos();
			glutBitmapCharacter(fontType,vertNumber[hundred]);

			int digit=(int)(_num-hundred*100)/10;
			glColor4f(r,g,b,1.0f);

			_position=_position+vec3<float>(0.1,0.0,0.0);
			_position.glRasterPos();
			glutBitmapCharacter(fontType,vertNumber[digit]);

			glColor4f(1.0f-r,1.0f-g,1.0f-b,1.0f);
			_num=_num-hundred*100-digit*10;
			_position=_position+vec3<float>(0.1,0.0,0.0);
			_position.glRasterPos();
			glutBitmapCharacter(fontType,vertNumber[_num]);

		}
	}
}
void Block::Info(const char* message,float _num){
	printf("%s[%.6f]:",message,_num);
	printf("level[%d]xyz[%d][%d][%d]\n",this->level,this->x,this->y,this->z);
}
void Block::renderBlockLines(float r,float g,float b,float alpha){
	vec3<float> *blockVertices=this->convertTexToLocal();
	glColor4f(r,g,b,alpha);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
	blockVertices[0].glVertex();blockVertices[1].glVertex();blockVertices[2].glVertex();blockVertices[3].glVertex();
	blockVertices[7].glVertex();blockVertices[6].glVertex();blockVertices[5].glVertex();blockVertices[4].glVertex();
	glEnd();
	glBegin(GL_LINES);
	blockVertices[0].glVertex();blockVertices[3].glVertex();
	blockVertices[7].glVertex();blockVertices[4].glVertex();
	blockVertices[6].glVertex();blockVertices[2].glVertex();
	blockVertices[5].glVertex();blockVertices[1].glVertex();
	glEnd();

}
vec3<float>* Block::convertTexToLocal(){
	vec3<float> result[8];
	vec3<float> texCoordVec[8] =                // 立方体の頂点 (8個) 　　　
{vec3<float>(0.0, 0.0, 0.0), vec3<float>(1.0, 0.0, 0.0), vec3<float>(1.0, 1.0, 0.0), vec3<float>(0.0, 1.0, 0.0),
vec3<float>(0.0, 0.0, 1.0),  vec3<float>(1.0, 0.0, 1.0),  vec3<float>(1.0, 1.0, 1.0),  vec3<float>(0.0, 1.0, 1.0)};


	vec3<float> offsetVec=vec3<float>(this->x,this->y,this->z);
	vec3<float> pushCenter=vec3<float>(0.0,0.0,0.5);
	offsetVec=(offsetVec*2.0f-this->bnumx)/(float)this->bnumx;
	float scale = 2.0f/this->bnumx;
	for(int i=0;i<8;i++){
		result[i]=texCoordVec[i]*scale+offsetVec+pushCenter;
	}
	return result;

}
