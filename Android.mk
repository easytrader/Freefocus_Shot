
################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, .)

#-----------------------------------------------------------
#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
LOCAL_C_INCLUDES += $(TOP)/bionic
LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter/inc/Scenario
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/hal/adapter/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter/Scenario/Shot/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/external/mpo/mpoencoder
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/common/hw
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/include/mtkcam/v1
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/include/mtkcam/v1/camutils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/featureio/drv/inc/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/include/mtkcam/exif
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/include/mtkcam/hal
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/Almalence
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/camera/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/hw
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/jpeg/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/m4u
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/drv/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/drv/lens

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/hal/adapter/inc
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/frameworks-ext/av/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/custom/common/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/camera/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/camera/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/hal/adapter/inc/Scenario
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/hal/adapter/Scenario/Shot/inc

#mpo
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/external/mhal/src/core/common/libmpo/mpoencoder
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/external/mhal/src/core/common/libmpo
#--------------------- Almalence start -----------------------
LOCAL_C_INCLUDES += $(TOP)/vendor/Almalence/inc
#--------------------- Almalence end -------------------------

LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/frameworks-ext/av/include/camera
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#LOCAL_WHOLE_STATIC_LIBRARIES += libalmashot_bshot
#LOCAL_WHOLE_STATIC_LIBRARIES += libgomp
#
#--------------------- Almalence start -----------------------
LOCAL_STATIC_LIBRARIES += libalmashot_bshot
LOCAL_STATIC_LIBRARIES += libgomp
#--------------------- Almalence end -------------------------
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES +=
#-----------------------------------------------------------
LOCAL_MODULE := libcam.camadapter.scenario.shot.freefocus

#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

