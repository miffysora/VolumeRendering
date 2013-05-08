//http://paulbourke.net/geometry/polygonise/
#include <sstream>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cassert>
#include <map>
#include <vector>
using namespace std;
#include <miffy/math/vec3.h>
#include <miffy/fileformat/ply.h>
#include <miffy/fileformat/jmesh.h>
#include "marching-cube-tables.h"
using namespace miffy;
using namespace std;

typedef struct sVert{
  vec3<float> pos;
	vec3<float> normal;
	string tostring(){return pos.toString()+" "+normal.toString();}
}Vert;
typedef struct sTriangle{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	void sort(){//a,b,cを小さい順に並べる
		if(a>b){swap(a,b);}
		if(a>c){swap(a,c);}
		if(b>c){swap(b,c);}
	}
	string toString(){
		stringstream ss;
		 ss<<a<<" "<<b<<" "<<c;
		return ss.str();}
}Triangle;
map<string,unsigned int> vertex_map;///< 最終的な出力頂点達
map<string,unsigned int> triangle_map;
//vector<Triangle> triangles;

typedef struct sVoxel{
	vec3<int> index;
	vec3<float> real;//リアル座標
	unsigned char value;
	sVoxel(){}
	sVoxel(int _x,int _y,int _z,int _voxelxy,unsigned char* _voldata){
		index.set(_x,_y,_z);
		real.x=(float)index.x/(float)_voxelxy-0.5;
		real.y=(float)index.y/(float)_voxelxy-0.5;
		real.z=(float)index.z/(float)_voxelxy-0.5;//これでよかったか不確か
		value=_voldata[(index.z*_voxelxy+index.y)*_voxelxy+index.x];
	}
	
	sVoxel operator=(const sVoxel _in){
		index=_in.index;
		real=_in.real;
		value=_in.value;
		return *this;
	}
}Voxel;
vec3<float> VertexInterp(Voxel& p1,Voxel& p2,const unsigned char _threshold){//各辺に存在する、等地面と交差する点を計算する。
	float mu;
	vec3<float> p;

	if (abs((int)(_threshold-p1.value)) < 1)
		return(p1.real);
	if (abs((int)(_threshold-p2.value)) <1)
		return(p2.real);
	if (abs((int)(p1.value-p2.value)) <1)
		return(p1.real);
	mu = (float)(_threshold - p1.value) /(float) (p2.value - p1.value);
	p=p1.real+(p2.real-p1.real)*mu;
	return(p);
}
int MakeTriangle(vector<Voxel> cell,unsigned char _threshold){
				int edgeindex=0;
				vec3<float> vertlist[12];
				unsigned char cubeindex=0;//どの頂点が閾値以下だったか、記録する
				if (cell[0].value < _threshold) cubeindex |= 1;
				if (cell[1].value < _threshold) cubeindex |= 2;
				if (cell[2].value < _threshold) cubeindex |= 4;
				if (cell[3].value < _threshold) cubeindex |= 8;
				if (cell[4].value < _threshold) cubeindex |= 16;
				if (cell[5].value < _threshold) cubeindex |= 32;
				if (cell[6].value < _threshold) cubeindex |= 64;
				if (cell[7].value < _threshold) cubeindex |= 128;
				edgeindex=edgeTable[(int)cubeindex];//どこの辺が交点を持っているか
				if(edgeindex==0){return 0;}//このキューブは完全に内側、あるいは外側である
				memset(vertlist,0,sizeof(vec3<float>)*12);
				//等値面と立方体の交点を求める
				if (edgeindex & 1   ){vertlist[0 ] = VertexInterp(cell[0],cell[1],_threshold);}
				if (edgeindex & 2   ){vertlist[1 ] = VertexInterp(cell[1],cell[2],_threshold);}
				if (edgeindex & 4   ){vertlist[2 ] = VertexInterp(cell[2],cell[3],_threshold);}
				if (edgeindex & 8   ){vertlist[3 ] = VertexInterp(cell[3],cell[0],_threshold);}
				if (edgeindex & 16  ){vertlist[4 ] = VertexInterp(cell[4],cell[5],_threshold);}
				if (edgeindex & 32  ){vertlist[5 ] = VertexInterp(cell[5],cell[6],_threshold);}
				if (edgeindex & 64  ){vertlist[6 ] = VertexInterp(cell[6],cell[7],_threshold);}
				if (edgeindex & 128 ){vertlist[7 ] = VertexInterp(cell[7],cell[4],_threshold);}
				if (edgeindex & 256 ){vertlist[8 ] = VertexInterp(cell[0],cell[4],_threshold);}
				if (edgeindex & 512 ){vertlist[9 ] = VertexInterp(cell[1],cell[5],_threshold);}
				if (edgeindex & 1024){vertlist[10] = VertexInterp(cell[2],cell[6],_threshold);}
				if (edgeindex & 2048){vertlist[11] = VertexInterp(cell[3],cell[7],_threshold);}
				//三角形を作る
				for (int i=0;triTable[cubeindex][i]!=-1;i+=3) {
					string a,b,c;
					a=vertlist[triTable[cubeindex][i  ]].toString();
					b=vertlist[triTable[cubeindex][i+1]].toString();
					c=vertlist[triTable[cubeindex][i+2]].toString();
					if(a==b || b==c || c==a){continue;}//縮退三角形なのでcontinue;
					Triangle tri;//頂点インデックスの経緯を記録するため
					vertex_map.insert(map<string,unsigned int>::value_type(a,vertex_map.size()));
					vertex_map.insert(map<string,unsigned int>::value_type(b,vertex_map.size()));
					vertex_map.insert(map<string,unsigned int>::value_type(c,vertex_map.size()));
					tri.a=vertex_map[a];
					tri.b=vertex_map[b];
					tri.c=vertex_map[c];
					tri.sort();
					triangle_map.insert(map<string,unsigned int>::value_type(tri.toString(),triangle_map.size());
					//duplicateなfaceを減らすには、毎回a,b,cをソートして、そしてまたmapに入れることだ。
					//triangles.push_back(tri);
				}
				return 1;
}
void MarchingCube(const unsigned char _threshold,int _voxelxy,int _voxelz,unsigned char* _voldata){
	for(int z=0;z<_voxelz-1;z++){
		for(int y=0;y<_voxelxy-1;y++){
			for(int x=0;x<_voxelxy-1;x++){
				vector<Voxel> cell;//8つのボクセルからなる立方体。*3
				cell.push_back(Voxel(x  ,y  ,z  ,_voxelxy,_voldata));
				cell.push_back(Voxel(x+1,y  ,z  ,_voxelxy,_voldata));
				cell.push_back(Voxel(x+1,y  ,z+1,_voxelxy,_voldata));
				cell.push_back(Voxel(x  ,y  ,z+1,_voxelxy,_voldata));

				cell.push_back(Voxel(x  ,y+1,z  ,_voxelxy,_voldata));
				cell.push_back(Voxel(x+1,y+1,z  ,_voxelxy,_voldata));
				cell.push_back(Voxel(x+1,y+1,z+1,_voxelxy,_voldata));
				cell.push_back(Voxel(x  ,y+1,z+1,_voxelxy,_voldata));
				MakeTriangle(cell,_threshold);
			}
		}
	}//end zindex
}
void WriteData(const char* _filename){
	//Ply ply;
	//ply.mElementNum=6;
	//ply.mVertNum=vertex_map.size();
	//ply.mTriangleNum=triangles.size();
	ofstream ofs;
	ofs.open(_filename,ios_base::trunc);
	ofs<<"ply"<<endl;
	ofs<<"format ascii 1.0"<<endl;
	ofs<<"comment miffy output"<<endl;
	ofs<<"element vertex "<<vertex_map.size()<<endl;
	ofs<<"property float x"<<endl;
	ofs<<"property float y"<<endl;
	ofs<<"property float z"<<endl;
	ofs<<"element face "<<triangle_map.size()<<endl;
	ofs<<"property list uchar int vertex_indices"<<endl;
	ofs<<"end_header"<<endl;
	//頂点情報書き込み
	map<string,unsigned int>::iterator it=vertex_map.begin();
	vector<string> verts(vertex_map.size());//頂点をid順に並べる
	while(it!=vertex_map.end()){
		verts[it->second]=it->first;
		it++;
	}//並び替え終了
	for(int i=0;i<verts.size();i++){
		ofs<<verts[i]<<endl;
	}
	it=triangle_map.begin();
	while(it!=triangle_map.end()){
		ofs<<"3 "<<it->first<<endl;
		it++;
	}
	/*for(int i=0;i<triangle_map.size();i++){
		ofs<<"3 "<<triangles[i].a<<" "<<triangles[i].b<<" "<<triangles[i].c<<endl;
	}*/
	ofs.close();
}
void main(){
	const int nVoxel=256;
	
	for(unsigned char threshold=2;threshold<=6;threshold+=2){
		for(int f=0;f<164;f++){  
			unsigned char* voldata;
			int voxelxy,voxelz;
			stringstream ss;
			ss<<"../cappi/cappi"<<f<<".jmesh";
			jmesh::Load(ss.str().c_str(),&voxelxy,&voxelz,&voldata);
			MarchingCube(2,voxelxy,voxelz,voldata);
			ss.str("");
			ss<<"../iso"<<threshold-2<<"/ply/cappi"<<f<<".ply";
			WriteData(ss.str().c_str());
		}
	}
	cout<<"終了"<<endl;
}
