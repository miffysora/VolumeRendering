# -*- coding: cp932 -*-
import numpy;
from PIL import Image
def GeneratLowResoData(_highsize,_constz,_data):#xy方向だけ圧縮
    lowsize=_highsize>>1;
    for z in range(0,_constz):
        zslice=Image.new('L',(_highsize,_highsize));
        start=_highsize*_highsize*z;
        zslice.putdata(_data[start:start+_highsize*_highsize]);
        lowered=zslice.resize((lowsize,lowsize));
        _data[start:start+_highsize*_highsize]=lowered.getdata();
    return lowsize;

filename="DBZ201208301319_512_z64_2byte.dat"
splitted=filename.split('_');

xynum=int(splitted[1]);print("xynum=%d"%xynum);
znum=int(splitted[2][1:]);
dataname=splitted[0];
fp=open(filename,"rb");
data=numpy.fromfile(fp,dtype='H');
print("最小値%d,最大値%d"%(numpy.amin(data),numpy.amax(data)));
print("サイズ%d"%len(data));
fp.close();    			
im=Image.new( 'L', ( xynum, xynum ) );
slicez=0;
maxval=numpy.amax(data)

fdata=numpy.array(data,dtype='f')
fdata=fdata/numpy.amax(fdata)*255.0;#0-255に正規化
uchardata=numpy.array(fdata,dtype='B');
im.putdata(uchardata[:xynum*xynum]);
im.save(dataname+".png");
lowsize=GeneratLowResoData(xynum,znum,uchardata);
print("lowsize=%d"%lowsize);
fp=open(dataname+"_"+str(lowsize)+"_z"+str(znum)+"_1byte.raw","wb");
uchardata[:lowsize*lowsize*znum].tofile(fp);
fp.close();
im=Image.new( 'L', (lowsize, lowsize) );
im.putdata(uchardata[:lowsize*lowsize]);
im.save(dataname+"_"+str(lowsize)+".png");
print("done");

