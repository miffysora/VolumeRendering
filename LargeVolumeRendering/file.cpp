#include "File.h"
using namespace std;

File::File() {}

File::~File() {}
bool File::blockExist(Block bl)
{
  if(this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex == NULL)
		return false;
	else
		return true;
}
/*!
@brief deprecatedかも
*/
void File::blockProhibitReplaceInMain(Block bl){
	
	MainMemManage* mb = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex;
	if(mb!=NULL){
	mb->replaceable = false;
	}
}
void File::blockPermitReplaceInMain(Block bl){
	MainMemManage* mb = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex;
	if(mb!=NULL){
	mb->replaceable = true;
	}
}

/*!
@brief 優先度スコアを5ビット右シフトする。優先度スコアは１フレームにつき５ビット使うから。
*/
void File::countMainLRU(void)
{
	LARGE_INTEGER temp;
	LARGE_INTEGER freq;
	static float time,timebase = 0.0f;//printf("timebase=%.4f\n",timebase);
	QueryPerformanceCounter(&temp);
	QueryPerformanceFrequency(&freq);
	time = (float)(temp.QuadPart)*1000/(float)freq.QuadPart;
	//timeは普通に、プログラム起動からの経過時間？なんでそんなもの使うの
	//static変数だからbasetimeの値が保持されるっぽい
	if (time - timebase > this->loadTime) { //平均ロード時間で1ビット繰り下げ　this->loadTimeは、HD toCPUの平均時間　ふつうにwhileの１ループ終わったら１ビット繰り下げと考えてよいのかな。。。？
		for(int i= 0; i < MAXBNUM;i ++){
			for(int j=0;j<ULONGLONGSIZE-1;j++){
				this->memblock[i].needflag[j]=(this->memblock[i].needflag[j]>>5)|((this->memblock[i].needflag[j+1] & LASTFIVEBITS)<<59);
			}
		}
		for(int i=0;  i < QSIZE;i++)
		{
			for(int j=0;j<ULONGLONGSIZE-1;j++){
				this->reqque[i].needflag[j]=(this->reqque[i].needflag[j]>>5)|((this->reqque[i].needflag[j+1] & LASTFIVEBITS)<<59);
			}
			this->reqque[i].needflag[ULONGLONGSIZE-1]=this->reqque[i].needflag[ULONGLONGSIZE-1]>>5;

			ULONGLONG multiply=0;
			//フラグの値がゼロになったらキューから追い出す
			if(multiply == 0)//reqqueはまだメインメモリにロードしてない必要なブロックリスト
			{
				if(this->reqque[i].block->level != -1)
				{//前のブロックの情報が残ってるようだったら消すってことかな
					Block bl = *this->reqque[i].block;
					this->mainMemMap[bl.level][bl.x][bl.y][bl.z].queIndex = NULL;
					this->reqque[i].block->level = -1;
				}		
			}
		}//if (time - timebase > this->loadTime) 

		timebase = time;		//while loop１個ごとに値が更新される
	}
}


