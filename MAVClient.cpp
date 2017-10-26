
#define LOG_TAG "MtkCam/CamClient1/MAVClient"

#include "MAVClient.h"
//
//==============================================
#include <camera/MtkCamera.h>
//
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <IParamsManager.h>
#include <ICamAdapter.h>
//==============================================
#include "jpeg_hal.h"
#include "PreviewFeatureBufMgr.h"

#include "IBaseCamExif.h"
#include "CamExif.h"
#include "aaa_hal_base.h"

#define restrict		// for pre-C99 standard compiling

#include <almashot.h>
#include <almabestshot.h>
#include <string>


using namespace android;
using namespace NSCamClient1;

//
/******************************************************************************
*
*******************************************************************************/
MAVClient*  MAVClientObj;
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define NO_ERROR 0
/******************************************************************************
 *
 ******************************************************************************/
#define debug
#ifdef debug
#include <fcntl.h>
#include <sys/stat.h>
bool
savedataToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}
#endif
/******************************************************************************
*
*******************************************************************************/
MBOOL
dumpImg(
    MUINT8 *addr,
    MUINT32 size,
    char const * const tag,
    char const * const filetype,
    uint32_t filenum)
{
#if 0
    char value[32] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);
    if (enable == 0)
    {
        return false;
    }
#endif
    //
    char fileName[64];
    sprintf(fileName, "/sdcard/%s_%d.%s",tag , filenum, filetype);
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        MY_LOGE("fail to open file to save img: %s", fileName);
        return false;
    }

    fwrite(addr, 1, size, fp);
    fclose(fp);

    return true;
}
/******************************************************************************
*
*******************************************************************************/

bool
dumpBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    CAM_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    CAM_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        CAM_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    CAM_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            CAM_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    CAM_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
MAVClient::
MAVClient(int ShotNum , sp<IParamsManager> mpParamsMgr)
    : MAVnum(ShotNum)
    , mpCamMsgCbInfo(new CamMsgCbInfo)
{
    MY_LOGD("Freefocus + this(%p) num MAVnum :%d", this,MAVnum);
    MAVClientObj = this;

	//char *tfilename;
	//If device resume , it will fail
	//tfilename = (char *)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string();
	//memcpy(filename , (char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string() , sizeof(char)*strlen((char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string()));
	//MY_LOGD("Freefocus MAVClient string length:%d",(int)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string());
	//MY_LOGD("Freefocus MAVClient end");
	//MY_LOGD("Freefocus tfilename :%s  , string length:%d", filename,strlen((char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string()));
}


/******************************************************************************
 *
 ******************************************************************************/
