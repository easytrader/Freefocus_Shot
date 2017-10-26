

#ifndef _MTK_HAL_CAMCLIENT_MAVCLIENT1_H_
#define _MTK_HAL_CAMCLIENT_MAVCLIENT1_H_
//
#include <CamUtils.h>
#include <system/camera.h>
#include <mtkcam/drv/imem_drv.h>
#include <pthread.h>
#include <semaphore.h>
#include <cutils/properties.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include "3DF_hal_base.h"
#include "MpoEncoder.h"
#include "hwstddef.h"
#include <IParamsManager.h>
#include <CamInfo.h>
//#include "inc/IFeatureClient.h"

//using namespace android;
//using namespace MtkCamUtils;
//
namespace android {
namespace NSCamClient1 {

typedef MBOOL   (*ImgDataCallback_t)(MVOID* const puJpegBuf, int u4SrcWidth, int u4SrcHeight);

/******************************************************************************
 *  Preview Client Handler.
 ******************************************************************************/
class MAVClient
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    //
    MAVClient(int ShotNum, sp<IParamsManager> mpParamsMgr);
    ~MAVClient();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    bool      init(int bufwidth,int bufheight);
    bool      uninit();
    //virtual MINT32    mHalCamFeatureProc(MVOID * bufadr, int32_t& mvX, int32_t& mvY, int32_t& dir, MBOOL& isShot);
    bool      stopFeature(int cancel , int curfocusindex);
    MVOID     setImgCallback(ImgDataCallback_t data_cb);
    MINT32    mHalCamFeatureCompress(int curfocusindex);
//==================Free Focus==========================================================
	bool     initThumbnailBuf(int bufsize);
	bool     uninitThumbnailBuf();
	IMEM_BUF_INFO tempBuf;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MAVClient.Scenario function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MAVClinet function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MBOOL     allocMem(IMEM_BUF_INFO &memBuf);
    MBOOL     deallocMem(IMEM_BUF_INFO &memBuf);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	sem_t             MAVSemThread;
protected:
    MBOOL             mCancel;
    MBOOL             mStop;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Image Buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	MUINT8*           pBufAdr;
	MUINT32           pBufSize;
protected:
    IMemDrv*          mpIMemDrv;
    IMEM_BUF_INFO     mpMotionBuffer;
    IMEM_BUF_INFO     mpMAVMotionBuffer;
    IMEM_BUF_INFO     mpWarpBuffer;
    IMEM_BUF_INFO     mpMAVWorkingBuf;
    int               mMAVFrameWidth;
    int               mMAVFrameHeight;
    int               mMAVFrameSize;
    ImgDataCallback_t mDataCb;
public:
	IMEM_BUF_INFO     mpframeBuffer[MAV_PIPE_MAX_IMAGE_NUM];
	MUINT8*           pMAVExifHeaderBuf;
	MUINT32           uMAVExifHeaderSize;
	MUINT8*           pMAVThumbJpegBuf;
	MUINT32           uMAVThumbJpegSize;
//==============================================================================
	//IMEM_BUF_INFO     mpCaptureYUVBuffer[MAV_PIPE_MAX_IMAGE_NUM];
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    int32_t 	        MAVnum;
    hal3DFBase* 	    mpMAVObj;
    int32_t 	        mMAVFrameIdx;
    int32_t			    mMAVaddImgIdx;
    int32_t 		    mJPGFrameAddr;
    MavPipeResultInfo   mpMAVResult;
    uint8_t  	      	SaveFileName[64];
    //mutable Mutex       mLock;
    //mutable Mutex 	    mLockUninit;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MPO file
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	MBOOL createMPO(MPImageInfo * pMPImageInfo, MUINT32 num, char* file, MUINT32 MPOType);
	MBOOL createPanoJpegImg(IMEM_BUF_INFO Srcbufinfo, int u4SrcWidth, int u4SrcHeight, IMEM_BUF_INFO jpgBuff, MUINT32 &u4JpegSize, MUINT32 index);
	MBOOL createJpegImgWithThumbnail(NSCamHW::ImgBufInfo const &rYuvImgBufInfo, IMEM_BUF_INFO jpgBuff, MUINT32 &u4JpegSize);
	MBOOL createJpegImg(NSCamHW::ImgBufInfo const & rSrcBufInfo, MUINT32 quality, bool fIsAddSOI, NSCamHW::ImgBufInfo const & rDstBufInfo, MUINT32 & u4EncSize);
	MBOOL makeExifHeader(MUINT32 const u4CamMode, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT8* puExifBuf, MUINT32 &u4FinalExifSize,MUINT32 const Width,MUINT32 const Height,MUINT8* FreeFcousData);

	sp<CamMsgCbInfo>                mpCamMsgCbInfo;
	void copyfilename(sp<IParamsManager> mpParamsMgr);
	void clearfilename(void);
	sp<IParamsManager> pParamsMgr;
	char filename[100];
};
}; // namespace NSCamClient1
}; // namespace android
#endif  //_MTK_HAL_CAMCLIENT_PREVIEW_PREVIEWCLIENT1_H_

