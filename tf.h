#ifndef MIFFY_TRANSFER_FUNCTION
#define MIFFY_TRANSFER_FUNCTION
/*! @file tf.h ボリュームデータを虹色に色づけるための関数
*/
template <typename T>
static color<float> inline tf(const T _data,T _dataMax,T _threshold){
  if(_data<_threshold){return color<float>(0.0f,0.0f,0.0f,0.0f);}
	if(_data>=_dataMax){return color<float>(1.0f,0.0f,0.0f,1.0f);}//赤
	T maxval=std::numeric_limits<T>::max();
	//_threshold～maxvalの範囲で青～赤に滑らかに変化するようにする。
	//180(青)～360(赤)に変化させたい。
	//青～赤にするため、180を掛け算する。
	double percentage=(double)(_data-_threshold)/(double)(_dataMax-_threshold);
	
	double hue=(1.0-percentage)*240.0;
	hue=fmod(hue,360.0);//360を超えないためのセーフティーネット
	return color<float>((unsigned int)hue,255,255);
}
/*!
	@brief 虹色彩色ボリュームデータを地面に投影するテクスチャを生成する
	ファイルの場所的に、この関数をここに置いていいのか微妙。
	ていうか引数多すぎかも。
	@param _src ボリュームデータ　強度だけ。
	@param _xy 投影面の面積（１辺じゃないことに注意）
	@param _dst アウトプットデータ。虹色彩色され、地面に投影されたボリュームデータになる。
	@param _dataMax この値以上は赤い色にしたい、という値
	@param _theshold この値以下は透明にしたい。　この値が青になる。
*/
template <typename T>
static void GenerateProjectedRainbow(T* _src,int _xy,int _z,T _dataMax,T _threshold,color<float>*& _dst){
	_dst=new color<float>[_xy];
	unsigned char* projectedIntensity=new unsigned char[_xy];//地面に投影された信号強度。これを元に虹色に彩色する。
	memset(projectedIntensity,0,_xy);
	unsigned char current_intensity;
	for(int z=0;z<_z;z++){
		for(int i=0;i<_xy;i++){
			current_intensity=_src[z*_xy+i];
			if(projectedIntensity[i]<current_intensity){//見栄えを良くするため最大値をとっている
				projectedIntensity[i]=current_intensity;//今までのより大きい値であれば更新
				_dst[i]=tf(current_intensity,_dataMax,_threshold);
			}//大きいかどうかの調査終わり。
		}
	}
	delete[] projectedIntensity;
}
#endif
