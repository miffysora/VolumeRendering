
//uniform int    frontIdx;
varying vec3 v_cam_local;//オブジェクト中心から見た時のカメラの座標・位置
varying vec3 uvw;//0.0-1.0に正規化された3Dテクスチャ座標
int nSequence[64] = 
{0,1,2,3,4,5,6,7,              
1,4,5,0,3,7,2,6,
2,5,6,0,1,7,3,4,
3,0,6,4,1,2,7,5,
4,3,7,1,0,6,5,2,
5,7,2,1,4,6,0,3,
6,2,7,3,0,5,4,1,
7,6,4,5,2,3,1,0};

vec3 vecVertices[8]= 
{ {1.0,1.0,1.0},
{1.0,1.0,0.0},
{1.0,0.0,1.0},
{ 0.0,1.0,1.0},
{0.0,1.0,0.0},
{1.0,0.0,0.0},
{0.0,0.0,1.0},
{0.0,0.0,0.0}};
//横軸はe=0=red,1=blue,2=green
int v1[24] = {0,0,1,4, 1,0,1,4, 0,0,2,5, 2,0,2,5, 0,0,3,6, 3,0,3,6};
int v2[24] = {0,1,4,7, 5,1,4,7, 0,2,5,7, 6,2,5,7, 0,3,6,7, 4,3,6,7};

void main(){
  vec3 vecTranslate=vec3(-0.5,-0.5,-0.5);
	v_cam_local=vec4(gl_ModelViewMatrixInverse*vec4(0.0,0.0,0.0,1.0)).xyz;
	int frontIdx;
	if(v_cam_local.x >= 0 && v_cam_local.y >= 0 && v_cam_local.z >= 0)
		frontIdx = 0;
	else if(v_cam_local.x >= 0 && v_cam_local.y >= 0 &&  v_cam_local.z  < 0)
		frontIdx = 1;
	else if(v_cam_local.x >= 0 && v_cam_local.y < 0 &&  v_cam_local.z  >= 0)
		frontIdx = 2;
	else if(v_cam_local.x < 0 && v_cam_local.y >= 0 &&  v_cam_local.z  >= 0)
		frontIdx = 3;
	else if(v_cam_local.x < 0 && v_cam_local.y >= 0 &&  v_cam_local.z  < 0)
		frontIdx = 4;
	else if(v_cam_local.x >= 0 && v_cam_local.y < 0 &&  v_cam_local.z < 0)
		frontIdx = 5;
	else if(v_cam_local.x < 0 && v_cam_local.y < 0 &&  v_cam_local.z  >= 0)
		frontIdx = 6;
	else 
		frontIdx = 7;
	vec3 vecView=normalize(v_cam_local);
	v_cam_local+=0.5;
	 
	float dPlane = gl_Vertex.y;
	vec3 Position=vec3(0.0,0.0,0.0);//25
	for(int e = 0; e < 4; e++){//26
		int vidx1 = int(nSequence[int(frontIdx * 8 + int(v1[gl_Vertex.x * 4 +e]))]);//27
		int vidx2 = int(nSequence[int(frontIdx * 8 + int(v2[gl_Vertex.x * 4 +e]))]);//28
		vec3 vecV1 =  vecVertices[vidx1];//29
		vec3 vecV2 =  vecVertices[vidx2];//30
		vec3 vecStart = vecV1+vecTranslate;//31
		vec3 vecDir =normalize(vecV2-vecV1);//32
		float denom=dot(vecDir,vecView);//33　立方体の辺とスライス面が完全に平行なとき０になる。
		float lambda = (denom!= 0.0) ?  (dPlane-dot(vecStart,vecView))/denom : -1.0;
		//どこで迷子になるのか？
		if((lambda >= 0.0) && (lambda <= 1.0)) {
			Position = vecStart+  lambda * vecDir;
			break;
		}//if
	}//for
	gl_Position = gl_ModelViewProjectionMatrix* vec4(Position,1.0);//40
	uvw=(Position)+0.5;
	return;
};
