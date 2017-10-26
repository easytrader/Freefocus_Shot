

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_BESTSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_BESTSHOT_H_

//
#include <vector>
#include "MAVClient.h"
#include <IParamsManager.h>

#if 0
#include <aaa_hal_base.h>
#endif
#include <mcu_drv.h>
//#include <CamUtils.h>
//#include <system/camera.h>
//#include <drv/imem_drv.h>
//#include <pthread.h>
//#include <semaphore.h>
//#include <cutils/properties.h>
//#include <sys/prctl.h>
//#include <sys/resource.h>
//#include "CamTypes.h"
//#include "3DF_hal_base.h"
//#include "inc/IFeatureClient.h"

using namespace std;
using namespace NSCamClient1;
#if 0
using namespace NS3A;
#endif

namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/

#define BEST_SHOT_COUNT 9


/******************************************************************************
 *
 ******************************************************************************/
class FreefocusShot : public ImpShot
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~FreefocusShot();
                                    FreefocusShot(
                                        char const*const pszShotName,
                                        uint32_t const u4ShotMode,
                                        int32_t const i4OpenId,
                                        sp<IParamsManager> pParamsMgr
                                    );

public:     ////                    Operations.

    //  This function is invoked when this object is firstly created.
    //  All resources can be allocated here.
    virtual bool                    onCreate(int32_t iPictureWidth, int32_t iPictureHeigh);

    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
    virtual void                    onDestroy();

    virtual bool                    sendCommand(
                                        uint32_t const  cmd,
                                        uint32_t const  arg1,
                                        uint32_t const  arg2
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual bool                    onCmd_reset();
    virtual bool                    onCmd_capture();
    virtual void                    onCmd_cancel();


protected:  ////                    callbacks for ICamShot
    static MBOOL fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg);
    static MBOOL fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg);

protected:
    MBOOL           handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size);
	MBOOL           handleYuvData(MUINT8* const puBuf, MUINT32 const u4Size);
    MBOOL           handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize);
    MBOOL           handleShutter();
    MBOOL           handleBestShotProcess();
    MBOOL           handleCaptureDone();

protected:  ////    Temp Buffers.
    typedef vector<MUINT8> TmpBuf_t;
	MBOOL dumpImg(MUINT8 *addr,MUINT32 size,char const * const tag,char const * const filetype,uint32_t filenum);
    MBOOL           saveToTmpBuf(TmpBuf_t& rvTmpBuf, MUINT8 const*const pData, MUINT32 const u4Size);
    MBOOL           loadFromTmpBuf(TmpBuf_t const& rvTmpBuf, MUINT8*const pData, MUINT32& ru4Size);

    TmpBuf_t        mvTmpBuf_postview[BEST_SHOT_COUNT];      //
    //TmpBuf_t        mvTmpBuf_yuv[BEST_SHOT_COUNT];      //
    TmpBuf_t        mvTmpBuf_jpeg[BEST_SHOT_COUNT];          //
    TmpBuf_t        mvTmpBuf_exif[BEST_SHOT_COUNT];          //

private:
    MUINT32         mu4CurrentCount;
    MUINT32         mu4TotalCount;
    MUINT32         mu4BestShotValue;       //
    MUINT32         mu4BestShotIndex;       //
    MBOOL volatile    mfgIsCanceled;
//=============================================================================
//================  MAV  ======================================================
protected:
	MAVClient* createInstance(int ShotNum);
	static MBOOL handleMAVImgCallBacknn(MVOID* const puJpegBuf, int u4SrcWidth, int u4SrcHeight);
private:
	MAVClient* MAVPObject;
	MBOOL           mMAVStopFlag;
	MUINT32 CropIndexTable(MUINT32 SnapNum);
	MUINT32 LookupIndexTable(MUINT32 SnapNum);
	MUINT32 GetFocalLengthIndex(MUINT32 StepIndex);
	sp<IParamsManager> mpParamsMgr;
	//3a
	//3A
	MCUDrv*     m_pMcuDrv;
	MUINT32     movestep;
#if 0
    Hal3ABase *m_p3AHal;
#endif
};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_BESTSHOT_H_

