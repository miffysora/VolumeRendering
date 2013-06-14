#pragma once
//#include <windows.h>
#include <GL/glew.h> 
#include <GL/glfw.h>
#include <GL/freeglut.h>
#include <sstream>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <list>
using namespace std;

//OpenCV関係
#include <opencv/cv.h>
#include <opencv/highgui.h>//cvLoadImageに必要
#include <opencv2/core/core.hpp>
#pragma comment(lib,"opencv_highgui240d.lib")
#pragma comment(lib,"opencv_core240d.lib")
//#pragma comment(lib,"opencv_imgproc240d.lib")//CalcHistに必要
#include <miffy/math/vec2.h>
#include <miffy/math/vec3.h>
#include <miffy/math/vec4.h>
#include<miffy/math/matrix.h>
#include <miffy/math/quaternion.h>
#define M_PI 6*asin( 0.5 )

const int nVoxelZ=256;
#define FILENAME "../../../data/raw/uchar/bonsai.raw"
#pragma comment(lib,"GLFW.lib")
#pragma comment(lib,"glew32.lib")
//const static double M_PI = 4.0*atan(1.0);
using namespace miffy;
#include "GLSLRayCasting.h"
class OpenGL{ 
public:
 mat4<float> m_modelview;
  mat4<float> m_rotation_matrix;
  mat4<float> m_proj_matrix;
  mat4<float> m_inv_modelview;
  mat4<float> m_inv_next_modelview;
  mat4<float> m_next_modelview;
  vec2<float> m_translate;
  double m_zoom;
  const   vec3<double> m_world_eye_pos;//ワールド座標での視点座標
  const float m_near;
  const float m_far;
  const float m_fovy;
  float m_aspect_ratio;
 CGLSLRayCasting raycastingshader;
public:
  vec2<int> m_win_size;
  vec2<int> m_last_pushed;
  quat<float> m_current_quaternion;
  quat<float> m_target_quaternion;
    OpenGL(void)
        :m_world_eye_pos(vec3<double>(0.0, 0.0, 9.0))
        ,m_near(0.1f)
        ,m_far(100.0f)
        ,m_fovy(30.0f)
        ,m_zoom(m_world_eye_pos.z)
    {
     
		glEnable(GL_MULTISAMPLE);
		
		raycastingshader.init("raycasting.vert","raycasting.frag");
		FILE* fp=fopen("../../../data/raw/uchar/bonsai.raw","rb");
		const int nVoxel=256;
		unsigned char* voldata=new unsigned char[nVoxel*nVoxel*nVoxel];
		fread(voldata,nVoxel*nVoxel*nVoxel,1,fp);
		raycastingshader.Add3DTexture(voldata,nVoxel,nVoxel,GL_UNSIGNED_BYTE);
		delete[] voldata;
		
    }

	void capture()
	{
		int i;
		int width , height;
		glfwGetWindowSize(&width,&height);


		cv::Mat cvmtx(cv::Size(width,height), CV_8UC4,cv::Scalar(0,0,0));//黒で初期化
		// 画像のキャプチャ
		glReadBuffer(GL_FRONT);// フロントを読み込む様に設定する
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 初期値は4
		glReadPixels(0, 0, width, height,GL_BGRA,GL_UNSIGNED_BYTE,(void*)cvmtx.data);
		//上下逆にする
		cv::flip(cvmtx,cvmtx,0);
		/* 画像の書き出し */
		cv::imwrite("output.png", cvmtx);
		}
    void display(){
        //描画
		 //glClearColor(0,0,0,0);
		glClearColor(0.2,0.2,0.2,1.0);
        //glClearColor(1.0,1.0,1.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);

        glPushMatrix();
 
        glLoadIdentity();

        gluLookAt(m_world_eye_pos.x, m_world_eye_pos.y, m_zoom,0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		
        glTranslated(m_translate.x,m_translate.y,0.0);/* 平行移動(奥行き方向) 　　*/
        m_target_quaternion.toMat4(const_cast<float*>(m_rotation_matrix.m));
        glMultMatrixf(m_rotation_matrix.m);//クォータニオンによる回転
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);
		raycastingshader.Draw();
		glColor4f(1.0,1.0,1.0,1.0);
		glLineWidth(1.0);
		glutWireCube(1.0);
        glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
        glPopMatrix();
 
    }//for display
 
    void RotateFromScreen(int _mx,int _my){
        float dx=(float)(_mx-m_last_pushed.x)/(float)m_win_size.x;
        float dy=(float)(_my-m_last_pushed.y)/(float)m_win_size.y;
        //過去との回転の合成をどうやっていいかわからない。やっぱクオータニオンが必要
        vec3<float> rotate_axis=vec3<float>(dy,dx,0.0);
        float axislength=rotate_axis.length();
        if(axislength!=0.0){
            float radian=(float)fmod(axislength*(float)M_PI,360.0);//画面いっぱいで調度一周になるようにする。
            rotate_axis.normalize();//軸の長さを1にする。
            quat<float> difq(cos(radian),rotate_axis.x*sin(radian),rotate_axis.y*sin(radian),0.0);
            m_target_quaternion=difq*m_current_quaternion;
 
        }
    }
 
    void reshape(int _width, int _height){
        m_win_size.set(_width,_height);
        //m_translate.x = 0.0;
        //m_translate.y = 0.0;
 
        glViewport(0, 0, (GLsizei) _width, (GLsizei) _height);
 
        m_aspect_ratio = (float)_width/(float)_height;
 
        //ブロック前後クリッピング初期化
        glMatrixMode(GL_PROJECTION);  /* 投影変換の設定 */
        glLoadIdentity(); 
        gluPerspective(m_fovy, m_aspect_ratio, m_near, m_far);//original
        glGetFloatv(GL_PROJECTION_MATRIX, m_proj_matrix.m);    
        glMatrixMode(GL_MODELVIEW);
 
    }
    void zoom(int _direction){
        m_zoom=(double)_direction*0.4+m_world_eye_pos.z;
 
    }
 
};
int main(int argc, char *argv[]){
    glutInit(&argc,argv);
    glfwInit();
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES,4);//glfwOpenWindowの前に呼ばないと効果がない！
    glfwOpenWindow(250,250,0,0,0,8,0,0, GLFW_WINDOW );
    glfwSetWindowTitle("Miffy OpenGL");
    glewInit();
    enum STATE{STATIC,ROTATING,SHIFT,ZOOM};
    OpenGL opengl;
	vec2<int> tra_lastpush;
    while (glfwGetWindowParam(GLFW_OPENED))//ウィンドウが開いている場合
    {
        int winx,winy;
        glfwGetWindowSize(&winx,&winy);
        opengl.reshape(winx,winy);
        int mx,my;
        glfwGetMousePos(&mx,&my);
        if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)){
            opengl.RotateFromScreen(mx,my);
        }else{//回転やめ
            opengl.m_last_pushed.set(mx,my);
            opengl.m_current_quaternion=opengl.m_target_quaternion;
        }
		if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)){//平行移動
			opengl.m_translate.x+=(float)(mx-tra_lastpush.x)/(float)winx;
			opengl.m_translate.y+=(float)(my-tra_lastpush.y)*-1.0f/(float)winy;
			tra_lastpush.set(mx,my);
		}else{
			tra_lastpush.set(mx,my);
		}
		if(glfwGetKey('C')){opengl.capture();}
        opengl.zoom(glfwGetMouseWheel());
        opengl.display();
        glfwSwapBuffers();
    }
    return 0;
}
