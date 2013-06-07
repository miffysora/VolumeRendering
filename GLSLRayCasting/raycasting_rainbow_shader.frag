vec3 shading(vec3 _normal,vec3 _viewvec,vec3 _lightpos){

  float diffuse_intensity=max(dot(_lightpos,_normal),0);//光源の拡散反射強度
	vec3 diffuse=gl_FrontLightProduct[0].diffuse*diffuse_intensity;

	if(diffuse_intensity<=0){//もし光の方向ベクトル・法線の内積が0以下だったら鏡面反射は発生しない。
		return /*gl_FrontLightProduct[0].ambient+*/diffuse;
	}else{//光の方向ベクトル・法線の内積が0より大きい場合だけ鏡面反射が発生する
		vec3 reflection = normalize(_lightpos+_viewvec);//プラスとマイナスでどう違うのかわからない。
		float specular_intensity=pow(max(dot(reflection,_normal),0.3),gl_FrontMaterial.shininess);
		vec3 specular=gl_FrontLightProduct[0].specular*specular_intensity;
		return /*gl_FrontLightProduct[0].ambient+*/diffuse+specular;
	}
}
#define DELTA (0.01)
#define THRESHOLD (0.25)
// フラグメントシェーダ
uniform sampler3D voltexture;
varying vec3 uvw;//0.0-1.0に正規化された3Dテクスチャ座標
vec3 volExtentMin=vec3(0,0,0);
vec3 volExtentMax=vec3(1.0,1.0,1.0);
//光関係
varying vec3 v_lightvec;
varying vec3 v_cam_local;

void main() {  
	vec4 dst=vec4(0.0,0.0,0.0,0);
	int samplenum=800;
	vec3 direction=normalize(uvw-v_cam_local)*pow(3.0,0.5)/float(samplenum);//111=0.009 10=0.17
	vec4 value;
	float scalar=0;
	for(int i=0;i<samplenum;i++){//64*√3より
		if(dst.a>0.9)break;
		vec4 src=texture3D(voltexture,uvw);
		//src.a=src.x;//なんとなくうまくいかないからこうした。
		scalar=src.x*0.05;//GL_LUMINANCEで渡すとxに強度が入る
		if(src.x<THRESHOLD){
			src=vec4(0.0,0.0,0.0,0.0);
		}else{
			vec3 sample1,sample2;
			sample1.x=texture3D(voltexture,uvw-vec3(DELTA,0.0,0.0)).x;
			sample2.x=texture3D(voltexture,uvw+vec3(DELTA,0.0,0.0)).x;
			sample1.y=texture3D(voltexture,uvw-vec3(0.0,DELTA,0.0)).x;
			sample2.y=texture3D(voltexture,uvw+vec3(0.0,DELTA,0.0)).x;
			sample1.z=texture3D(voltexture,uvw-vec3(0.0,0.0,DELTA)).x;
			sample2.z=texture3D(voltexture,uvw+vec3(0.0,0.0,DELTA)).x;
			vec3 normal = sample2-sample1;
			float diff=normal.length();//最大で1かなぁ
			normal=normalize(gl_NormalMatrix*normal);//
			vec3 lightvec = normalize(gl_LightSource[0].position.xyz  - uvw);
			vec3 viewvec = normalize(v_cam_local - uvw);
			float phase=(dot(lightvec,viewvec)+1.0)*0.5;//dotの結果は±1.0なので0.0-1.0の範囲になるように正規化する
			//src=vec4(phase,phase,phase,1.0);
			float hue=((0.65-src.x)/0.45)*240.0;
			int grid = (int)floor(hue/60.0);//6通りにわかれる
			float dVariant =(hue/ 60.0) - (float)grid;//区間スタートからどのぐらい離れているのか
			if( !(grid & 1)){ dVariant = 1.0 - dVariant;} //inが偶数ならだんだん上がる
			float dChanging = ( 1.0 -dVariant );//だんだん上がるor 下がる要素
			//scalar*=diff;
         vec4 c;
			switch( grid ){
           //赤=常に255,緑=だんだん上に上がる状態
           case 0: c =vec4(1.0,dChanging ,0.0,scalar); break;
           case 1: c =vec4(dChanging,1.0,0,scalar); break;
		   case 2: c =vec4(0.0,1.0, dChanging ,scalar); break;
           case 3: c =vec4(0.0,dChanging,1.0,scalar ) ; break;
           case 4: c =vec4(dChanging,0.0,1.0,scalar) ; break;
           case 5: c =vec4(1.0,0.0,dChanging,scalar); break;
  
        }
			src=c+vec4(shading(normal,viewvec,lightvec),0.0);
			src.a*=diff;
			//src.xyz*=phase;
			//src=vec4(shading(normal,viewvec,lightvec),scalar);
		}
		//front-to-back
		dst.xyz = (1.0-dst.a) * src.a * src.xyz + dst.xyz;//薄いとこは明るくなってほしい
		dst.a   = (1.0-dst.a)  * src.a   + dst.a;
		uvw=uvw+direction;//1/110 レイに沿って進める
		//back to front
		//dst.rgb = (1.0-src.a) * dst.rgb + src.a*src.rgb;
        //dst.a   = (1.0-src.a)  * dst.a   + src.a;
		//uvw=uvw-direction;
		vec3 temp1=sign(uvw-volExtentMin);
		vec3 temp2=sign(volExtentMax-uvw);
		float inside=dot(temp1,temp2);
		if(inside<3.0)
			break;
	}
	gl_FragColor =dst;  

}
