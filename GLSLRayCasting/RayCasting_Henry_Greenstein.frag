vec3 shading(vec3 _normal,vec3 _viewvec,vec3 _lightpos,float _phase){

  float diffuse_intensity=max(dot(_lightpos,_normal),0);//光源の拡散反射強度
	vec3 diffuse=gl_FrontLightProduct[0].diffuse*diffuse_intensity;

	if(diffuse_intensity<=0){//もし光の方向ベクトル・法線の内積が0以下だったら鏡面反射は発生しない。
		return gl_FrontLightProduct[0].ambient*_phase+diffuse;
	}else{//光の方向ベクトル・法線の内積が0より大きい場合だけ鏡面反射が発生する
		vec3 reflection = normalize(_lightpos+_viewvec);//プラスとマイナスでどう違うのかわからない。
		float specular_intensity=pow(max(dot(reflection,_normal),0.3),gl_FrontMaterial.shininess);
		vec3 specular=gl_FrontLightProduct[0].specular*specular_intensity;
		return gl_FrontLightProduct[0].ambient*_phase+diffuse+specular;
	}
}
#define DELTA (0.01)

// フラグメントシェーダ
uniform sampler3D voltexture;
uniform float threshold;
varying vec3 uvw;//0.0-1.0に正規化された3Dテクスチャ座標
vec3 volExtentMin=vec3(0,0,0);
vec3 volExtentMax=vec3(1.0,1.0,1.0);
//光関係
//varying vec3 v_lightvec;
varying vec3 v_cam_local;
float M_PI = 3.14159265358979323846264;
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
		scalar=src.x;//*0.05;//GL_LUMINANCEで渡すとxに強度が入る
		if(src.x<threshold){
			src=vec4(0.0,0.0,0.0,0.0);
		}else{
			vec3 sample1,sample2;
			sample1.x=texture3D(voltexture,uvw-vec3(DELTA,0.0,0.0)).x;
			sample2.x=texture3D(voltexture,uvw+vec3(DELTA,0.0,0.0)).x;
			sample1.y=texture3D(voltexture,uvw-vec3(0.0,DELTA,0.0)).x;
			sample2.y=texture3D(voltexture,uvw+vec3(0.0,DELTA,0.0)).x;
			sample1.z=texture3D(voltexture,uvw-vec3(0.0,0.0,DELTA)).x;
			sample2.z=texture3D(voltexture,uvw+vec3(0.0,0.0,DELTA)).x;
			vec3 normal =normalize(sample2-sample1);
			
			
			vec3 lightvec = normalize(uvw-gl_LightSource[0].position.xyz);
			vec3 viewvec = normalize(v_cam_local - uvw);
			//位相関数の実装
            //float g=((src.x-THRESHOLD)/0.45)*2.0-1.0;
            float g=((0.65-src.x)/0.45)*2.0-1.0;//±1.0に正規化 gには1-alphaを入れたい
            float maxdenom=1.0;
            if(g>0.0){
                maxdenom=(pow((4.0*M_PI)*(1.0+g*g-2.0*g),1.5));
            }
            else{
                maxdenom=(pow((4.0*M_PI)*(1.0+g*g+2.0*g),1.5));
            }
            float maxval=1.0;
            if(maxdenom!=0){
                maxval=(1.0-g*g)/maxdenom;
            }
            if(maxval==0){maxval=1.0;}
            float theta=(dot(lightvec,viewvec)+1.0)*0.5*2.0*M_PI;//dotの結果は±1.0なので0.0-1.0の範囲になるように正規化する
            float denom=pow((4*M_PI)*(1.0+g*g-2.0*g*cos(theta)),1.5);
            float phase=0.0;
            if(denom!=0){
                phase=(1.0-g*g)/denom;}
            else{
                phase=0;
			}
            phase=phase/maxval;
            phase=1.0-phase;
			src=vec4(gl_FrontLightProduct[0].ambient.rgb+phase*0.5,scalar);
			//src=vec4(shading(normal,viewvec,lightvec,phase),scalar);
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
