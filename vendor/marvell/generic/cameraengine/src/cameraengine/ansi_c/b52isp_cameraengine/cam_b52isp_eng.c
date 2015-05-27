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

static CAM_Error _set_sensor_id( _CAM_B52IspState *pCameraState, CAM_Int32s iSensorID )
{
/*to do*/

	pCameraState->stShotParamSetting.eShotMode = pCameraState->bRecordingHint ?
		pCameraState->eVideoShotMode : pCameraState->eCaptureShotMode;

	error = isp_set_shotmode( pCameraState->hIspHandle, pCameraState->stShotParamSetting.eShotMode );
	ASSERT_CAM_ERROR( error );

	error = isp_get_shotparams( pCameraState->hIspHandle, &pCameraState->stShotParamSetting );
	ASSERT_CAM_ERROR( error );

  /* stShotParamSetting by eShotmode*/
  TRACE(CAM_ERROR, "cuizm== bRecordingHint is %d, eShotmode is %d", pCameraState->bRecordingHint, pCameraState->stShotParamSetting.eShotMode);
  TRACE(CAM_ERROR, "cuizm== eFocusMode is %d", pCameraState->stShotParamSetting.eFocusMode);

/*to do*/

}
