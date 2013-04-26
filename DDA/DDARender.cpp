#include "DDARender.h"


DDARender::DDARender(int _voxel)
  :nVoxel(_voxel)
	,planePos(0)
	,insertedPlane(0)
	,dPlane(0)
	,startIndex(0)
{
	 columnbegin= new int[nVoxel];
	 insertedColumn= new int[nVoxel];
	 planeList = new Block**[nVoxel];//面リスト　２次元配列へのポインタの配列
	 unRenderedVoxelIt= new int[nVoxel*nVoxel];//未描画のボクセルを示す
	 dLine=new float[nVoxel];
	 inverseFlag[0]=false;
	 inverseFlag[1]=false;
	 inverseFlag[2]=false;
	 memset(ini,0,sizeof(int)*3);

}



DDARender::~DDARender(void)
{
}

void DDARender::init(mat4<float>& _modelview){
	vec3<float> viewvec(-_modelview.m[2],-_modelview.m[6],-_modelview.m[10]);
	vec3<float> delta;		
	//視線ベクトルの逆ベクトルを得る//なんで逆なんだっけ、スライス平面欲しいから？
	if(viewvec.x!=0){
		delta.x=1.0f/(float)fabs((double)viewvec.x);}
	else{delta.x=1.0f;}
	if(viewvec.y!=0){	
		delta.y=1.0f/(float)fabs((double)viewvec.y);
	}else{delta.y=1.0f;}
	if(viewvec.z!=0){
		delta.z=1.0f/(float)fabs((double)viewvec.z);}
	else{delta.z=1.0f;}
	printf("delta[%.4f][%.4f][%.4f]\n",delta.x,delta.y,delta.z);
	//deltaの大きいのが描画の基準軸になる。
	//逆に考えれば、視線ベクトルの成分の一番小さいのが描画の基準軸だよね。
	if(delta.x>=delta.y && delta.y>=delta.z){
		m_first_orientation=0;//'x';
		m_second_orientation=1;//'y';
		slopeLine=delta.y/delta.x;
		slopePlane=delta.z/delta.x;
	}else if(delta.x>=delta.z && delta.z>=delta.y){
		m_first_orientation=0;//'x';
		m_second_orientation=2;//'z';
		slopeLine=delta.z/delta.x;
		slopePlane=delta.y/delta.x;
	}else if(delta.y>=delta.z && delta.z>=delta.x){
		m_first_orientation=1;//'y';
		m_second_orientation=2;//'z';
		slopeLine=delta.z/delta.y;
		slopePlane=delta.x/delta.y;
	}else if(delta.y>=delta.x && delta.x>=delta.z){
		m_first_orientation=1;//'y';
		m_second_orientation=0;//'x';
		slopeLine=delta.x/delta.y;
		slopePlane=delta.z/delta.y;
	}else if(delta.z>=delta.x && delta.x>=delta.y){
		m_first_orientation=2;//'z';
		m_second_orientation=0;//'x';
		slopeLine=delta.x/delta.z;
		slopePlane=delta.y/delta.z;
	}else{//z,y,x
		m_first_orientation=2;//'z';
		m_second_orientation=1;//'y';
		slopeLine=delta.y/delta.z;
		slopePlane=delta.x/delta.z;
	}
	//	printf("slopeLine=%.8f,slopePlane=%.8f\n",slopeLine,slopePlane);
	//最も視点から離れたバウンディングボックスの頂点を探す。
	//printf("描画方向1st[%d],2nd[%d]\n",m_first_orientation,m_second_orientation);
	int maxId=boundingCube.FarthestIndex(_modelview);
	//eyeVector.print("eyeVector");
	printf("今最も遠い頂点＝%d\n",maxId);//t番近い、、だよ。なんで？？？？
	//逆さフラグ初期化
	inverseFlag[0]=false;inverseFlag[1]=false;inverseFlag[2]=false;
	switch(maxId){//最も離れた頂点がどこかによってまずスタート位置が決まる
	case 0:
		break;
	case 1:
		inverseFlag[0]=true;//xだけ逆方向
		break;
	case 2:
		inverseFlag[0]=true;inverseFlag[1]=true;
		break;
	case 3:
		inverseFlag[1]=true;
		break;
	case 4:
		inverseFlag[2]=true;
		break;
	case 5:
		inverseFlag[0]=true;inverseFlag[2]=true;
		break;
	case 6:
		inverseFlag[0]=true;inverseFlag[1]=true;inverseFlag[2]=true;
		break;
	case 7:
		inverseFlag[1]=true;inverseFlag[2]=true;
		break;
	}//end switch


}
void DDARender::display( ){
	
	glGetFloatv(GL_MODELVIEW_MATRIX,  m_modelview.m);
	glPopMatrix();
	glEnable(GL_BLEND);//original これがないと、ボリュームが真黒になる。
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);                  // 奥行テスト利用
	glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);
	DrawCoordinate(1.0);//座標軸の描画
	//ブロックラインの表示
	
		for(int i=0;i<nVoxel*nVoxel*nVoxel;i++)
		{manyblock[i].renderBlockLines();}

	init(m_modelview);
	DDA();
	puts("");
	planePos=0;
	glLineWidth(4);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	boundingCube.DrawWireCube();
	boundingCube.DrawVertexNumers();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
}//for display

