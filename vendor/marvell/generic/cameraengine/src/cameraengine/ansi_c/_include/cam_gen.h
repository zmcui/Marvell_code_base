/*******************************************************************************
//(C) Copyright [2009 - 2011] Marvell International Ltd.
//All Rights Reserved
*******************************************************************************/

#ifndef _CAM_GEN_H_
#define _CAM_GEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cam_utility.h"

typedef void _CAM_DeviceData;

typedef struct
{
	const char          *pName;
	CAM_Error (*fnOpen) ( void ** );
	CAM_Error (*fnClose) ( void ** );
	CAM_Error (*fnEnumSensors) ( CAM_Int32s *, CAM_DeviceProp[] );
	CAM_Error (*fnSetSensorID) ( _CAM_DeviceData *, CAM_Int32s );
	CAM_Error (*fnGetSensorID) ( _CAM_DeviceData *, CAM_Int32s * );
	CAM_Error (*fnQueryCameraCap) ( _CAM_DeviceData *, CAM_Int32s, CAM_CameraCapability * );
	CAM_Error (*fnSetEventHandler) ( _CAM_DeviceData *, CAM_EventHandler, void * );
	CAM_Error (*fnSetFrameHandler) ( _CAM_DeviceData *, CAM_FrameHandler, void * );
	CAM_Error (*fnSetState) ( _CAM_DeviceData *, CAM_CaptureState );
	CAM_Error (*fnGetState) ( _CAM_DeviceData *, CAM_CaptureState * );
	CAM_Error (*fnPortEnqueueBuffer) ( _CAM_DeviceData *, CAM_Int32s, CAM_ImageBuffer * );
	CAM_Error (*fnPortFlushBuffer) ( _CAM_DeviceData *, CAM_Int32s );
	CAM_Error (*fnPortGetBufReq) ( _CAM_DeviceData *, CAM_Int32s, CAM_ImageBufferReq * );
	CAM_Error (*fnPortSetConfig) ( _CAM_DeviceData *, CAM_Int32s, CAM_PortConfig * );
	CAM_Error (*fnPortGetConfig) ( _CAM_DeviceData *, CAM_Int32s, CAM_PortConfig * );
	CAM_Error (*fnStartFocus) ( _CAM_DeviceData * );
	CAM_Error (*fnCancelFocus) ( _CAM_DeviceData * );
	CAM_Error (*fnSetCameraInfo) ( _CAM_DeviceData *, CAM_CameraInfo );
	CAM_Error (*fnStartFacialDetection) ( _CAM_DeviceData *, CAM_Int32s );
	CAM_Error (*fnCancelFacialDetection) ( _CAM_DeviceData * );
	CAM_Error (*fnSetRecordingHint) ( _CAM_DeviceData *, CAM_Bool );
	CAM_Error (*fnGetRecordingHint) ( _CAM_DeviceData *, CAM_Bool * );
	CAM_Error (*fnSetCaptureShotMode) ( _CAM_DeviceData *, CAM_ShotMode );
	CAM_Error (*fnSetVideoShotMode) ( _CAM_DeviceData *, CAM_ShotMode );
	CAM_Error (*fnQueryCaptureShotModeCap) ( _CAM_DeviceData *, CAM_ShotMode, CAM_ShotModeCapability * );
	CAM_Error (*fnQueryVideoShotModeCap) ( _CAM_DeviceData *, CAM_ShotMode, CAM_ShotModeCapability * );
	CAM_Error (*fnGetCaptureShotMode) ( _CAM_DeviceData *, CAM_ShotMode * );
	CAM_Error (*fnGetVideoShotMode) ( _CAM_DeviceData *, CAM_ShotMode * );
	CAM_Error (*fnSetShootingParameters) ( _CAM_DeviceData *, CAM_ShotParam * );
	CAM_Error (*fnGetShootingParameters) ( _CAM_DeviceData *, CAM_ShotParam * );
	CAM_Error (*fnResetCamera) ( _CAM_DeviceData *, CAM_Int32s );
	CAM_Int32s (*fnPreviewEnabled) ( _CAM_DeviceData * );
	CAM_Error (*fnUpdateTuningData) ( _CAM_DeviceData * );
	CAM_Error (*fnDumpTuningData) ( _CAM_DeviceData * );
	CAM_Error (*fnSetValidPipe) ( _CAM_DeviceData *, CAM_Bool );
	CAM_Error (*fnStartStream) ( _CAM_DeviceData *, CAM_StreamType );
	CAM_Error (*fnStopStream) ( _CAM_DeviceData *, CAM_StreamType );
	CAM_Int32s (*fnSendCommand)(_CAM_DeviceData *, CAM_Cmd, CAM_Int32s, CAM_Int32s);
} _CAM_DriverEntry;

