#ifndef MIFFYVOLDATA
#define MIFFYVOLDATA
/*!
  @file voldata.h ３次元ボリュームデータを作る段階で使う関数
*/
namespace miffy{
		template <typename T>
		void WriteVolData(const char* _filename,int _size,T* _data){
			FILE* fp;
			fp=fopen(_filename,"wb");
			fwrite(_data,sizeof(T),_size*_size*_size,fp);
			fclose(fp);
			memset(_data,_size*_size*_size,sizeof(T));//終わったら0にしてあげる
	
		}
		template <typename T>
		void WriteVolData(const char* _filename,int _size,int _sizez,T* _data){
			FILE* fp;
			fp=fopen(_filename,"wb");
			fwrite(_data,sizeof(T),_size*_size*_sizez,fp);
			fclose(fp);
			memset(_data,_size*_size*_sizez,sizeof(T));//終わったら0にしてあげる
	
		}

	//zだけ補間して引き伸ばしてくれるという関数
	template <typename T>
	void ZInterpolate(T* _in,int _size,int _sizez,int _outz,T* _output){
		printf("zの線形補間\n");//昔はMakeVoldataInterpolateという名前だった。
		//隙間を線形補間して埋める。
		int zinterval=_size/(_sizez-1);
		for(int z=0;z<_outz;z++){//zはZINTERVAL個とばしずつ
			int redundant=z%(zinterval);//あまり
			int quotient=z/(zinterval);//商
			if(redundant!=0){//補間対象
				float position=(float)redundant/(float)(zinterval);//このzの相対的な位置（beginに近いほど0,endに近いほど1）

				for(int y=0;y<_size;y++){
					for(int x=0;x<_size;x++){
						int index=(z*_size+y)*_size+x;//出力先
						float begindata=(float)_in[(quotient*_size+y)*_size+x];
						float enddata;
						if(quotient==_sizez-1){//商が終わりの端っこなら
							enddata=0;
						}else{
							enddata=(float)_in[((quotient+1)*_size+y)*_size+x];
						}
						_output[index]=(T)(begindata*(1.0f-position)+(enddata*position));
					}
				}
			}else{//補間対象じゃなかったら、普通に１枚埋める。
					memcpy(&_output[z*_size*_size],&_in[quotient*_size*_size],_size*_size*sizeof(T));
				
			}
		}//補間して膨らませ作業終わり。

	}
	template <typename T>
	void MakeVoldataBSplineZInterpolate(T* _in,int _size,int _inz,int _outz,T* _output){
		printf("B-Spline補間\n");//隙間を線形補間して埋める。(z方向だけ
		int zinterval=_size/(_inz);//切り捨てで zinterval*±2がフィルタの幅になる
		int redundant=_outz-zinterval*(_inz-1);
		float w[4];//補間に使う４つのオリジナルデータ値
		float wd[4];//４つのデータ値と補間場所との距離
		float coef[4];
		
		//最初の数枚(w0が存在しない部分)　雲らしく0で補完するか。
		for(int z=0;z<zinterval;z++){//zはZINTERVAL個とばしずつ
			memcpy(&_output[z*_size*_size],&_in[0],sizeof(T)*_size*_size);
			/*w[0]=0;	
			wd[1]=z;	wd[0]=1+wd[1]+1;	wd[2]=1-wd[1];	wd[3]=2-wd[1];
			coef[1]=wd[1]*wd[1]*(0.5*wd[1]-1.0)+2.0/3.0;
			coef[2]=wd[2]*wd[2]*(0.5*wd[2]-1.0)+2.0/3.0;
			coef[3]=wd[3]*(wd[3]*(-1.0/6.0*wd[3]+1.0)-2.0)+4.0/3.0;
				for(int y=0;y<_size;y++){
					for(int x=0;x<_size;x++){
						w[1]=(float)_in[(y)*_size+x];		w[2]=(float)_in[(_size+y)*_size+x];	w[3]=(float)_in[(2*_size+y)*_size+x];
						_output[(z*_size+y)*_size+x]=(unsigned char)(coef[1]*w[1]+coef[2]*w[2]+coef[3]*w[3]);
					}
				}*/
		}
		//端っこじゃない部分の補間
		for(int z=zinterval;z<_outz-redundant-zinterval;z++){//zはZINTERVAL個とばしずつ
			int w1index=z/zinterval;	
			
			wd[1]=(float)(z-w1index*zinterval)/(float)zinterval;	wd[0]=1.0+wd[1];	wd[2]=1.0-wd[1];	wd[3]=2.0-wd[1];
			coef[0]=wd[0]*(wd[0]*(-1.0/6.0*wd[0]+1.0)-2.0)+4.0/3.0;
			coef[1]=wd[1]*wd[1]*(0.5*wd[1]-1.0)+2.0/3.0;
			coef[2]=wd[2]*wd[2]*(0.5*wd[2]-1.0)+2.0/3.0;
			coef[3]=(float)1.0-coef[0]-coef[1]-coef[2];
			/**if(coef[0]+coef[1]+coef[2]+coef[3]!=1.0){
				//coefが1個でも2.0/3.0より大きくなってたら間違いよね
				printf("coef sum=%f(%f,%f,%f,%f)\n",coef[0]+coef[1]+coef[2]+coef[3],coef[0],coef[1],coef[2],coef[3]);
				assert(!"error!!");
			}*/
				for(int y=0;y<_size;y++){
					for(int x=0;x<_size;x++){
						w[0]=(float)_in[((w1index-1)*_size+y)*_size+x];
						w[1]=(float)_in[ (w1index   *_size+y)*_size+x];		
						w[2]=(float)_in[((w1index+1)*_size+y)*_size+x];	
						w[3]=(float)_in[((w1index+2)*_size+y)*_size+x];
						_output[(z*_size+y)*_size+x]=(unsigned char)(coef[0]*w[0]+coef[1]*w[1]+coef[2]*w[2]+coef[3]*w[3]);
					}
				}
		}
		//終端部分の補間
		
		for(int z=_outz-redundant-zinterval;z<_outz-redundant;z++){//zはZINTERVAL個とばしずつ
			//memcpy(&_output[z*_size*_size],&_in[18*_size*_size],sizeof(T)*_size*_size);
			int w1index=_inz-2;	
			wd[1]=(float)(z-w1index*zinterval)/(float)zinterval;	wd[0]=1.0+wd[1]; wd[2]=1.0-wd[1];
			coef[0]=wd[0]*(wd[0]*(-1.0/6.0*wd[0]+1.0)-2.0)+4.0/3.0;
			coef[1]=wd[1]*wd[1]*(0.5*wd[1]-1.0)+2.0/3.0;
			coef[2]=wd[2]*wd[2]*(0.5*wd[2]-1.0)+2.0/3.0;
				for(int y=0;y<_size;y++){
					for(int x=0;x<_size;x++){
						w[0]=(float)_in[((w1index-1)*_size+y)*_size+x];
						w[1]=(float)_in[ (w1index   *_size+y)*_size+x];
						w[2]=(float)_in[((w1index+1)*_size+y)*_size+x];
						_output[(z*_size+y)*_size+x]=(unsigned char)(coef[0]*w[0]+coef[1]*w[1]+coef[2]*w[2]);
					}
				}
		}//補間して膨らませ作業終わり。
		for(int z=_outz-redundant;z<_outz;z++){//zはZINTERVAL個とばしずつ
			//memcpy(&_output[z*_size*_size],&_in[18*_size*_size],sizeof(T)*_size*_size);
			int w1index=_inz-1;	
			wd[1]=(float)(z-w1index*zinterval)/(float)zinterval;	wd[0]=1.0+wd[1];
			coef[0]=wd[0]*(wd[0]*(-1.0/6.0*wd[0]+1.0)-2.0)+4.0/3.0;
			coef[1]=wd[1]*wd[1]*(0.5*wd[1]-1.0)+2.0/3.0;
			
				for(int y=0;y<_size;y++){
					for(int x=0;x<_size;x++){
						w[0]=(float)_in[((w1index-1)*_size+y)*_size+x];
						w[1]=(float)_in[ (w1index   *_size+y)*_size+x];
						
						_output[(z*_size+y)*_size+x]=(unsigned char)(coef[0]*w[0]+coef[1]*w[1]);
					}
				}
		}//補間して膨らませ作業終わり。

	}
	//zだけでなくすべての方向の補間をしてくれる関数
	template <typename T>
	void TrilinearInterpolationVector(T* _in,int _sizexy,int _outxy,int _sizez,int _outz,T* _output){
union FloatInt
{
    float f;
    int i;
};
#define GETIN3D(_x,_y,_z) ((_z)*_sizexy+(_y))*_sizexy+(_x)
#define GETOUT3D(_x,_y,_z) ((_z)*_outxy+(_y))*_outxy+(_x)

		printf("補間\n");
		//隙間を線形補間して埋める。
		int xyinterval=_outxy/(_sizexy-1);//|■□□□□□□||■□□□□□□...
		int zinterval=_outz/(_sizez-1);
		T xyincrement=1.0/(T)xyinterval;
		T zincrement=1.0/(T)zinterval;
						
		for(int z=0;z<_sizez-1;z++){//zはZINTERVAL個とばしずつ
				for(int y=0;y<_sizexy-1;y++){
					int redundanty=y%(xyinterval);
					int quatienty=y/(xyinterval);
					
					for(int x=0;x<_sizexy-1;x++){
						
						    //  h-------g
							// /|      / |
							//d--------c |
							//| |      | |
							//| e------| /f
							//|/       |/
							//a--------b
						//abcdefg=８近傍
						for(int i=0;i<3;i++){
							float a=_in[(GETIN3D(x,y,z))*3+i];	float b=_in[(GETIN3D((x+1),y,z))*3+i];	float c=_in[(GETIN3D((x+1),(y+1),z))*3+i];	float d=_in[(GETIN3D(x,(y+1),z))*3+i];
							float e=_in[(GETIN3D(x,y,(z+1)))*3+i];float f=_in[(GETIN3D((x+1),y,(z+1)))*3+i];float g=_in[(GETIN3D((x+1),(y+1),(z+1)))*3+i];float h=_in[(GETIN3D(x,(y+1),(z+1)))*3+i];
							int outx=x*xyinterval;
							int outy=y*xyinterval;
							int outz=z*zinterval;
							int outindex=(GETOUT3D(outx,outy,outz))*3+i;
							voldata[outindex]=a;//最初の一個をそのまま埋める
							
							for(T iz=0.0;iz<1.0;iz+=zincrement){
								for(T iy=0.0;iy<1.0;iy+=xyincrement){
									for(T ix=0.0;ix<1.0;ix+=xyincrement){
										//float posx=;//このxの相対的な位置（beginに近いほど0,endに近いほど1）
										//abcd面でバイリニア補間した結果
										T abcd=(1.0-ix)*(1.0-iy)*a+(1-ix)*iy*d*ix*(1.0-iy)*b+ix*iy*c;
										//efgh面でバイリニア補間した結果
										T efgh=(1.0-ix)*(1.0-iy)*e+(1-ix)*iy*f*ix*(1.0-iy)*g+ix*iy*h;
										int outx=x*xyinterval+(int)(ix*(float)xyinterval);
										int outy=y*xyinterval+(int)(iy*(float)xyinterval);
										int outz=z*zinterval+(int)(iz*(float)zinterval);
										int outindex=(GETOUT3D(outx,outy,outz))*3+i;
										if(outindex<=_outxy*_outxy*_outz*3){
											_output[outindex]=(1.0-iz)*abcd+iz*efgh;
											if(_output[outindex]>10E+27){
												printf("%f,%f,iz=%f\n",abcd,efgh,iz);
											}
										}
										//printf("xyz(%f)\n",ix*(float)xyinterval);
										if(outx==84 && outy==72 && outz==36)
										{
											FloatInt fi;
											fi.f=voldata[outindex];
											bitset<32> bits(fi.i);
											printf("o%f,%x\n",_output[outindex],_output[outindex]);
											cout<<bits<<endl;
										}
									}//end of i
								}//end inner x
							}//end inner y
						}//end inner z
					}//end of x
				}//end of y
			
		}//end of z
		//補間して膨らませ作業終わり。
#undef GETIN3D
#undef GETOUT3D
	}
	//zだけでなくすべての方向の補間をしてくれる関数
	template <typename T>
	void TrilinearInterpolation(T* _in,int _sizexy,int _outxy,int _sizez,int _outz,T* _output){
#define GETIN3D(_x,_y,_z) (_z*_sizexy+_y)*_sizexy+_x
#define GETOUT3D(_x,_y,_z) (_z*_outxy+_y)*_outxy+_x

		printf("補間\n");
		//隙間を線形補間して埋める。
		int xyinterval=_outxy/(_sizexy-1);//|■□□□□□□||■□□□□□□...
		int zinterval=_outz/(_sizez-1);
		T xyincrement=1.0/(T)xyinterval;
		T zincrement=1.0/(T)zinterval;
						
		for(int z=0;z<_sizez;z++){//zはZINTERVAL個とばしずつ
				for(int y=0;y<_sizexy;y++){
					int redundanty=y%(xyinterval);
					int quatienty=y/(xyinterval);
					if(redundanty!=0){}
					for(int x=0;x<_sizexy;x++){
						
						    //  h-------g
							// /|      / |
							//d--------c |
							//| |      | |
							//| e------| /f
							//|/       |/
							//a--------b
						//abcdefg=８近傍
							T a=_in[(GETIN3D(x,y,z))];	T b=_in[(GETIN3D((x+1),y,z))];	T c=_in[(GETIN3D((x+1),(y+1),z))];	T d=_in[(GETIN3D(x,(y+1),z))];
							T e=_in[GETIN3D(x,y,(z+1))];T f=_in[GETIN3D((x+1),y,(z+1))];T g=_in[GETIN3D((x+1),(y+1),(z+1))];T h=_in[GETIN3D(x,(y+1),(z+1))];
							_output[GETOUT3D((z*zinterval),(y*xyinterval),(x*xyinterval))]=a;						//最初の一個をそのまま埋める
							for(T iz=0.0;iz<1.0;iz+=zincrement){
								for(T iy=0.0;iy<1.0;iy+=xyincrement){
									for(T ix=0.0;ix<1.0;ix+=xyincrement){
										//float posx=;//このxの相対的な位置（beginに近いほど0,endに近いほど1）
										//abcd面でバイリニア補間した結果
										T abcd=(1.0-ix)*(1.0-iy)*a+(1-ix)*iy*d*ix*(1.0-iy)*b+ix*iy*c;
										//efgh面でバイリニア補間した結果
										T efgh=(1.0-ix)*(1.0-iy)*e+(1-ix)*iy*f*ix*(1.0-iy)*g+ix*iy*h;
										_output[GETOUT3D((int)(ix*xyinterval),(int)(iy*xyinterval),(int)(iz*zinterval))]=(1.0-iz)*abcd+iz*efgh;
								}//end inner x
							}//end inner y
						}//end inner z
					}//end of x
				}//end of y
			
		}//end of z
		//補間して膨らませ作業終わり。
#undef GETIN3D
#undef GETOUT3D
	}

