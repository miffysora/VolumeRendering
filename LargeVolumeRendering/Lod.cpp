#include "Lod.h"
void* font=GLUT_BITMAP_HELVETICA_10;
static const int MAXFPSPIXEL =300;
static const float  RENDERMAX =1000.0f;//
static const unsigned short int howManyVisibleBlock[NUMLEVEL]={2596,457,92,23,4};//各レベルに,empty-spaceじゃない、レンダリングする価値のあるブロックが何個あるのかという事前情報（計算した）

CLod::CLod(int _w,int _h)
  :m_Bclipresio(INITFAR / INITNEAR)
	, normalnbn(0)//描画に必要な現解像度ブロックの数
	, normalebn( 0)//既に存在している現解像度ブロックの数
	, backnbn(0)
	, backebn(0)
	, idealBlockNum(0)//最適解像度ブロックの数
	,realResoNumInRender(0)
	, backupOrNot(false)
	, renderBlockNum(0)//実際にレンダリングしているブロックの数
	,render_normal_num(0)
	, bltime_sum(0)//1フレームごとの足し算
	, blcount(0)//1フレームごと
	,NFrustumSize(INITNFRUSTUMSIZE)
	,frame_tex_pro(0)
	,tex_req_frame(0)
	,totalrendertime_sum(0)
	,loadtexblockcount(0)
	,angleDelta(0.0)
	,miffytime(0)
	,timebase(0)
	,order(0)
	,nowprocessingflag(false)
	,frustumstate( frustum<double>::render)
	,showGridFlag(false)
	,axisX(0.0)
	,axisY(0.0)
	,axisZ(0.0)
	,raycasttime_lastframe(0)
	,ini_cam(vec4<double>(0.0f,0.0f,0.0f))
	,ini_l(vec3<double>(0.0f,0.0f,-1.0f))
	,ini_u(vec3<double>(0.0f,1.0f, 0.0f))
	,loadtime_lastframe(0)
	,FPSMAX(60)
	,render_loadTimeLimit(0)
	,renderTimeLimit(RENDERMAX)
	,practicalnum(0)
	,renderEachBlockTime(0.5)//TF Shadeありなら0.5msec
	,realResolutionNum(0)
	,realResolutionNum_perFrame(0)
	,realResolutionNumCount(0)
	,renderblockcount(0)
	,practicalRenderBlockNum(32)
	,backupNum(0)
	
{
	m_WinSize.set(_w,_h);
	
	m_wBasicCamPos.set(0.0,0.0,9.0);
}
/// @name ライティング関係
	///@{
	float globalAmbient[3] = {0.5f, 0.5f, 0.5f};
	vec4<double> lightAPosition(0.0, 0.0, 20.0, 1.0);
	vec4<double>  lightPosition;
	float lightColor[3] = {1.0f, 1.0f, 1.0f};

	float Ke[3] = {0.0f, 0.0f, 0.0f};
	float Ka[3] = {0.2f, 0.2f, 0.2f};
	float Kd[3] = {0.7f, 0.7f, 0.7f};
	float Ks[3] = {0.9f, 0.9f, 0.9f};
	float shininess = 50.0f;
	///@}
	void resetPerspectiveProjection() {
	// set the current matrix to GL_PROJECTION
	glMatrixMode(GL_PROJECTION);
	// restore previous settings
	glPopMatrix();
	// get back to GL_MODELVIEW matrix
	glMatrixMode(GL_MODELVIEW);
}
void renderBitmapString(float x, float y, void *font,char *string)//画面に文字を出力
{

	char *c;
	// set position to start drawing fonts
	glRasterPos2f(x, y);
	// loop all the characters in the string
	for (c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}
static void check_framebuffer_status() 
{ 
	GLenum status; 
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
	switch(status) { 
  case GL_FRAMEBUFFER_COMPLETE_EXT: 
	  break; 
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT: 
	  fprintf(stderr, "choose different formats\n"); 
	  break; 
  default: 
	  fprintf(stderr, "programming error; will fail on all hardware: %04x\n", status); 
	} 
}
/* 立方根を求める */
static double GetCbRoot(double x)
{
    double stv;
	 if (x >= 0)        /* x が正なら平方根を返す */
        stv = (sqrt(x));
    if (x < 0)      /* x が負なら絶対値の平方根 */
        stv = (-sqrt(-x));    /* に-を付けて返す */


	double dx = stv / 10;
	
    if (x == 0.0)        /* x が０なら平方根も０ */
        return (0.0);

    if (x < stv * stv * stv)
        dx *= -1.0;

    while (1) {    /* ２重の無限ループ */
        while (1) {
            if (( dx > 0 && (x < stv * stv * stv)) || 
                (dx < 0 && (x > stv * stv * stv))) 
                              /* 立方根と近似値の大小関係が変化したら */
                break;        /* 内側の無限ループから抜ける */
            else
                stv += dx;
        }
        if (fabs(dx) < 0.00000001)
                      /* 小数点以下８桁まで精度が出たら */
            break;    /* 外側の無限ループから抜ける */
        else          /* まだならここ */
            dx *= -0.1;
    }
    return (stv);
}
static void setOrthographicProjection(int _winwidth,int _winheght) {

	// switch to projection mode
	glMatrixMode(GL_PROJECTION);
	// save previous matrix which contains the 
	//settings for the perspective projection
	glPushMatrix();
	// reset matrix
	glLoadIdentity();
	// set a 2D orthographic projection
	gluOrtho2D(0, _winwidth, 0, _winheght);
	// invert the y axis, down is positive
	glScalef(1, -1, 1);
	// mover the origin from the bottom left corner
	// to the upper left corner
	glTranslatef(0, -(GLfloat)_winheght, 0);
	glMatrixMode(GL_MODELVIEW);
}

void CLod::Idle(){
	
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix.m);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(m_Translate.x,m_Translate.y,m_Translate.z);     /* 平行移動(奥行き方向) 　　*/
	glMultMatrixd(modelMatrix.m);
	glRotated(angleDelta, axisX, axisY, axisZ);   /* 回転　　　　　　*/
	
	eyeDis -= m_Translate.z;
	SetClippingPlane();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();                       //Displayイベント発生 
}

CLod::~CLod(void)
{
}
vec3<double> CLod::getViewvector(vec3<double>* view_z){

	*view_z = m_InvModelView*(*view_z);
	
	if(view_z->x == 0.0f)
		view_z->x = 0.0001f;
	if(view_z->y == 0.0f)
		view_z->y = 0.0001f;
	if(view_z->z == 0.0f)
		view_z->z = 0.0001f;



	//front_to_back だからベクトル逆にしている
	view_z->x = -view_z->x;
	view_z->y = -view_z->y;
	view_z->z = -view_z->z;


	vec3<double> abview;

	abview.x = 1.0f/(abs(view_z->x)*SPX);
	abview.y = 1.0f/(abs(view_z->y)*SPY);
	abview.z = 1.0f/(abs(view_z->z)*SPZ);

	abview.normalize();
	return abview;
}
/*!
@brief 読込み視錐台用 1フレーム先回りして読み込み要求発行。
*/
void CLod::testLoadFrustum(Block block,int* octreeReso,int* existblcount,unsigned int** renderblock,bool **** needCountflag,bool **** existCountflag, list<Block>* blockQue)
{
	
	int id = m_File.getIndexblock(block);//idは0か1の値 データが存在してたら１なんだと思うが。
	block.setBlockState(Block::notexist,renderblock);//配列renderblockにブロックステートを記録
	if(id != 0)//blockにデータが入っているか判定
	{
		if(block.IsInFrustum(m_localNextFrustum) != frustum<double>::OUTSIDE)//このブロックが視錐台の内側だったら。
		{
			if(block.resoChange(modelMatrix,projMatrix,m_WinSize,0))
			{//
				m_File.blockPermitReplaceInMain(block);
				ProcessNextResoLoad(block,octreeReso,existblcount,renderblock,needCountflag,existCountflag,blockQue);
			}//↑ここでちょっと値を変えてまたtestLoadFrustumが呼び出される。blockstateは、nextresoに設定される。実際に1段階高解像度のノードの探索。
			else
			{//最適解像度ブロックになったら
				//何段階か上	
				
				Block mlowblock = block.getMultiLowblock(*octreeReso);//現解像度ブロックに対して1段階低解像度なブロックを予めバックアップとして主メモリに読み込んでおく。描画に必要な現解像度ブロックが主メモリに読み込まれていない場合には、このブロックを代わりに用いることで、結果が欠けてしまう事態を防ぐ。
				//octreeResoは詳細度レベルを表す。関数の引数の時点でoctreeResoはポインタだからintの値に直すために*をつけてる。
				//mlowblockはバックアップブロックということになる？
				m_File.updateNeedScoreInMain(Next_modelViewMatrix.m,Next_projMatrix.m,mlowblock,m_localNextFrustum, frustum<double>::normal);
				
				if(!needCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z])
				{
					blockQue->push_back(mlowblock);//mlowblockと同じ値の要素を両端キューの末尾に追加する。
					needCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z] = true;
				}

				if(m_File.blockExist(mlowblock))//main memory exist or not
				{
					Block::blockState bstate = block.getLowState(mlowblock);//blockとmlowblockのレベルの差によって決まる
					block.setBlockState(bstate,renderblock);
					
					if(!existCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z])
					{
						*existblcount += 1;
						existCountflag[mlowblock.level][mlowblock.x][mlowblock.y][mlowblock.z] = true;
					}
					if(m_File.texBlockExist(mlowblock)){//もうテクスチャメモリへ転送済なら上書き可能にしてもよい
						m_File.blockPermitReplaceInMain(block);
					}else{//まだ転送し終わってないのなら上書き禁止
						m_File.blockProhibitReplaceInMain(block);//メインメモリで上書き禁止
					}
				}
				else{
					block.setBlockState(Block::waitblock,renderblock);}
			}
		}//視錐台の外側だったら。
		else{
			m_File.blockPermitReplaceInMain(block);
			block.setBlockState(Block::frameout,renderblock);}
	}
}