void File::countTexLRU(void)
{
	for(int i= 0; i < g_MaxTexNum;i ++){
		for(int j=0;j<ULONGLONGSIZE-1;j++){
			this->texblock[i].needflag[j]=(this->texblock[i].needflag[j]>>5)|((this->texblock[i].needflag[j+1] & LASTFIVEBITS)<<59);
		}
		this->texblock[i].needflag[ULONGLONGSIZE-1]=this->texblock[i].needflag[ULONGLONGSIZE-1]>>5;//最後だけ特別な処理する
	}
}
void File::deleteMemory(btexstruct* bstex)
{
	bstex->texdataIndex = NULL;
}
void File::deleteMemory(bstruct* _mainMemMap)
{
	_mainMemMap->dataIndex = NULL;
}
int File::getMaxIndexInReq(){
	ULONGLONG maxcontent=0;
	int mostWantedIndex = -1;//reqqueが空なら-1を返す
	int firstNonZeroPlace=ULONGLONGSIZE-1;
	
	//欲しいブロックリストから書き込むブロックを1つ選択
	for(int i=0;i<QSIZE;i++)
	{
		//欲しいブロックリストから最もneedflagの値の高いものを選ぶ
		ULONGLONG needvalue[ULONGLONGSIZE];
		for(int j=0;j<ULONGLONGSIZE;j++){needvalue[j] = this->reqque[i].needflag[j];}
		
		if(this->reqque[i].block->level != -1)//reqquenに何かブロックが入っているなら、
		{//最大値を探す
			int j;
		for(j=ULONGLONGSIZE-1;j>-1;j--){
			if(needvalue[j]!=0){
				break;
			}
		
			if(j<=0){//一番下の桁に行ってもダメだったとき
				//全ての数が完全に0
				break;//j roopを抜ける
			}//needflag完全に0のblock index
		}//j
		//ここまでで初めての0じゃないjがどこであるかが決まる
		if(firstNonZeroPlace<j){
			mostWantedIndex=i;
			firstNonZeroPlace=j;
			maxcontent=needvalue[j];
			
		}else if(firstNonZeroPlace==j && maxcontent<needvalue[j]){
			maxcontent=needvalue[j];
			mostWantedIndex=i;
			
		}else if(firstNonZeroPlace==j && maxcontent==needvalue[j]){//同じ値があったら
			//更に深くたどる
			int j2=firstNonZeroPlace-1;
			while(j2>=0){
				if(this->reqque[mostWantedIndex].needflag[j]<needvalue[j2]){
					this->reqque[mostWantedIndex].needflag[j]=needvalue[j2];
					mostWantedIndex=i;
					break;
				}else if(this->reqque[mostWantedIndex].needflag[j]==needvalue[j2]){
					j2--;
					this->reqque[mostWantedIndex].needflag[j]= this->reqque[mostWantedIndex].needflag[j];
								
				}else{//今のmaxの方が大きかったらそのままにする
					break;}
			}//end j2
		}//同じ値があったら
		//j<firstNonZeroPlaceの場合はシカトして次のiに行く
	}
	
	}//reqque辿り終わり
	return mostWantedIndex;
}
int File::getMinIndexInMain(ULONGLONG *reqquevalue){
	//これからメインメモリに入れる場所を決める
	int insertPlace=-1;
			 
			//まず空いてる場所を探す
			if(!mainmemfullflag){
			for(int index = 0;index <MAXBNUM;index++)
			{if(index>=MAXBNUM-1){mainmemfullflag=true;}
			if(this->memblock[index].block->level==-1&& !this->memblock[index].loadStartFlag && this->memblock[index].replaceable)
					{
					return index;
					}
				
			}
			}
	//最小値を探す
	
	
	int firstNonZeroPlace=ULONGLONGSIZE-1;
	ULONGLONG mincontent=0xffffffffffffffff;
	for(int i = 0;i <MAXBNUM;i++){
		if(!this->memblock[i].loadStartFlag){//ロードが始まってなかったら		
		if(this->memblock[i].replaceable)//上書き可能なのかどうか（今必要なブロックで、テクスチャメモリ転送中じゃない）
				{
					ULONGLONG needvalue[ULONGLONGSIZE];
					for(int j=0;j<ULONGLONGSIZE;j++){needvalue[j]=this->memblock[i].needflag[j];}		
					int j;
		for(j=ULONGLONGSIZE-1;j>-1;j--){
			if(needvalue[j]!=0){
				break;
			}
		
			if(j<=0){//一番下の桁に行ってもダメだったとき
					Block minbl;
					//書き込まれる新しいブロック確定	
					memcpy(&minbl,memblock[i].block,sizeof(Block));
					//書き込まれる側の古いブロック削除
					this->deleteMemory(&this->mainMemMap[minbl.level][minbl.x][minbl.y][minbl.z]);
					
				return i;
				break;
			}//needflag完全に0のblock index
		}//j
			//ここまでで初めての0じゃないjがどこであるかが決まる
		if(firstNonZeroPlace>j){
			insertPlace=i;
			firstNonZeroPlace=j;
			mincontent=needvalue[j];
			
		}else if(firstNonZeroPlace==j && mincontent>needvalue[j]){
			mincontent=needvalue[j];
			insertPlace=i;
			
		}else if(firstNonZeroPlace==j && mincontent==needvalue[j]){//同じ値があったら
			//更に深くたどる
			int j2=firstNonZeroPlace-1;
			while(j2>=0){
				if(this->memblock[insertPlace].needflag[j]>needvalue[j2]){

					insertPlace=i;
					break;
				}else if(this->memblock[insertPlace].needflag[j]==needvalue[j2]){
					j2--;
							
				}else{//今のminの方が小さかったらそのままにする
					break;}
			}//end j2
		}//同じ値があったら
		}
		}//ロードが始まったのかどうか	
		
		}//for(int i = 0;i <MAXBNUM;i++)
		
			//reqque value が mamblockより大きくないかチェック
				//j==reqqueの最初に0でない場所
			int j=ULONGLONGSIZE-1;
			for(j=ULONGLONGSIZE-1;j>-1;j--){
			if(reqquevalue[j]!=0){
				break;//j roopを抜ける
			}
		
			if(j<=0){//一番下の桁に行ってもダメだったとき
				return -1;//reqqueの値が０だったとき
				break;//j roopを抜ける
			}//needflag完全に0のblock index
		}//j
			//j==reqqueの最初に0でない場所がどこか決まった
		if(firstNonZeroPlace>j){//reqqueの方が値が小さい場合
			return -1;
		}else if(firstNonZeroPlace==j && memblock[insertPlace].needflag[j]>reqquevalue[j]){
			return -1;//
			
		}else if(firstNonZeroPlace==j && mincontent==reqquevalue[j]){//同じ値があったら
			//更に深くたどる
			int j2=firstNonZeroPlace-1;
			while(j2>=0){
				if(this->memblock[insertPlace].needflag[j2]>reqquevalue[j2]){
					return -1;
				}else if(this->memblock[insertPlace].needflag[j2]==reqquevalue[j2]){
					j2--;	
				}else{//今のminの方が小さかったらそのままにする
					break;}//j2 roopを抜ける
			}//end j2
		}//同じ値があったら
		if(memblock[insertPlace].data!=NULL){
					Block minbl;
					//書き込まれる新しいブロック確定	
					memcpy(&minbl,memblock[insertPlace].block,sizeof(Block));
					//書き込まれる側の古いブロック削除
					this->deleteMemory(&this->mainMemMap[minbl.level][minbl.x][minbl.y][minbl.z]);
		}
					
					return insertPlace;		
}
void File::Init(string _path, string _dataname){
	texmemfullflag=false;
	mainmemfullflag=false;
	
	m_Path = _path;
	m_DataName = _dataname;


	//配列ポインタ作成（NULL初期化)
	this->mainMemMap = new bstruct ***[NUMLEVEL];
	this->texMemMap = new btexstruct ***[NUMLEVEL];
	


	for(int a= 0; a < NUMLEVEL;a ++)
	{
		int num = (int)pow(2.0,(NUMLEVEL-1.0)-a);//
		int numx = INIBLX*num;
		int numy = INIBLY*num;
		int numz = INIBLZ*num;


		this->mainMemMap[a] = new bstruct **[numx];
		this->texMemMap[a] = new btexstruct **[numx];

	
		//indexblockファイル読み出し
		indexblock[a] = new unsigned int[numx*numy*numz];
		stringstream indexblockfile;
		indexblockfile<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<a<<"/"<<"VertebralBody"<<"IndexBlock-"<<a<<".dff";
		//static string path =  "../../";
		//static string dataname = "VertebralBody_Brick";
		
		loadIndexFile(indexblockfile.str(),indexblock[a],2,numx,numy,numz);
		
		if(a>0){texCompressInfo[a-1]= new unsigned int[numx*numy*numz];
		stringstream texCompressfile;
		texCompressfile<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<a<<"/"<<"VertebralBody"<<"TexCompress-"<<a<<".dff";
		loadIndexFile(texCompressfile.str(),texCompressInfo[a-1],0,numx,numy,numz);
		}
		//サブ画面用ブロック確保
		Block::nRequestBlock[a] = new unsigned int[numx*numy*numz];
		Block::bRequestBlock[a] = new unsigned int[numx*numy*numz];
		Block::rRequestBlock[a] = new unsigned int[numx*numy*numz];//様々なブロックステートを保持する


		for(int i=0 ;i < numx;i++)
		{
			this->mainMemMap[a][i] = new bstruct *[numy];
			this->texMemMap[a][i] = new btexstruct *[numy];

			for(int j=0;j < numy;j++)
			{
				this->mainMemMap[a][i][j] = new bstruct [numz];
				this->texMemMap[a][i][j] = new btexstruct[numz];


				for(int k=0;k <numz;k++)
				{
					this->mainMemMap[a][i][j][k].dataIndex = NULL;
					this->mainMemMap[a][i][j][k].queIndex = NULL;

					this->texMemMap[a][i][j][k].texdataIndex = NULL;

					Block::nRequestBlock[a][(numy*i+j)*numz+k] = 0;
					Block::bRequestBlock[a][(numy*i+j)*numz+k] = 0;
					Block::rRequestBlock[a][(numy*i+j)*numz+k] = 0;
				}
			}
		}
	}
	//メモリプール確保
	
	int size=MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2);
	//size=(int)(DATAPOOLALPHA*MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2));
	//MAXBNUM=DATAPOOLALPHA*MAXBNUM;//値更新
	if(size>=0x7fffffff){//0x7fffffff(2147483647バイトメモリ以上の配列はnew出来ない)
		//size=size/(4096*100);
		printf("メモリ0x7fffffff以上。%dメガバイト注意",MAXBNUM);
	}
	try{printf("datapool確保%dメガバイト\n",MAXBNUM);
	//this->datapool=(float *)calloc(MAXBNUM,sizeof(float));
	this->datapool = new float[MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2)];//ここでエラー
	}catch(bad_alloc){
		cerr<<"bad_alloc new失敗"<<MAXBNUM*(BLX+2)*(BLY+2)*(BLZ+2)*4/1024/1024<<"Mbytes"<<endl;
		abort();
	}
	catch(...){cerr<<"そのほか謎のエラー"<<endl;
	abort();
	}

	//printf("maxbnum%i\n",MAXBNUM);
	this->texName = new GLuint[g_MaxTexNum];
	
	
	//プールフラグ定義and初期化(false:ブロック入っていない,true：入っている)
	this->memblock = new MainMemManage[MAXBNUM];
	this->mTimeStart = new LARGE_INTEGER[MAXBNUM];
	this->mTimeEnd =new LARGE_INTEGER[MAXBNUM];
	this->temp_count = 1;
	for(int i=0;i<MAXBNUM;i++){
		this->memblock[i].block = new Block;
		this->memblock[i].block->level = -1;
		this->memblock[i].FromLoadTime = 1;
		this->memblock[i].data = NULL;
		this->memblock[i].replaceable = false;
		for(int j=0;j<ULONGLONGSIZE;j++){this->memblock[i].needflag[j] = 0;}
		this->memblock[i].loadStartFlag = false;
		this->memblock[i].ol = new OVERLAPPED;
		this->memblock[i].Fl = NULL;
	}

	//テクスチャメモリフラグ定義and初期化
	this->texblock = new TexMemManage[g_MaxTexNum];
	for(int i= 0;i<g_MaxTexNum;i++)
	{
		this->texblock[i].block = new Block;
		this->texblock[i].block->level = -1;
		for(int j=0;j<ULONGLONGSIZE;j++){this->texblock[i].needflag[j] = 0;}
		this->texblock[i].texdata = NULL;
	}


	for(int i=0;i < NUMLEVEL;i++)
	{	
		Block::CLR[i][0] = CLR[i][0];
		Block::CLR[i][1] = CLR[i][1];
		Block::CLR[i][2] = CLR[i][2];
	}

	texLoadflag = false;

	this->reqque = new RequestQueue[QSIZE];//これからメインメモリにロードしたいものリスト
	for(int i=0;i<QSIZE;i++)
	{//初期化
		for(int j=0;j<ULONGLONGSIZE;j++){this->reqque[i].needflag[j] = 0;}
		this->reqque[i].block = new Block;
		this->reqque[i].block->level = -1;
	}
	

	}
