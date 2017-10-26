
#define LOG_TAG "MtkCam/Shot"
//
#include <fcntl.h>
#include <sys/stat.h>
//
#include <cutils/properties.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
//
#include <mtkcam/common/camutils/CamFormat.h>
#include <mtkcam/v1/camutils/CamInfo.h>
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
//
#include <mtkcam/hal/sensor_hal.h>
//
using namespace android;


//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "FreeFocus.h"
//
//#include <camera/MtkCameraParameters.h>


//
using namespace android;
using namespace NSShot;

//=====================================
//====== MAVClient.h ==================
using namespace NSCamClient1;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD1(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)       CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
sp<IParamsManager>  pParamsMgrBestShot;
extern "C"
sp<IShot>
createInstance_FreefocusShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    int32_t const       iPictureWidth,
    int32_t const       iPictureHeigh,
    sp<IParamsManager>  pParamsMgr
)
{
    sp<IShot>       pShot = NULL;
    sp<FreefocusShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new FreefocusShot(pszShotName, u4ShotMode, i4OpenId, pParamsMgr);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new Best Shot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate(iPictureWidth,iPictureHeigh) ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
FreefocusShot::
onCreate(int32_t iPictureWidth, int32_t iPictureHeigh)
{
#warning "[TODO] BestShot::onCreate()"
    bool ret = true;

    mu4CurrentCount = 0;
    mu4TotalCount = 0;
    mu4BestShotValue = 0;
    mu4BestShotIndex = 0;

	MAVPObject = createInstance(BEST_SHOT_COUNT);
	//It should get current capture size
	//EImageFormat ePostViewFmt = (android::MtkCamUtils::FmtUtils::queryImgWidthStride(mShotParam.mi4PictureWidth));
	//==================================
	MY_LOGD("Freefocus onCreate() mShotParam.mi4PictureWidth:%d , mShotParam.mi4PictureHeight:%d",mShotParam.mi4PictureWidth,mShotParam.mi4PictureHeight);
	MAVPObject->init(iPictureWidth,iPictureHeigh);
	//MAVPObject->setImgCallback(handleMAVImgCallBacknn);
	m_pMcuDrv = MCUDrv::createInstance(/*m_i4CurrLensId*/0);
	if (!m_pMcuDrv)   {
        MY_LOGE("McuDrv::createInstance fail");
        //return E_AF_NULL_POINTER;
    }

	//init 3a
#if 0
	if(m_p3AHal == NULL)
    {
        m_p3AHal = Hal3ABase::createInstance(0/*devID*/);
    }
	//=====================
#endif
	MY_LOGD("Freefocus BestShot onCreate()");
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
handleMAVImgCallBacknn(MVOID* const puJpegBuf, int u4SrcWidth, int u4SrcHeight)
{
    MY_LOGD("[handleJpegCallBack] + (puJpgBuf, u4SrcWidth, u4SrcHeight) = (%p, %d , %d)", puJpegBuf, u4SrcWidth, u4SrcHeight);

	//(IMEM_BUF_INFO*)puJpegBuf = (IMEM_BUF_INFO*)reinterpret_cast<MUINT8 *>(mvTmpBuf_yuv[mu4CurrentCount].begin());
    return 1;//MTRUE;

}

/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
FreefocusShot::
onDestroy()
{
    MAVPObject->uninit();
#warning "[TODO] BestShot::onDestroy()"
}


/******************************************************************************
 *
 ******************************************************************************/
FreefocusShot::
FreefocusShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId,
    sp<IParamsManager> pParamsMgr
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mvTmpBuf_postview()
    //, mvTmpBuf_yuv()
    //, mvTmpBuf_jpeg()
    //, mvTmpBuf_exif()
    , mu4CurrentCount(0)
    , mu4TotalCount(0)
    , mu4BestShotValue(0)
    , mu4BestShotIndex(0)
    , mfgIsCanceled(MFALSE)
    , mpParamsMgr(pParamsMgr)
{

}


/******************************************************************************
 *
 ******************************************************************************/
FreefocusShot::
~FreefocusShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FreefocusShot::
sendCommand(
    uint32_t const  cmd,
    uint32_t const  arg1,
    uint32_t const  arg2
)
{
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2);
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
FreefocusShot::
CropIndexTable(MUINT32 SnapNum)
{
    MUINT32 CropIndex;
    enum SnapOrder{FirstPic = 0, SecondPic, ThirdPic, FourthPic, FifthPic, SixthPic, SeventhPic, EighthPic, NinthPic, TenthPic};
    switch(SnapNum)
    {
        case FirstPic:
            CropIndex = 106;
            break;
        case SecondPic:
            CropIndex = 105;
            break;
        case ThirdPic:
            CropIndex = 104;
            break;
        case FourthPic:
            CropIndex = 104;
            break;
        case FifthPic:
            CropIndex = 103;
            break;
        case SixthPic:
            CropIndex = 103;
            break;
        case SeventhPic:
            CropIndex = 102;
            break;
        case EighthPic:
            CropIndex = 102;
            break;
        case NinthPic:
            CropIndex = 102;
            break;
        case TenthPic:
            CropIndex = 100;
            break;
        default:
            CropIndex = 100;
    }
    return CropIndex;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
FreefocusShot::
LookupIndexTable(MUINT32 SnapNum)
{
	MUINT32 StepIndex;
	enum SnapOrder{FirstPic = 0, SecondPic, ThirdPic, FourthPic, FifthPic, SixthPic, SeventhPic, EighthPic, NinthPic, TenthPic};
	switch(SnapNum)
	{
		case FirstPic:
			StepIndex = 184;
			break;
		case SecondPic:
			StepIndex = 214;
			break;
		case ThirdPic:
			StepIndex = 252;
			break;
		case FourthPic:
			StepIndex = 292;
			break;
		case FifthPic:
			StepIndex = 332;
			break;
		case SixthPic:
			StepIndex = 377;
			break;
		case SeventhPic:
			StepIndex = 432;
			break;
		case EighthPic:
			StepIndex = 480;
			break;
		case NinthPic:
			StepIndex = 540;
			break;
		case TenthPic:
			StepIndex = 381;
			break;
		default:
			StepIndex = 210;
	}
	return StepIndex;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32
FreefocusShot::
GetFocalLengthIndex(MUINT32 StepIndex)
{
    MUINT32 FocalLengthIndex = 0;
    enum FocalLengthOrder{FirstFocalLength = 0, SecondFocalLength, ThirdFocalLength, FourthFocalLength, FifthFocalLength, SixthFocalLength, SeventhFocalLength, EighthFocalLength, NinthFocalLength, TenthFocalLength};

    if(StepIndex <= 184)
        FocalLengthIndex = FirstFocalLength;
    else if(StepIndex > 184 && StepIndex <= 214)
        FocalLengthIndex = SecondFocalLength;
    else if(StepIndex > 214 && StepIndex <= 252)
        FocalLengthIndex = ThirdFocalLength;
    else if(StepIndex > 252 && StepIndex <= 292)
        FocalLengthIndex = FourthFocalLength;
    else if(StepIndex > 292 && StepIndex <= 332)
        FocalLengthIndex = FifthFocalLength;
    else if(StepIndex > 332 && StepIndex <= 377)
        FocalLengthIndex = SixthFocalLength;
    else if(StepIndex > 377 && StepIndex <= 432)
        FocalLengthIndex = SeventhFocalLength;
    else if(StepIndex > 432)
        FocalLengthIndex = EighthFocalLength;

    return FocalLengthIndex;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
FreefocusShot::
onCmd_reset()
{
#warning "[TODO] BestShot::onCmd_reset()"
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
FreefocusShot::
onCmd_capture()
{
    MBOOL ret = MTRUE;
    MUINT32     croprate = 100;
    mcuMotorInfo mMCUInfo;

    NSCamShot::ISingleShot *pSingleShot = NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "BestShot");
    //
    pSingleShot->init();

    //
    pSingleShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_SOF);
    //
    EImageFormat ePostViewFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat));

    pSingleShot->enableDataMsg(NSCamShot::ECamShot_DATA_MSG_JPEG | NSCamShot::ECamShot_DATA_MSG_YUV
                               | ((ePostViewFmt != eImgFmt_UNKNOWN) ? NSCamShot::ECamShot_DATA_MSG_POSTVIEW : NSCamShot::ECamShot_DATA_MSG_NONE)
                               );


    // shot param
    NSCamShot::ShotParam rShotParam(eImgFmt_NV21,         //yuv format
                         mShotParam.mi4PictureWidth,      //picutre width
                         mShotParam.mi4PictureHeight,     //picture height
                         mShotParam.mi4Rotation,          //picture rotation
                         0,                               //picture flip
                         ePostViewFmt,                    //postview format
                         mShotParam.mi4PostviewWidth,      //postview width
                         mShotParam.mi4PostviewHeight,     //postview height
                         0,                               //postview rotation
                         0,                               //postview flip
                         mShotParam.mu4ZoomRatio           //zoom
                        );

    // jpeg param
    NSCamShot::JpegParam rJpegParam(NSCamShot::ThumbnailParam(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight,
                                                                mJpegParam.mu4JpegThumbQuality, MTRUE),
                                                        mJpegParam.mu4JpegQuality,       //Quality
                                                        MFALSE                            //isSOI
                         );

    //
    NSCamShot::SensorParam rSensorParam(static_cast<MUINT32>(MtkCamUtils::DevMetaInfo::queryHalSensorDev(getOpenId())),                             //Device ID
                             ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,         //Scenaio
                             10,                                       //bit depth
                             MFALSE,                                   //bypass delay
                             MFALSE                                   //bypass scenario
                            );
	  MY_LOGD("Freefocus  onCmd_capture mShotParam.mi4PictureWidth:%d ,mShotParam.mi4PictureHeight:%d",mShotParam.mi4PictureWidth,mShotParam.mi4PictureHeight );
    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    ret = pSingleShot->setShotParam(rShotParam);

    //
    ret = pSingleShot->setJpegParam(rJpegParam);

#if 1
	  MY_LOGD("Freefocus onCmd_capture filename :%s  ", (char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string() );
	  MAVPObject->copyfilename(mpParamsMgr);
#endif

#if 0
	  //debug
	  String8 const test =  pParamsMgrBestShot->getStr(MtkCameraParameters::KEY_FOCUS_MODE);
	  MY_LOGD("Freefocus GetFocusMode : %s", test.string() );
#endif
	  movestep = 0;
	m_pMcuDrv->getMCUInfo(&mMCUInfo);
	MY_LOGD("Freefocus mMCUInfo.u4CurrentPosition  : %d", mMCUInfo.u4CurrentPosition);
	  //
    mu4TotalCount = BEST_SHOT_COUNT;
    for (MUINT32 i = 0; i < mu4TotalCount; i++)
    {
        MY_LOGD("Freefocus onCmd_capture i : %d", i);

	// increase the shot speed, bypass to set the sensor again
        if (0 != i)
        {
            rSensorParam.fgBypassDelay = MTRUE;
            rSensorParam.fgBypassScenaio = MTRUE;
        }
		//It should look up the index table
#if 0
		    if(i<2)
			      movestep = movestep + 50;
		    else if(i>=2 && i<6)
			      movestep = movestep + 30;
		    else
			      movestep = movestep + 20;
#else
		    movestep = LookupIndexTable(i);
#endif
		    MY_LOGD("Freefocus movestep : %d", movestep);
		    m_pMcuDrv->moveMCU(movestep);
        //
        croprate = CropIndexTable(i);
        MY_LOGD("Freefocus croprate : %d", croprate);
        mShotParam.mu4ZoomRatio  = croprate;
        NSCamShot::ShotParam rShotParam(eImgFmt_NV21,      //yuv format
                             mShotParam.mi4PictureWidth,   //picutre width
                             mShotParam.mi4PictureHeight,  //picture height
                             mShotParam.mi4Rotation,       //picture rotation
                             0,                            //picture flip
                             ePostViewFmt,                 //postview format
                             mShotParam.mi4PostviewWidth,  //postview width
                             mShotParam.mi4PostviewHeight, //postview height
                             0,                            //postview rotation
                             0,                            //postview flip
                             mShotParam.mu4ZoomRatio       //zoom
                            );
        ret = pSingleShot->setShotParam(rShotParam);

#if 1
        ret = pSingleShot->startOne(rSensorParam);
#else//for crop
		    ImgBufInfo rImgBufInfo;

		    ret = pSingleShot->startOne(ImgBufInfo const & rImgBufInfo);
#endif
		//

        // Jpeg callback
        mpShotCallback->onCB_CompressedImage(0,
                                             mvTmpBuf_jpeg[mu4BestShotIndex].size(),
                                             reinterpret_cast<uint8_t const*>(mvTmpBuf_jpeg[mu4BestShotIndex].begin()),
                                             mvTmpBuf_exif[mu4BestShotIndex].size(),					   //header size
                                             reinterpret_cast<uint8_t const*>(mvTmpBuf_exif[mu4BestShotIndex].begin()), 				   //header buf
                                             i, 					  //callback index
                                             0,					  //final image
                                             MTK_CAMERA_MSG_EXT_DATA_FREEFOCUS
                                             );


        mu4CurrentCount++;
	//MY_LOGD("Freefocus stop %s",(char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_FREEFOCUS_STOP).string());
        if (0 == ::strcmp((char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_FREEFOCUS_STOP).string(), "on"))
        {
            //MY_LOGD("Freefocus stop break ");
            break;
        }
    }
    m_pMcuDrv->moveMCU(mMCUInfo.u4CurrentPosition);
    //=========================
    //MY_LOGD("Freefocus mMAVStopFlag  : %d", mMAVStopFlag);
    if (0 == ::strcmp((char*)mpParamsMgr->getStr(MtkCameraParameters::KEY_FREEFOCUS_STOP).string(), "on"))
    {
        MAVPObject->stopFeature( 0 , GetFocalLengthIndex(mMCUInfo.u4CurrentPosition));
		MY_LOGD("leo before handleCaptureDone");
		handleCaptureDone();
    }else{
        MAVPObject->stopFeature( 1 , GetFocalLengthIndex(mMCUInfo.u4CurrentPosition));
		MY_LOGD("leo before handleCaptureDone");
		handleCaptureDone();
    }
	MY_LOGD("leo Freefocus stop off");
    mpParamsMgr->set(MtkCameraParameters::KEY_FREEFOCUS_STOP,"Off");
    //
    MY_LOGD("1 leo Freefocus stop off");
    ret = pSingleShot->uninit();
    //
    MY_LOGD("2 leo Freefocus stop off");
    pSingleShot->destroyInstance();
	MY_LOGD("3 leo Freefocus stop off");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
FreefocusShot::
onCmd_cancel()
{
    mfgIsCanceled = MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FreefocusShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    FreefocusShot *pBestShot = reinterpret_cast <FreefocusShot *>(user);
    if (NULL != pBestShot)
    {
        // In best shot, the shutter callback only do at last image
        if (NSCamShot::ECamShot_NOTIFY_MSG_SOF == msg.msgType)
        {
            pBestShot->handleShutter();
        }
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FreefocusShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
	MY_LOGD("Freefocus fgCamShotDataCb ");
    FreefocusShot *pBestShot = reinterpret_cast<FreefocusShot *>(user);
    if (NULL != pBestShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            //pBestShot->handlePostViewData(msg.puData, msg.u4Size);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pBestShot->handleJpegData(msg.puData, msg.u4Size, reinterpret_cast<MUINT8*>(msg.ext1), msg.ext2);
        }
		else if (NSCamShot::ECamShot_DATA_MSG_YUV == msg.msgType)
        {
            pBestShot->handleYuvData(msg.puData, msg.u4Size);
        }
    }

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
    MY_LOGI("Current  index: %d", mu4CurrentCount);
    if (mu4CurrentCount < mu4TotalCount)
    {
        //  Save Postview
        saveToTmpBuf(mvTmpBuf_postview[/*mu4CurrentCount*/0], puBuf, u4Size);
    }
    //
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
handleYuvData(MUINT8* const puBuf, MUINT32 const u4Size)
{
	  MY_LOGD("Freefocus handleYuvData  mu4CurrentCount : %d",mu4CurrentCount);
    MY_LOGI("Current index: %d", mu4CurrentCount);
    if (mu4CurrentCount < mu4TotalCount)
    {
        //  Save YuvData
        //mvTmpBuf_yuv only use here,
        //It can move to MAVClient.h to init
        //saveToTmpBuf(mvTmpBuf_yuv[mu4CurrentCount], puBuf, u4Size);
		    //dumpImg(/*reinterpret_cast<MUINT8 *>(mvTmpBuf_yuv[mu4CurrentCount].begin())*/puBuf,u4Size,"TmpBuf_bestshot","yuv",mu4CurrentCount);
		    MY_LOGD("1 Freefocus  handleYuvData TmpBuf_bestshot u4Size : %d , mu4CurrentCount : %d",u4Size,mu4CurrentCount);
		    //==========================================================

		    //AddImg here
		    //The memory has been created in init function
		    memcpy((MUINT8 *)MAVPObject->mpframeBuffer[mu4CurrentCount].virtAddr,puBuf,u4Size);
		    MAVPObject->mpframeBuffer[mu4CurrentCount].size = u4Size;
		    //==========================================================
		    MY_LOGD("Freefocus  mu4CurrentCount : %d , u4Size : %d",mu4CurrentCount,u4Size);
		    MY_LOGD("Freefocus  handleYuvData  mpframeBuffer[%d].size : %d , mpframeBuffer[mu4CurrentCount].virtAddr : %x",mu4CurrentCount,MAVPObject->mpframeBuffer[mu4CurrentCount].size,MAVPObject->mpframeBuffer[mu4CurrentCount].virtAddr);
		    //dumpImg((MUINT8 *)MAVPObject->mpframeBuffer[mu4CurrentCount].virtAddr,MAVPObject->mpframeBuffer[mu4CurrentCount].size,"bestshot","yuv",mu4CurrentCount);
		    //sem_post here ,make MAVthreadFunc while loop can continue.
		    //sem_post(&MAVPObject->MAVSemThread);
		    //Temp work around
		    if( mu4CurrentCount == (mu4TotalCount-1))
		    {
			      MY_LOGD("Freefocus handleYuvData  mu4CurrentCount == (mu4TotalCount-1) ");
			      MY_LOGD("Freefocus handleYuvData  finish ");

		    }
	  }
    //
    return  MTRUE;
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
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
MBOOL
FreefocusShot::
handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize)
{
	  MY_LOGD("Freefocus handleJpegData Current index: %d",mu4CurrentCount);
    MY_LOGI("Current index: %d", mu4CurrentCount);
    if (mu4CurrentCount < mu4TotalCount)
    {
        //It should save the exif header here
        MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024];
        MUINT32 u4ExifHeaderSize = 0;
        makeExifHeader(eAppMode_PhotoMode, /*puThumbBuf*/NULL, /*u4ThumbSize*/0, puExifHeaderBuf, u4ExifHeaderSize);
        MY_LOGD("(thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)",
                          puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);
		#if 0
		if(mu4CurrentCount==0){
			memcpy(MAVPObject->pMAVExifHeaderBuf,puExifHeaderBuf,128 * 1024);
			MAVPObject->uMAVExifHeaderSize = u4ExifHeaderSize;
			//MY_LOGD("Freefocus handleJpegData mu4CurrentCount[] : %d , u4ExifHeaderSize[m] : %d",mu4CurrentCount,u4ExifHeaderSize[mu4CurrentCount]);
		}
		#endif
		//  Save  Jpeg and exif
        saveToTmpBuf(mvTmpBuf_jpeg[mu4CurrentCount], puJpegBuf, u4JpegSize);
        saveToTmpBuf(mvTmpBuf_exif[mu4CurrentCount], puExifHeaderBuf, u4ExifHeaderSize);
        //
        delete [] puExifHeaderBuf;
    }
	//======================= BestShot flow =================================================
	//It should choose the image
	#if 1
	if(mu4CurrentCount == mu4TotalCount-1){
		MAVPObject->initThumbnailBuf(u4ThumbSize);
		memcpy(MAVPObject->pMAVThumbJpegBuf,puThumbBuf,sizeof(MUINT8)*u4ThumbSize);
	}
	#endif
	//=======================  End          =================================================
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
handleShutter()
{
    if (mu4TotalCount-1 == mu4CurrentCount)
    {
        mpShotCallback->onCB_Shutter(true, 0);
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
handleCaptureDone()
{
    MBOOL   ret = MTRUE;
    //
    MY_LOGD("Freefocus handleCaptureDone()");
    MY_LOGI("the last best-shot: %d is selected", mu4BestShotIndex);
    //  [1] restore the buffers of best index.
    MUINT32 u4PostviewSize = 0;
    MUINT32 u4JpgPictureSize = 0;
    MUINT32 u4ExifSize = 0;

    // [2] invoke callbacks.
    // postView callback
    mpShotCallback->onCB_PostviewDisplay(0,
                                         mvTmpBuf_postview[mu4BestShotIndex].size(),
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_postview[mu4BestShotIndex].begin())
                                        );

    // Jpeg callback
    mpShotCallback->onCB_CompressedImage(0,
                                         mvTmpBuf_jpeg[mu4BestShotIndex].size(),
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_jpeg[mu4BestShotIndex].begin()),
                                         mvTmpBuf_exif[mu4BestShotIndex].size(),                       //header size
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_exif[mu4BestShotIndex].begin()),                    //header buf
                                         8,                       //callback index
                                         true,
                                         MTK_CAMERA_MSG_EXT_DATA_FREEFOCUS
                                         );
    // Jpeg callback
    mpShotCallback->onCB_CompressedImage(0,
                                         mvTmpBuf_jpeg[mu4BestShotIndex].size(),
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_jpeg[mu4BestShotIndex].begin()),
                                         mvTmpBuf_exif[mu4BestShotIndex].size(),                       //header size
                                         reinterpret_cast<uint8_t const*>(mvTmpBuf_exif[mu4BestShotIndex].begin()),                    //header buf
                                         0,                       //callback index
                                         true                     //final image
                                         );
    //

    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
saveToTmpBuf(TmpBuf_t& rvTmpBuf, MUINT8 const*const pData, MUINT32 const u4Size)
{
    MY_LOGI("+ (data,size)=(%p,%d)", pData, u4Size);
    //
    if  ( ! pData || 0 == u4Size )
    {
        MY_LOGW("bad arguments - (pData,u4Size)=(%p,%d)", pData, u4Size);
        return  MFALSE;
    }
    //
    rvTmpBuf.resize(u4Size);
    ::memcpy(rvTmpBuf.begin(), pData, u4Size);
    //
    MY_LOGI("- TmpBuf:(size,capacity)=(%d,%d)", rvTmpBuf.size(), rvTmpBuf.capacity());
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
loadFromTmpBuf(TmpBuf_t const& rvTmpBuf, MUINT8*const pData, MUINT32& ru4Size)
{
    MY_LOGI("+ data(%p); TmpBuf:(size,capacity)=(%d,%d)", pData, rvTmpBuf.size(), rvTmpBuf.capacity());
    //
    if  ( ! pData )
    {
        MY_LOGW("bad arguments - pData(%p)", pData);
        return  MFALSE;
    }
    //
    ru4Size = rvTmpBuf.size();
    ::memcpy(pData, rvTmpBuf.begin(), ru4Size);
    //
    MY_LOGI("-");
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FreefocusShot::
handleBestShotProcess()
{
    MBOOL ret = MTRUE;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.bestshot.dump", value, "0");
    MUINT32 u4DumpFlag = ::atoi(value);

    // compare the jpeg size
    MUINT32 u4MaxSize = mvTmpBuf_jpeg[0].size();
    for (MUINT32 i = 1; i < BEST_SHOT_COUNT; i++)
    {
        if (mvTmpBuf_jpeg[i].size() > u4MaxSize)
        {
            mu4BestShotIndex = i;
            u4MaxSize = mvTmpBuf_jpeg[i].size();
        }
    }

    if (u4DumpFlag != 0)
    {
        char fileName[256] = {'\0'};
        for (MUINT32 i = 0; i < BEST_SHOT_COUNT; i++)
        {
            ::memset(fileName, '\0', 256);
            ::sprintf(fileName, "//sdcard//%d_best_shot_%d.jpg", mu4BestShotIndex,  i );
            MY_LOGD("opening file [%s]", fileName);
            int fd = ::open(fileName, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR|S_IRUSR);
            if (fd < 0) {
                MY_LOGE("failed to create file [%s]: %s", fileName, ::strerror(errno));
                return false;
            }

            ::write(fd,
                      reinterpret_cast<void const*>(mvTmpBuf_exif[i].begin()),
                      mvTmpBuf_exif[i].size()
                     );
            ::write(fd,
                      reinterpret_cast<void const*>(mvTmpBuf_jpeg[i].begin()),
                      mvTmpBuf_jpeg[i].size()
                     );
            ::close(fd);
        }
    }

    return ret;
}

MAVClient*
FreefocusShot::
createInstance(int ShotNum)
{
    MY_LOGD("createInstance ShotNum %d", ShotNum);

	//filename = (char *)mpParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH).string();

    return  new MAVClient(ShotNum,mpParamsMgr);

}