typedef struct
{
	CAM_Int32s       iTotalCnt;
	CAM_Int32s       iExtIspCnt;
	CAM_DeviceProp   stDeviceProp[CAM_MAX_SUPPORT_CAMERA];

	// event handler
	CAM_EventHandler fnEventHandler;
	void             *pEventUserData;
	// frame handler
	CAM_FrameHandler fnFrameHandler;
	void             *pFrameUserData;

	CAM_Int32s       iSensorID;
	_CAM_DriverEntry *pEntry;
	_CAM_DeviceData  *pDeviceData;
} _CAM_DeviceState;

// supported entries
extern const _CAM_DriverEntry entry_socisp;
extern const _CAM_DriverEntry entry_extisp;
extern const _CAM_DriverEntry entry_b52isp;

typedef struct
{
	CAM_Int32s       iWidth;  // width before rotate
	CAM_Int32s       iHeight; // height before rotate
	CAM_ImageFormat  eFormat;
	CAM_FlipRotate   eRotation;
	CAM_JpegParam    *pJpegParam;
	CAM_Bool         bIsStreaming;
} _CAM_ImageInfo;

//  @_CAM_PortState: indicate the port status like: output image attribute, buffer queue state and buffer requirement
typedef struct
{
	CAM_Int32s         iWidth;  // this is the width before port rotation
	CAM_Int32s         iHeight; // this is the height before port rotation
	CAM_ImageFormat    eFormat;
	CAM_FlipRotate     eRotation;
	CAM_JpegParam      stJpegParam;

	_CAM_Queue         qEmpty;

	CAM_ImageBufferReq stDriverBufReq;  // buffer requirement of driver
	CAM_ImageBufferReq stPortBufReq;    // buffer requirement of port

	// PPU related
	CAM_Int32s         iPpuModuleId;
	CAM_Bool           bUsePrivateBuffer;
	CAM_Int8u          *pPPUMemHeap;

	CAM_ImageBuffer    astPpuSrcImg[CAM_MAX_PORT_BUF_CNT];    // the array index is used to fill the CAM_ImageBuffer::iUserIndex
	CAM_Bool           abPpuSrcImgUsed[CAM_MAX_PORT_BUF_CNT]; // whether the position on corresponding index is using
	CAM_Int32s         iPpuSrcImgAllocateCnt;                 // indicate how many image buffers in stPpuSrcImage are allocated by camera-enigne itself
	CAM_Int32s         iPpuSrcImgUsedCnt;                     // indicate how many image buffers in stPpuSrcImage are in-used by camera-enigne itself
	_CAM_Queue         qUnpairedExtBufList;                   // list of external buffers pending by insufficient PPU buffer

	// buffer ring
	_CAM_Queue         qPpuEmptyBuffer; // PPU src buffer, waiting for data.
	_CAM_Queue         qPpuUVDNSBuffer; // PPU src buffer, waiting for UVDNS.
	_CAM_Queue         qPpuDataBuffer; // PPU src buffer, waiting for process.
	_CAM_Queue         qPpuZSLBuffer; // PPU zsl buffer, used for online zsl.
	CAM_ImageBuffer   astPpuZSLImg[CAM_MAX_PORT_BUF_CNT];
	_CAM_Queue         qPpuZoomBuffer; // PPU zoom buffer, used for gcu zoom.
	CAM_ImageBuffer   astPpuZoomImg[CAM_PPU_ZOOM_BUF_COUNT];
	CAM_Bool           bIsOnLineProcess; // only used by b52

	CAM_Int32s         iStillRestCount;
	CAM_Bool           bIsPpuSrcImgQueued;

	// port mutex
	void               *hMutex;
} _CAM_PortState;

#ifdef __cplusplus
}
#endif

#endif  // _CAM_GEN_H_