void CLod::SetNextFrustum()
{
	float next_eyeDis = eyeDis;
	next_eyeDis -= m_Translate.z;
	float Next_nearSlice = next_eyeDis / ( 1.0f + Bclipeyeresio*( m_Bclipresio -1.0f));
	float Next_farSlice = Next_nearSlice * m_Bclipresio;
	

	glMatrixMode(GL_PROJECTION);               /* 投影変換の設定            */
	glPushMatrix();
	glLoadIdentity(); 
	gluPerspective(m_FovY, aspect, Next_nearSlice, Next_farSlice);
	
	float N_a = GetCbRoot(NFrustumSize);//立方根を求める
	float N_m = Next_nearSlice*(1.0f + m_Bclipresio)/2.0f;
	float N_l = Next_nearSlice*(m_Bclipresio - 1.0f)*N_a/2.0f;
	float next_fovy = atan(Next_nearSlice*N_a*tan(m_FovY*M_PI/180.0f)/(N_m - N_l))*180.0f/M_PI;//画角を導き出した
	glGetDoublev(GL_PROJECTION_MATRIX, Next_projMatrix.m);
	m_localNextFrustum.setFromPerspective(Next_projMatrix.m,next_fovy,aspect,N_m - N_l,N_m + N_l);	
	
}
void  CLod::SetRenderFrustum()
{
	glMatrixMode(GL_PROJECTION);               /* 投影変換の設定            */
	glLoadIdentity(); 
	dPlaneStart = eyeDis - nearSlice;     //スライス開始位置
	cg.SetParameter(cg.dPlaneStartParam , dPlaneStart);
	cg.SetParameter(cg.farSliceParam,farSlice); 
	gluPerspective(m_FovY, aspect, nearSlice, farSlice);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix.m);	
	m_localRenderFrustum.setFromPerspective(projMatrix.m,m_FovY,aspect,nearSlice,farSlice);
	glMatrixMode(GL_MODELVIEW);

}
/*!Block blを与えられ、詳細度レベル一個上のを作ってる。bl一個につき一つ高解像度のやつを８個作ってる。
*/
void CLod::ProcessNextResoLoad(Block bl,int* countA,int* countB,unsigned int** rblock,bool **** needCountflag,bool **** existCountflag,list<Block>* blockQue)//NExt->次のより高解像度のやつって意味。
{	for(int i=0;i<2;i++)//x loop
	{
		for(int j=0;j<2;j++)
		{
			for(int k=0;k<2;k++)
			{//countAは0〜4の値。詳細度レベルを示していると思われる。
				testLoadFrustum(Block(2*bl.x+i,2*bl.y+j,2*bl.z+k,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2),countA,countB,rblock,needCountflag,existCountflag,blockQue);
			}//Block(2*bl.x+i,2*bl.y+j,2*bl.z+k,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2).getMultiLowblock(countA)という処理をブロックがblockQueに登録される。
		}
	}//x loop
	bl.setBlockState(Block::nextreso,rblock);
}
void CLod::DDAblockRender(vec3<double>& _viewvec, vec3<double>& _abview,Block bl,int* normalreso,int* existblcount,list<Block>* _blockQue,int _levelLimit)
{// abviewとviewvecは対の関係みたいになってる。
	int DDAb[3];
	if(bl.level == NUMLEVEL-1)
	{
		DDAb[0] = INIBLX;//2
		DDAb[1] = INIBLY;//2
		DDAb[2] = INIBLZ;//2
	}
	else
	{
		DDAb[0] = 2;
		DDAb[1] = 2;
		DDAb[2] = 2;
	}

	int deltal;
	int deltam;                  
	int deltas;                  
	int num;
	int llength;
	int mlength;
	int slength;
	//_abview.print("_abview");//_abviewの成分は0か1
	if(_abview.x >= _abview.y && _abview.y >= _abview.z)
	{
		deltam = (int)(DDAb[2]*_abview.y/_abview.z);
		deltas =  DDAb[2];
		deltal = (int)(DDAb[2]*_abview.x/_abview.z);
		llength = DDAb[0];
		mlength = DDAb[1];
		slength = DDAb[2];
		num = 0;
	}
	else if(_abview.x >= _abview.z && _abview.z >= _abview.y)
	{

		deltam = (int)(DDAb[1]*_abview.z/_abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*_abview.x/_abview.y);
		llength = DDAb[0];
		mlength = DDAb[2];
		slength = DDAb[1];
		num = 1;

	}
	else if(_abview.y >= _abview.z && _abview.z >= _abview.x)
	{


		deltam = (int)(DDAb[0]*_abview.z/_abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*_abview.y/_abview.x);
		llength = DDAb[1];
		mlength = DDAb[2];
		slength = DDAb[0];
		num = 2;
	}
	else if(_abview.y >= _abview.x && _abview.x >= _abview.z)
	{

		deltam = (int)(DDAb[2]*_abview.x/_abview.z);
		deltas = DDAb[2];
		deltal = (int)(DDAb[2]*_abview.y/_abview.z);
		llength = DDAb[1];
		mlength = DDAb[0];
		slength = DDAb[2];
		num = 3;
	}
	else if(_abview.z >= _abview.x && _abview.x >= _abview.y)
	{

		deltam = (int)(DDAb[1]*_abview.x/_abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*_abview.z/_abview.y);
		llength = DDAb[2];
		mlength = DDAb[0];
		slength = DDAb[1];
		num = 4;
	}
	else
	{

		deltam = (int)(DDAb[0]*_abview.y/_abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*_abview.z/_abview.x);
		llength = DDAb[2];
		mlength = DDAb[1];
		slength = DDAb[0];
		num =5;
	}




	int a;
	int* x;
	int* y;
	int* z;
	int i,j;
	int count = 0;
	double temp_sum= 0.0;

	
	int sg[3];
	int vol[3];
	int vec_num;
	//_viewvec.print("_viewvec");//_viewvecの成分は、0か1か-1 8通りの組み合わせ。
	if(_viewvec.x >= 0.0 && _viewvec.y >= 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=1;
		vol[0]=0;vol[1]=0;vol[2]=0;
		vec_num = 0;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y < 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 1;
	}
	else if(_viewvec.x >= 0.0 && _viewvec.y >= 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=-1;
		vol[0]=0;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 2;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y < 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 3;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y >= 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 4;
	}
	else if(_viewvec.x >= 0.0 && _viewvec.y < 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=-1;sg[2]=1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 5;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y >= 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=0;
		vec_num = 6;
	}
	else
	{
		sg[0]=1;sg[1]=-1;sg[2]=-1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 7;
	}

	if(deltam*llength < deltal && deltam <= deltas*llength && deltas <= deltam )
	{
		int end_s = 0;
		int sum = -deltam;
		int *m_value;
		m_value = new int[slength];
		//初期化
		for(i=0;i<slength;i++){
			m_value[i] = 0;
		}
		int d = deltas;
		int start_s = 0;



		switch(num) 
		{
		case 0:
			x = &j;y = &a;z = &i;
			break;
		case 1:
			x = &j;y = &i;z = &a;
			break;
		case 2:
			x = &i;y = &j;z = &a;
			break;
		case 3:
			x = &a;y = &j;z = &i;
			break;
		case 4:
			x = &a;y = &i;z = &j;
			break;
		default:
			x = &i;y = &a;z = &j;
			break;
		}

//最も離れているボクセル面をPに入れる
		do{
			for(i = start_s ;i<=end_s;i++){//P内の全各ボクセル面index
				int &vl = m_value[i];
				a = vl;
				for(j = 0;j < llength ; j++)//s[index]内の全各ボクセル列
				{
					Block block(vol[0]+sg[0]**x+bl.x,vol[1]+sg[1]**y+bl.y,vol[2]+sg[2]**z+bl.z,bl.level,bl.bnumx,bl.bnumy,bl.bnumz);
					testRenderFrustum(block,normalreso,existblcount,_blockQue,_levelLimit);
					//視点から最も離れている未描画ボクセル１つを描画する。
				}
				vl++;
				if(vl == mlength)
					start_s++;
			}	
			sum += d;//△y/△z
			if(sum >0 && end_s<slength-1){//Pの先頭ボクセル面内の全ボクセルが描画済み。
				end_s++;
				sum -= deltam;//先頭ボクセル面をPから外す
			}
		}while(start_s <= slength-1);	//Pが空ではない。

		delete []m_value;



	}
	else
	{   


		int ms = deltam*deltas;//△y/△z
		int ls = deltal*deltas;
		int lm = deltal*deltam;


		int end_s = 0;
		int sum_s = -lm;//
		int start_s = 0;

		int *end_m;
		end_m = new int[slength];	
		int** l_value;
		l_value = new int*[slength];


		int* sum_m;
		sum_m = new int[slength];
		int* start_m;
		start_m = new int[slength];




		for(i = 0;i<slength;i++)
		{
			start_m[i] = 0;
			sum_m[i] = -ls;
			end_m[i] = 0;

			l_value[i] = new int [mlength];

			for(j = 0;j < mlength;j++)
				l_value[i][j] = 0;	
		}


		switch(num) 
		{
		case 0:
			x = &a;y = &j;z = &i;
			break;
		case 1:
			x = &a;y = &i;z = &j;
			break;
		case 2:
			x = &i;y = &a;z = &j;
			break;
		case 3:
			x = &j;y = &a;z = &i;
			break;
		case 4:
			x = &j;y = &i;z = &a;
			break;
		default:
			x = &i;y = &j;z = &a;
			break;
		}
//最も離れているボクセル面をPに入れる
		do
		{
			for(i = start_s;i<= end_s;i++)//P内の全各ボクセル面index
			{   
				for(j = start_m[i];j <= end_m[i]; j++)//s[index]内の全各ボクセル列
				{	
					int &vl = l_value[i][j];
					a = vl;
					Block block(vol[0]+sg[0]**x+bl.x,vol[1]+sg[1]**y+bl.y,vol[2]+sg[2]**z+bl.z,bl.level,bl.bnumx,bl.bnumy,bl.bnumz);
					testRenderFrustum(block,normalreso,existblcount,_blockQue,_levelLimit);
				//もっとも遠いボクセルを一つ描画する。
					vl++;
				}
				sum_m[i] += ms;//ds[index]+=△y/△z
				if(sum_m[i] >0 && end_m[i]<mlength-1){//ds[index]>=1.0
					end_m[i]++;//視点から最も離れている未描画のボクセル列をS[index]の最後に加える。
					sum_m[i] -= ls;//ds[index]-=1.0;
				}
				if(l_value[i][start_m[i]] == llength )//S[index]の先頭ボクセル列内の全ボクセルが描画済み。
					start_m[i]++;//先頭ボクセル列をS[index]から外す。
			}	
			sum_s += ms;//dp+=△x/△z
			if(sum_s >0 && end_s<slength-1){
				end_s++;
				sum_s -= lm;
				sum_m[end_s] = sum_s;
			}
			if(start_m[start_s] >  mlength-1)//Pの先頭ボクセル面内の全ボクセルが描画済み。
				start_s++;//先頭ボクセル面をPから外す
		}while(start_s <= slength - 1);	//Pが空ではない。

		delete []end_m;
		for(i=0;i < slength;i++)
			delete []l_value[i];
		delete []l_value;

		delete []sum_m;
		delete []start_m;

	}


	//	cout <<"vec_num"<<vec_num<<endl;
	//	cout <<"num"<<num<<endl;

}
void CLod::DDAblockFile(vec3<double> _viewvec, vec3<double> _abview,Block _bl,int* _reso,int* _exist_in_main_count,unsigned int** _blockstate,bool ****needCountMap,bool **** existInMainMap,list<Block>* blockQue)
{// abviewとviewvecは対の関係みたいになってる。
	int DDAb[3];
	if(_bl.level == NUMLEVEL-1)
	{
		DDAb[0] = INIBLX;//2
		DDAb[1] = INIBLY;//2
		DDAb[2] = INIBLZ;//2
	}
	else
	{
		DDAb[0] = 2;
		DDAb[1] = 2;
		DDAb[2] = 2;
	}

	int deltal;
	int deltam;                  
	int deltas;                  
	int num;
	int llength;
	int mlength;
	int slength;
	//abview.print("abview");//abviewの成分は0か1
	if(_abview.x >= _abview.y && _abview.y >= _abview.z)
	{
		deltam = (int)(DDAb[2]*_abview.y/_abview.z);
		deltas =  DDAb[2];
		deltal = (int)(DDAb[2]*_abview.x/_abview.z);
		llength = DDAb[0];
		mlength = DDAb[1];
		slength = DDAb[2];
		num = 0;
	}
	else if(_abview.x >= _abview.z && _abview.z >= _abview.y)
	{

		deltam = (int)(DDAb[1]*_abview.z/_abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*_abview.x/_abview.y);
		llength = DDAb[0];
		mlength = DDAb[2];
		slength = DDAb[1];
		num = 1;

	}
	else if(_abview.y >= _abview.z && _abview.z >= _abview.x)
	{


		deltam = (int)(DDAb[0]*_abview.z/_abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*_abview.y/_abview.x);
		llength = DDAb[1];
		mlength = DDAb[2];
		slength = DDAb[0];
		num = 2;
	}
	else if(_abview.y >= _abview.x && _abview.x >= _abview.z)
	{

		deltam = (int)(DDAb[2]*_abview.x/_abview.z);
		deltas = DDAb[2];
		deltal = (int)(DDAb[2]*_abview.y/_abview.z);
		llength = DDAb[1];
		mlength = DDAb[0];
		slength = DDAb[2];
		num = 3;
	}
	else if(_abview.z >= _abview.x && _abview.x >= _abview.y)
	{

		deltam = (int)(DDAb[1]*_abview.x/_abview.y);
		deltas = DDAb[1];
		deltal = (int)(DDAb[1]*_abview.z/_abview.y);
		llength = DDAb[2];
		mlength = DDAb[0];
		slength = DDAb[1];
		num = 4;
	}
	else
	{

		deltam = (int)(DDAb[0]*_abview.y/_abview.x);
		deltas = DDAb[0];
		deltal = (int)(DDAb[0]*_abview.z/_abview.x);
		llength = DDAb[2];
		mlength = DDAb[1];
		slength = DDAb[0];
		num =5;
	}




	int a;
	int* x;
	int* y;
	int* z;
	int i,j;
	int count = 0;
	double temp_sum= 0.0;

	//QueryPerformanceCounter(&temp1);
	//					QueryPerformanceCounter(&temp2);
	//			QueryPerformanceFrequency(&freq);					
	//			temp_sum += (double)(temp2.QuadPart-temp1.QuadPart)/(double)freq.QuadPart;
	//		count++;


	int sg[3];
	int vol[3];
	int vec_num;
	//viewvec.print("viewvec");//viewvecの成分は、0か1か-1 8通りの組み合わせ。
	if(_viewvec.x >= 0.0 && _viewvec.y >= 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=1;
		vol[0]=0;vol[1]=0;vol[2]=0;
		vec_num = 0;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y < 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 1;
	}
	else if(_viewvec.x >= 0.0 && _viewvec.y >= 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=1;sg[1]=1;sg[2]=-1;
		vol[0]=0;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 2;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y < 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=-1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 3;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y >= 0.0 && _viewvec.z < 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=-1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=DDAb[2]-1;
		vec_num = 4;
	}
	else if(_viewvec.x >= 0.0 && _viewvec.y < 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=1;sg[1]=-1;sg[2]=1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=0;
		vec_num = 5;
	}
	else if(_viewvec.x < 0.0 && _viewvec.y >= 0.0 && _viewvec.z >= 0.0)
	{
		sg[0]=-1;sg[1]=1;sg[2]=1;
		vol[0]=DDAb[0]-1;vol[1]=0;vol[2]=0;
		vec_num = 6;
	}
	else
	{
		sg[0]=1;sg[1]=-1;sg[2]=-1;
		vol[0]=0;vol[1]=DDAb[1]-1;vol[2]=DDAb[2]-1;
		vec_num = 7;
	}



	if(deltam*llength < deltal && deltam <= deltas*llength && deltas <= deltam )
	{


		int end_s = 0;
		int sum = -deltam;
		int *m_value;
		m_value = new int[slength];
		//初期化
		for(i=0;i<slength;i++){
			m_value[i] = 0;
		}
		int d = deltas;
		int start_s = 0;



		switch(num) 
		{
		case 0:
			x = &j;y = &a;z = &i;
			break;
		case 1:
			x = &j;y = &i;z = &a;
			break;
		case 2:
			x = &i;y = &j;z = &a;
			break;
		case 3:
			x = &a;y = &j;z = &i;
			break;
		case 4:
			x = &a;y = &i;z = &j;
			break;
		default:
			x = &i;y = &a;z = &j;
			break;
		}

//最も離れているボクセル面をPに入れる
		do{
			for(i = start_s ;i<=end_s;i++){//P内の全各ボクセル面index
				int &vl = m_value[i];
				a = vl;
				for(j = 0;j < llength ; j++)//s[index]内の全各ボクセル列
				{
					Block block(vol[0]+sg[0]**x+_bl.x,vol[1]+sg[1]**y+_bl.y,vol[2]+sg[2]**z+_bl.z,_bl.level,_bl.bnumx,_bl.bnumy,_bl.bnumz);
					//testRenderFrustum(block,normalreso,backupreso,existblcount,blockQue);
					testLoadFrustum(block,_reso,_exist_in_main_count,_blockstate,needCountMap,existInMainMap,blockQue);
					

					//視点から最も離れている未描画ボクセル１つを描画する。
				}
				vl++;
				if(vl == mlength)
					start_s++;
			}	
			sum += d;//△y/△z
			if(sum >0 && end_s<slength-1){//Pの先頭ボクセル面内の全ボクセルが描画済み。
				end_s++;
				sum -= deltam;//先頭ボクセル面をPから外す
			}
		}while(start_s <= slength-1);	//Pが空ではない。

		delete []m_value;



	}
	else
	{   


		int ms = deltam*deltas;//△y/△z
		int ls = deltal*deltas;
		int lm = deltal*deltam;


		int end_s = 0;
		int sum_s = -lm;//
		int start_s = 0;

		int *end_m;
		end_m = new int[slength];	
		int** l_value;
		l_value = new int*[slength];


		int* sum_m;
		sum_m = new int[slength];
		int* start_m;
		start_m = new int[slength];




		for(i = 0;i<slength;i++)
		{
			start_m[i] = 0;
			sum_m[i] = -ls;
			end_m[i] = 0;

			l_value[i] = new int [mlength];

			for(j = 0;j < mlength;j++)
				l_value[i][j] = 0;	
		}


		switch(num) 
		{
		case 0:
			x = &a;y = &j;z = &i;
			break;
		case 1:
			x = &a;y = &i;z = &j;
			break;
		case 2:
			x = &i;y = &a;z = &j;
			break;
		case 3:
			x = &j;y = &a;z = &i;
			break;
		case 4:
			x = &j;y = &i;z = &a;
			break;
		default:
			x = &i;y = &j;z = &a;
			break;
		}
//最も離れているボクセル面をPに入れる
		do
		{
			for(i = start_s;i<= end_s;i++)//P内の全各ボクセル面index
			{   
				for(j = start_m[i];j <= end_m[i]; j++)//s[index]内の全各ボクセル列
				{	
					int &vl = l_value[i][j];
					a = vl;
					Block block(vol[0]+sg[0]**x+_bl.x,vol[1]+sg[1]**y+_bl.y,vol[2]+sg[2]**z+_bl.z,_bl.level,_bl.bnumx,_bl.bnumy,_bl.bnumz);
					//testRenderFrustum(block,normalreso,backupreso,existblcount,blockQue);
					testLoadFrustum(block, _reso,_exist_in_main_count,_blockstate,needCountMap, existInMainMap,blockQue);
				//もっとも遠いボクセルを一つ描画する。
					vl++;
				}
				sum_m[i] += ms;//ds[index]+=△y/△z
				if(sum_m[i] >0 && end_m[i]<mlength-1){//ds[index]>=1.0
					end_m[i]++;//視点から最も離れている未描画のボクセル列をS[index]の最後に加える。
					sum_m[i] -= ls;//ds[index]-=1.0;
				}
				if(l_value[i][start_m[i]] == llength )//S[index]の先頭ボクセル列内の全ボクセルが描画済み。
					start_m[i]++;//先頭ボクセル列をS[index]から外す。
			}	
			sum_s += ms;//dp+=△x/△z
			if(sum_s >0 && end_s<slength-1){
				end_s++;
				sum_s -= lm;
				sum_m[end_s] = sum_s;
			}
			if(start_m[start_s] >  mlength-1)//Pの先頭ボクセル面内の全ボクセルが描画済み。
				start_s++;//先頭ボクセル面をPから外す
		}while(start_s <= slength - 1);	//Pが空ではない。

		delete []end_m;
		for(i=0;i < slength;i++)
			delete []l_value[i];
		delete []l_value;

		delete []sum_m;
		delete []start_m;

	}


	//	cout <<"vec_num"<<vec_num<<endl;
	//	cout <<"num"<<num<<endl;

}