LARGE_INTEGER File::getLaddress(long long address)
{
	address = address * sizeof(float);

	int lower; 
	int upper;
	memcpy(&lower,(unsigned char*)&address,   sizeof(int));
	memcpy(&upper,(unsigned char*)&address  + sizeof(int) ,   sizeof(int));

	LARGE_INTEGER laddr;
	laddr.LowPart = lower;
	laddr.HighPart = upper;

	return laddr;
}
/*!
	@brief 私は、引数が無駄に多いと思って、減らして改良したのだ。
*/
void File::loadFile(string _file,int _header,int _size,MainMemManage* _mmm)
{
	HANDLE handle;
	
	DWORD dwWriteSize;
	//wchar_t *wc = new wchar_t[file.size()+1];
	//mbstowcs(wc,file.data(),file.size()+1);

	handle = CreateFile(_file.c_str(),GENERIC_READ,NULL,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	
	if(handle == INVALID_HANDLE_VALUE)
	{
		LPVOID lpMsgBuf;
		//ここにチェックしたい処理を書く

		FormatMessage(				//エラー表示文字列作成
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);

		MessageBox(NULL, (LPCSTR)lpMsgBuf, NULL, MB_OK);	//メッセージ表示

		LocalFree(lpMsgBuf);

		fprintf(stderr,"open failed");
	}
	LARGE_INTEGER laddr;
	//初期化
	laddr = getLaddress(_header);
		
//SetFilePointerEx(handle,laddr,NULL,FILE_BEGIN);//ヘッダの読み飛ばし。なぜかあってもなくても変わらない。・。。！！？

	_mmm->Fl = handle;
	ZeroMemory(_mmm->ol,sizeof(*_mmm->ol));
	_mmm->ol->Offset = laddr.LowPart;
	_mmm->ol->OffsetHigh = 0;
	_mmm->ol->hEvent = NULL;
	
	BOOL bRet = ReadFile(handle,_mmm->data,sizeof(float)*_size,&dwWriteSize, _mmm->ol);

	DWORD dwGle = GetLastError();
    if(!bRet && !(!bRet && ERROR_IO_PENDING == dwGle))
	{
		cout<<"Error file load"<<endl;
		abort();
	}

	//delete []wc;
	
}
void File::loadIndexFile(string file,unsigned int* data,int header,int fx, int fy, int fz)//indexファイル読み込み用
{
	HANDLE Fl;
	DWORD dwWriteSize;
	//wchar_t *wc = new wchar_t[file.size()+1];
	//mbstowcs(wc,file.data(),file.size()+1);

	Fl = CreateFile(file.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(Fl == INVALID_HANDLE_VALUE)
	{
		LPVOID lpMsgBuf;
		//ここにチェックしたい処理を書く

		FormatMessage(				//エラー表示文字列作成
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);
		cerr<< (const char*)lpMsgBuf<<endl;
		abort();
		MessageBox(NULL, (LPCSTR)lpMsgBuf, NULL, MB_OK);	//メッセージ表示

		LocalFree(lpMsgBuf);

		fprintf(stderr,"open failed");
	}
	LARGE_INTEGER laddr;
	//初期化
	laddr = getLaddress(header);
	SetFilePointerEx(Fl,laddr,NULL,FILE_BEGIN);
	ReadFile(Fl,data,sizeof(int)*fx*fy*fz, &dwWriteSize, NULL);
	//delete []wc;
	CloseHandle(Fl);
}


GLuint* File::getTexaddress(int index){
	return &this->texName[index];
}
int File::getTexCompressInfo(Block bl){
	bl=bl.returnParent();
	int id=this->texCompressInfo[bl.level][(bl.x*bl.bnumy+bl.y)*bl.bnumz+bl.z];
	printf("id=%d,",id);
	return id;
}
/*! @brief これはmiffyオリジナル。優先度スコアの工夫*/
int File::getMinIndexInTex(ULONGLONG *reqquevalue){
	int tex_minIndex=-1;
	
	
//	if(texmemfullflag){printf("true\n");}else{printf("false\n");}
	if(!texmemfullflag){
	int myindex =0;
	while(myindex<g_MaxTexNum-1){//まず空いてるところを探す
			//if(myindex>=MAXTEXNUM){printf("index,");break;}
			if(myindex>=g_MaxTexNum-1){texmemfullflag=true; break;}
			if(this->texblock[myindex].texdata==NULL && this->texblock[myindex].loadStartFlag==false){
				
				return myindex;
			}
			myindex++;
		}
	
	}else{//もしどの場所にもデータが詰まってたら
	
	//printf("minIndex=%d\n",tex_minIndex);
	int firstNonZeroPlace=ULONGLONGSIZE-1;
	ULONGLONG mincontent=0xffffffffffffffff;	
	for(int i = 0;i<=g_MaxTexNum;i++)
	{	
		
		ULONGLONG needvalue[ULONGLONGSIZE];
		for(int j=0;j<ULONGLONGSIZE;j++){needvalue[j]= this->texblock[i].needflag[j];}
		int j;
		for(j=ULONGLONGSIZE-1;j>-1;j--){
			if(needvalue[j]!=0){
				break;
			}
		
			if(j<=0){//一番下の桁に行ってもダメだったとき
				//printf("i=%d,",i);
				tex_minIndex=i;//全ての数が完全に0のindex
				
				if(this->texblock[i].texdata!=NULL ){
				Block minbl;
				//書き込まれるブロック確定	古いブロック情報を削除
				memcpy(&minbl,texblock[i].block,sizeof(Block));
				//書き込まれる側のブロック削除
				this->deleteMemory(&this->texMemMap[minbl.level][minbl.x][minbl.y][minbl.z]);	
				
				}
				return tex_minIndex;
				
			}//needflag完全に0のblock index
		}//j
		//if(firstNonZeroPlace>=ULONGLONGSIZE){printf("error ulonglong");exit(0);}
		//ここまでで初めての0じゃないjがどこであるかが決まる
		if(firstNonZeroPlace>j){
			tex_minIndex=i;
			firstNonZeroPlace=j;
			mincontent=needvalue[j];
			
		}else if(firstNonZeroPlace==j && mincontent>needvalue[j]){
			mincontent=needvalue[j];
			tex_minIndex=i;
			
		}else if(firstNonZeroPlace==j && mincontent==needvalue[j]){//同じ値があったら
			//更に深くたどる
			int j2=firstNonZeroPlace-1;
			while(j2>=0){
				if(this->texblock[tex_minIndex].needflag[j2]>needvalue[j2]){
					tex_minIndex=i;
				
				
					break;
				}else if(this->texblock[tex_minIndex].needflag[j2]==needvalue[j2]){
					j2--;
				}else{//今のminの方が小さかったらそのままにする
					
					break;}
			}//end j2
		}//同じ値があったら
		
		
		}//for i
	//reqque value が mamblockより大きくないかチェック

		int j=ULONGLONGSIZE-1;
			for(j=ULONGLONGSIZE-1;j>-1;j--){
			if(reqquevalue[j]!=0){
				break;//j roopを抜ける
			}
		
			if(j<=0){//一番下の桁に行ってもダメだったとき
				return -1;//reqqueの値が０だったとき
				break;//j roopを抜ける
			}//needflag完全に0のblock index
		}//j
			//j==reqqueの最初に0でない場所がどこか決まった
		if(firstNonZeroPlace>j){//reqqueの方が値が小さい場合
			return -1;
		}else if(firstNonZeroPlace==j && texblock[tex_minIndex].needflag[j]>reqquevalue[j]){
			return -1;//
			
		}else if(firstNonZeroPlace==j && mincontent==reqquevalue[j]){//同じ値があったら
			//更に深くたどる
			int j2=firstNonZeroPlace-1;
			while(j2>=0){
				if(this->texblock[tex_minIndex].needflag[j2]>reqquevalue[j2]){
					return -1;
				}else if(this->texblock[tex_minIndex].needflag[j2]==reqquevalue[j2]){
					j2--;	
				}else{//今のminの方が小さかったらそのままにする
					break;}//j2 roopを抜ける
			}//end j2
		}//同じ値があったら
	

	//if(tex_minIndex>=MAXTEXNUM){printf("minfFindijou desu!\n");exit(0);}
	if(this->texblock[tex_minIndex].texdata!=NULL ){	
	Block minbl;
		//書き込まれるブロック確定	古いブロック情報を削除
		memcpy(&minbl,texblock[tex_minIndex].block,sizeof(Block));
		//書き込まれる側のブロック削除
		this->deleteMemory(&this->texMemMap[minbl.level][minbl.x][minbl.y][minbl.z]);	
		
	}
	
	
	}//もし満杯だった場合
	return tex_minIndex;
}
void File::loadMainToTex(Block bl,Cg* Cg,CGparameter decal,ULONGLONG _reqvalue)
{

	LARGE_INTEGER temp1, temp2;
	LARGE_INTEGER freq;
	
	//this->RockBlock(bl);
	if(this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex!=NULL){
		this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex->replaceable=false;//テクスチャメモリにこれから転送するからメインメモリで上書き不可にしておく 
	}
	
	int	minIndex=this->getMinIndexInTex(&_reqvalue);
	if(minIndex==-1){return;}//-1だったらロードは諦めるべし
	
		if(texblock[minIndex].block->level != -1)
		{
			Block minbl;
			//書き込まれるブロック確定	
			memcpy(&minbl,texblock[minIndex].block,sizeof(Block));
			//書き込まれる側のブロック削除
			this->deleteMemory(&this->texMemMap[minbl.level][minbl.x][minbl.y][minbl.z]);	
		}
		this->texblock[minIndex].loadStartFlag=true;
		//ブロック登録
		this->texblock[minIndex].texdata = getTexaddress(minIndex);
		*this->texblock[minIndex].block = bl;
		this->texblock[minIndex].needflag[ULONGLONGSIZE-1] =_reqvalue;// = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex->needflag;

		LARGE_INTEGER temp2_1, temp2_2;
		LARGE_INTEGER freq2;
		static float temp_sum2;
		static int temp_count = 1;
		static int    count2= 1;

		//時間測定用．ないほうが速いかも！！
		if(count2 >= SAMPLEINTERVAL)
		{
			glFinish();
			QueryPerformanceCounter(&temp2_1);
			QueryPerformanceFrequency(&freq2);
		}

		glBindTexture(GL_TEXTURE_3D, *this->texblock[minIndex].texdata);
		MainMemManage* mb = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex;
		glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,(BLX+2),(BLY+2),(BLZ+2),GL_ALPHA,GL_FLOAT,mb->data);//これがないとボリュームがなくなる。
		glBindTexture(GL_TEXTURE_2D, 0);//これはなくても支障がない。	
		//ブロック登録
		this->texMemMap[bl.level][bl.x][bl.y][bl.z].texdataIndex = &this->texblock[minIndex];	

		if(count2 >= SAMPLEINTERVAL)
		{
			//時間測定用．ないほうが速いかも！！
			glFinish();
			QueryPerformanceCounter(&temp2_2);
			temp_sum2 += (float)(temp2_2.QuadPart-temp2_1.QuadPart)*1000/(float)freq2.QuadPart;

			this->texLoadTime =  temp_sum2/temp_count;
			//CPUからGPUへのテクスチャ転送時間．
			count2 = 0;
			this->texLoadflag = true;

			temp_count++;
		}
		count2++;
	

	//this->UnRockBlock(bl);
	
}
void File::initTexLoad(int _level){
	//初めにテクスチャメモリにいっぱいロードするぞ
	int counter=0;
	for(int level=4;level>=0;level--){
		int numx=(int)pow(2.0,(double)(NUMLEVEL-level));
		int numy=(int)pow(2.0,(double)(NUMLEVEL-level));
		int numz=(int)pow(2.0,(double)(NUMLEVEL-level-1));
		//x:9,y=18,z=1でダメになる。たぶん、GPUメモリのせい。
		for(int x=0;x<numx;x++){
			for(int y=0;y<numy;y++){
				for(int z=0;z<numz;z++){
					Block block(x,y,z,level,numx,numy,numz);
					int id=getIndexblock(block);
					if(counter<g_MaxTexNum){
						if(id!=0){
								//ブロック登録
								texblock[counter].texdata = getTexaddress(counter);//texName[counter]
								*texblock[counter].block = block;
								for(int j=0;j<ULONGLONGSIZE-1;j++){this->texblock[counter].needflag[j] = 0;}//this->bs[block.level][block.x][block.y][block.z].dataIndex->needflag;
								glBindTexture(GL_TEXTURE_3D, *this->texblock[counter].texdata);
								MainMemManage* mb =this->mainMemMap[block.level][block.x][block.y][block.z].dataIndex; 
								glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,(BLX+2),(BLY+2),(BLZ+2),GL_ALPHA,GL_FLOAT,mb->data);
								//ブロック登録
								texMemMap[block.level][block.x][block.y][block.z].texdataIndex = &this->texblock[counter];	
								HDblockLoadCount[block.level][block.x][block.y][block.z]++;
								counter++;
						}
					}
				}
			}
		}
	}//end level
}
void File::initMainLoad(int _level){
	//初期状態としてめいっぱい、低解像度順にロードしておく
	int counter=0;
	for(int level=4;level>=0;level--){
	int numx=(int)pow(2.0,(double)(NUMLEVEL-level));
	int numy=(int)pow(2.0,(double)(NUMLEVEL-level));
	int numz=(int)pow(2.0,(double)(NUMLEVEL-level-1));
	
		for(int x=0;x<numx;x++){
			for(int y=0;y<numy;y++){
				for(int z=0;z<numz;z++){
					Block block(x,y,z,level,numx,numy,numz);
					int id=this->getIndexblock(block);
					if(counter<MAXBNUM){
					if(id!=0){
					//ブロック登録
					memblock[counter].data = &datapool[counter*(BLX+2)*(BLY+2)*(BLZ+2)];//minIndex=0〜４まである。
					*this->memblock[counter].block = block;
					//memblock[counter].FromLoadTime = 1;	
					//memblock[counter].needflag =reqque[counter].needflag;
					stringstream filename;
					filename<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/level"<<level<<"/VertebralBody-"<<level<<"_"<<x<<"_"<<y<<"_"<<z<<".dff";
					//printf("%s\n",filename.str().c_str());
					this->memblock[counter].replaceable = true;//ただ今ロード中
					loadFile(filename.str(),8,(BLX+2)*(BLY+2)*(BLZ+2),&memblock[counter]);
					this->mainMemMap[level][x][y][z].dataIndex = &this->memblock[counter];//ここでエラー
					
					//this->CheckLoadComplete(counter);
					
					memblock[counter].replaceable = true;
				//ロードし終わったら、キューから消去
				//this->reqque[counter].block->level = -1;//ここでエラー
				//this->bs[block.level][block.x][block.y][block.z].queIndex = NULL;
					HDblockLoadCount[level][x][y][z]++;
				counter++;
					}
					}
		}

		}

		}
	}//end level
}
void File::setTexBlock(Cg* Cg,CGparameter decal,Block block)//ブロックごとのテクスチャ名をシェーダに渡したりする。
{
	TexMemManage* tb = this->texMemMap[block.level][block.x][block.y][block.z].texdataIndex;
	Cg->SetTextureParameter(Cg->vdecalParam,*tb->texdata);//texdataにはテクスチャ名（GLuint）が入ってる。1〜4の値がえんえんと入る。
	
}

