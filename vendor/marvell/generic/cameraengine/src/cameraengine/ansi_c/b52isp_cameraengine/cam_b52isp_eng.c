/******************************************************************************
*(C) Copyright [2013 - 2014] Marvell International Ltd.
* All Rights Reserved
******************************************************************************/

/*****************************************************************************************/
/* Shot mode setting: 1.OEM customization; 2. CameraEngine default setting strategy      */
/*****************************************************************************************/

/* Native Functions Declaration */

// Camera Engine entry functions

// Camera Engine internal functions
static CAM_Error _get_camera_capability( _CAM_B52IspState *pCameraState, CAM_Int32s iSensorID, CAM_CameraCapability *pCameraCap );

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

static CAM_Error _QueryCameraCap( _CAM_DeviceData *pDeviceData, CAM_Int32s iSensorID, CAM_CameraCapability *pCapability )
{
	LOG_E();
	CAM_Error              error         = CAM_ERROR_NONE;
	_CAM_B52IspState       *pCameraState = NULL;

	_CHECK_BAD_POINTER( pCapability );

	pCameraState = (_CAM_B52IspState *)pDeviceData;

	error = _get_camera_capability( pCameraState, iSensorID, pCapability );

	return error;
}

static CAM_Error _get_camera_capability( _CAM_B52IspState *pCameraState, CAM_Int32s iSensorID, CAM_CameraCapability *pCameraCap )
{
	CAM_Error       error = CAM_ERROR_NONE;
	void            *hIspHandle = NULL;
	CAM_Int32s      iPpuFmtCnt = 0;
	CAM_ImageFormat stPpuFmtCap[CAM_MAX_SUPPORT_IMAGE_FORMAT_CNT] = {0};
	CAM_Int32s      i;
	CAM_DeviceProp  astCameraProp[CAM_MAX_SUPPORT_CAMERA];
	CAM_Bool        bIsBackSensor = CAM_FALSE;
	CAM_Int32s      iCameraNum;

	_CHECK_BAD_POINTER( pCameraCap );

	memset( pCameraCap, 0, sizeof( CAM_CameraCapability ) );

	// query isp capability
	if ( pCameraState == NULL )
	{
		hIspHandle = NULL;
	}
	else
	{
		hIspHandle = pCameraState->hIspHandle;
	}

	isp_enum_sensor( &iCameraNum, astCameraProp );
	ASSERT( iSensorID >= 0 && iSensorID < iCameraNum );

	bIsBackSensor = astCameraProp[iSensorID].iFacing == CAM_SENSOR_FACING_BACK;

	// query isp capability first.
	// (Size/ShotModeCapability---pCameraState->stCameraCap->stSupported(Capture/Video)ShotParams)
	error = isp_query_capability( hIspHandle, iSensorID, pCameraCap );
	ASSERT_CAM_ERROR( error );

	error = _add_ppu_shotmode_cap( astCameraProp[iSensorID].iFacing, CAM_SHOTMODE_LIMIT, CAM_TRUE, &pCameraCap->stSupportedCaptureShotParams );
	ASSERT_CAM_ERROR( error );

	error = _add_ppu_shotmode_cap( astCameraProp[iSensorID].iFacing, CAM_SHOTMODE_LIMIT, CAM_FALSE, &pCameraCap->stSupportedVideoShotParams );
	ASSERT_CAM_ERROR( error );

	// query ppu capability. (Format/Rotation)
	{
		CAM_JpegCapability stJpegCap;
		CAM_PortCapability *pstPortCap;
		CAM_Int32s         iFmtCnt;

		// we do not consider resolution factor, all resize is perform by ISP
		pstPortCap = &pCameraCap->stPortCapability[CAM_PORT_PREVIEW];
		error = ppu_query_csc_cap( pstPortCap->eSupportedFormat,
				pstPortCap->iSupportedFormatCnt,
				stPpuFmtCap,
				&iPpuFmtCnt, &stJpegCap );
		ASSERT_CAM_ERROR( error );

		iFmtCnt = pstPortCap->iSupportedFormatCnt;
		for ( i = iFmtCnt; i < _MIN( ( iPpuFmtCnt + iFmtCnt ), CAM_MAX_SUPPORT_IMAGE_FORMAT_CNT ); i++ )
		{
			if ( stPpuFmtCap[i - iFmtCnt] == CAM_IMGFMT_JPEG )
			{
				continue;
			}
			pstPortCap->eSupportedFormat[pstPortCap->iSupportedFormatCnt] = stPpuFmtCap[i - iFmtCnt];
			pstPortCap->iSupportedFormatCnt++;
		}

		iPpuFmtCnt = 0;
		pstPortCap = &pCameraCap->stPortCapability[CAM_PORT_VIDEO];
		error = ppu_query_csc_cap( pstPortCap->eSupportedFormat,
				pstPortCap->iSupportedFormatCnt,
				stPpuFmtCap,
				&iPpuFmtCnt, &stJpegCap );
		ASSERT_CAM_ERROR( error );
		iFmtCnt = pstPortCap->iSupportedFormatCnt;
		for ( i = iFmtCnt; i < _MIN( ( iPpuFmtCnt + iFmtCnt ), CAM_MAX_SUPPORT_IMAGE_FORMAT_CNT ); i++ )
		{
			if ( stPpuFmtCap[i - iFmtCnt] == CAM_IMGFMT_JPEG )
			{
				continue;
			}
			pstPortCap->eSupportedFormat[pstPortCap->iSupportedFormatCnt] = stPpuFmtCap[i - iFmtCnt];
			pstPortCap->iSupportedFormatCnt++;
		}

		iPpuFmtCnt = 0;
		pstPortCap = &pCameraCap->stPortCapability[CAM_PORT_STILL];
		error = ppu_query_csc_cap( pstPortCap->eSupportedFormat,
				pstPortCap->iSupportedFormatCnt,
				stPpuFmtCap,
				&iPpuFmtCnt, &stJpegCap );
		ASSERT_CAM_ERROR( error );
		iFmtCnt = pstPortCap->iSupportedFormatCnt;
		for ( i = iFmtCnt; i < _MIN( ( iPpuFmtCnt + iFmtCnt ), CAM_MAX_SUPPORT_IMAGE_FORMAT_CNT ); i++ )
		{
			pstPortCap->eSupportedFormat[pstPortCap->iSupportedFormatCnt] = stPpuFmtCap[i - iFmtCnt];
			pstPortCap->iSupportedFormatCnt++;
		}

		error = ppu_query_rotator_cap( pstPortCap->eSupportedRotate, &pstPortCap->iSupportedRotateCnt );
		ASSERT_CAM_ERROR( error );

		if ( stJpegCap.bSupportJpeg )
		{
			pCameraCap->stSupportedJPEGParams = stJpegCap;
		}
	}

	return error;
}

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
