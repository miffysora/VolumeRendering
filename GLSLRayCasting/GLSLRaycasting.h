#pragma once
#include <map>
#ifndef __GLEW_H__
#include <GL/glew.h>
#endif
using namespace std;
#ifndef MIFFY_VEC3
#include <miffy/math/vec3.h>
#endif
#ifndef MIFFY_COLOR
#include <miffy/math/color.h>
#endif

#include <miffy/gl/glutility.h>
#include <miffy/volren/glvolren.h>
using namespace miffy;
/*!
  @brief シェーダ上でのIDとデータをまとめておく。
	この構造体を使うのは、GUIから頻繁にアクセスしなきゃいけないようなやつ
	RayCastingにしてから、閾値をインタラクティブに変えることができた
*/
class CGLSLRayCasting
{

public:
	CGLSLRayCasting();
	CGLSLRayCasting(const string& _vertfile,const string& _fragfile);
	void init(const string& _vertfile,const string& _fragfile);
	~CGLSLRayCasting(void);
	void UpdateTextureUniform();
	void UpdateThresholdUniform(float _threshold);
	template <typename T>
	void Add3DTexture(T* _data,int _xy,int _z,int _type){
		glActiveTexture(GL_TEXTURE0);
		LoadVolumeDataFromMemory(_data,&m_TexId,_xy,_z, _type);

	}
	template <typename T>
	void Update3DTexture(T* _data,int _xy,int _z,int _type){
		glBindTexture ( GL_TEXTURE_3D, m_TexId );
		glTexSubImage3D( GL_TEXTURE_3D, 0,0,0,0, _xy, _xy, _z, GL_LUMINANCE, _type, _data );
		//Set3DTexParameter();
	}
	void Draw();
private:
	void GetLocations();

private:
	
public:
	unsigned int  m_Program;//本体
	unsigned int m_TexId;
	float m_threshold;
	map<string,int> m_Map;//名前と値loc
};