/*! @brief int mostWantedIndex は私が加えた
	@ret false=新しくロードすべきものが何もないとき true=何かロードしたよん。
*/
bool File::loadHDToMain(int mostWantedIndex)
{//printf("bnumx[%d]\n",reqque[3].block->bnumx);
	if(mostWantedIndex == -1){return false;}//新しくロードすべきものが何もないとき
	Block mostWantedBlock;
	mostWantedBlock = *this->reqque[mostWantedIndex].block;
			//一番欲しいブロックがまだメインメモリにロードされていないなら
		if(this->mainMemMap[mostWantedBlock.level][mostWantedBlock.x][mostWantedBlock.y][mostWantedBlock.z].dataIndex == NULL)
		{//それを入れる場所を探す


			int insertPlace =this->getMinIndexInMain(this->reqque[mostWantedIndex].needflag);
			if(memblock[insertPlace].data!=NULL && insertPlace!=-1){
					Block minbl;

					//書き込まれる新しいブロック確定	
					memcpy(&minbl,memblock[insertPlace].block,sizeof(Block));
					//書き込まれる側の古いブロック削除
					this->deleteMemory(&this->mainMemMap[minbl.level][minbl.x][minbl.y][minbl.z]);
			}
		
			//書き込み先が確定できたら読込み
			if(insertPlace == -1){return false;}
			

				this->memblock[insertPlace].loadStartFlag=true;//これからここにロードするって決まったから
				this->memblock[insertPlace].replaceable=false;//とりあえず、上書き禁止！
				//ブロック登録
				this->memblock[insertPlace].data = &this->datapool[insertPlace*(BLX+2)*(BLY+2)*(BLZ+2)];
				*this->memblock[insertPlace].block = mostWantedBlock;
				for(int j=0;j<ULONGLONGSIZE;j++){memblock[insertPlace].needflag[j]=reqque[mostWantedIndex].needflag[j];}

				stringstream volumefile;
				volumefile<<m_Path<<m_DataName<<"_"<<BLX<<"_"<<BLY<<"_"<<BLZ<<"/"<<"level"<<mostWantedBlock.level<<"/"<<"VertebralBody"<<"-"<<mostWantedBlock.level<<"_"<<mostWantedBlock.x<<"_"<<mostWantedBlock.y<<"_"<<mostWantedBlock.z<<".dff";
				HDblockLoadCount[mostWantedBlock.level][mostWantedBlock.x][mostWantedBlock.y][mostWantedBlock.z]++;
				//mostWantedBlock.printBlockInfo("HDtoMain",insertPlace);			
				//時間測定用．ないほうが速いかも！！
				QueryPerformanceCounter(&this->mTimeStart[insertPlace]);
				QueryPerformanceFrequency(&this->freq);			
				loadFile(volumefile.str(),8,(BLX+2)*(BLY+2)*(BLZ+2),&memblock[insertPlace]);
				this->mainMemMap[mostWantedBlock.level][mostWantedBlock.x][mostWantedBlock.y][mostWantedBlock.z].dataIndex = &this->memblock[insertPlace];//ここでエラー
				QueryPerformanceCounter(&this->mTimeEnd[insertPlace]);
				this->temp_sum += (float)(this->mTimeEnd[insertPlace].QuadPart-this->mTimeStart[insertPlace].QuadPart)*1000.0f/(float)this->freq.QuadPart;
				this->loadTime =  this->temp_sum/this->temp_count;
				//ハードディスクからブロック読み込み
				//floadtime.str("");
				//floadtime<<"avarage block HD to CPU transfer time [msec]:"<<this->loadTime;
				this->temp_count++;
				//ロードし終わったら、キューから消去
				this->reqque[mostWantedIndex].block->level = -1;
				this->mainMemMap[mostWantedBlock.level][mostWantedBlock.x][mostWantedBlock.y][mostWantedBlock.z].queIndex = NULL;
				
				this->memblock[insertPlace].loadStartFlag=false;
				return true;
			
		}//if(this->mainMemMap[maxbl.level][maxbl.x][maxbl.y][maxbl.z].dataIndex == NULL)
}//void File::loadHDToMain()

