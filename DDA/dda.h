#ifndef MIFFY_VEC3
#include <miffy/math/vec3.h>
#endif
#ifndef MIFFY_CUBE
#include <miffy/math/cube.h>
#endif
namespace miffy{
template <typename T>
  class DDA{
	private:
		enum {X=1,Y=0x2,Z=0x4}AXIS;//xyz軸の方向
		unsigned char m_inverse_flag=0;//どの軸が逆向きなのか
		AXIS m_first_orientation;//最初に詰め込む方角
		AXIS m_second_orientation;
	public:
		void DDAinit(vec3<T> _eyeVector){
	//視線ベクトルの逆ベクトルを得る
	if(_eyeVector.x!=0){
		deltaX=1.0f/(float)fabs((double)_eyeVector.x);}
	else{deltaX=1.0f;}
	if(_eyeVector.y!=0){	
		deltaY=1.0f/(float)fabs((double)_eyeVector.y);
	}else{deltaY=1.0f;}
	if(_eyeVector.z!=0){
		deltaZ=1.0f/(float)fabs((double)_eyeVector.z);}
	else{deltaZ=1.0f;}
	//deltaの一番大きいのが描画の最初の基準方向になる
	//deltaの大きい順に描く
	if(deltaX>=deltaY && deltaY>=deltaZ){
		//xが一番大きい場合
		m_first_orientation=X;//'x';
		m_second_orientation=Y;//'y';
		slopeLine=deltaY/deltaX;
		slopePlane=deltaZ/deltaX;
	}else if(deltaX>=deltaZ && deltaZ>=deltaY){
		m_first_orientation=X;//'x';
		m_second_orientation=Z;//'z';
		slopeLine=deltaZ/deltaX;
		slopePlane=deltaY/deltaX;
	}else if(deltaY>=deltaZ && deltaZ>=deltaX){
		m_first_orientation=Y;//'y';
		m_second_orientation=Z;//'z';
		slopeLine=deltaZ/deltaY;
		slopePlane=deltaX/deltaY;
	}else if(deltaY>=deltaX && deltaX>=deltaZ){
		m_first_orientation=Y;//'y';
		m_second_orientation=X;//'x';
		slopeLine=deltaX/deltaY;
		slopePlane=deltaZ/deltaY;
	}else if(deltaZ>=deltaX && deltaX>=deltaY){
		m_first_orientation=Z;//'z';
		m_second_orientation=X;//'x';
		slopeLine=deltaX/deltaZ;
		slopePlane=deltaY/deltaZ;
	}else{//z,y,x
		m_first_orientation=Z;//'z';
		m_second_orientation=Y;//'y';
		slopeLine=deltaY/deltaZ;
		slopePlane=deltaX/deltaZ;
	}
	//最も視点から離れたバウンディングボックスの頂点を探す。
	int maxId=boundingbox.calculateFarthestVertex(eyeVector);
	//逆さフラグ初期化
	switch(maxId){//最も離れた頂点がどこかによってまずスタート位置が決まる
	case 0://どれも順方向
		m_inverse_flag=0;
		break;
	case 1:
		m_inverse_flag=X;//xだけ逆方向
		break;
	case 2://xとyが逆方向
		m_inverse_flag=(X|Y);
		break;
	case 3://yだけ逆方向
		m_inverse_flag=Y;
		break;
	case 4://zだけ逆方向
		m_inverse_flag=Z;
		break;
	case 5://xとzが逆方向
		m_inverse_flag=(X|Z);
		break;
	case 6://xyz全部逆方向
		m_inverse_flag=(X|Y|Z);
		break;
	case 7://yとzが逆方向
		m_inverse_flag=(Y|Z);
		break;
	default:
		assert(!"異常");
		break;
	}//end switch


}
/*!
@brief 2^3個ずつ、再帰的に行うタイプのもの
*/
void DDA(cube<T> thisblock[8]){
	//初期化
	cube<T> planeList[2][2][2];//面リスト　２次元配列へのポインタの配列
	int unRenderedVoxelIt[4];//未描画のボクセルを示す
	float dPlane=0;//1.0以上だったら次の面を挿入する
	float dLine[2];//1.0以上だったら次の列を挿入する
	int columnbegin[2];
	int insertedColumn[2];
	int planeBegin=0;
	int insertedPlane=0;
	for(int i=0;i<2;i++){
		insertedColumn[i]=0;
		columnbegin[i]=0;
	}
	//ボクセル列リストs[index]とボクセル面リストPを空にする
	//dPlane=0で初期化する （論文２行目）
	for(int i=0;i<2;i++){dLine[i]=0;}
	dPlane=0;

	for(int i=0;i<2;i++){//まずは１列入れる
		for(int xyz=0;xyz<3;xyz++){//方向修正
			if(inverseFlag[xyz]==true ){
				if( m_first_orientation==xyz){ini[xyz]=2-i-1;}else{ini[xyz]=2-1;}
			}else{
				if( m_first_orientation==xyz){ini[xyz]=i;}else{ini[xyz]=0;}
			}
		}//end xyz
		planeList[0][0][i]=thisblock[((ini[2])*2+ini[1])*2+ini[0]];//ここでエラー
		//planeList[0][0][i].renderBlockLines();	
	}//最初の１列
	//イテレータの初期化
	unRenderedVoxelIt[0]=0;
	for(int i=1;i<8;i++){unRenderedVoxelIt[i]=0;}
	insertedPlane=0;
	insertedColumn[insertedPlane]++;//1列入れられました
	if(insertedColumn[insertedPlane]>2){assert(!"error");}
	insertedPlane++;
	columnbegin[0]=0;



	do{
		for(int planeIndex=planeBegin;planeIndex<insertedPlane;planeIndex++){
			for(int columnIndex=columnbegin[planeIndex];columnIndex<insertedColumn[planeIndex];columnIndex++)
			{//columnList[planeindex]内の前ボクセル列について ここで進む
				if(planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*2+columnIndex]].pixelLODcontroll(modelMatrix,projMatrix,(int)winwidth,(int)winheight) && planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*2+columnIndex]].level!=0)
				{
					//	printf("再追跡 %d\n",size);
					octreeTraversal(planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*2+columnIndex]]);//再追跡
					unRenderedVoxelIt[planeIndex*2+columnIndex]++;
				}//child LOD test
				else{//この子でOK
					//blockList.push_back(childList[i]);
					planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*2+columnIndex]].renderBlockQUADS();
					//planeList[planeIndex][columnIndex][unRenderedVoxelIt[planeIndex*2+columnIndex]].renderBlockLines();
					//printf("レンダリング %d\n",size);
					unRenderedVoxelIt[planeIndex*2+columnIndex]++;
					blocksizeCounter++;
				}//end else
			}//列リストの追跡

			dLine[planeIndex]=dLine[planeIndex]+slopeLine;//(論文９行目)
			if(dLine[planeIndex]>=1.0f|| unRenderedVoxelIt[(planeIndex)*2+insertedColumn[planeIndex]-1]==2){
				if(insertedColumn[planeIndex]!=2){
					//視点から最も離れている未描画のボクセル列をSの最後に加える
					for(int i=0;i<2;i++){
						for(int xyz=0;xyz<3;xyz++){//x,y,zの123
							if(inverseFlag[xyz]==true){
								if(m_first_orientation==xyz){ini[xyz]=2-i-1;}
								else if(m_second_orientation==xyz){ini[xyz]=2-1-insertedColumn[planeIndex];}
								else{ini[xyz]=2-1-planeIndex;}
							}else{
								if(m_first_orientation==xyz){ini[xyz]=i;}
								else if(m_second_orientation==xyz){ini[xyz]=insertedColumn[planeIndex];}
								else{ini[xyz]=planeIndex;}
							}
						}//end j
						planeList[planeIndex][insertedColumn[planeIndex]][i]=thisblock[(ini[2]*2+ini[1])*2+ini[0]];//ここでエラー
					}//end i
					unRenderedVoxelIt[planeIndex*2+insertedColumn[planeIndex]]=0;
					insertedColumn[planeIndex]++;//列カウントインクリメント
				}//if insertedColumn!=blockNum	
				if(unRenderedVoxelIt[(planeIndex)*2+insertedColumn[planeIndex]-1]!=2){
					dLine[planeIndex]=dLine[planeIndex]-1.0f;}
			}//if dLine>1.0f
			//ii　先頭ボクセル列が全部描画済みになったら
			if(unRenderedVoxelIt[planeIndex*2+columnbegin[planeIndex]]==2){
				columnbegin[planeIndex]++;//columnListのstart位置が変わるから
			}
		}//end while全部の面
		dPlane+=slopePlane;
		if(dPlane>=1.0f|| unRenderedVoxelIt[(insertedPlane-1)*2+2-1]==2){
			if(insertedPlane!=2){
				//視点から最も離れている未描画のボクセル面をPの最後に加える
				//	planeList[insertedPlane][0]=new Block[blockNum];
				for(int i=0;i<2;i++){
					for(int xyz=0;xyz<3;xyz++){//x,y,zの123
						if(inverseFlag[xyz]==true){
							if(m_first_orientation==xyz){ini[xyz]=2-i-1;}
							else if(m_second_orientation==xyz){ini[xyz]=2-1;}
							else{ini[xyz]=2-1-insertedPlane;}
						}else{
							if(m_first_orientation==xyz){ini[xyz]=i;}
							else if(m_second_orientation==xyz){ini[xyz]=0;}
							else{ini[xyz]=insertedPlane;}
						}
					}//end j
					planeList[insertedPlane][0][i]=thisblock[((ini[2])*2+ini[1])*2+ini[0]];
					//挿入した面をレンダリング
				}//i
				columnbegin[insertedPlane]=0;
				insertedColumn[insertedPlane]=1;
				unRenderedVoxelIt[insertedPlane*2+0]=0;//初期化
				insertedPlane++;//面の枚数カウントをインクリメント
			}
			if(unRenderedVoxelIt[(insertedPlane-1)*2+2-1]!=2){
				dPlane=dPlane-1.0f;}
		}//if dPlane>1.0f
		//もしPの先頭ボクセル面内の前ボクセルが描画済みなら
		if(planeBegin!=2 &&  unRenderedVoxelIt[planeBegin*2+2-1]==2){
			//先頭ボクセル面をPから外す
			//	delete [] planeList[planeBegin];//ここでエラー
			planeBegin++;
		}
	}while(planeBegin!=2);//PlaneListが空ではない場合
	//delete[] planeList;
}
};
}
