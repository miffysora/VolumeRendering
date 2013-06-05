#include "globaldefs.h"
void wheel(int wheel_number,int direction,int x,int y){

  gZoom-=(float)direction*0.1;
	glutPostRedisplay();
}
void initCamera(int _w,int _h){

	gWidth = _w;
	gHeight = _h;
	gfWinWidth=(float) _w;
	gfWinHeight=(float)_h;



	if(gfWinHeight<gfWinWidth){
		gRatio=gfWinHeight/gfWinWidth;
	}else{
		gRatio=gfWinWidth/gfWinHeight;
	}
	printf("ratio=%d\n",gRatio);
	// 表示物の姿勢設定
	gProj.gluPerspective(gFovy,gRatio,1.0,1000);
	
	
	
}
void InitShader() {
	FILE* fp;
	fp=fopen("raycasting.frag","rb");
	if(fp==NULL){
		char buf[200];
		GetCurrentDirectory(200,buf);
		printf("current directory:%s\n",buf);
		assert(!"file could not open!");}
	//ファイルの長さを調べる
	fseek(fp,0,SEEK_END);
	int size= ftell(fp);
	fseek(fp,0,SEEK_SET);//先頭まで移動
	gFragmentShader=new char[size+1];
	fread(gFragmentShader,sizeof(char),size,fp);
	fclose(fp);
	gFragmentShader[size]='\0';//終わりだよ印をつける
	gProgram = miffy::CreateShaderProgram(gVertexShader, gFragmentShader);
	gSimpleProgram = miffy::CreateShaderProgram(gSimpleVertexShader, gSimpleFragmentShader);//boundingboxのために必要。
	projectionUniform = glGetUniformLocation(gProgram, "Projection");
	modelviewUniform = glGetUniformLocation(gProgram, "Modelview");
	gCamUniLoc	=	glGetUniformLocation(gProgram,"camera");
	gTexSamplerLoc= glGetUniformLocation ( gProgram, "voltexture" );
	gTFTexSamplerLoc= glGetUniformLocation ( gProgram, "trans_func" );
	printf("glUni=%d,%d",gTFTexSamplerLoc,gTexSamplerLoc);
	// 頂点座標の設定
	glBindAttribLocation(gProgram, ATTRIB_VERTEX, "Position");
	glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, verts);

}
void LoadTFData(){
	FILE* fp;
	fp=fopen(TFFILE,"rb");
	//データ名の最初がデータの長さになっている。。。けど、桁数変わるし、難しいなぁ。
	fseek(fp,0,SEEK_END);//ファイル終端から+0ファイルポインタの位置を移動させる
	int filebytes=ftell(fp);//現在のファイルポインタの位置を返す
	fseek(fp,0,SEEK_SET); 
	tfdata=new float[filebytes];//
	printf("filebytes=%d\n",filebytes);
	fread(tfdata,1,filebytes,fp);
	fclose(fp);
	miffy::LoadTFData(tfdata,filebytes,&texId[1]);
	delete[] tfdata;
}
void LoadVolumeData(){
	voldata=new unsigned char[nVoxel*nVoxel*nVoxelZ];
	FILE* fp;
	fp=fopen(FILENAME,"rb");
	if(fp==NULL){printf("could not find file\n");}
	fread(voldata,sizeof(unsigned char),nVoxel*nVoxel*nVoxelZ,fp);
	fclose(fp);
	miffy::LoadVolumeData(voldata,texId,nVoxel,nVoxelZ);
	printf("texname=%d\n",texId[0]);
}
void InitGeometry(){
	//vertsはglAttributeで使うもの
	memcpy(verts,cubeVerticesForQuads,sizeof(float)*3*24);
}
void init() {
	InitGeometry();
	InitShader();
	glClearColor(.1f, .1f, .3f, 1.f);
	
	LoadVolumeData();
	LoadTFData();
}
void reshape(int _w,int _h){
	gWidth=_w;
	gHeight=_h;
	initCamera(gWidth,gHeight);
}
void DrawVolume(){    
	glUseProgram(gProgram);
	// 透視変換行列の設定

	glUniformMatrix4fv(projectionUniform, 1, 0, gProj.m);
	glUniformMatrix4fv(modelviewUniform, 1, 0, gModelView.m);
	glUniform3f(gCamUniLoc,gLocalCoordCam.x,gLocalCoordCam.y,gLocalCoordCam.z);
	
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Bind the texture
	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_3D, texId[0] );
	glEnable ( GL_TEXTURE_3D );
	glUniform1i ( gTexSamplerLoc, 0);// texId[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture ( GL_TEXTURE_1D, texId[1] );
	glEnable(GL_TEXTURE_1D);
	glUniform1i(gTFTexSamplerLoc, 1);//texId[1]);
	//頂点の設定
	glEnableVertexAttribArray(ATTRIB_VERTEX);
	glDrawArrays(GL_QUADS,0,24);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	glUseProgram(0);

}
void drawBoundingBox(bool _moreinfo){
	//clipping planeも書きたくなった
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_LINE_LOOP);
	for(int i=0;i<18;i++){
		glColor4f(cubeColors[cubeLineIndices[i]*4+0],cubeColors[cubeLineIndices[i]*4+1],cubeColors[cubeLineIndices[i]*4+2],cubeColors[cubeLineIndices[i]*4+3]);
		glVertex3f(cubeVertices[cubeLineIndices[i]*3+0],cubeVertices[cubeLineIndices[i]*3+1],cubeVertices[cubeLineIndices[i]*3+2]);
	}
	glEnd();
	if(_moreinfo){
		/*float sbwin_size=1.0;
		//near clipping plane
		glColor4f(1.0,1.0,1.0,0.3);
		glBegin(GL_QUADS);
		glVertex3f(-sbwin_size,-sbwin_size,-3);
		glVertex3f(sbwin_size,-sbwin_size,-3);
		glVertex3f(sbwin_size,sbwin_size,-3);
		glVertex3f(-sbwin_size,sbwin_size,-3);
		glEnd();*/
		//視点描画
		glColor3f(0,1.0,1.0);
		glPointSize(8);
		glBegin(GL_POINTS);
		gLocalCoordCam.glVertex();
		glEnd();
		glColor3f(1,1,1);
		for(int i=0;i<8;i++){
			glBegin(GL_LINES);
			gLocalCoordCam.glVertex();
			glVertex3f(cubeVertices[i*3+0],cubeVertices[i*3+1],cubeVertices[i*3+2]);
			glEnd();
		}
	}


}
void renderSubWindow(){
	//下方1/4スペースに描く。
	if(gfWinWidth >= gfWinHeight)
		glViewport(gfWinHeight - gfWinHeight/4,0, gfWinHeight/4, gfWinHeight/4);
	else
		glViewport(gfWinWidth - gfWinWidth/4,0, gfWinWidth/4, gfWinWidth/4);

	//サブ画面背景作成
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0,0.0,gZoom);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	float sbwin_size=1.0;//どんな値でも変わらない。
	glOrtho(-sbwin_size,sbwin_size,-sbwin_size,sbwin_size,0.25,20.0);
	//サブ画面を半透明でオーバーレイする
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//サブ画面それ自体の四角を描く 
	glColor4f(1.0,1.0,1.0,0.2);
	glBegin(GL_QUADS);  
	glVertex2f(-sbwin_size,-sbwin_size);
	glVertex2f(sbwin_size,-sbwin_size);
	glVertex2f(sbwin_size,sbwin_size);
	glVertex2f(-sbwin_size,sbwin_size);
	glEnd();
	glDisable(GL_BLEND);
	//サブ画面中身描画
	glLoadIdentity();
	float eyeDis=gLocalCoordCam.length();
	glOrtho(-eyeDis,eyeDis,-eyeDis,eyeDis,0.25,20.0);
	glMatrixMode(GL_MODELVIEW);
	glRotatef(45.0f,1.0f,-1.0f,-0.29f);//ずれた視点から見るポイント
	//描画したいものを描く(draw bounding box)
	//drawBoundingBox(true); 
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	//元に戻す
	glViewport(0, 0, gfWinWidth, gfWinHeight);

}
void display(void ){
	glClearColor(.0f, .0f, .0f, 1.f);
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	gModelView.LoadIdentity();
	gModelView.gluLookAt(0,0,gZoom, 0,0,0, 0,1.0,0);//gZoom初期値は-5
	   
	gModelView=gModelView*rotation;
	gIniCam.set(0,0,0);
	gModelView.inv(&gInvModelView);
	gLocalCoordCam=gInvModelView*gIniCam;
	gLocalCoordCam=(gLocalCoordCam+1.0f)/2.0f;//z
	DrawVolume();

	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glLoadIdentity();
	//glMultMatrixf(gProj.m);
	//glPopMatrix();
	////gProj.print("proj");
	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glLoadIdentity();
	//glMultMatrixf((float*)gModelView.m);
	//DrawOriginalPoint();
	//drawBoundingBox(false);
//	glPopMatrix();
	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();
	//renderSubWindow();
	//glPopMatrix();
	glutSwapBuffers();
}
void move(int _x,int _y){
	float dx=((float)_x-previous_x)*1.0f/800.0f;
	float dy=((float)-_y-previous_y)*1.0f/1176.0f;
	//クォータニオンの長さ
	float length=sqrt(dx*dx+dy*dy);
	if(length !=0.0){
		float radian=length*M_PI;//M_PIは適当な換算係数 M_PIにしておけば、画面いっぱい動かした時に調度一回転になる。
		float theta=sin(radian)/length;
		//LOGI("radian=%f,theta=%f",radian,theta);

		miffy::quat<float> after(cos(radian),dy*theta,dx*theta,0.0);//回転後の姿勢
		target_quaternion=after*current_quaternion;
		target_quaternion.toMat4(rotation.m);
	}
	glutPostRedisplay();

}
void mouse(int button ,int touch_state, int x, int y){
	switch(touch_state){
	case GLUT_DOWN:
		previous_x=x;
		previous_y=-y;
		break;
	case GLUT_UP://Quaternionの姿勢を保存
		current_quaternion=target_quaternion;
		break;
	}
	glutPostRedisplay();
}
int main(int argc, char *argv[]){
	glutInit(&argc, argv);    
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);

	glutInitWindowPosition(100,0);
	glutInitWindowSize(gWidth,gHeight);
	glutCreateWindow("volren");
	glewInit();
	init();

	glutDisplayFunc(display);
	//	glutIdleFunc(idle);
	glutMotionFunc(move);
	glutMouseFunc(mouse);
	glutReshapeFunc(reshape);
	//glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(wheel);
	glutMainLoop();
	return 0;
}