int File::getIndexblock(Block bl)
{
	int id;
	id = this->indexblock[bl.level][(bl.x*bl.bnumy+bl.y)*bl.bnumz+bl.z];//idは0か1の値 この３次元配列変！？ でもこっちが正しいみたい。

	

	return id;
}

ULONGLONG File::getReqbit(double* modelmatrix,double* projmatrix,Block bl,frustum<double> fr,frustum<double>::FrustumName fname)
{
	
	float blockPosition[4];
	//ブロックの中心点計算
	float x = (2.0f*bl.x-bl.bnumx+1.0f)*Block::brX*Block::iniX/bl.bnumx;
	float y = (2.0f*bl.y-bl.bnumy+1.0f)*Block::brY*Block::iniY/bl.bnumy;
	float z = (2.0f*bl.z-bl.bnumz+1.0f)*Block::brZ*Block::iniZ/bl.bnumz;

	blockPosition[0] = x;
	blockPosition[1] = y;
	blockPosition[2] = z;
	blockPosition[3] = 1.0f;
	
	Block::mMtx(modelmatrix, blockPosition, blockPosition);
	Block::mMtx(projmatrix,  blockPosition, blockPosition);
	//なんでprojMatrixをかける？？
	float projx = blockPosition[0]/blockPosition[3];
	float projy = blockPosition[1]/blockPosition[3];
	float projz = blockPosition[2]/blockPosition[3];

	float blockdis =  sqrt(projx*projx+projy*projy);
	
	
	//初期化
	ULONGLONG reqbit = 0;


	//高，低フラスタムフラグ(バックアップ優先)
	if(fname == frustum<double>::back)
		reqbit = reqbit|0x8000000000000000;//0が15+8
	

	//距離フラグ
	if(projz > fr.dis2)
	{//領域3 01
		reqbit = reqbit|0x2000000000000000;//
	}
	else if(projz > fr.dis1)
	{//領域２ 10
		reqbit = reqbit|0x4000000000000000;
	}
	else
	{//領域1 11（一番必要）
		reqbit = reqbit|0x2000000000000000;
		reqbit = reqbit|0x4000000000000000;
	}

	//画面中心からの距離フラグ	スクリーン中心分割
	if(blockdis > fr.radious2)
	{//領域3　01
		reqbit = reqbit|0x0800000000000000;
	}
	else if(blockdis > fr.radious1)
	{//領域2　10
		reqbit = reqbit|0x100000000000000;
	}
	else
	{//領域１ 11(最も必要)
		reqbit = reqbit|0x0800000000000000;
		reqbit = reqbit|0x100000000000000;
	}



	return reqbit;
}
int File::getMinIndexInReq(){
			//最も少ないカウントのブロックに転送
		int minIndex;
		Block minbl;
		
		//まず空いてるところを探す
		static bool fullflag = false;
		int firstNonZeroPlace=ULONGLONGSIZE-1;
		for(int i = 0;i <QSIZE;i++)
		{		
			if(this->reqque[i].block->level == -1)
			{//reqqueで空いてるところがあればそこに入れる
				minIndex = i;
				return i;
			}
			if(i==QSIZE-1){fullflag=true;}
		}
	
		
			ULONGLONG mincontent=0xffffffffffffffff;
			for(int i = 0;i < QSIZE;i++)
			{
				ULONGLONG needvalue[ULONGLONGSIZE];
				for(int j=0;j<ULONGLONGSIZE;j++){needvalue[j]= this->reqque[i].needflag[j];}//現在の値をコピー
				int j;
				for(j=ULONGLONGSIZE-1;j>-1;j--){
			if(needvalue[j]!=0){
				break;
			}
		
			if(j<=0){//一番下の桁に行ってもダメだったとき
				//minIndex=i;//全ての数が完全に0のindex
				return i;
				break;
			}//needflag完全に0のblock index
				}
					//ここまでで初めての0じゃないjがどこであるかが決まる
		if(firstNonZeroPlace>j){
			minIndex=i;
			firstNonZeroPlace=j;
			mincontent=needvalue[j];
			
		}else if(firstNonZeroPlace==j && mincontent>needvalue[j]){
			mincontent=needvalue[j];
			minIndex=i;
			
		}else if(firstNonZeroPlace==j && mincontent==needvalue[j]){//同じ値があったら
			//更に深くたどる
			int j2=firstNonZeroPlace-1;
			while(j2>=0){
				if(this->reqque[minIndex].needflag[j2]>needvalue[j2]){
					minIndex=i;
					break;
				}else if(this->reqque[minIndex].needflag[j2]==needvalue[j2]){
					j2--;
				}else{//今のminの方が小さかったらそのままにする
					break;}
			}//end j2
		}//同じ値があったら
	}//end que
			//書き込まれるブロック確定
			memcpy(&minbl,this->reqque[minIndex].block,sizeof(Block));
			this->mainMemMap[minbl.level][minbl.x][minbl.y][minbl.z].queIndex = NULL;

		return minIndex;
}