MAVClient::
~MAVClient()
{
    MY_LOGD("-");
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
MAVClient::
allocMem(IMEM_BUF_INFO &memBuf)
{
    if (mpIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error \n");
        return MFALSE;
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (mpIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error \n");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
MAVClient::
deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (mpIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }

    if (mpIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MAVClient::
init(int bufwidth,int bufheight)
{

    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    MY_LOGD("Freefocus init bufwidth : %d , bufheight : %d",bufwidth,bufheight);
    mMAVFrameWidth  = bufwidth;
    mMAVFrameHeight = bufheight;
    //mMAVFrameSize   = (mMAVFrameWidth * mMAVFrameHeight * 3 / 2);
	mMAVFrameSize   = (mMAVFrameWidth * mMAVFrameHeight * 3/2);
	mCancel = MTRUE;
    mStop = MFALSE;
    //
    // (1) Create frame buffer buffer
    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        MY_LOGE("g_pIMemDrv is NULL \n");
        return false;
    }
    MY_LOGD("Freefocus mMAVFrameWidth %d mMAVFrameHeight %d mMAVFrameSize %d MAVnum %d",mMAVFrameWidth,mMAVFrameHeight,mMAVFrameSize,MAVnum);
    for(int i=0;i<MAVnum;i++)
    {
        mpframeBuffer[i].size =  mMAVFrameSize;
        if(!(allocMem(mpframeBuffer[i])))
        {
            mpframeBuffer[i].size = 0;
            MY_LOGE("Freefocus [init] mpframeBuffer alloc fail");
            return false;
        }
        MY_LOGD("Freefocus [init] mpframeBuffer alloc index %d adr 0x%x",i,mpframeBuffer[i].virtAddr);
		MY_LOGD("Freefocus phyAddr [init] mpframeBuffer alloc index %d adr 0x%x",i,mpframeBuffer[i].phyAddr);
	}
    // (4) reset member parameter
    mMAVaddImgIdx = 0;
    mMAVFrameIdx = 0;
    // (5) ExifHeaderBuf create

    pMAVExifHeaderBuf = new MUINT8[128 * 1024];
	uMAVExifHeaderSize = 0;

    ret = true;
    MY_LOGD("-");
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MAVClient::
uninit()
{
    MY_LOGD("+");
	MY_LOGD("MAVClient uninit ");
    for(int i=0;i<MAVnum;i++)
    {
        MY_LOGD("mpframeBuffer free %d adr 0x%x",i,mpframeBuffer[i].virtAddr);
		MY_LOGD("mpframeBuffer phyAddr free %d adr 0x%x",i,mpframeBuffer[i].phyAddr);
        if(!(deallocMem(mpframeBuffer[i])))
        {
            mpframeBuffer[i].size = 0;
            MY_LOGE("[uninit] mpframeBuffer alloc fail");
            return  MFALSE;
        }
    }
	//uninitThumbnailBuf();
	delete [] pMAVExifHeaderBuf;
    MY_LOGD("mpframeBuffer free done");
    MY_LOGD("-");
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MAVClient::
initThumbnailBuf(int bufsize)
{
	MY_LOGD(" MAVClient:: initThumbnailBuf ");

	tempBuf.size = bufsize;
    //mpframeBuffer[i].size =  mMAVFrameSize;
    if(!(allocMem(tempBuf)))
    {
        tempBuf.size = 0;
        MY_LOGE("Freefocus [init] mpframeBuffer alloc fail");
        return false;
    }
	uMAVThumbJpegSize = bufsize;
	pMAVThumbJpegBuf = (MUINT8*)tempBuf.virtAddr;
	return 0;
}
/******************************************************************************
 *
 ******************************************************************************/
#if 1
bool
MAVClient::
uninitThumbnailBuf()
{
	MY_LOGD(" MAVClient:: uninitThumbnailBuf ");


	//tempBuf.size = bufsize;
    //mpframeBuffer[i].size =  mMAVFrameSize;
    if(!(deallocMem(tempBuf)))
    {
        tempBuf.size = 0;
        MY_LOGE("Freefocus [uninit] mpframeBuffer alloc fail");
        return false;
    }

	return 0;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAVClient::
setImgCallback(ImgDataCallback_t data_cb)
{
    MY_LOGD("(notify_cb)=(%p)", data_cb);
    mDataCb = data_cb;
}

/******************************************************************************
 *
 ******************************************************************************/
void
MAVClient::
copyfilename(sp<IParamsManager> mpParamsMgr)
{
	MY_LOGD("Freefocus copyfilename filename :%s  ", (char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string());
	memcpy(filename , (char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string() , sizeof(char)*56/*mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string()*/);
	pParamsMgr = mpParamsMgr;
	MY_LOGD("Freefocus copyfilename finish  filename:%s",filename);
}

void
MAVClient::
clearfilename(void)
{
	//MY_LOGD("Freefocus copyfilename filename :%s  ", (char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string());
	memset(filename , '\0' , sizeof(char)*100 );
	MY_LOGD("Freefocus copyfilename finish  filename:%s",filename);
}

/******************************************************************************
 *
 ******************************************************************************/
bool
MAVClient::
stopFeature(int cancel , int curfocusindex)
{
    MY_LOGD("+");
    bool ret = false;
	int err;
    MY_LOGD("Freefocus CAM_CMD_STOP_MAV, do merge %d mMAVaddImgIdx %d MAVnum %d", cancel,mMAVaddImgIdx,MAVnum);
    mCancel = cancel;
    mStop = MTRUE;

    MY_LOGD("Freefocus  mMAVaddImgIdx : %d , MAVnum : %d",mMAVaddImgIdx,MAVnum);
    //if (mMAVaddImgIdx == MAVnum)
    {
        if(cancel)
	    mHalCamFeatureCompress(curfocusindex);

        //Temp add the releasing buf here
        uninit();
        //if (err != NO_ERROR) return err;

    }
	clearfilename();
	//
	//MY_LOGD("performCallback MAV capture");
    //mpCamMsgCbInfo->mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_MAV, 0, mpCamMsgCbInfo->mCbCookie);
	 MY_LOGD("leo Freefocus stopFeature");
    MY_LOGD("-");
    return  true;
}
#if 0
void printIndexArray(int* indexarray,int idxwidth , int idxheight)
{
	MY_LOGD("Freefocus printIndexArray start");
	#if 0
	for(int s=0;s<idxwidth;s++)//y
	{
		for(int t=0; t<idxheight ; t++)//x
		{


		}
	}
	#endif

}
#endif
/******************************************************************************
*
*******************************************************************************/
void
delIndexMatrixFile(void)
{

    remove("/sdcard/IndexMatrix.txt");
	remove("/sdcard/HighestValueMatrix.txt");

}

/******************************************************************************
*
*******************************************************************************/
MBOOL
dumpIndexMatrix(
    MUINT8 Index,
	MUINT8 height_block_index,
	MUINT8 width_block_index,
	MUINT8 total_height_block,
	MUINT8 total_width_block
	)
{
    //
    char c1[20];
    char fileName[64];
    strcpy(fileName, "/sdcard/IndexMatrix.txt");
    FILE *fp = fopen(fileName, "a");
    if (NULL == fp)
    {
        MY_LOGE("fail to open file to save img: %s", fileName);
        return false;
    }
	//MY_LOGE(" dumpIndexMatrix width_block_index:%d , height_block_index:%d ,total_width_block:%d",width_block_index,height_block_index,total_width_block);
	if(width_block_index==(total_width_block-1)){
		MY_LOGE(" width_block_index==(total_width_block-1)");
		fprintf(fp,"%d\n",Index);
	}else{
		fprintf(fp,"%d,",Index);
	}
	fclose(fp);

    return true;
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
dumpHighestValueMatrix(
    MUINT32 Index,
	MUINT8 height_block_index,
	MUINT8 width_block_index,
	MUINT8 total_height_block,
	MUINT8 total_width_block
	)
{
	//
    char c1[20];
    char fileName[64];
    strcpy(fileName, "/sdcard/HighestValueMatrix.txt");
    FILE *fp = fopen(fileName, "a");
    if (NULL == fp)
    {
        MY_LOGE("fail to open file to save img: %s", fileName);
        return false;
    }
	//MY_LOGE(" dumpIndexMatrix width_block_index:%d , height_block_index:%d ,total_width_block:%d",width_block_index,height_block_index,total_width_block);
	if(width_block_index==(total_width_block-1)){
		MY_LOGE(" width_block_index==(total_width_block-1)");
		fprintf(fp,"%d\n",Index);
	}else{
		fprintf(fp,"%d,",Index);
	}
	fclose(fp);

    return true;
}

/******************************************************************************
*for test
*******************************************************************************/
int NextStreamingBestPicSelect(MUINT8* pFreeFcousData, MUINT32 pFreeFcousDataLength , IMEM_BUF_INFO* YUVBufferInfo , UINT8 YUVNum , MUINT32 YUVWidth , MUINT32 YUVHeight , MUINT32 HorizontalBlockNum , MUINT32 VerticalBlockNum , MUINT32 CurFocusIndex)
{
	FILE *f;
	int i, y;
	unsigned sx, sy;
	int err;
	Uint8 *in[10];
	void *inst;
	int nImg;
	int len;
    float FramesScores[10];
    int BestFrames[10];
	int countnum;
    struct timeval t1,t2,t3,t4;
	unsigned long usec = 0;
	//================= FreeFocus ===============================
	MUINT8* pFreeFcousData1 = NULL;
#if 0
	MUINT32 Bwidth = 16;
	MUINT32 Bheight = 32;
	MUINT8 FREEFOCUS_NAME_DATASIZE = 9;
	MUINT8 FREEFOCUS_IMAGENUM_DATASIZE = 1;//CHAR
	MUINT8 FREEFOCUS_HORIZONTL_BLOCK_DATASIZE = 1;
	MUINT8 FREEFOCUS_VERTICAL_BLOCK_DATASIZE = 1;
	MUINT8 FREEFOCUS_BLOCK_WIDTH_DATASIZE = 1;
	MUINT8 FREEFOCUS_BLOCK_HEIGHT_DATASIZE = 1;
	MUINT32 FREEFOCUS_TOTAL_ARRAY_SIZE = FREEFOCUS_NAME_DATASIZE+FREEFOCUS_IMAGENUM_DATASIZE+FREEFOCUS_HORIZONTL_BLOCK_DATASIZE+FREEFOCUS_VERTICAL_BLOCK_DATASIZE+FREEFOCUS_BLOCK_WIDTH_DATASIZE+FREEFOCUS_BLOCK_HEIGHT_DATASIZE+Bwidth*Bheight/2+1;
	MUINT8 FreeFcousData[FREEFOCUS_TOTAL_ARRAY_SIZE];
	MUINT8* pFreeFcousData = NULL;
	memset(FreeFcousData,0x00,sizeof(char) * FREEFOCUS_TOTAL_ARRAY_SIZE);
#endif
	//16:9
	//6M 3328X1872 ok 32x18
	//4M 2560X1440 ok 32x18
	//2M 1792X1008 ok 28x12
	//1M 1280X720  ok 20x10
	//4:3
	//5M 2560X1920 ok 32x24 or 32x16,32x20 fail
	//3M 2048X1536 ok 32x24 or 32x16
	//2M 1600X1200 ok 25x16

	//6M,4M
	if((YUVWidth == 1872 || YUVHeight == 1872) || (YUVWidth == 1440 || YUVHeight == 1440))
	{
		//for rotate
		if(YUVWidth > YUVHeight)
		{
			HorizontalBlockNum = 32;
			VerticalBlockNum = 18;
		}else{
			HorizontalBlockNum = 18;
			VerticalBlockNum = 32;
		}
	}else if(YUVWidth == 1920 || YUVHeight == 1920)//5M
	{
		//for rotate
		if(YUVWidth > YUVHeight)
		{
			HorizontalBlockNum = 32;
			VerticalBlockNum = 16;//20 error
		}else{
			HorizontalBlockNum = 16;//20 error
			VerticalBlockNum = 32;
		}
	}else if(YUVWidth == 1536 || YUVHeight == 1536)//3M
	{
		//for rotate
		if(YUVWidth > YUVHeight)
		{
			HorizontalBlockNum = 32;
			VerticalBlockNum = 16;
		}else{
			HorizontalBlockNum = 16;
			VerticalBlockNum = 32;
		}
	}else if(YUVWidth == 1008 || YUVHeight == 1008)//2M
	{
		//for rotate
		if(YUVWidth > YUVHeight)
		{
			HorizontalBlockNum = 28;
			VerticalBlockNum = 12;
		}else{
			HorizontalBlockNum = 12;
			VerticalBlockNum = 28;
		}
	}else if(YUVWidth == 720 || YUVHeight == 720)//1M
	{
		//for rotate
		if(YUVWidth > YUVHeight)
		{
			HorizontalBlockNum = 20;
			VerticalBlockNum = 10;
		}else{
			HorizontalBlockNum = 10;
			VerticalBlockNum = 20;
		}
	}else if(YUVWidth == 1200 || YUVHeight == 1200)//2M
	{
		//for rotate
		if(YUVWidth > YUVHeight)
		{
			HorizontalBlockNum = 25;
			VerticalBlockNum = 16;
		}else{
			HorizontalBlockNum = 16;
			VerticalBlockNum = 25;
		}
	}
	/*
	5M
	1440/18 = 80
	2560/32 = 80

	5M test
	1440/9 = 160
	2560/16= 160

	3M
	1152/16 = 72
	2048/32 = 64

	2M
	864 / 9 = 96
	1536 / 16 = 96
	*/
	MUINT32 Block_Width = YUVWidth/HorizontalBlockNum;//64;
	MUINT32 Block_Height = YUVHeight/VerticalBlockNum;//Block_w;

	MY_LOGD ("Freefocus YUVWidth: %d , YUVHeight:%d  , HorizontalBlockNum:%d  ,VerticalBlockNum:%d \n", YUVWidth,YUVHeight,HorizontalBlockNum,VerticalBlockNum);
	MY_LOGD ("Freefocus Block_Width: %d , Block_Height:%d ",Block_Width,Block_Height);
	//===========================================================
	//5M size
	sx = YUVWidth;//1440;
	sy = YUVHeight;//2560;

	nImg = YUVNum;//8;//ac-1;

	//clear buf
	memset(BestFrames,0,sizeof(int)*10);
	memset(FramesScores,0,sizeof(float)*10);
#if 1
	for (i=0; i<nImg; ++i)
	{
		in[i] = (Uint8 *)malloc(sx*sy+sx*sy/2);
        MY_LOGD("in[%d] =  %d\n", i, in[i]);
	}
#endif
	MY_LOGD ("N input frames: %d\n", nImg);

#if 1
	//make YUV buffer array
	//If it don't use memcpy ,it will crash.
	//I don't know why
	for (i=0; i<nImg; ++i)
	{
		MY_LOGD ("Reading image: %d\n", i);
        memcpy(in[i],(Uint8 *)YUVBufferInfo[i].virtAddr,sx*sy+sx*sy/2);
	}

#else
	char filearray[8][50]= { "/storage/sdcard0/bestshot_0.yuv", "/storage/sdcard0/bestshot_1.yuv", "/storage/sdcard0/bestshot_2.yuv", "/storage/sdcard0/bestshot_3.yuv", "/storage/sdcard0/bestshot_4.yuv", "/storage/sdcard0/bestshot_5.yuv", "/storage/sdcard0/bestshot_6.yuv", "/storage/sdcard0/bestshot_7.yuv" };
	for (i=0; i<nImg; ++i)
	{
		MY_LOGD ("Reading image: %s\n", filearray[i]);

		f = fopen(filearray[i],"rb");
		(void)fread(in[i],sx*sy+sx*sy/2,1,f);
		fclose (f);
	}
#endif
    gettimeofday(&t1, NULL);

	err = AlmaShot_Initialize(1);
	if (err)
	{
		MY_LOGD ("Errors initializing almashot (%d)...\n", err);
		exit(-4);
	}

	MY_LOGD ("Init completed ...\n");

	gettimeofday(&t3, NULL);
	//=================== Makernote=============================
	//FreeFcousData[FREEFOCUS_TOTAL_ARRAY_SIZE] = '\0';
#if 1
	//fill freefocus name
	memcpy(pFreeFcousData,"FREEFOCUS",sizeof(char)*9);
	//fill image num
	memset(pFreeFcousData+9,YUVNum/*0x8*/,sizeof(char));
	//fill horizontal block
	memset(pFreeFcousData+10,HorizontalBlockNum/*0x10*/,sizeof(char));
	//fill vertical block
	memset(pFreeFcousData+11,VerticalBlockNum/*0x20*/,sizeof(char));
	//fill block width
	memset(pFreeFcousData+12,Block_Width/*0x80*/,sizeof(char));
	//fill block height
	memset(pFreeFcousData+13,Block_Height/*0x80*/,sizeof(char));
	pFreeFcousData1 = pFreeFcousData+14;
#endif
	//==========================================================
//   it causes error return
//    if (x_st < 0
//        || y_st < 0
//        || x_en > sx - 1
//        || y_en > sy - 1
//        || x_en - x_st < 128 - 1
//        || y_en - y_st < 128 - 1)
//        return ALMA_INVALID_BUFFERS;
	int ret = 5;

#if 1
//for verified 64x64
#if 0
	for(int i = 0; i < VerticalBlockNum ; i++)
	{
		Uint8 BackupBestIndex = 0;

		for(int j = 0; j < HorizontalBlockNum ; j++)
		{
			ret = BestShot_SelectfromTiles(in,
		                            sx,
		                            sy,
		                            j*128,
		                            i*128,
		                            j*128+128-1,
		                            i*128+128-1,
		                            nImg,
		                            BestFrames,
		                            FramesScores,
		                            nImg
		                            );

			MY_LOGE ("BestShot_SelectfromTiles START\n");
			MY_LOGE ("SXj:%d  , SYi:%d , SXj2:%d , SYi2:%d , BestFrames[0] : %d , FramesScores[0] : %f\n",j*128,i*128,j*128+128-1,i*128+128-1,BestFrames[0],FramesScores[0]);
			MY_LOGE ("BestShot_SelectfromTiles END\n");
			if ( (j % 2) == 1 )
			{
				MY_LOGE ("BestShot_SelectfromTiles j % 2 == 1");
				MY_LOGE ("(BestFrames[0]<<10000 & 0xF0) : %x  , BackupBestIndex : %x",(BestFrames[0]<<10000 & 0xF0),BackupBestIndex);
				memset(pFreeFcousData1,(BestFrames[0]<<10000 & 0xF0) | BackupBestIndex,sizeof(char));
				BackupBestIndex = 0;
				//Move to next
				pFreeFcousData1++;
			} else {
				MY_LOGE ("BestShot_SelectfromTiles j % 2 == 0");
				BackupBestIndex = BestFrames[0];
			}
		}
	}
#else
	#if 0//save index into file
	delIndexMatrixFile();
	#endif
	countnum = 0;
	MY_LOGE ("Freefocus1 BestShot_SelectfromTiles  (YUVWidth / HorizontalBlockNum):%d\n",(YUVWidth / HorizontalBlockNum));
	MY_LOGE ("Freefocus1 BestShot_SelectfromTiles (YUVHeight / VerticalBlockNum):%d\n",(YUVHeight / VerticalBlockNum));
	//For 5M picture size
	for(int i = 0; i < VerticalBlockNum ; i++)
	{
		Uint8 BackupBestIndex = 0;

		for(int j = 0; j < HorizontalBlockNum ; j++)
		{
			ret = BestShot_SelectfromTiles(in,
		                            sx,
		                            sy,
		                            j*Block_Width,
		                            i*Block_Height,
		                            j*Block_Width+Block_Width-1,
		                            i*Block_Height+Block_Height-1,
		                            nImg,
		                            BestFrames,
		                            FramesScores,
		                            nImg
		                            );

			MY_LOGE ("BestShot_SelectfromTiles START\n");
			MY_LOGE ("SXj:%d  , SYi:%d , SXj2:%d , SYi2:%d , BestFrames[0] : %d , FramesScores[0] : %f\n",j*64,i*64,j*64+64-1,i*64+64-1,BestFrames[0],FramesScores[0]);
			MY_LOGE ("j*Block_Width:%d ,i*Block_Height:%d , j*Block_Width+Block_Width-1:%d ,i*Block_Height+Block_Height-1:%d",j*Block_Width,i*Block_Height,j*Block_Width+Block_Width-1,i*Block_Height+Block_Height-1);
			MY_LOGE ("BestShot_SelectfromTiles END\n");

			//Check the value is bigger than the threshold
			//if(BestFrames[0] < 200)
				//BestFrames[0] = 20;

			#if 0//save index into file
			dumpIndexMatrix(BestFrames[0] , i , j , VerticalBlockNum , HorizontalBlockNum );
			dumpHighestValueMatrix(FramesScores[0] , i , j , VerticalBlockNum , HorizontalBlockNum );
			#endif
			if ( (j % 2) == 1 )
			{
				MY_LOGE ("BestShot_SelectfromTiles j % 2 == 1");
				MY_LOGE ("BestFrames[0]:%x  (BestFrames[0]<<10000 & 0xF0) : %x  , BackupBestIndex : %x",BestFrames[0],(BestFrames[0]<<4 & 0xF0),BackupBestIndex);
				MY_LOGE ("last index :%d  current index : %d",BackupBestIndex,BestFrames[0]);
				//memset(pFreeFcousData1,(BestFrames[0]<<4 & 0xF0) | BackupBestIndex,sizeof(char));
				memset(pFreeFcousData1,(BackupBestIndex<<4 & 0xF0) | BestFrames[0],sizeof(char));
				BackupBestIndex = 0;
				//Move to next
				pFreeFcousData1++;
			} else {
				MY_LOGE ("BestShot_SelectfromTiles j % 2 == 0");
				BackupBestIndex = BestFrames[0];
			}
			countnum++;
		}
	}
	MY_LOGE ("Freefocus1 countnum :%d",countnum);
	//set initial index
	//MY_LOGE ("Freefocus1 CurFocusIndex :%d",CurFocusIndex);
	memset(pFreeFcousData1,CurFocusIndex,sizeof(char));
#endif

#else
	ret = BestShot_SelectfromTiles(in,
                            sx,
                            sy,
                            700,
                            1250,
                            700+128-1,
                            1250+128-1,
                            nImg,
                            BestFrames,
                            FramesScores,
                            nImg
                            );
#endif
	gettimeofday(&t4, NULL);
    if (err)
	{
		MY_LOGD ("BestShot_Select (%d)...\n", err);
		exit(-4);
	}

	MY_LOGD ("BestShot_Select BestFrames ret : %d",ret);

	for(int m=0; m<nImg ; m++)
	{
		MY_LOGD ("BestShot_Select BestFrames:%d , FramesScores:%f\n", BestFrames[m],FramesScores[m]);
	}

	MY_LOGD ("Processing completed...\n");


	if (AlmaShot_Release())
		MY_LOGD ("Errors releasing almashot resources...\n");

    gettimeofday(&t2, NULL);
    usec = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);
	MY_LOGD("BestShot Process elapsed time = %dms\n", usec/1000);

	usec = (t4.tv_sec-t3.tv_sec)*1000000+(t4.tv_usec-t3.tv_usec);
	MY_LOGD("BestShot One Region Compare elapsed time = %dms\n", usec/1000);

    for (i = 0;i < nImg;i++)
    {
        MY_LOGD ("FramesScores[%d]= %.2f \n", i, FramesScores[i]);
    }

    for (i = 0;i < nImg;i++)
    {
        MY_LOGD ("BestFrames[%d]= %d \n", i, BestFrames[i]);
    }

	MY_LOGD ("Done...\n");
    //Here is releasing the freefocus temp yuv buffer
    for (i=0; i<nImg; ++i)
	{
        MY_LOGD ("i = %d\n", i);
		free(in[i]);
	}

	return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
MAVClient::
mHalCamFeatureCompress(int curfocusindex)
{
    MY_LOGD("[mHalCamFeatureCompress]");

    MINT32 err = NO_ERROR;
    int32_t mShotNum = MAVnum;
	int u4SrcWidth = mMAVFrameWidth;//mpMAVResult.ClipWidth;
	int u4SrcHeight = mMAVFrameHeight;//mpMAVResult.ClipHeight;

	//================= FreeFocus ===============================
	MUINT32 HORIZONTL_BLOCK_NUM = 18;//18;
	MUINT32 VERTICAL_BLOCK_NUM = 32;//32;
	#if 0
	if(mMAVFrameWidth>mMAVFrameHeight){
		HORIZONTL_BLOCK_NUM = 32;//18;
		VERTICAL_BLOCK_NUM = 18;//32;
	}else{
		HORIZONTL_BLOCK_NUM = 18;//18;
		VERTICAL_BLOCK_NUM = 32;//32;
	}
	#endif
#define 	MAKERNOTE_MAX_SIZES (2+18)*32/2
	MUINT8 FREEFOCUS_NAME_DATASIZE = 9;
	MUINT8 FREEFOCUS_IMAGENUM_DATASIZE = 1;//CHAR
	MUINT8 FREEFOCUS_HORIZONTL_BLOCK_DATASIZE = 1;
	MUINT8 FREEFOCUS_VERTICAL_BLOCK_DATASIZE = 1;
	MUINT8 FREEFOCUS_BLOCK_WIDTH_DATASIZE = 1;
	MUINT8 FREEFOCUS_BLOCK_HEIGHT_DATASIZE = 1;
	MUINT32 FREEFOCUS_TOTAL_ARRAY_SIZE = FREEFOCUS_NAME_DATASIZE+FREEFOCUS_IMAGENUM_DATASIZE+FREEFOCUS_HORIZONTL_BLOCK_DATASIZE+FREEFOCUS_VERTICAL_BLOCK_DATASIZE+FREEFOCUS_BLOCK_WIDTH_DATASIZE+FREEFOCUS_BLOCK_HEIGHT_DATASIZE+HORIZONTL_BLOCK_NUM*VERTICAL_BLOCK_NUM/2;
	MUINT8 FreeFcousData[MAKERNOTE_MAX_SIZES];
	MUINT8* pFreeFcousData = NULL;
	memset(FreeFcousData,0x00,sizeof(char) * MAKERNOTE_MAX_SIZES);
	MY_LOGD("Freefocus FREEFOCUS_TOTAL_ARRAY_SIZE : %d ",FREEFOCUS_TOTAL_ARRAY_SIZE);
#if 0
	//fill freefocus name
	memcpy(FreeFcousData,"FREEFOCUS",sizeof(char)*9);
	//fill image num
	memset(FreeFcousData+9,0x8,sizeof(char));
	//fill horizontal block
	memset(FreeFcousData+10,0x10,sizeof(char));
	//fill vertical block
	memset(FreeFcousData+11,0x20,sizeof(char));
	//fill block width
	memset(FreeFcousData+12,0x80,sizeof(char));
	//fill block height
	memset(FreeFcousData+13,0x80,sizeof(char));
#endif
	//===========================================================

	//===========================================================

	MY_LOGD("Freefocus mShotNum : %d , MAVnum : %d",mShotNum,MAVnum);
    // (1) confirm merge is done; so mutex is not necessary

    MY_LOGD("Freefocus get MAVmergeDone semaphore");
    MY_LOGD("Freefocus mHalCamFeatureCompress 0x%x 0x%x",mpframeBuffer[0].virtAddr,mpframeBuffer[0]);
    MY_LOGD("Freefocus mHalCamFeatureCompress 0x%x 0x%x",mpframeBuffer,&mpframeBuffer);
    //mDataCb((MVOID*)mpframeBuffer,mpMAVResult.ClipWidth , mpMAVResult.ClipHeight);
    MY_LOGD("Freefocus mHalCamFeatureCompress clip image size mpMAVResult.ClipWidth :%d , mpMAVResult.ClipHeight : %d",mpMAVResult.ClipWidth,mpMAVResult.ClipHeight);
#if 1
	//PIXEL_FORMAT_YUV422I is useless now.
	String8 const format =  String8(MtkCameraParameters::PIXEL_FORMAT_YUV422I);
	sp<PREVIEWFEATUREBuffer> jpegBuf = new PREVIEWFEATUREBuffer(u4SrcWidth, u4SrcHeight,
                       FmtUtils::queryBitsPerPixel(format.string()),
                       FmtUtils::queryImgBufferSize(format.string(), u4SrcWidth, u4SrcHeight),
                       format,"PREVIEWFEATUREBuffer");
	IMEM_BUF_INFO Jpginfo;
    Jpginfo.size = jpegBuf->getBufSize();
    Jpginfo.virtAddr = (MUINT32)jpegBuf->getVirAddr();
    Jpginfo.phyAddr = (MUINT32)jpegBuf->getPhyAddr();
    Jpginfo.memID = (MINT32)jpegBuf->getIonFd();
	MY_LOGD("Freefocus Jpginfo.size : %d ",Jpginfo.size);
	MY_LOGD("Freefocus Jpginfo.virtAddr : %d ",Jpginfo.virtAddr);
#endif
	//=====================================================================================
	MUINT32 u4JpegSize;
    MPImageInfo * pMPImageInfo = new MPImageInfo[mShotNum];

    MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024];
    MUINT32 u4ExifHeaderSize = 0;

	IMEM_BUF_INFO* Srcbufinfo=(IMEM_BUF_INFO*)mpframeBuffer;

	//=====================Bestshot Compare Start==========================================
	//It should check the pic size for block sizes
	#if 0
	if(u4SrcWidth == 1440){
		HORIZONTL_BLOCK_NUM = 18;
	}else if(u4SrcWidth == 1152){
		HORIZONTL_BLOCK_NUM = 16;
	}
	#endif
	MY_LOGD("NextStreamingBestPicSelect input  u4SrcWidth: %d , u4SrcHeight:%d , HORIZONTL_BLOCK_NUM : %d , VERTICAL_BLOCK_NUM : %d",u4SrcWidth,u4SrcHeight,HORIZONTL_BLOCK_NUM,VERTICAL_BLOCK_NUM);
	NextStreamingBestPicSelect( (MUINT8*)FreeFcousData , FREEFOCUS_TOTAL_ARRAY_SIZE , mpframeBuffer , mShotNum , u4SrcWidth , u4SrcHeight , HORIZONTL_BLOCK_NUM , VERTICAL_BLOCK_NUM , curfocusindex);


	//=====================Bestshot Compare End============================================

	MY_LOGD("Freefocus mHalCamFeatureCompress  u4SrcWidth: %d , u4SrcHeight : %d",u4SrcWidth,u4SrcHeight);
#if 1//test
	MY_LOGD("Freefocus uMAVThumbJpegSize  : %d ", uMAVThumbJpegSize);
	makeExifHeader(eAppMode_PhotoMode, pMAVThumbJpegBuf, uMAVThumbJpegSize, puExifHeaderBuf, u4ExifHeaderSize, u4SrcWidth, u4SrcHeight,FreeFcousData);
#else
	makeExifHeader(eAppMode_PhotoMode, NULL, 0, puExifHeaderBuf, u4ExifHeaderSize, u4SrcWidth, u4SrcHeight);
#endif
	MY_LOGD("[handleJpegData] (exifHeaderBuf, size) = (%p, %d)", puExifHeaderBuf, u4ExifHeaderSize);

	#if 0//dump the yuv buffer
    char sourceFiles0[80];
	for (MUINT8 i = 0; i < mShotNum; i++){
		sprintf(sourceFiles0, "%s%d.yuv", "/sdcard/noncastMAV",i);
		MY_LOGD("Freefocus mpframeBuffer[i].virtAddr : %x ,mpframeBuffer[i].size : %d ",mpframeBuffer[i].virtAddr,mpframeBuffer[i].size);
		dumpBufToFile((char *) sourceFiles0, (MUINT8 *)mpframeBuffer[i].virtAddr, mpframeBuffer[i].size);
	}
	/*
	for (MUINT8 i = 0; i < mShotNum; i++){
		sprintf(sourceFiles0, "%s%d.yuv", "/sdcard/MAV",i);
		dumpBufToFile((char *) sourceFiles0, (MUINT8 *)Srcbufinfo[i].virtAddr, Srcbufinfo[i].size);
	}
	*/
	#endif
    createPanoJpegImg(Srcbufinfo[0],u4SrcWidth,u4SrcHeight,Jpginfo,u4JpegSize,0);
    pMPImageInfo[0].imageBuf = (char*)Jpginfo.virtAddr;
    pMPImageInfo[0].imageSize = u4JpegSize ;
    pMPImageInfo[0].sourceType = SOURCE_TYPE_BUF;

    for (MUINT8 i = 1; i < mShotNum; i++) {
        MY_LOGD("MAV NUM: %d", i);

        createPanoJpegImg(Srcbufinfo[i],u4SrcWidth,u4SrcHeight,Srcbufinfo[i-1],u4JpegSize,0);
        pMPImageInfo[i].imageBuf = (char*)Srcbufinfo[i-1].virtAddr;
        pMPImageInfo[i].imageSize = u4JpegSize ;
        pMPImageInfo[i].sourceType = SOURCE_TYPE_BUF;
    }
    for (MUINT8 i = (mShotNum - 1); i > 0 ; i--) {
        MY_LOGD("MAV  JPG: %d Adr 0x%x", i, (MUINT32)Srcbufinfo[i].virtAddr);
        memcpy((void*)Srcbufinfo[i].virtAddr , puExifHeaderBuf, u4ExifHeaderSize);
        memcpy((void*)(Srcbufinfo[i].virtAddr +u4ExifHeaderSize), (void*)Srcbufinfo[i-1].virtAddr, pMPImageInfo[i].imageSize);
        pMPImageInfo[i].imageBuf = (char*)Srcbufinfo[i].virtAddr;
        pMPImageInfo[i].imageSize = pMPImageInfo[i].imageSize + u4ExifHeaderSize;
		#if 0
		char sourceFiles[80];
        sprintf(sourceFiles, "%s%d.jpg", "/sdcard/2MPO", i);
        dumpBufToFile((char *) sourceFiles, (MUINT8 *)pMPImageInfo[i].imageBuf, pMPImageInfo[i].imageSize);
        #endif
    }
    pMPImageInfo[0].imageBuf = (char*)Srcbufinfo[0].virtAddr;
    pMPImageInfo[0].imageSize += u4ExifHeaderSize;
    memcpy((void*)Srcbufinfo[0].virtAddr , puExifHeaderBuf, u4ExifHeaderSize);
    memcpy((void*)(Srcbufinfo[0].virtAddr +u4ExifHeaderSize), (void*)Jpginfo.virtAddr, pMPImageInfo[0].imageSize);

	MY_LOGD("MAV NUM filename:%s", filename);

	string name;
	//string name1 = "12";
	name = pParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string();
	//name.insert(26,name1);
	MY_LOGD("MAV  NUM name : %s",name.c_str());
	MY_LOGD("MAV  NUM pParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH):%s", (char*)pParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string());
	MY_LOGD("leo before create mpo file");
	//Make MPO file
	createMPO(pMPImageInfo, mShotNum,(char*)name.c_str(), SOURCE_TYPE_BUF);
	MY_LOGD("leo mHalCamFeatureCompress finish");
EXIT:
    delete [] puExifHeaderBuf;
    return err;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
MAVClient::
createJpegImg(NSCamHW::ImgBufInfo const & rSrcBufInfo
      , MUINT32 quality
      , bool fIsAddSOI
      , NSCamHW::ImgBufInfo const & rDstBufInfo
      , MUINT32 & u4EncSize)
{
    MBOOL ret = MTRUE;
    // (0). debug
    MY_LOGD("Freefocus [createJpegImg] - rSrcImgBufInfo.u4BufVA=0x%x", rSrcBufInfo.u4BufVA);
    MY_LOGD("Freefocus [createJpegImg] - rSrcImgBufInfo.eImgFmt=%d", rSrcBufInfo.eImgFmt);
    MY_LOGD("Freefocus [createJpegImg] - rSrcImgBufInfo.u4ImgWidth=%d", rSrcBufInfo.u4ImgWidth);
    MY_LOGD("Freefocus [createJpegImg] - rSrcImgBufInfo.u4ImgHeight=%d", rSrcBufInfo.u4ImgHeight);
    MY_LOGD("Freefocus [createJpegImg] - jpgQuality=%d", quality);
    //
    // (1). Create Instance
    JpgEncHal* pJpgEncoder = new JpgEncHal();
    // (1). Lock
    if(!pJpgEncoder->lock())
    {
        MY_LOGE("can't lock jpeg resource");
        goto EXIT;
    }
    // (2). size, format, addr
    if (eImgFmt_YUY2 == rSrcBufInfo.eImgFmt)
    {
        MY_LOGD("Freefocus jpeg source YUY2");
        pJpgEncoder->setEncSize(rSrcBufInfo.u4ImgWidth, rSrcBufInfo.u4ImgHeight,
                                JpgEncHal:: kENC_YUY2_Format);
        pJpgEncoder->setSrcAddr((void *)rSrcBufInfo.u4BufVA, (void *)NULL);
        pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride() ,rSrcBufInfo.u4BufSize, 0);
    }
    else if (eImgFmt_NV21 == rSrcBufInfo.eImgFmt)
    {
        MY_LOGD("Freefocus jpeg source NV21");
        pJpgEncoder->setEncSize(rSrcBufInfo.u4ImgWidth, rSrcBufInfo.u4ImgHeight,
                                JpgEncHal:: kENC_NV21_Format);
        pJpgEncoder->setSrcAddr((void *)rSrcBufInfo.u4BufVA, (void *)(rSrcBufInfo.u4BufVA + rSrcBufInfo.u4ImgWidth * rSrcBufInfo.u4ImgHeight));
        pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride(), rSrcBufInfo.u4ImgWidth * rSrcBufInfo.u4ImgHeight,
                                                 rSrcBufInfo.u4ImgWidth * rSrcBufInfo.u4ImgHeight / 2);
    }
    else
    {
        MY_LOGE("Freefocus Not support image format:0x%x", rSrcBufInfo.eImgFmt);
        goto EXIT;
    }
    // (3). set quality
    pJpgEncoder->setQuality(quality);
    // (4). dst addr, size
    pJpgEncoder->setDstAddr((void *)rDstBufInfo.u4BufVA);
    pJpgEncoder->setDstSize(rDstBufInfo.u4BufSize);
    // (6). set SOI
    pJpgEncoder->enableSOI((fIsAddSOI > 0) ? 1 : 0);
    // (7). ION mode
    if ( rSrcBufInfo.i4MemID > 0 )
    {
        pJpgEncoder->setIonMode(1);
        pJpgEncoder->setSrcFD(rSrcBufInfo.i4MemID, rSrcBufInfo.i4MemID);
        pJpgEncoder->setDstFD(rDstBufInfo.i4MemID);
    }

    // (8).  Start
    if (pJpgEncoder->start(&u4EncSize))
    {
        MY_LOGD("Freefocus Jpeg encode done, size = %d", u4EncSize);
        ret = MTRUE;
    }
    else
    {
        pJpgEncoder->unlock();
        goto EXIT;
    }

    pJpgEncoder->unlock();

EXIT:
    delete pJpgEncoder;

    MY_LOGD("[init] - X. ret: %d.", ret);
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
MAVClient::
createJpegImgWithThumbnail(NSCamHW::ImgBufInfo const &rYuvImgBufInfo, IMEM_BUF_INFO jpgBuff, MUINT32 &u4JpegSize)
{
    MBOOL ret = MTRUE;
    MUINT32 stride[3];
    MY_LOGD("[createJpegImgWithThumbnail] in");

    NSCamHW::ImgInfo    rJpegImgInfo(eImgFmt_JPEG, rYuvImgBufInfo.u4ImgWidth, rYuvImgBufInfo.u4ImgHeight);
    NSCamHW::BufInfo    rJpegBufInfo((int)jpgBuff.size,(MUINT32)jpgBuff.virtAddr, (MUINT32)jpgBuff.phyAddr, jpgBuff.memID);
    NSCamHW::ImgBufInfo   rJpegImgBufInfo(rJpegImgInfo, rJpegBufInfo, stride);

    ret = createJpegImg(rYuvImgBufInfo, /*mpParamsMgr->getInt(CameraParameters::KEY_JPEG_QUALITY)*/100, MFALSE, rJpegImgBufInfo, u4JpegSize);

    MY_LOGD("[createJpegImgWithThumbnail] out");
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
MAVClient::
createPanoJpegImg(IMEM_BUF_INFO Srcbufinfo, int u4SrcWidth, int u4SrcHeight, IMEM_BUF_INFO jpgBuff, MUINT32 &u4JpegSize , MUINT32 index)
{
    MY_LOGD("[createPanoJpegImg] in image index :%d",index);
    MBOOL ret = MTRUE;
    MUINT32     u4Stride[3];
    u4Stride[0] = u4SrcWidth;
    u4Stride[1] = u4SrcWidth >> 1;
    u4Stride[2] = u4SrcWidth >> 1;

    MUINT32         u4ResultSize = Srcbufinfo.size;
    NSCamHW::ImgInfo    rYuvImgInfo(/*eImgFmt_YUY2*/eImgFmt_NV21, u4SrcWidth , u4SrcHeight);
    NSCamHW::BufInfo    rYuvBufInfo(u4ResultSize, (MUINT32)Srcbufinfo.virtAddr, 0, Srcbufinfo.memID);
    NSCamHW::ImgBufInfo   rYuvImgBufInfo(rYuvImgInfo, rYuvBufInfo, u4Stride);
    MY_LOGD("Freefocus [createPanoJpegImg] Srcbufinfo.memID :%d , u4ResultSize :%d",Srcbufinfo.memID,u4ResultSize);
    ret = createJpegImgWithThumbnail(rYuvImgBufInfo,jpgBuff,u4JpegSize);
    MY_LOGD("Freefocus [createPanoJpegImg] out in image index : %d",index);
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
MAVClient::
createMPO(MPImageInfo * pMPImageInfo, MUINT32 num, char* file, MUINT32 MPOType)
{
    MINT32 err = NO_ERROR;
    MBOOL ok;
    MpoEncoder* mpoEncoder = new MpoEncoder();
    if (mpoEncoder) {
        ok = mpoEncoder->setJpegSources(TYPE_Disparity, pMPImageInfo, num);

        if (!ok) {
            MY_LOGE("Freefocus  mpoEncoder->setJpegSources fail \n");
            err = 1;
            goto mHalCamMAVMakeMPO_EXIT;
        }

        ok = mpoEncoder->encode(file);

        if (!ok) {
            MY_LOGE("Freefocus  mpoEncoder->encode fail \n");
            err = 1;
            goto mHalCamMAVMakeMPO_EXIT;
        }

        MY_LOGD("Freefocus [mHalCamMAVMakeMPO]  Done,file: %s \n", file);
    }
    else
    {
        MY_LOGD("Freefocus new MpoEncoder() fail");
        return false;
    }

mHalCamMAVMakeMPO_EXIT:
    delete mpoEncoder;
    delete [] pMPImageInfo;
    if(err!=NO_ERROR)
        return false;
    else
        return true;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
MAVClient::
makeExifHeader(MUINT32 const u4CamMode,
				       MUINT8* const puThumbBuf,
				       MUINT32 const u4ThumbSize,
				       MUINT8* puExifBuf,
				       MUINT32 &u4FinalExifSize,
				       MUINT32 const Width,
				       MUINT32 const Height,
				       MUINT8* FreeFcousData)
{
    //
    MY_LOGD("+ (u4CamMode, puThumbBuf, u4ThumbSize, puExifBuf) = (%d, %p, %d, %p)",
                            u4CamMode,  puThumbBuf, u4ThumbSize, puExifBuf);
#if 1
    if (u4ThumbSize > 63 * 1024)
    {
        MY_LOGW("The thumbnail size is large than 63K, the exif header will be broken");
    }
    bool ret = true;
    uint32_t u4App1HeaderSize = 0;
    uint32_t u4AppnHeaderSize = 0;

    uint32_t exifHeaderSize = 0;
    CamExif rCamExif;
    CamExifParam rExifParam;
    CamDbgParam rDbgParam;

    // ExifParam (for Gps)
    #if 0
    if (! mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).isEmpty() && !mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).isEmpty())
    {
        rExifParam.u4GpsIsOn = 1;
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLatitude), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLongitude), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSTimeStamp), mpParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSProcessingMethod), mpParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).length());
        rExifParam.u4GPSAltitude = ::atoi(mpParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE).string());
    }
    #endif
    rExifParam.u4Orientation = 0;//mpParamsMgr->getInt(CameraParameters::KEY_ROTATION);;
    rExifParam.u4ZoomRatio = 0;//mpParamsMgr->getZoomRatio();
    //
    rExifParam.u4ImgIndex = 0;//u4ImgIndex;
    rExifParam.u4GroupId = 0;//u4GroupId;
    //
    //! CamDbgParam (for camMode, shotMode)
    rDbgParam.u4CamMode = u4CamMode;
    //
    rCamExif.init(rExifParam,  rDbgParam);
    MY_LOGD("3A get exif");
	#if 0
	Hal3ABase* p3AHal = Hal3ABase::createInstance(MtkCamUtils::DevMetaInfo::queryHalSensorDev(0));
    p3AHal->set3AEXIFInfo(&rCamExif);
    #endif

	//
	//=================== Write FreeFocus Data =========================
	MUINT32 Bwidth = 18;
	MUINT32 Bheight = 32;

#if 0

	MUINT8 FREEFOCUS_NAME_DATASIZE = 9;
	MUINT8 FREEFOCUS_IMAGENUM_DATASIZE = 1;//CHAR
	MUINT8 FREEFOCUS_HORIZONTL_BLOCK_DATASIZE = 1;
	MUINT8 FREEFOCUS_VERTICAL_BLOCK_DATASIZE = 1;
	MUINT8 FREEFOCUS_BLOCK_WIDTH_DATASIZE = 1;
	MUINT8 FREEFOCUS_BLOCK_HEIGHT_DATASIZE = 1;
	MUINT32 FREEFOCUS_TOTAL_ARRAY_SIZE = FREEFOCUS_NAME_DATASIZE+FREEFOCUS_IMAGENUM_DATASIZE+FREEFOCUS_HORIZONTL_BLOCK_DATASIZE+FREEFOCUS_VERTICAL_BLOCK_DATASIZE+FREEFOCUS_BLOCK_WIDTH_DATASIZE+FREEFOCUS_BLOCK_HEIGHT_DATASIZE+Bwidth*Bheight/2+1;
	MUINT8 FreeFcousData[FREEFOCUS_TOTAL_ARRAY_SIZE];
#endif
#if 0
	memset(FreeFcousData,0xEB,sizeof(char) * FREEFOCUS_TOTAL_ARRAY_SIZE);
	//3*9 = 27

	FreeFcousData[FREEFOCUS_TOTAL_ARRAY_SIZE] = '\0';
#if 1
	//fill freefocus name
	memcpy(FreeFcousData,"FREEFOCUS",sizeof(char)*9);
	//fill image num
	memset(FreeFcousData+9,0x8,sizeof(char));
	//fill horizontal block
	memset(FreeFcousData+10,0x10,sizeof(char));
	//fill vertical block
	memset(FreeFcousData+11,0x20,sizeof(char));
	//fill block width
	memset(FreeFcousData+12,0x80,sizeof(char));
	//fill block height
	memset(FreeFcousData+13,0x80,sizeof(char));
#endif
#endif
	//==============================================================
	MY_LOGD("Freefocus test BestIndex : %s",FreeFcousData);
    // the bitstream already rotated. it need to swap the width/height
    if (90 == rExifParam.u4Orientation || 270 == rExifParam.u4Orientation)
    {
        rCamExif.makeFreeFocusExifApp1((MUINT8*)FreeFcousData,Bwidth,Bheight,0x8,Height,  Width, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
    else
    {
        rCamExif.makeFreeFocusExifApp1((MUINT8*)FreeFcousData,Bwidth,Bheight,0x8,Width, Height, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
#if 1
    // copy thumbnail image after APP1
    MUINT8 *pdest = puExifBuf + u4App1HeaderSize;
    ::memcpy(pdest, puThumbBuf, u4ThumbSize) ;
    //
	MY_LOGD("Freefocus test u4ThumbSize : %d",u4ThumbSize);
    pdest = puExifBuf + u4App1HeaderSize + u4ThumbSize;
#endif
	//
    //rCamExif.makeExifApp3(0,pdest,&u4AppnHeaderSize);
    rCamExif.appendDebugExif(pdest, &u4AppnHeaderSize);
    rCamExif.uninit();

	 MY_LOGD("Freefocus test u4AppnHeaderSize : %d",u4AppnHeaderSize);
    //p3AHal->destroyInstance();
    u4FinalExifSize = u4App1HeaderSize + u4ThumbSize + u4AppnHeaderSize;

    MY_LOGD("- (app1Size, appnSize, exifSize) = (%d, %d, %d)",
                          u4App1HeaderSize, u4AppnHeaderSize, u4FinalExifSize);
#endif
    return ret;
}
//Start MAV example

