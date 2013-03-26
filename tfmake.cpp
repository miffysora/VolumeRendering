#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <sstream>
using namespace std;
#include <miffy/math/color.h>

const int tfsize=256;
miffy::color<float> fTf[tfsize];//rgba4色
miffy::color<unsigned char> ucTf[tfsize];//rgba4色
bool makeFloatFile=false;
bool makeUCharFile=false;
const string name="rainbow";
void HSV2RGB( double *rr, double *gg, double *bb, double H, double S, double V )
{
    int in;
    double fl;
    double m, n;
    in = (int)floor( H / 60 );
    fl = ( H / 60 ) - in;
    if( !(in & 1)) fl = 1 - fl; // if i is even
 
    m = V * ( 1 - S );
    n = V * ( 1 - S * fl );
  if(V>256.0){assert(!"異常値");}
	if(n>256.0){assert(!"異常値");}
	if(m>256.0){assert(!"異常値");}
    switch( in ){
       case 0: *rr = V; *gg = n; *bb = m; break;
       case 1: *rr = n; *gg = V; *bb = m; break;
       case 2: *rr = m; *gg = V; *bb = n; break;
       case 3: *rr = m; *gg = n; *bb = V; break;
       case 4: *rr = n; *gg = m; *bb = V; break;
       case 5: *rr = V; *gg = m; *bb = n; break;
    }
}
void TFBitmapOut(){
	const int height=50;
	using namespace System;
	Drawing::Bitmap ^bitmap = gcnew Drawing::Bitmap(tfsize,height);
    for(int y=0;y<height;y++){
    for(int x=0;x<tfsize;x++){
        Drawing::Color color;
        color=Drawing::Color::FromArgb(255,(int)(ucTf[x].r),(int)(ucTf[x].g),(int)(ucTf[x].b));
        bitmap->SetPixel(x,y,color);
    }
    }
	stringstream outname;
	outname<<"../float/"<<name.c_str()<<tfsize<<".png";
	String^ str=gcnew System::String(outname.str().c_str());
	bitmap->Save(str);
	/*outname<<"../uchar/"<<name.c_str()<<tfsize<<".png";
			str=gcnew System::String(outname.str().c_str());
	bitmap->Save(str);*/
    delete bitmap;
}
int main(){
	stringstream outname;
	unsigned char* color=new unsigned char[8*4];
	double r,g,b;
	int threshold=0;//雲の閾値
	for(int i=0;i<threshold;i++){
		fTf[i].set(0.0f,0.0f,0.0f,0.0f);
		ucTf[i].set(0,0,0,0);
	}
	for(int i=threshold;i<tfsize;i++){
		//240-80がdefault
		double hue=(float)(tfsize-(i-threshold))/(float)(tfsize-threshold)*240.0;
		//double hue=(float)(tfsize-(i-threshold))/(float)(tfsize-threshold)*480.0-420;//青～赤にしたいから、240にした。
		//double hue=(float)(tfsize-i)/(float)tfsize*240.0+240.0;//青～赤にしたいから、240にした。
		hue=fmod(hue,360.0);//0.0-240.0の値にしたい。
		//printf("%f\n",hue);
		//HSV2RGB(&r,&g,&b,hue,1.0,1.0);
		fTf[i].setFromHSV(miffy::HSV((int)hue,255,255));
		fTf[i].a=1.0f;//iの場合もある。
		ucTf[i].setFromHSV(miffy::HSV((int)hue,255,255));
		ucTf[i].a=255;//iの場合もある。
	}
	/*color[0]=0;color[1]=0;color[2]=0;color[3]=0;
	color[4]=139;color[5]=243;color[6]=255;color[7]=25;
	color[8]=139;color[9]=188;color[10]=255;color[11]=25;
	color[12]=139;color[13]=188;color[14]=255;color[15]=25;
	color[16]=139;color[17]=226;color[18]=255;color[19]=25;
	color[20]=139;color[21]=243;color[22]=255;color[23]=25;
	color[24]=141;color[25]=168;color[26]=255;color[27]=25;//
	color[28]=153;color[29]=139;color[30]=255;color[31]=25;
	for(int j=0;j<8;j++){
		for(int i=tfsize*j/8;i<tfsize*(j+1)/8;i++){
			for(int rgba=0;rgba<=4;rgba++){
				tf[i*4+rgba]=(float)color[j*4+rgba]/255.0;
			}
		}
	}*/
	outname<<"../float/"<<name.c_str()<<"_"<<tfsize<<".tf";
	FILE* fp;
	if(makeFloatFile){
		fp=fopen(outname.str().c_str(),"wb");
		fwrite(&fTf[0].r,sizeof(miffy::color<float>),tfsize,fp);
		fclose(fp);
	}
	if(makeUCharFile){
		outname.str("");
		outname<<"../uchar/"<<name.c_str()<<"_"<<tfsize<<".tf";
		fp=fopen(outname.str().c_str(),"wb");
		fwrite(&ucTf[0].r,sizeof(miffy::color<unsigned char>),tfsize,fp);
		fclose(fp);
	}

	TFBitmapOut();
	//次はGIMPのパレットファイルを作る
	outname.str("");
	outname<<"F:/Program Files/GIMP 2/share/gimp/2.0/palettes/"<<name.c_str()<<"_"<<tfsize<<".gpl";
	//outname<<"../uchar/"<<name.c_str()<<"_"<<tfsize<<".gpl";
	ofstream ofs(outname.str().c_str(),ios_base::trunc);
	ofs<<"GIMP Palette"<<endl;
	ofs<<"Name: "<<name.c_str()<<"_"<<tfsize<<endl;
	ofs<<"#"<<endl;
	for(int i=0;i<tfsize;i++){
		ofs<<(int)ucTf[i].r<<" "<<(int)ucTf[i].g<<" "<<(int)ucTf[i].b<<" "<<endl;
	}
	ofs.close();
	printf("おわりました");
	//int a;scanf("%d",&a);
	return 0;

}