void File::mainBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<double> fr,frustum<double>::FrustumName fname)
{
	ULONGLONG reqbit = this->getReqbit(modelmatrix,projmatrix,bl,fr,fname);
	MainMemManage* mb = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex;
	RequestQueue*  qb = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].queIndex;
	if(mb != NULL) //メモリにあるとき
	{
		mb->needflag[ULONGLONGSIZE-1] = mb->needflag[ULONGLONGSIZE-1]|reqbit;
	}
	else if(qb == NULL) //メモリになくてキューにも入っていないときキューに入れる
	{//どこに入れますか？
		int minIndex=this->getMinIndexInReq();
		//ブロック登録 ここ大事
		qb = &this->reqque[minIndex];
		*this->reqque[minIndex].block = bl;
		for(int j=0;j<ULONGLONGSIZE;j++){this->reqque[minIndex].needflag[j] = 0;}
		this->reqque[minIndex].needflag[ULONGLONGSIZE-1] = this->reqque[minIndex].needflag[ULONGLONGSIZE-1]|reqbit;//新しい情報を入れる
		this->mainMemMap[bl.level][bl.x][bl.y][bl.z].queIndex = &this->reqque[minIndex];	//あとでloadHDtoMainで使う
}
	else //メモリにはないがキューにはあるとき
	{	
		qb->needflag[ULONGLONGSIZE-1] = qb->needflag[ULONGLONGSIZE-1]|reqbit;//情報を更新
	}

}

