// フラグメントシェーダ
  uniform sampler3D voltexture;
	uniform sampler1D trans_func;
	varying vec3 vpos;//0.0-1.0に正規化された3Dテクスチャ座標
	uniform vec3 camera;
	vec3 volExtentMin=vec3(0,0,0);
	vec3 volExtentMax=vec3(1.0,1.0,1.0);
    void main() {  
		vec4 dst=vec4(0,0,0,0);
		vec3 position=vpos;
		int samplenum=800;
		vec3 direction=normalize(position-camera)*pow(3.0,0.5)/float(samplenum);//111=0.009 10=0.17
		vec4 value;
		float scalar=0;
		for(int i=0;i<samplenum;i++){//64*√3より
			if(dst.a>0.9)break;
			value=texture3D(voltexture,position);
			scalar=value.x;
			///if(scalar>0.3){
			//vec4 src=vec4(scalar,scalar,scalar,scalar*0.01);
			vec4 src=texture1D(trans_func,scalar);
			//	dst=(1.0-dst.a)*src+dst;
			//}
			//front-to-back
			dst.xyz = (1.0-dst.a) * src.a * src.xyz + dst.xyz;//薄いとこが暗くなるのが嫌。
			dst.a   = (1.0-dst.a)  * src.a   + dst.a;
			
			position=position+direction;//1/110
			vec3 temp1=sign(position-volExtentMin);
			vec3 temp2=sign(volExtentMax-position);
			float inside=dot(temp1,temp2);
			if(inside<3.0)
			break;
		}
		  gl_FragColor =dst;  
}