void CLod::ProcessNextResoRender(Block _bl,int* countA,int* countC,list<Block>* _blockQue,int _levelLimit)//1段階高解像度のブロックのノードを探索
{
	Block nextbl = _bl.getHighblock();//仮に1段階高解像度のブロックのノードを探索
	vec3<double> vz = nextbl.getBlockVec(modelViewMatrix);
	vec3<double> ab = getViewvector(&vz);
	DDAblockRender(vz,ab,nextbl,countA,countC,_blockQue,_levelLimit);
	_bl.setBlockState(Block::nextreso,Block::rRequestBlock);
}
void CLod::initCountflag(int diff,bool **** needCountflag,bool **** existCountflag)
{
	for(int a= diff; a < NUMLEVEL;a ++)
	{
		int num = (int)pow(2.0,(NUMLEVEL-1.0)-a);
		int numx = INIBLX*num;
		int numy = INIBLY*num;
		int numz = INIBLZ*num;

		for(int i=0 ;i < numx;i++)
		{
			for(int j=0;j < numy;j++)
			{
				for(int k=0;k <numz;k++)
				{
					needCountflag[a][i][j][k] = false;
					existCountflag[a][i][j][k] = false;
				}
			}
		}
	}
}
void  CLod::setFrontSlice(Cg& _cg)
{
	vec3<double> view_z(0.0,0.0,1.0);
	view_z = m_InvModelView*view_z;


	//スライスの頂点決定
	int frontIdx;
	float vecView[3] = {(float)view_z.x,(float)view_z.y,(float)view_z.z};
	if(vecView[0] >= 0 && vecView[1] >= 0 && vecView[2] >= 0)
		frontIdx = 0;
	else if(vecView[0] >= 0 && vecView[1] >= 0 && vecView[2] < 0)
		frontIdx = 1;
	else if(vecView[0] >= 0 && vecView[1] < 0 && vecView[2] >= 0)
		frontIdx = 2;
	else if(vecView[0] < 0 && vecView[1] >= 0 && vecView[2] >= 0)
		frontIdx = 3;
	else if(vecView[0] < 0 && vecView[1] >= 0 && vecView[2] < 0)
		frontIdx = 4;
	else if(vecView[0] >= 0 && vecView[1] < 0 && vecView[2] < 0)
		frontIdx = 5;
	else if(vecView[0] < 0 && vecView[1] < 0 && vecView[2] >= 0)
		frontIdx = 6;
	else 
		frontIdx = 7;
	_cg.SetParameter(_cg.frontIdxParam,(float)frontIdx);
	_cg.SetParameter(_cg.vecViewParam ,vecView );
}
void CLod::display(){
	//printf("描画開始,");
	frameTime=0;
	order=0;
	SetLighting();
	//読み取りよう視錐台のモデルビュー行列作成
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix.m);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(m_Translate.x,m_Translate.y, m_Translate.z);     /* 平行移動(奥行き方向) 　　*/
	glMultMatrixd(modelMatrix.m);	
	glRotated(angleDelta, axisX, axisY, axisZ);  
	glGetDoublev(GL_MODELVIEW_MATRIX,  Next_modelViewMatrix.m);//file threadで使う。
	glPopMatrix();
	//描画

	/*二つのFBOを初期化*/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);//ここから、テクスチャRECTANGLEとして扱いたい描画内容のフレームバッファの内容を描く。
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix.m);
	
	modelViewMatrix.inv(&m_InvModelView);
	vec4<double> cam = m_InvModelView*ini_cam;//ini_camは(0.0,0.0,0.0,1.0)
	float camera[3] = {cam.x,cam.y,cam.z};
	cg.SetParameter(cg.cameraParam,camera);
	vec3<double> lookat = m_InvModelView*ini_l;//注視点
	vec3<double> upper = m_InvModelView*ini_u;//視界の上方向を決めるupper vector
	
	m_localRenderFrustum.setCamDef(cam.toVec3(),lookat,upper);//gluLookAtと同じ（カメラ、注視点、アッパーベクタ）　カメラパラメータから視錐台を構成する。　s_RenderFrustumはfrustum<double>クラス
	Block root_block = Block::GetRootBlock();  
	vec3<double> viewvec = root_block.getBlockVec(modelViewMatrix);
	setFrontSlice(cg);//消えても問題ない。
	vec3<double> abview = getViewvector(&viewvec);
	float utilityTime=totalrender.totalrendertime-(raycasttime_lastframe+loadtime_lastframe);
	FPSMAX=(int)(1000.0/utilityTime);
	unsigned int total_pixel_num= m_WinSize.x*m_WinSize.y;
	if(total_pixel_num>2031360){FPSMAX=26;}else 
		if(total_pixel_num>1048576){FPSMAX=28;}else
			if(total_pixel_num>69224){FPSMAX=39;}else
				if(total_pixel_num>495616){FPSMAX=37;}
				if(FPSMAX>60){FPSMAX=60;}

	if(utilityTime>(1000.0/(double)FPSMAX)){utilityTime=(1000.0/(double)FPSMAX)*0.9;}
	render_loadTimeLimit=renderTimeLimit-utilityTime;//render+load time情報の更新
				
	practicalnum=(int)(render_loadTimeLimit/(renderEachBlockTime));//file.getTexLoadTime();
				
	//これらが変である。
	int octreelevel=0;
	if(practicalnum<=howManyVisibleBlock[4]){octreelevel=4;}
	else if(practicalnum<=howManyVisibleBlock[3]){octreelevel=3;}
	else if(practicalnum<=howManyVisibleBlock[2]){octreelevel=2;}
	else if(practicalnum<=howManyVisibleBlock[1]){octreelevel=1;}
	else{octreelevel=0;}
	//abview.print("abview");
	renderFrustum(viewvec,abview,root_block,octreelevel);//これを消したらボリュームとブロックの線が見えなくなる。ここに一番大事なレンダリング情報が入ってる。

}
void CLod::renderBlock(Block block,bool real)
{
		Block lowResoblock = block;
		do{//do-whileループを外すとちらちら見えなくなるブロックが増える
			//初めにテクスチャメモリを探す
			if(m_File.texBlockExist(lowResoblock))
			{				
				m_File.setTexBlock(&cg,cg.vdecalParam,lowResoblock);//シェーダにテクスチャ名を渡したりする。setTextureParameterする。
				int dif = lowResoblock.level - block.level;
				if(dif==0 && real){
					realResolutionNum++;
					realResolutionNum_perFrame++;
				m_File.texBlockRequest(modelMatrix.m,projMatrix.m,lowResoblock,m_localRenderFrustum,frustum<double>::render);
				}else{
					m_File.texBlockRequest(modelMatrix.m,projMatrix.m,lowResoblock,m_localRenderFrustum,frustum<double>::back);//優先度更新
				}
			
				block.RayCast(&cg,dif);//ここでボリューム描画
				
				Block::blockState bstate = block.getLowState(lowResoblock);
				//printf("bstate=%d\n",bstate);
				block.setBlockState(bstate,Block::rRequestBlock);
				
				break;//無事にレンダリング出来たらループを抜ける
			}
			else
			{
				block.setBlockState(Block::waitblock,Block::rRequestBlock);}//テクスチャメモリへのロード待ち
			if(lowResoblock.level == NUMLEVEL -1)
			{	break;
			}else{//if(lowResoblock.level==3){block.printBlockInfo("最低段階行？",0);}
				lowResoblock = lowResoblock.getLowblock();//1段階下にしてやりなおし。ｓ
			}
		}while(1);//end while
	
}
void  CLod::Init(){
	/*//////////////////////////////////////////////////////////////
	テクスチャ1(高解像度ブロック×g_MaxTexNum）
	//////////////////////////////////////////////////////////////*/
	
	for(int i=0;i < g_MaxTexNum;i++)
	{
		glGenTextures(1, m_File.getTexaddress(i));
		glBindTexture(GL_TEXTURE_3D, m_File.getTexName(i));
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexImage3D(GL_TEXTURE_3D,0,GL_ALPHA,BLX+2,BLY+2,BLZ+2,0,GL_ALPHA, GL_FLOAT,NULL);//これがないとボリュームがなくなる。
		
	}//なぜ２のべき乗でなくて平気なのか？
	//glBindTexture(GL_TEXTURE_2D, 0);//これがなくても支障はない。
	//トータルレンダリング時間計測構造体初期化
	totalrender.totalrendertime = 0.0;
	totalrender.flag = false;
}

