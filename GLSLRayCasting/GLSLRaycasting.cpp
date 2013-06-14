
#include "GLSLRaycasting.h"



CGLSLRayCasting::CGLSLRayCasting()
	:m_threshold(0.125)
{
	m_boundingbox.set(vec3<float>(0.0,0.0,0.0),0.5);
}
CGLSLRayCasting::CGLSLRayCasting(const string& _vertfile,const string& _fragfile)
	:m_threshold(0.0)
{
	init(_vertfile,_fragfile);
	UpdateThresholdUniform(m_threshold);
}
void CGLSLRayCasting::init(const string& _vertfile,const string& _fragfile){
	
	m_SimpleVertexShader = CreateShaderFromFile(_vertfile);
	
	m_FragmentShader = CreateShaderFromFile(_fragfile);
	m_Program = glCreateProgram();
	glAttachShader(m_Program, m_SimpleVertexShader);
	glAttachShader(m_Program, m_FragmentShader);GetGLError("glTexImage3D");
	glLinkProgram(m_Program);
	string names[]={"voltexture","threshold"};
	for(int i=0;i<sizeof(names)/sizeof(names[0]);i++){
		m_Map[0].insert(pair<string,int>(names[i],glGetUniformLocation(m_Program,names[i].c_str())));
		
	}GetGLError("glTexImage3D");
	glUseProgram(m_Program);
	glUniform1f(m_Map[0]["threshold"],m_threshold);
	glUniform1i(m_Map[0]["voltexture"],0);
	glUseProgram(0);
	m_SliceVertexShader= CreateShaderFromFile("slice.vert");
	m_SliceProgram = glCreateProgram();GetGLError("glTexImage3D");
	glAttachShader(m_SliceProgram, m_SliceVertexShader);GetGLError("glTexImage3D");
	glAttachShader(m_SliceProgram, m_FragmentShader);GetGLError("glTexImage3D");
	glLinkProgram(m_SliceProgram);GetGLError("glTexImage3D");
	checklinkstatus(m_SliceProgram);GetGLError("glTexImage3D");
	glUseProgram(m_SliceProgram);GetGLError("glTexImage3D");
	string names2[]={"voltexture","threshold"};//,"frontIdx"};
	for(int i=0;i<sizeof(names2)/sizeof(names2[0]);i++){
		m_Map[1].insert(pair<string,int>(names2[i],
			glGetUniformLocation(m_SliceProgram,names2[i].c_str())));GetGLError("glTexImage3D");
	}
	
	
	
}

void CGLSLRayCasting::UpdateTextureUniform(){
	glUseProgram(m_Program);
	glUniform1i(m_Map[0]["voltexture"],0);
	glUseProgram(0);
}
void CGLSLRayCasting::Draw(float _zoom){
	glPushAttrib(GL_ENABLE_BIT);					
	glActiveTexture ( GL_TEXTURE0 );							
	glEnable( GL_TEXTURE_3D );
	//データはちゃんと変わってるみたい？？？要チェック
	//glBindTexture ( GL_TEXTURE_3D,mProgram->m_TexId );
	glUseProgram(m_Program);
	glUniform1f(m_Map[0]["threshold"],m_threshold);							
	
	glutSolidCube(1.0);
	
	glUseProgram(m_SliceProgram);
	glUniform1f(m_Map[1]["threshold"],m_threshold);	
	//頂点配列有効 レイキャスティング+ブロック化による不自然なクリッピング面問題を解決するための部分
	
	glBegin(GL_POLYGON);
		glVertex2f(0.0,_zoom);
		glVertex2f(1.0,_zoom);
		glVertex2f(2.0,_zoom);
		glVertex2f(3.0,_zoom);
		glVertex2f(4.0,_zoom);
		glVertex2f(5.0,_zoom);
	glEnd();
	glUseProgram(0);
	
	glPopAttrib();//light off
	//glColor4f(1.0,1.0,1.0,1.0);
	//m_boundingbox.DrawVertexNumers();
	
}
void CGLSLRayCasting::GetLocations(){
	string names[]={"voltexture","threshold"};
	for(int i=0;i<sizeof(names)/sizeof(names[0]);i++){
		m_Map[0].insert(pair<string,int>(names[i],glGetUniformLocation(m_Program,names[i].c_str())));
		if(m_Map[0][names[i]]<0){assert(!"存在しないユニフォーム変数名");}
		GetGLError("glTexImage3D");
	}
}
CGLSLRayCasting::~CGLSLRayCasting(void)
{
}

void CGLSLRayCasting::UpdateThresholdUniform(float _threshold){//最初しか使わない。
	m_threshold=_threshold;
	glUseProgram(m_Program);
	glUniform1f(m_Map[0]["threshold"],_threshold);
	glUseProgram(m_SliceProgram);
	glUniform1f(m_Map[1]["threshold"],_threshold);
	glUseProgram(0);
}