	template <typename T>
	void GeneratLowResoData(int _highsize,int _highsizez,T* _data){
		int lowsize=_highsize>>1;
		int lowsizez=_highsizez>>1;
		for(int z=0;z<lowsizez;z++){
			for(int y=0;y<lowsize;y++){
				for(int x=0;x<lowsize;x++){
					float sum=0;
					for(int inner_z=z*2;inner_z<z*2+2;inner_z++){
						for(int inner_y=y*2;inner_y<y*2+2;inner_y++){
							for(int inner_x=x*2;inner_x<x*2+2;inner_x++){
								sum+=(float)_data[(inner_z*_highsize+inner_y)*_highsize+inner_x]/8.0f;
							}
						}
					}//end inner
					_data[(z*lowsize+y)*lowsize+x]=(T)sum;
				}
			}
		}
		
	}
	template <typename T>
	void GeneratLowResoData(int _highsize,int _highsizez,T* _data,int _stride){
		int lowsize=_highsize>>1;
		int lowsizez=_highsizez>>1;
		for(int z=0;z<lowsizez;z++){
			for(int y=0;y<lowsize;y++){
				for(int x=0;x<lowsize;x++){
					float *sum=new float[_stride];
					memset(sum,0,sizeof(float)*_stride);
					for(int inner_z=z*2;inner_z<z*2+2;inner_z++){
						for(int inner_y=y*2;inner_y<y*2+2;inner_y++){
							for(int inner_x=x*2;inner_x<x*2+2;inner_x++){
								for(int s=0;s<_stride;s++){
									sum[s]+=(float)_data[((inner_z*_highsize+inner_y)*_highsize+inner_x)*_stride+s]/8.0f;
								}
							}
						}
					}//end inner
					for(int s=0;s<_stride;s++){
						_data[((z*lowsize+y)*lowsize+x)*_stride+s]=(T)sum[s];
					}
				}
			}
		}
		
	}
	template <typename T>
	void GenerateLowResoData(int _numlevels,T* _sheeddata,int _sheedsize,int _sheedsizez,const char* _dir,const char* _name,int _stride){
			int lowersize=_sheedsize;	
			int lowersizez=_sheedsizez;
			miffysstream outname;
			for(int i=0;i<_numlevels;i++){
				GeneratLowResoData(lowersize,lowersizez,_sheeddata,_stride);
				lowersize=lowersize>>1;
				lowersizez=lowersizez>>1;
				if(lowersize==0 || lowersizez==0){break;}
				outname.str("");
				outname<<_dir<<_name<<lowersize<<"_z"<<lowersizez<<".raw";
				cout<<outname.str()<<endl;
				WriteVolData(outname.str().c_str(),lowersize,lowersizez,_sheeddata);	
				}

	}
	template <typename T>
	void GenerateLowResoData(int _numlevels,T* _sheeddata,int _sheedsize,int _sheedsizez,const char* _dir,const char* _name){
			int lowersize=_sheedsize;	
			int lowersizez=_sheedsizez;
			miffysstream outname;
			for(int i=0;i<_numlevels;i++){
				GeneratLowResoData(lowersize,lowersizez,_sheeddata);
				lowersize=lowersize>>1;
				lowersizez=lowersizez>>1;
				if(lowersize==0 || lowersizez==0){break;}
				outname.str("");
				outname<<_dir<<_name<<lowersize<<"_z"<<lowersizez<<".raw";
				cout<<outname.str()<<endl;
				WriteVolData(outname.str().c_str(),lowersize,lowersizez,_sheeddata);	
				}


	}
	template <typename T>
	void GeneratXYLowResoData(int _highsize,T* _src,T* _dest,int _znum){//XY方向のみ低解像にする
		int lowsize=_highsize>>1;
		for(int z=0;z<_znum;z++){
			for(int y=0;y<lowsize;y++){
				for(int x=0;x<lowsize;x++){
					float sum=0;
						for(int inner_y=y*2;inner_y<y*2+2;inner_y++){
							for(int inner_x=x*2;inner_x<x*2+2;inner_x++){
								sum+=(float)_src[(z*_highsize+inner_y)*_highsize+inner_x]/4.0f;
							}
						}//end inner
					_dest[(z*lowsize+y)*lowsize+x]=(T)sum;
				}
			}
		}
		
	}
	template <typename T>
	void GeneratXYLowResoData(int _highsize,int _znum,T* _src){//XY方向のみ低解像にする
		int lowsize=_highsize>>1;
		for(int z=0;z<_znum;z++){
			for(int y=0;y<lowsize;y++){
				for(int x=0;x<lowsize;x++){
					float sum=0;
						for(int inner_y=y*2;inner_y<y*2+2;inner_y++){
							for(int inner_x=x*2;inner_x<x*2+2;inner_x++){
								sum+=(float)_src[(z*_highsize+inner_y)*_highsize+inner_x]/4.0f;
							}
						}//end inner
					_src[(z*lowsize+y)*lowsize+x]=(T)sum;
				}
			}
		}
		
	}
	template <typename T>
	void GeneratXYLowResoData(int _numlevels,T* _sheeddata,int _sheedsize,int _sheedsizez,const char* _dir,const char* _name){//XY方向のみ低解像にする
		int lowersize=_sheedsize;	
			miffysstream outname;
			for(int i=0;i<_numlevels;i++){
				GeneratXYLowResoData(lowersize,_sheedsizez,_sheeddata);
				lowersize=lowersize>>1;
				if(lowersize==0){break;}
				outname.str("");
				outname<<_dir<<_name<<lowersize<<"_z"<<_sheedsizez<<".raw";
				cout<<outname.str()<<endl;
				WriteVolData(outname.str().c_str(),lowersize,_sheedsizez,_sheeddata);	
				}

	}
	template <typename T>
	void GeneratLowResoData(int _highsize,T* _data){
		int lowsize=_highsize>>1;
		for(int z=0;z<lowsize;z++){
			for(int y=0;y<lowsize;y++){
				for(int x=0;x<lowsize;x++){
					float sum=0;
					for(int inner_z=z*2;inner_z<z*2+2;inner_z++){
						for(int inner_y=y*2;inner_y<y*2+2;inner_y++){
							for(int inner_x=x*2;inner_x<x*2+2;inner_x++){
								sum+=(float)_data[(inner_z*_highsize+inner_y)*_highsize+inner_x]/8.0f;
							}
						}
					}//end inner
					_data[(z*lowsize+y)*lowsize+x]=(T)sum;
				}
			}
		}
		
	}
	template <typename T>
	void GenerateLowResoData(int _numlevels,T* _sheeddata,int _sheedsize,const char* _dir,const char* _name){
			int lowersize=_sheedsize;	
			miffysstream outname;
			for(int i=0;i<_numlevels;i++){
				GeneratLowResoData(lowersize,_sheeddata);
				lowersize=lowersize>>1;
				if(lowersize==0){break;}
				outname.str("");
				outname<<_dir<<_name<<lowersize<<".raw";
				cout<<outname.str()<<endl;
				WriteVolData(outname.str().c_str(),lowersize,_sheeddata);	
				}

	}	
	void WriteZShrinkVolData(const char* _filename,int _size,int _zsize,unsigned char* _data){
		FILE* fp;
		fp=fopen(_filename,"wb");
		fwrite(_data,sizeof(unsigned char),_size*_size*_zsize,fp);
		fclose(fp);
		memset(_data,_size*_size*_zsize,sizeof(unsigned char));//終わったら0にしてあげる
	}
	void WriteZShrinkVolData(const char* _filename,int _size,int _zsize,short* _data){
		FILE* fp;
		fp=fopen(_filename,"wb");
		fwrite(_data,sizeof(short),_size*_size*_zsize,fp);
		fclose(fp);
		memset(_data,_size*_size*_zsize,sizeof(short));//終わったら0にしてあげる
	}
	void WriteZShrinkVolData(const char* _filename,int _sweep,int _binnum,int _zsize,short* _data){
		FILE* fp;
		fp=fopen(_filename,"wb");
		fwrite(_data,sizeof(short),_sweep*_binnum*_zsize,fp);
		fclose(fp);
		memset(_data,_sweep*_binnum*_zsize,sizeof(short));//終わったら0にしてあげる
	}
}
#endif