void DDARender::geometryInit(){
	blockLength=2.0/(float)nVoxel;
	manyblock= new Block[nVoxel*nVoxel*nVoxel];
	int i=0;
	//BlockInit
	for(int z=0;z<nVoxel;z++){
		for(int y=0;y<nVoxel;y++){
			for(int x=0;x<nVoxel;x++){
				manyblock[i]=Block(x,y,z,nVoxel,blockLength);
				manyblock[i].mySerialNumber=i;
				i++;
			}}}
	for(int y=0;y<nVoxel;y++){planeList[y]= new Block*[nVoxel];
	for(int x=0;x<nVoxel;x++){
		planeList[y][x]=new Block[nVoxel];//メモリ確保
	}
	}

}
void DDARender::DDA(){
	for(int i=0;i<nVoxel;i++){
		insertedColumn[i]=0;
		columnbegin[i]=0;
	}
	//ボクセル列リストs[index]とボクセル面リストPを空にする
	//dPlane=0で初期化する （論文２行目）
	for(int i=0;i<nVoxel;i++){dLine[i]=0;}
	dPlane=0;
	for(int i=0;i<nVoxel;i++){//まずは１列入れる
		for(int xyz=0;xyz<3;xyz++){//方向修正
			if(inverseFlag[xyz]==true ){
				if( m_first_orientation==xyz){ini[xyz]=nVoxel-i-1;}else{ini[xyz]=nVoxel-1;}
			}else{
				if( m_first_orientation==xyz){ini[xyz]=i;}else{ini[xyz]=0;}
			}
		}//end xyz
		planeList[0][0][i]=manyblock[((ini[2])*nVoxel+ini[1])*nVoxel+ini[0]];//ここでエラー
	}
	//イテレータの初期化
	unRenderedVoxelIt[0]=0;
	for(int i=1;i<nVoxel*nVoxel;i++){unRenderedVoxelIt[i]=0;}
	insertedPlane=0;
	insertedColumn[insertedPlane]++;//1列入れられました
	insertedPlane++;
	columnbegin[0]=0;
	do{
		for(int planeIndex=planePos;planeIndex<insertedPlane;planeIndex++){//面リスト内のすべてのボクセル面に関して
			for(int columnIndex=columnbegin[planeIndex];columnIndex<insertedColumn[planeIndex];columnIndex++)
			{//columnList[planeindex]内の全ボクセル列について
				if(unRenderedVoxelIt[planeIndex*nVoxel+columnIndex]<nVoxel){//nVoxel以上にならないようにしてあるから大丈夫
					planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*nVoxel+columnIndex]].renderQUADS();//最も遠いボクセル１つを描画する　（論文７行目）

					printf("id=%d\n",planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*nVoxel+columnIndex]].mySerialNumber);
					//planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*nVoxel+columnIndex]].renderBlockLines();
					unRenderedVoxelIt[planeIndex*nVoxel+columnIndex]++;//最も遠いボクセル１つを描画する　（論文７行目）

				}//未描画印がnVoxel未満だったら
			}//for planeIndex内のすべてのボクセル列

			dLine[planeIndex]=dLine[planeIndex]+slopeLine;//(論文９行目)

			if(dLine[planeIndex]>=1.0f){
				if(insertedColumn[planeIndex]<nVoxel){
					//視点から最も離れている未描画のボクセル列をSの最後に加える
					for(int i=0;i<nVoxel;i++){
						for(int xyz=0;xyz<3;xyz++){//x,y,zの123
							if(inverseFlag[xyz]==true){
								if(m_first_orientation==xyz){ini[xyz]=nVoxel-i-1;}
								else if(m_second_orientation==xyz){ini[xyz]=nVoxel-1-insertedColumn[planeIndex];}
								else{ini[xyz]=nVoxel-1-planeIndex;}
							}else{
								if(m_first_orientation==xyz){ini[xyz]=i;}
								else if(m_second_orientation==xyz){ini[xyz]=insertedColumn[planeIndex];}
								else{ini[xyz]=planeIndex;}
							}
						}//end xyz
						planeList[planeIndex][insertedColumn[planeIndex]][i]=manyblock[(ini[2]*nVoxel+ini[1])*nVoxel+ini[0]];//ここでエラー

					}//end i
					unRenderedVoxelIt[planeIndex*nVoxel+insertedColumn[planeIndex]]=0;
					insertedColumn[planeIndex]++;//列カウントインクリメント

				}//if insertedColumn<nVoxel	

				dLine[planeIndex]=dLine[planeIndex]-1.0f;
			}//if dLine>1.0f


			//ii planeIndex内の　先頭ボクセル列が全部描画済みになったら
			if(unRenderedVoxelIt[planeIndex*nVoxel+columnbegin[planeIndex]]==nVoxel){
				columnbegin[planeIndex]++;//先頭ボクセル列をplaneIndexから外す　columnListのstart位置が変わるから

			}//もし先頭ボクセルが全て描画済なら
		}//end while全部の面
		dPlane+=slopePlane;

		if(dPlane>=1.0f){
			if(insertedPlane<nVoxel){
				//視点から最も離れている未描画のボクセル面をPの最後に加える
				for(int i=0;i<nVoxel;i++){
					for(int xyz=0;xyz<3;xyz++){//x,y,zの123
						if(inverseFlag[xyz]==true){
							if(m_first_orientation==xyz){ini[xyz]=nVoxel-i-1;}
							else if(m_second_orientation==xyz){ini[xyz]=nVoxel-1;}
							else{ini[xyz]=nVoxel-1-insertedPlane;}
						}else{
							if(m_first_orientation==xyz){ini[xyz]=i;}
							else if(m_second_orientation==xyz){ini[xyz]=0;}
							else{ini[xyz]=insertedPlane;}
						}
					}//end j
					planeList[insertedPlane][0][i]=manyblock[((ini[2])*nVoxel+ini[1])*nVoxel+ini[0]];//ここで列を入れてみる（正しいこと？）
				}//i
				columnbegin[insertedPlane]=0;
				insertedColumn[insertedPlane]=1;
				unRenderedVoxelIt[insertedPlane*nVoxel+0]=0;//初期化
				insertedPlane++;//面の枚数カウントをインクリメント
			}//insertedPlane<nVoxel

			dPlane=dPlane-1.0f;

		}//if dPlane>1.0f
		//もしPの先頭ボクセル面内の前ボクセルが描画済みなら
		if(/*planePos<nVoxel && */ unRenderedVoxelIt[planePos*nVoxel+nVoxel-1]==nVoxel){
			//先頭ボクセル面をPから外す
			planePos++;
		}
	}while(planePos<nVoxel);//PlaneListが空ではない場合

}