int File::getTexLoadCompressInfo(Block testBlock){//既に存在しているものが何個あるか
	int p_exist=0;int c_exist=0;
	int p_notExist=0;int c_notExist=0;
	if(this->getIndexblock(testBlock)){
	if(this->texBlockExist(testBlock)){
		//p_Exist++;
	}else{
		p_notExist++;
	}
	}
	for(int z=0;z<2;z++){
	for(int y=0;y<2;y++){
	for(int x=0;x<2;x++){
		Block child(testBlock.x*2+x,testBlock.y*2+y,testBlock.z*2+z,testBlock.level-1,testBlock.bnumx*2,testBlock.bnumy*2,testBlock.bnumz*2);
		if(this->getIndexblock(child)){
			if(this->texBlockExist(child)){
			//	c_Exist++;
			}else{
				c_notExist++;//欲しいけどない数
			}	
		}
	}}
	}
	
	return (c_notExist-p_notExist);
}
void File::texBlockRequest(double* modelmatrix,double* projmatrix,Block bl,frustum<double> fr,frustum<double>::FrustumName fname)
{
	
	ULONGLONG reqbit = this->getReqbit(modelmatrix,projmatrix,bl,fr,fname);
	
	TexMemManage* tb = this->texMemMap[bl.level][bl.x][bl.y][bl.z].texdataIndex;
	tb->needflag[ULONGLONGSIZE-1] = tb->needflag[ULONGLONGSIZE-1]|reqbit;
	

}
	