void CLod::testRenderFrustum(Block _block,int* Preso,int* NeedInTex_ExistInMainBlcount,list<Block>* _blockQue,int _levelLimit)//DDAブロックメソッドの中で使われる
{//Preso=normalreso,Sreso=backupreso
	//blockにデータが入っているか判定
	int id = m_File.getIndexblock(_block);
	_block.setBlockState(Block::notexist,Block::rRequestBlock);
	if(id != 0)//そのブロックにちょっとでもデータが入ってれば
	{
		//もしビューボリュームにblockが入っていれば
		if(_block.IsInFrustum(m_localRenderFrustum) != frustum<double>::OUTSIDE)//視錐台の中に入ってたら いつもOUTSIDEと判定される。
		{
			if(_block.resoChange(modelMatrix,projMatrix,m_WinSize,_levelLimit))
			//1ボクセルの大きさが、1ピクセルよりも大きいかどうかチェック。大きかったらtrue
			{
				ProcessNextResoRender(_block,Preso,NeedInTex_ExistInMainBlcount,_blockQue,_levelLimit);//実際に1段階高解像度のノードを探索
			}
			else{  //1ボクセルの大きさが、1ピクセルよりも小さい場合　or すべてが最適解像度ブロックになったら
				//何段階か上	
				if(!render_normal_blocks[_block.level][_block.x][_block.y][_block.z])
				{	
					_blockQue->push_back(_block);//Pblockと同じ値の要素を両端キューの末尾に追加する。
					render_normal_blocks[_block.level][_block.x][_block.y][_block.z] = true;
					render_normal_num++;//miffy added
				}

				
				if(!m_File.texBlockExist(_block))//テクスチャメモリになく
				{
					if(m_File.blockExist(_block))//メインメモリにあるブロックを探す
					{
						if(!r_existInMainMemNotInTexMem[_block.level][_block.x][_block.y][_block.z])
						{
							*NeedInTex_ExistInMainBlcount += 1;
							r_existInMainMemNotInTexMem[_block.level][_block.x][_block.y][_block.z] = true;
						}
					}
				}
			}//1ボクセルが1ピクセルよりも大きいかどうか判定
		}//視錐台の中に入ってるかどうか判定	
		else
		{//クリップアウト 
			_block.setBlockState(Block::frameout,Block::rRequestBlock);
		}
	}//そのブロックにちょっとでもデータが入ってるかどうか
}
///どんな様子かを俯瞰図で見るためのもの
void CLod::renderDebugWindow(){
	
		unsigned int** rb;//blockstate
		frustum<double> fs;
		if(frustumstate == frustum<double>::normal)//この視錐台はノーマル
		{
			rb = Block::nRequestBlock;
			fs = m_localNextFrustum;
		}
		else if(frustumstate == frustum<double>::back)//この視錐台はバックアップ用
		{
			rb = Block::bRequestBlock;
			fs = m_localNextFrustum;
		}
		else//この視錐台はレンダリング用
		{	
			rb = Block::rRequestBlock;//ピンク色
			fs = m_localRenderFrustum;
		}
		if(showGridFlag){
		//ブロックグリッド線描画(メイン用)
		for(int i=0; i< INIBLX;i++)
		{
			for(int j=0;j < INIBLY;j++)
			{
				for(int k=0;k< INIBLZ;k++){//rb=blockstate
					Block::renderSubblock(rb,Block(i,j,k,NUMLEVEL-1,INIBLX,INIBLY,INIBLZ),0.15);
				}
			}
		}
		}

		if(m_WinSize.x >= m_WinSize.y)
			glViewport(m_WinSize.x - m_WinSize.y/2,0, m_WinSize.y/2, m_WinSize.y/2);
		else
			glViewport(m_WinSize.x - m_WinSize.x/2,0, m_WinSize.x/2, m_WinSize.x/2);



		//サブ画面背景作成
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0.0,0.0,-7.0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-1.0,1.0,-1.0,1.0,0.25,20.0);

		glColor4f(0.0,0.0,0.0,0.5);
		glBegin(GL_QUADS);  
		glVertex2f(-1.0,-1.0);
		glVertex2f(1.0,-1.0);
		glVertex2f(1.0,1.0);
		glVertex2f(-1.0,1.0);
		glEnd();

		//サブ画面描画ブロック表示
		glLoadIdentity();
		glOrtho(-eyeDis*1.5,eyeDis*1.5,-eyeDis*1.5,eyeDis*1.5,0.25,20.0);


		glMatrixMode(GL_MODELVIEW);
		glRotatef(45.0f,1.0f,-1.0f,-0.29f);

		//ブロック描画
		for(int i=0; i< INIBLX;i++)
		{
			for(int j=0;j < INIBLY;j++)
			{
				for(int k=0;k< INIBLZ;k++)//rb=blockstate
					Block::renderSubblock(rb,Block(i,j,k,NUMLEVEL-1,INIBLX,INIBLY,INIBLZ),0.15);
			}
		}

		//画角描画
		glColor4f(RED[0],RED[1],RED[2],0.25);
		fs.drawLines();//視錐台の線を赤いので描画。
		glColor4f(0.42,0.84,0.14,0.25);
		m_localNextFrustum.drawLines();//	ファイル用視錐台を緑で描画
		glColor4f(1.0,1.0,1.0,1.0);

		glMultMatrixd(m_InvModelView.m);

		//視点描画
		glRotatef(180.0,0.0,1.0,0.0);
		glTranslatef(0.0,0.0,-0.5);
		glColor3f(0.0,1.0,0.0);
		glutSolidCone(0.05,0.5,5,5);


		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();


		glViewport(0, 0, m_WinSize.x, m_WinSize.y);



	

}
void CLod::requestNormalFrustum(vec3<double> viewvec,vec3<double> abview,Block block)//file threadで使う
{	
	nowprocessingflag=true;
	s_normalreso = -1;//LowBlockを作る回数に関わる変数。
	int counter=0;
	Block backupBlockList[16*16*8];//ファイルスレッドの方
	s_normalreso++;
	normalebn = 0;//exist blocks of normal reso
	nRequestQueue.clear();//両端キューからすべての要素を削除する。
	initCountflag(s_normalreso,n_needInMain,n_existInMain);
	DDAblockFile(viewvec,abview,block,&s_normalreso,&normalebn,Block::nRequestBlock,n_needInMain,n_existInMain,&nRequestQueue);
	idealBlockNum=nRequestQueue.size();
	float idealFrameTime=1000.0f;
	int usableblockNum=0;//実際に使うバックアップブロック個数
	int compressedNum=0;
	int overblockNum=idealBlockNum-g_MaxTexNum;
	backupNum=0;
	if(overblockNum>0){//多すぎたらテクスチャ圧縮処理
		int index2=0;
		//まずは、最適解像度ブロックから、最初のバックアップブロックリストを作る
		while(!nRequestQueue.empty()){
			Block testBlock=nRequestQueue.back();
			if((idealBlockNum-compressedNum+backupNum)>practicalRenderBlockNum){//まだ圧縮し足りなかったら

				Block parent=testBlock.returnParent();
				//兄弟だったらバックアップリストに入れない
				bool samefind=false;
				for(int i=0;i<backupNum;i++){
					if(backupBlockList[i].isSameBlock(parent)){
						nRequestQueue.pop_back();
						samefind=true;
						break;}//iを抜ける
				}
				if(!samefind){
					compressedNum+=m_File.getCompressInfo(parent);
					backupBlockList[backupNum]=parent;//バックアップリストに入れるぞ
					backupNum++;}
			}
			nRequestQueue.pop_back();
		}
	}//多すぎたらテクスチャ圧縮処理
	int count=0;
	while(!nRequestQueue.empty() && count<g_MaxTexNum)//呼び出し先の両端キューが空の場合はtrueを返し、そうでない場合はfalseを返す。
	{/*(Block)nRequestQueue.front()がまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。(Block)nRequestQueue.front()がもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
		m_File.mainBlockRequest(modelMatrix.m,projMatrix.m,(Block)nRequestQueue.front(),m_localNextFrustum,frustum<double>::normal);
		nRequestQueue.pop_front();//両端キューの最初の要素を削除する。
		count++;
	}
	for(int i=0;i<backupNum;i++){m_File.mainBlockRequest(modelMatrix.m,projMatrix.m,backupBlockList[i],m_localNextFrustum,frustum<double>::normal);}
	//論文のp35 (file.getThreadTime()=ファイルスレッドのwhile1ループにかかる時間
	//BACKUPTHRE = (int)((file.getThreadTime()/file.getMainLoadTime())*HDMAINALPHA);
	BACKUPTHRE = (int)((totalrender.totalrendertime/m_File.getMainLoadTime())*HDMAINALPHA);
	//論文通りだとこっちが正しいと思うが。。。
	//BACKUPTHRE=1フレームあたりに主メモリに読み込むことが出来るブロックの最大数
	if(BACKUPTHRE == 0)
		BACKUPTHRE = 1;
	/*バックアップブロックの解像度調整*/
	backupreso = s_normalreso;//論文p34
	int BBLoadMargin = MAXBNUM - normalnbn;//normalnbn=描画に必要な現解像度ブロックの数
	if(normalnbn>=MAXBNUM){BBLoadMargin=0;}
	// printf("BBLoadMArgin=%d\n",BBLoadMargin);
	//BBLoadMargin=バックアップブロックを読み込んでもよい数
	do{
		backupreso++;//0-4の値をとるみたい
		backebn = 0;
		bRequestQueue.clear();//両端キューからすべての要素を削除する。
		initCountflag(backupreso,b_needInMain,b_existInMain);
		for(int i=0; i< INIBLX;i++)
		{
			for(int j=0;j < INIBLY;j++)
			{
				for(int k=0;k< INIBLZ;k++)
					//void testLoadFrustum(Block block,int* octreeReso,int* existblcount,unsigned int** renderblock,bool **** needCountflag,bool **** existCountflag, deque<Block>* blockQue)
						testLoadFrustum(Block(i,j,k,NUMLEVEL-1,INIBLX,INIBLY,INIBLZ),&backupreso,&backebn,Block::bRequestBlock,b_needInMain,b_existInMain,&bRequestQueue);
			}
		}

	}while(backupreso < NUMLEVEL-1 && (bRequestQueue.size() > BBLoadMargin || (bRequestQueue.size()-backebn) > BACKUPTHRE));
	
}
void CLod::renderFrustum(vec3<double> _viewvec, vec3<double> _abview,Block _rootBlock,int _levelLimit)//これがなくなると、ボリュームとブロックの線がなくなる。
{	

	bltime_perframe=0;
	bltime_sum=0;
	blcount=0;
	loadtexblockcount = 0;

	
	static int count = 0;
	static float temp_num2 = 0.0;
	static float temp_occnum = 0.0;
	static float temp_rennum = 0.0;
	static int ltncount = 0;


	//最大テクスチャロード数計算
	//1フレーム内にレンダリングしてもよいブロック数
	//通常解像度octree追跡
	int normalNeedInTex_ExistInMainBn = 0;//通常解像度で、メモリに既に四方こまれていて、まだテクスチャメモリに存在いない、これからロードする必要のあるもの
	rnRequestQueue.clear();
	render_normal_num=0;
	initCountflag(0,render_normal_blocks,r_existInMainMemNotInTexMem);//テクスチャメモリになくてメインメモリにあるブロック表がすべてfalseに初期化される
	//ここでrnRequestQueueにいろいろ詰め込まれる。
	DDAblockRender(_viewvec,_abview,_rootBlock,0,&normalNeedInTex_ExistInMainBn,&rnRequestQueue,_levelLimit);
	backupOrNot=false;
	realResoNumInRender=rnRequestQueue.size();//最適解像度ブロックの数	
	double realresoLoadTime=((double)normalNeedInTex_ExistInMainBn)*m_File.getTexLoadTime();//テクスチャロードにかかる時間の予測
	double realresoRenderTime=((double)realResoNumInRender)*renderEachBlockTime;
	raycasttime_lastframe=0;//
	loadtime_lastframe=0;//
	double overtime=realresoLoadTime+realresoRenderTime-render_loadTimeLimit;
	int overRenderNum=rnRequestQueue.size()-practicalnum;

	if(overtime<0 ){//大丈夫な場合	
		//まずは最適解像度のレンダリング
		while(!rnRequestQueue.empty() )
		{	
			Block bl=rnRequestQueue.front();//両端キューの先頭への参照を返す。;
			if(!m_File.texBlockExist(bl))
			{
				if(m_File.blockExist(bl))
				{
					m_File.blockProhibitReplaceInMain(bl);//これからロードするから禁止
					ULONGLONG reqvalue=m_File.getReqbit(modelMatrix.m,projMatrix.m,bl,m_localRenderFrustum,frustum<double>::normal);
					m_File.loadMainToTex(bl,&cg,cg.vdecalParam,reqvalue);
					loadtexblockcount += 1;//ここでエラー
				}
			}//レンダリングしたいブロックがテクスチャメモリにあるかどうか
			renderBlock(bl,true);

			realResolutionNumCount++;
			rnRequestQueue.pop_front();


		}//while(!rnRequestQueue.empty())

	}else{//バックアップレンダリングが必要な場合	

		LARGE_INTEGER bb,ba,bf;
		QueryPerformanceCounter(&bb);
		QueryPerformanceFrequency(&bf);


		int practicalLoadTime=realresoLoadTime;
		int practicalRenderTime=realresoRenderTime; 
		int backupBlockNum=0;
		int compressedLoadNum=0;
		int compressedRenderNum=0;
		int processcount=0;	
		int previousBackNum=rnRequestQueue.size();;
		list<Block>::iterator testBlockIt;//現解像度を追う
		list<Block>::iterator backIt;//バックアップゾーンの最初を示す

		bool first=true;
		int howdeep=0;
		while( render_loadTimeLimit<(practicalLoadTime+practicalRenderTime) && miffybackupBlock.size()<4){
			Block testBlock;
			if(first){testBlock=rnRequestQueue.back();//後ろから順にテスト
			}else{
				testBlockIt=miffybackupBlock.end();
				testBlockIt--;
				int k=0;
				if((int)miffybackupBlock.size() > backupBlockNum){
					while(k<backupBlockNum){
						testBlockIt--;
						k++;
					}
				}
				testBlock=(Block)(*testBlockIt);//miffybackupBlock.back();
			}
			if(testBlock.level>=4){printf("testBlockがlevel4なので抜ける\n");break;}

			Block parent=testBlock.returnParent();
			//	兄弟だったらバックアップリストに入れない
			bool samefind=false;
			if(!miffybackupBlock.empty() && backupBlockNum!=0){
				//	兄弟だったらバックアップリストに入れない
				backIt=miffybackupBlock.end();
				int count=0;
				while(count<backupBlockNum-1){

					backIt--;
					Block siblingTestBlock=testBlock.returnParent();
					samefind=siblingTestBlock.isSameBlock(*backIt);
					if(samefind){
						//兄弟でした
						if(first){rnRequestQueue.pop_back();break;}else{
							bool decrement=false;
							if(testBlockIt!=miffybackupBlock.begin() && miffybackupBlock.size()>4){decrement=true;}else{decrement=false;}

							if(decrement){
								testBlockIt=	miffybackupBlock.erase(testBlockIt);
								backIt=testBlockIt;
								testBlockIt--;}
							break;
						}

						break;//iを抜ける
					}


					count++;	
				}//while
			}//兄弟かどうかチェック終わり

			if(!samefind){//兄弟じゃなかったら
				//compressedRenderNum+=file.getCompressInfo(parent);
				//compressedLoadNum+=file.getTexLoadCompressInfo(parent);

				practicalRenderTime-=(m_File.getCompressInfo(parent)*renderEachBlockTime);
				practicalLoadTime-=(m_File.getTexLoadCompressInfo(parent)*m_File.getTexLoadTime());
				if(first){rnRequestQueue.pop_back();}else{
					bool decrement=false;
					if(testBlockIt!=miffybackupBlock.begin() && miffybackupBlock.size()>4){decrement=true; }else{decrement=false;}

					if(decrement){testBlockIt=miffybackupBlock.erase(testBlockIt);
					backIt=testBlockIt;
					testBlockIt--;}

				}
				miffybackupBlock.push_back(parent);//pushbackして前から後ろ順に作られる
				practicalLoadTime+=m_File.getTexLoadTime();//自分の分カウント
				practicalRenderTime+=renderEachBlockTime;
				backupBlockNum++;
			}

			processcount++;
			if(rnRequestQueue.empty() && first){
				first=false;
				testBlockIt=miffybackupBlock.end();
				testBlockIt--;

				previousBackNum=miffybackupBlock.size();
				backupBlockNum=0;
			}
			if(!first && (int)miffybackupBlock.size() <= backupBlockNum){
				printf("3周目入るぞ\n");
				testBlockIt=miffybackupBlock.end();
				testBlockIt--;
				howdeep=3;
				previousBackNum=miffybackupBlock.size();
				backupBlockNum=0;
			}

		}//miffy圧縮処理終わり
		printf("before制限時間=%.4f,最適解像度時間=%.4f\n",render_loadTimeLimit,(realresoLoadTime+realresoRenderTime));
		printf("after制限時間=%.4f,最適解像度時間=%.4f\n",render_loadTimeLimit,(practicalLoadTime+practicalRenderTime));

		//最適解像度のレンダリング
		int realCount=0;//前から後ろへ
		while(!rnRequestQueue.empty() /*&&  realCount< (realResoNumInRender-compressedRenderNum)*/ )
		{	
			Block bl=rnRequestQueue.front();//両端キューの先頭への参照を返す。;
			if(!m_File.texBlockExist(bl))
			{
				if(m_File.blockExist(bl))
				{
					m_File.blockProhibitReplaceInMain(bl);//これからロードするから禁止
					ULONGLONG reqvalue=m_File.getReqbit(modelMatrix.m,projMatrix.m,bl,m_localRenderFrustum,frustum<double>::normal);
					m_File.loadMainToTex(bl,&cg,cg.vdecalParam,reqvalue);
					loadtexblockcount += 1;//ここでエラー
				}
			}//レンダリングしたいブロックがテクスチャメモリにあるかどうか
			renderBlock(bl,true);

			realResolutionNumCount++;
			rnRequestQueue.pop_front();


		}//while(!rnRequestQueue.empty())


		//バックアップブロックリストを使ったレンダリング
		while(!miffybackupBlock.empty() ){
			Block bl=miffybackupBlock.back();//backupListInRender[i];
			//bl.renderBlockLines(1.0,1.0,0.0,0.5);
			if(!m_File.texBlockExist(bl))
			{
				if(m_File.blockExist(bl))
				{
					m_File.blockProhibitReplaceInMain(bl);//これからロードするから禁止
					ULONGLONG reqvalue=m_File.getReqbit(modelMatrix.m,projMatrix.m,bl,m_localRenderFrustum,frustum<double>::back);
					m_File.loadMainToTex(bl,&cg,cg.vdecalParam,reqvalue);
					loadtexblockcount += 1;//ここでエラー
				}
			}//レンダリングしたいブロックがテクスチャメモリにあるかどうか
			renderBlock(bl,false);
			miffybackupBlock.pop_front();
		}
		//	

		QueryPerformanceCounter(&ba);
		double	backuptime=(double)(ba.QuadPart-bb.QuadPart)*1000.0/(double)bf.QuadPart;
		printf("バックアップ解像度処理時間[%d]=%.4f\n",backupBlockNum,backuptime);


	}

	//	printf("renderEachBlockTime=%.4f\n",renderEachBlockTime);
	if(renderblockcount!=0){
		renderEachBlockTime=raycasttime_lastframe/(double)renderblockcount;
	}else{renderEachBlockTime=0.5;}
	//各ブロックにかかるレンダリング時間を更新する

	m_File.countTexLRU();//1フレーム終わるごとに優先度スコアを５ビット下げる

}
void CLod::Reshape(int _w,int _h){
	m_WinSize.set(_w,_h);
	glMatrixMode(GL_MODELVIEW);

	//FBO作り直し
	createFBO();

	glLoadIdentity();
	gluLookAt(m_wBasicCamPos.x, m_wBasicCamPos.y, m_wBasicCamPos.z,0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	/* 視点と注視点の指定 */
	angleDelta = 0.0;   
	m_Translate.x = 0.0;
	m_Translate.y = 0.0;
	m_Translate.z = 0.0;  
	aspect = (float)_w/(float)_h;

	if( _w < _h ){
		m_FovY = atan(tan(M_PI/12.0f)*_h/_w)*360.0f/M_PI;
	}
	else {
		m_FovY = 30.0;
	}
	eyePosition=modelMatrix*m_wBasicCamPos;
	eyeDis = (float)m_wBasicCamPos.z;
	lightPosition=modelMatrix*lightAPosition;
               
	glViewport(0, 0, _w,_h);                    /* Window上での描画領域設定  */
	glGetIntegerv(GL_VIEWPORT,m_ViewPort);//あとでgluUnProjectで使う。
	//ブロック前後クリッピング初期化
	nearSlice = INITNEAR;
	farSlice  = INITFAR;
	Bclipeyeresio = (eyeDis-nearSlice)/(farSlice - nearSlice);
}

void CLod::SetClippingPlane(){
	nearSlice = eyeDis / ( 1.0f + Bclipeyeresio*( m_Bclipresio -1.0f));
	farSlice = nearSlice * m_Bclipresio;
	SetRenderFrustum();
	SetNextFrustum();
}
/// だんだんとただのタイトルバー出力にとどまらない機能を持ってきた。
void CLod::PrintInfoOnWindow(){
	
	maintotextime=m_File.getMainToTexTime();
		
	stringstream ssmessage;
	ssmessage<<"最適解像度"<<idealBlockNum<<" backup?"<<backupOrNot<<" レンダリングブロック数"<<renderBlockNum;
	glutSetWindowTitle((char*)ssmessage.str().c_str());//ウィンドウタイトルバーに文字を表示
	
}
void CLod::FileInit(string _path, string _dataname){
	m_File.Init(_path,_dataname);
	m_File.initMainLoad(4);//最初にメインメモリにいっぱいデータを載せておくか？
	
}
void CLod::Translate(float _shift){
	nearSlice -= _shift;
	farSlice = m_Bclipresio * nearSlice;
	Bclipeyeresio = (eyeDis-nearSlice)/(farSlice-nearSlice);
	SetRenderFrustum();
	SetNextFrustum();
}
void CLod::FileRun(){
	m_File.countMainLRU();
	m_File.countThreadTime();/*ハードディスク読込み時間測定用*/
	//Next_modelMatrix[]はdisplay関数でゲットしたもの。
	mat4<double> next_inv_modelView;
	Next_modelViewMatrix.inv(&next_inv_modelView);	
	vec4<double> N_cam = next_inv_modelView*ini_cam;
	vec3<double> N_l = next_inv_modelView*ini_l;
	vec3<double> N_u = next_inv_modelView*ini_u;
	//printf(",set file,");
	m_localNextFrustum.setCamDef(N_cam.toVec3(),N_l,N_u);
	//N_cam.print("file");
	Block block=Block::GetRootBlock();
	vec3<double> vz=block.getBlockVec(Next_modelViewMatrix);
	vec3<double> abview = getViewvector(&vz);
	requestNormalFrustum(vz,abview,block);//ここで、必要な現解像度ブロック、バックアップ解像度ブロックの数とかがわかる。
	int mostWantedIndex=m_File.getMaxIndexInReq();
	if(mostWantedIndex!=-1){//必要なブロックリストが空でなければロードする
		m_File.loadHDToMain(mostWantedIndex);
	}else{
		idleFileRequest(vz,abview,block);//やることないときはこっちロードする
		int mostWantedIndex=m_File.getMaxIndexInReq();
		if(mostWantedIndex!=-1){m_File.loadHDToMain(mostWantedIndex);}
	}
	
}
void CLod::idleFileRequest(vec3<double> viewvec,vec3<double> abview,Block block){//やることがないときこっちをロードするする　
	/*バックアップブロックの解像度調整*/
	 int BackUpRegion = MAXBNUM - g_MaxTexNum;//描画に必要な現解像度ブロックの数
	//BackUpRegion=バックアップブロックを読み込んでもよい数
		backupreso=1;//0-4の値をとるみたい
		backebn = 0;
		bRequestQueue.clear();//両端キューからすべての要素を削除する。
		initCountflag(backupreso,b_needInMain,b_existInMain);
		DDAblockFile(viewvec,abview,block,&backupreso,&backebn,Block::bRequestBlock,b_needInMain,b_existInMain,&bRequestQueue);
		int b_realResoLimit=bRequestQueue.size()-BackUpRegion;
		Block backupBlockList[92];
		int b_backupNum=0;
		if(b_realResoLimit>0){//miffyのテクスチャ圧縮処理発動
			
			int b_compressedNum=0;
			
			list<Block>::iterator realResoIt=bRequestQueue.end();
			realResoIt--;
			//まずは、最適解像度ブロックから、最初のバックアップブロックリストを作る
			int realReso_backup_wholeBlockNum=bRequestQueue.size();//
			if(nRequestQueue.empty()){return;}//エラーになっちゃうからとりあえずリターンした
			while(nRequestQueue.begin()!=realResoIt &&realReso_backup_wholeBlockNum>BackUpRegion){
				Block testBlock=*realResoIt;
				if(realReso_backup_wholeBlockNum>practicalRenderBlockNum){//まだ圧縮し足りなかったら
					
					Block parent=testBlock.returnParent();
					//兄弟だったらバックアップリストに入れない
					bool samefind=false;
					for(int i=0;i<backupNum;i++){
						if(backupBlockList[i].isSameBlock(parent)){
							samefind=true;
							break;}//iを抜ける
					}
					if(!samefind){
					b_compressedNum+=m_File.getCompressInfo(parent);
					if(m_File.blockExist(parent)){backebn++;}
					backupBlockList[b_backupNum]=parent;//バックアップリストに入れるぞ
					realReso_backup_wholeBlockNum=-b_compressedNum+b_backupNum;
					b_backupNum++;
					
					}
				}
				realResoIt--;
			}
		}//end miffyのテクスチャ圧縮処理発動
		
	
	
	int count=0;
	
	while(!bRequestQueue.empty() &&   count<=1)
	{/*bRequestQueue.front()がまだreqqueに入れられてないなら空いてる場所orreqqueが満杯なら優先度の一番低いところに入れる。bRequestQueue.front()がもうreqque、またはメインメモリに転送済みにあるなら優先度を更新する*/
		Block bl=bRequestQueue.front();
		if(!m_File.blockExist(bl)){
		m_File.mainBlockRequest(modelMatrix.m,projMatrix.m,(Block)bRequestQueue.front(),m_localRenderFrustum,frustum<double>::normal);
		count++;
		}
		bRequestQueue.pop_front();
		
	}
	for(int i=0;i<b_backupNum;i++){
		if(count<=1){
		if(!m_File.blockExist((Block)bRequestQueue.front())){
		m_File.mainBlockRequest(modelMatrix.m,projMatrix.m,backupBlockList[i],m_localRenderFrustum,frustum<double>::back);
		count++;
		}
		}else{break;}
	}
	
}
void CLod::FileTexInit(){
	m_File.initTexLoad(4);//最初にテクスチャメモリにデータをいっぱい載せるか？
	
}
/// 終了時にファイル出力したいから作った覚えがある。
void CLod::Exit(){
	
	m_File.deleteMemory();
}
/// ブロックに関するフラグを初期化する。
void CLod::InitBflag()
{
	b_needInMain = new bool ***[NUMLEVEL];
	b_existInMain = new bool ***[NUMLEVEL];
	n_needInMain = new bool ***[NUMLEVEL];
	n_existInMain = new bool ***[NUMLEVEL];
	render_normal_blocks = new bool ***[NUMLEVEL];
	r_existInMainMemNotInTexMem = new bool ***[NUMLEVEL];

	for(int a= 0; a < NUMLEVEL;a ++)
	{
		int num = (int)pow(2.0,(NUMLEVEL-1.0)-a);
		int numx = INIBLX*num;
		int numy = INIBLY*num;
		int numz = INIBLZ*num;


		b_needInMain[a] = new bool **[numx];
		b_existInMain[a] = new bool **[numx];
		n_needInMain[a] = new bool **[numx];
		n_existInMain[a] = new bool **[numx];
		render_normal_blocks[a] = new bool **[numx];
		r_existInMainMemNotInTexMem[a] = new bool **[numx];
	//	Block::occludedIndex[a]=new bool **[numx];

		for(int i=0 ;i < numx;i++)
		{
			b_needInMain[a][i] = new bool *[numy];
			b_existInMain[a][i] = new bool *[numy];
			n_needInMain[a][i] = new bool *[numy];
			n_existInMain[a][i] = new bool *[numy];
			render_normal_blocks[a][i] = new bool *[numy];
			r_existInMainMemNotInTexMem[a][i] = new bool *[numy];
	//		Block::occludedIndex[a][i]= new bool *[numy];

			for(int j=0;j < numy;j++)
			{
				b_needInMain[a][i][j] = new bool [numz];
				b_existInMain[a][i][j] = new bool [numz];
				n_needInMain[a][i][j] = new bool [numz];
				n_existInMain[a][i][j] = new bool [numz];
				render_normal_blocks[a][i][j] = new bool [numz];
				r_existInMainMemNotInTexMem[a][i][j] = new bool [numz];
			//	Block::occludedIndex[a][i][j]=new bool [numz];

				for(int k=0;k <numz;k++)
				{
					b_needInMain[a][i][j][k] = false;
					b_existInMain[a][i][j][k] = false;
					n_needInMain[a][i][j][k] = false;
					n_existInMain[a][i][j][k] = false;
					render_normal_blocks[a][i][j][k] = false;
					r_existInMainMemNotInTexMem[a][i][j][k] = false;
				//	Block::occludedIndex[a][i][j][k]=false;
				}
			}
		}
	}
}
void CLod::destroyFBO()
{
	glDeleteFramebuffers(1,&fb);
	glDeleteTextures(1, &fbTex);
	glDeleteTextures(1, &fbDep);
	fb = 0;
	fbTex = 0;
	fbDep = 0;
}
void CLod::createFBO(){
	if(fb > 0)
		destroyFBO();

 /* フレームバッファオブジェクトを生成 */
	glGenFramebuffersEXT(1,&fb);//1=フレームバッファの名前(0はウィンドウシステムから与えられた名前だから使っちゃダメ),fb=フレームバッファの名前を入れるところ。つまり、1という値をfbに入れるところ。
	//書き込まれるテクスチャの設定
	glGenTextures(1, &fbTex);
	 /* フレームバッファオブジェクトを結合 */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);//fbという名前のフレームバッファにGL_FRAMEBUFFERというフレームバッファオブジェクトをboundする。たいていGL_FRAMEBUFFERらしい。
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,fbTex);

	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA,m_WinSize.x,m_WinSize.y,0,GL_RGBA,GL_FLOAT,NULL);//これがないと画面が白くなる。
	 /* フレームバッファオブジェクトに２Dのテクスチャオブジェクトを結合する */
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB,fbTex, 0 );//これがないとボリュームがなくなる。
	glBindTexture(GL_TEXTURE_2D,0);
	  /* フレームバッファオブジェクトの結合を解除する */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	cgGLSetTextureParameter(cg.fdecalParam,fbTex);
	cgGLSetTextureParameter(cg.fdecal2Param,fbTex);//なくても問題ない。


	glGenTextures(1, &fbDep);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,fbDep);

	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//use bilinear filtering
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_DEPTH_COMPONENT24,m_WinSize.x,m_WinSize.y,0,GL_DEPTH_COMPONENT,GL_INT,NULL);//これがないとボリュームがなくなる。
	//テクスチャをフレームバッファにアタッチします。
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB,fbDep, 0 );

	glBindTexture(GL_TEXTURE_2D,0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	check_framebuffer_status();
}
void CLod::SetLighting(void) {                      /* ライトの指定 　　　　　　*/
	cg.SetMatrixParameter(cg.matModelViewNormalParam,CG_GL_MODELVIEW_MATRIX,CG_GL_MATRIX_INVERSE_TRANSPOSE);
	cg.SetParameter(cg.lightColorParam,lightColor);
	cg.SetParameter(cg.lightPositionParam,&lightPosition.x);
	cg.SetParameter(cg.globalAmbientParam,globalAmbient);
	cg.SetParameter(cg.eyePositionParam,&eyePosition.x);
	cg.SetParameter(cg.KeParam,Ke);
	cg.SetParameter(cg.KaParam,Ka);
	cg.SetParameter(cg.KdParam,Kd);
	cg.SetParameter(cg.KsParam,Ks);
	cg.SetParameter(cg.shininessParam,shininess);
}
void CLod::PasteFBOTexture(){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	cgGLEnableTextureParameter(cg.transfer_functionParam);	cg.CheckCgError();
	
	// //FBO解除
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);//ここから、↑で描いた内容をウィンドウのどこに描くのかが決められる。

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_RECTANGLE, fbTex);//これを消すと画面が白くなる。フレームバッファのカラーコンポネント
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0,1.0,-1.0,1.0,1.0,20.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(0.0,0.0,-5);
	glEnable(GL_TEXTURE_RECTANGLE);
	glColor4f(1.0,1.0,1.0,1.0);//オブジェクト全体の色　というか、画面全体を覆うテクスチャの色かも。
	glBegin(GL_QUADS); //これは、画面全体を覆うテクスチャ。
	glTexCoord2i(0, 0);
	glVertex2f(-1.0, -1.0);

	glTexCoord2i(0,m_WinSize.y);glVertex2f(-1.0, 1.0);

	glTexCoord2i(m_WinSize.x,m_WinSize.y);glVertex2f(1.0,1.0);

	glTexCoord2i(m_WinSize.x,0);glVertex2f(1.0, -1.0);
	glEnd();
	glDisable(GL_TEXTURE_RECTANGLE);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);//これを消してもレンダリング結果は変わらない。
}
