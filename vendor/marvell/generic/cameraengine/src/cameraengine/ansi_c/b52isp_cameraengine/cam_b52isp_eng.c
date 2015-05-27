/******************************************************************************
*(C) Copyright [2013 - 2014] Marvell International Ltd.
* All Rights Reserved
******************************************************************************/

/* global variables */
const _CAM_DriverEntry entry_b52isp =
{
	"camera_b52isp",
	_Open,
	_Close,
	_EnumSensors,
	_SetSensorID,
	_GetSensorID,
	_QueryCameraCap,
	_SetEventHandler,
	_SetFrameHandler,
	_SetState,
	_GetState,
	_PortEnqueueBuffer,
	_PortFlushBuffer,
	_PortGetBufReq,
	_PortSetConfig,
	_PortGetConfig,
	_StartFocus,
	_CancelFocus,
	_SetCameraInfo,
	_StartFacialDetection,
	_CancelFacialDetection,
	_SetRecordingHint,
	_GetRecordingHint,
	_SetCaptureShotMode,
	_SetVideoShotMode,
	_QueryCaptureShotModeCap,
	_QueryVideoShotModeCap,
	_GetCaptureShotMode,
	_GetVideoShotMode,
	_SetShootingParameters,
	_GetShootingParameters,
	_ResetCamera,
	_PreviewEnabled,
	_UpdateTuningData,
	_DumpTuningData,
	_SetValidPipe,
	_StartStream,
	_StopStream,
	_SendCommand,
};

