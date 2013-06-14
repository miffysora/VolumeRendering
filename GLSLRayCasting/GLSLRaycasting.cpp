#include "GLSLRaycasting.h"
#include <fstream>
#include <assert.h>
#include <miffy/gl/shaderutility.h>
#include <miffy/gl/glutility.h>
CGLSLRayCasting::CGLSLRayCasting()
  :m_threshold(0.125)
{
}
CGLSLRayCasting::CGLSLRayCasting(const string& _vertfile,const string& _fragfile)
	:m_threshold(0.0)
{
	init(_vertfile,_fragfile);
	UpdateThresholdUniform(m_threshold);
}
void CGLSLRayCasting::init(const string& _vertfile,const string& _fragfile){
	char* vertexShader;char* fragmentShader;
	ifstream is(_vertfile,ifstream::binary);
	if(!is.is_open()){assert(!"頂点シェーダファイルが存在しません。");}
	//ファイルの長さを調べる
	is.seekg (0, ios::end);
	long long filebytes= is.tellg();
	is.seekg (0, ios::beg);//先頭まで移動
	vertexShader=new char[filebytes+1];
	//is>>gVertexShader;
	is.read (vertexShader,filebytes);
	is.close();
		
	vertexShader[filebytes]='\0';//終わりだよ印をつける
	is.open(_fragfile,ifstream::binary);
	if(!is.is_open()){assert(!"フラグメントシェーダファイルが存在しません。");}
	//ファイルの長さを調べる
	is.seekg (0, ios::end);
	filebytes= is.tellg();
	is.seekg (0, ios::beg);//先頭まで移動
	fragmentShader=new char[filebytes+1];
	is.read (fragmentShader,filebytes);
	is.close();
	fragmentShader[filebytes]='\0';//終わりだよ印をつける
	m_Program=CreateShaderProgram(vertexShader,fragmentShader);
	GetLocations();
	miffy::GetGLError("GetLocations");
	UpdateThresholdUniform(m_threshold);
}
void CGLSLRayCasting::UpdateTextureUniform(){
	glUseProgram(m_Program);
	glUniform1i(m_Map["voltexture"],0);GetGLError("glTexImage3D");
	glUseProgram(0);
}
void CGLSLRayCasting::Draw(){
	glPushAttrib(GL_ENABLE_BIT);					
	glActiveTexture ( GL_TEXTURE0 );							
	glEnable( GL_TEXTURE_3D );
	//データはちゃんと変わってるみたい？？？要チェック
	//glBindTexture ( GL_TEXTURE_3D,mProgram->m_TexId );
	glUseProgram(m_Program);
	glUniform1f(m_Map["threshold"],m_threshold);							
	//glPushMatrix();
	//glTranslatef(0.0,0.0,0.5);
	glutSolidCube(1.0);
	//glPopMatrix();
	glUseProgram(0);
	glPopAttrib();//light off
	
}
void CGLSLRayCasting::GetLocations(){
	string names[]={"voltexture","threshold"};
	for(int i=0;i<sizeof(names)/sizeof(names[0]);i++){
		m_Map.insert(pair<string,int>(names[i],glGetUniformLocation(m_Program,names[i].c_str())));
		GetGLError("glTexImage3D");
	}
	
}


CGLSLRayCasting::~CGLSLRayCasting(void)
{
}

void CGLSLRayCasting::UpdateThresholdUniform(float _threshold){//最初しか使わない。
	m_threshold=_threshold;
	glUseProgram(m_Program);GetGLError("glTexImage3D");
	glUniform1f(m_Map["threshold"],_threshold);GetGLError("glTexImage3D");
	glUseProgram(0);
}