Block File::lowBlockExist(Block bl)
{
	do{
		bl = bl.getLowblock();
	}while(this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex == NULL && bl.level != NUMLEVEL - 1);

	if(this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex != NULL)
		return bl;
	else
	{
		Block noblock;
		noblock.level = -1;
		return noblock;
	}
}

Block File::lowBlockExistFromRoot(Block bl)
{
	do{
		bl = bl.getMultiLowblock((NUMLEVEL - 1) - bl.level);
	}while(this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex == NULL && bl.level != 0);

	if(this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex != NULL)
		return bl;
	else
	{
		Block noblock;
		noblock.level = -1;
		return noblock;
	}
}




bool File::highBlockExist(Block bl)
{
	bool flag = false;

	if(bl.level != 0)
	{
		if(this->blockExist(Block(2*bl.x  ,2*bl.y  ,2*bl.z       ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||	this->blockExist(Block(2*bl.x+1,2*bl.y  ,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x  ,2*bl.y+1,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x  ,2*bl.y  ,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x+1,2*bl.y+1,2*bl.z  ,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x  ,2*bl.y+1,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x+1,2*bl.y  ,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true
			||  this->blockExist(Block(2*bl.x+1,2*bl.y+1,2*bl.z+1,bl.level-1,bl.bnumx*2,bl.bnumy*2,bl.bnumz*2)) == true)
			flag = true;
	}
	return flag;
}



bool File::texBlockExist(Block bl)
{
	if(this->texMemMap[bl.level][bl.x][bl.y][bl.z].texdataIndex != NULL)
		return true;
	else
		return false;
}


Block File::texLowBlockExist(Block bl)
{
	do{
		bl = bl.getLowblock();
	}while(this->texMemMap[bl.level][bl.x][bl.y][bl.z].texdataIndex == NULL && bl.level != NUMLEVEL - 1);

	if(this->texMemMap[bl.level][bl.x][bl.y][bl.z].texdataIndex != NULL)
		return bl;
	else
	{
		Block noblock;
		noblock.level = -1;
		return noblock;
	}
}




GLuint File::getTexName(int index)
{
	return this->texName[index];
}


float File::getTexLoadTime()
{
	return this->texLoadTime;
}


bool  File::getTexLoadflag()
{
	return this->texLoadflag;
}

float File::getMainLoadTime()
{
	return this->loadTime;
}


void File::countThreadTime()
{/*ハードディスク読込み時間測定用らしい*/
	QueryPerformanceCounter(&this->fTime);
	QueryPerformanceFrequency(&freq);
	this->fileThreadTime = (float)(this->fTime.QuadPart-this->fTimeprev.QuadPart)*1000.0f/(float)freq.QuadPart;
	this->fTimeprev = this->fTime;
}
int File::getCompressInfo(Block testBlock){
	return texCompressInfo[testBlock.level-1][(testBlock.x*testBlock.bnumy+testBlock.y)*testBlock.bnumz+testBlock.z];
}
float File::getThreadTime()//論文のp35 (file.getThreadTime()=1フレーム描画するのにかかる時間！！？？
{
	return this->fileThreadTime;/*ハードディスク読込み時間測定用らしい*/
}

double File::getMainToTexTime(){
	return this->maintotextime;
}
void File::deleteMemory(){
	delete[] datapool;
}
int File::returnMainMemInfo(int _i){
	return this->memblock[_i].block->level;
}
int File::returnTexMemInfo(int _i){
	return this->texblock[_i].block->level;
}
void File::updateNeedScoreInMain(double* modelmatrix,double* projmatrix,Block bl,frustum<double> fr,frustum<double>::FrustumName fname){
	ULONGLONG reqbit = this->getReqbit(modelmatrix,projmatrix,bl,fr,fname);
	MainMemManage* mb = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].dataIndex;
	RequestQueue*  qb = this->mainMemMap[bl.level][bl.x][bl.y][bl.z].queIndex;
	if(mb != NULL) //メモリにあるとき
	{//優先度スコアの上位桁に対してor演算する
		mb->needflag[ULONGLONGSIZE-1] = mb->needflag[ULONGLONGSIZE-1]|reqbit;
	}
	if(qb!=NULL){
		qb->needflag[ULONGLONGSIZE-1] = qb->needflag[ULONGLONGSIZE-1]|reqbit;
	}
}
