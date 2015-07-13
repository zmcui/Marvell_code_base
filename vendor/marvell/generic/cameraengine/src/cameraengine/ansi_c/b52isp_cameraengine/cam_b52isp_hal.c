/******************************************************************************
//(C) Copyright [2013 - 2014] Marvell International Ltd.
//All Rights Reserved
******************************************************************************/

CAM_Error isp_query_capability( void *hIspHandle, CAM_Int32s iSensorID, CAM_CameraCapability *pCameraCap )
{
	LOG_E();
	_CAM_IspState *pIspState = (_CAM_IspState*)hIspHandle;
	_CAM_SensorInfo       stSensorInfo;
	_CAM_SystemConfig     stSystemConfig;
	CAM_PortCapability    *pPortCap = NULL;
	CAM_Int8u             i;
	CAM_Bool              bIsBigSensor, bIsDualCamera;
	CAM_Int32s            ret = 0;
	CAM_Bool			  bIsBackSensor;
	CAM_Int32s            iSensorWidth;
	CAM_ModuleInfo        *pModuleInfo;
	char value_rawdump[PROPERTY_VALUE_MAX];

	_CHECK_BAD_POINTER( pCameraCap );
	_CHECK_BAD_POINTER( hIspHandle );

	ASSERT( 0 < giCurrentSensorCnt );
	ASSERT( 0 <= iSensorID && iSensorID < giCurrentSensorCnt );

	property_get(PROP_RAW_DUMP, value_rawdump, "0");
	// get sensor attribute
	stSensorInfo = gCurrentSensorList[iSensorID];
	pModuleInfo = &(stSensorInfo.stSensorProp.stModuleInfo);

	if (pIspState->bRecordingHint)
	{
		iSensorWidth = pModuleInfo->stResolution.iWidth / pModuleInfo->fSensorBin[BIN_MODE_VIDEO];
	}
	else
	{
		iSensorWidth = pModuleInfo->stResolution.iWidth / pModuleInfo->fSensorBin[BIN_MODE_PREVIEW];
	}
	bIsBigSensor = iSensorWidth > B52_LINE_BUFFER_LENGTH;

	bIsBackSensor = gCurrentSensorList[iSensorID].stSensorProp.iFacing == CAM_SENSOR_FACING_BACK;

	if ( pIspState )
	{
		bIsDualCamera = pIspState->bIsDualCamera;
	}
	/*else
	{
		bIsDualCamera = 0;   // coverity issue CID 30822 : Logically dead code
	}*/

	if ( hIspHandle && pIspState->stSystemConfig.iPipelineNumber > 0 )
	{
		stSystemConfig = pIspState->stSystemConfig;
	}
	else
	{
		// get pipeline strategy according to cfg file, single camera by dflt.
		ret = parse_pipeline_configuration( B52_PIPELINE_CONFIG_FILE, &stSystemConfig, bIsBigSensor, gCurrentSensorList[iSensorID].stSensorProp.stModuleInfo.fFullSizeFps,
				bIsDualCamera, gCurrentSensorList[iSensorID].stSensorProp.iFacing);
		ASSERT( ret == 0 );
	}

	// TODO: get preview/video/still/video_snapshot port capability according to sensor, ISP & cfg info
	pPortCap = &pCameraCap->stPortCapability[CAM_PORT_PREVIEW];

	pPortCap->bArbitrarySize  = CAM_TRUE;
	pPortCap->stMin.iWidth    = 176;
	pPortCap->stMin.iHeight   = 144;
	pPortCap->stMax.iWidth    = 1920;
	pPortCap->stMax.iHeight   = 1080;

	char value[PROPERTY_VALUE_MAX];
	property_get(PROP_PLATFORM_CMTB, value, "");

	if (strcmp(value, "")) {
		pPortCap->stMax.iWidth  = pIspState->stSensor.stSensorProp.stModuleInfo.stResolution.iWidth;
		pPortCap->stMax.iHeight = pIspState->stSensor.stSensorProp.stModuleInfo.stResolution.iHeight;
	}

	pPortCap->iSupportedFormatCnt = B52_MAX_OUTPUT_FORMAT;
	for ( i = 0; i < pPortCap->iSupportedFormatCnt; i++ )
	{
		pPortCap->eSupportedFormat[i] = gB52OutputFormatList[i];
	}

	pPortCap = &pCameraCap->stPortCapability[CAM_PORT_VIDEO];

	pPortCap->bArbitrarySize  = CAM_TRUE;
	pPortCap->stMin.iWidth    = 176;
	pPortCap->stMin.iHeight   = 144;
	pPortCap->stMax.iWidth    = 1920;
	pPortCap->stMax.iHeight   = 1080;

	pPortCap->iSupportedFormatCnt = B52_MAX_OUTPUT_FORMAT;
	for ( i = 0; i < pPortCap->iSupportedFormatCnt; i++ )
	{
		pPortCap->eSupportedFormat[i] = gB52OutputFormatList[i];
	}

	pPortCap = &pCameraCap->stPortCapability[CAM_PORT_STILL];

	pPortCap->bArbitrarySize  = CAM_TRUE;
	pPortCap->stMin.iWidth    = 176;
	pPortCap->stMin.iHeight   = 144;
	pPortCap->stMax.iWidth    = pIspState->stSystemConfig.eStrategy == CAM_PIPELINE_STRATEGY_OFFLINE || 2 == atoi(value_rawdump) ?
		pIspState->stCurrentSensorRawSize.iWidth : pIspState->stCurrentIDIRawSize.iWidth;
	pPortCap->stMax.iHeight   = pIspState->stSystemConfig.eStrategy == CAM_PIPELINE_STRATEGY_OFFLINE || 2 == atoi(value_rawdump) ?
		pIspState->stCurrentSensorRawSize.iHeight : pIspState->stCurrentIDIRawSize.iHeight;

	CELOG( "cuizm== CAM_PORT_STILL %s IN || iWidth = %d, iHeight = %d\n", __func__, pPortCap->stMax.iWidth, pPortCap->stMax.iHeight);

	pPortCap->iSupportedFormatCnt = B52_MAX_OUTPUT_FORMAT;
	for ( i = 0; i < pPortCap->iSupportedFormatCnt; i++ )
	{
		pPortCap->eSupportedFormat[i] = gB52OutputFormatList[i];
	}
	// add supported raw format.
	pPortCap->eSupportedFormat[pPortCap->iSupportedFormatCnt] = gCurrentSensorList[iSensorID].stSensorProp.stModuleInfo.eRawFormat;
	pPortCap->iSupportedFormatCnt++;
	pPortCap = &pCameraCap->stPortCapability[CAM_PORT_VIDEO_SNAPSHOT];

	pPortCap->bArbitrarySize  = CAM_TRUE;
	pPortCap->stMin.iWidth    = 176;
	pPortCap->stMin.iHeight   = 144;
	pPortCap->stMax.iWidth    = pIspState->stSystemConfig.eStrategy == CAM_PIPELINE_STRATEGY_OFFLINE ? pIspState->stCurrentSensorRawSize.iWidth :
		pIspState->stCurrentIDIRawSize.iWidth;
	pPortCap->stMax.iHeight   = pIspState->stSystemConfig.eStrategy == CAM_PIPELINE_STRATEGY_OFFLINE ? pIspState->stCurrentSensorRawSize.iHeight :
		pIspState->stCurrentIDIRawSize.iHeight;


	pPortCap->iSupportedFormatCnt = B52_MAX_OUTPUT_FORMAT;
	for ( i = 0; i < pPortCap->iSupportedFormatCnt; i++ )
	{
		pPortCap->eSupportedFormat[i] = gB52OutputFormatList[i];
	}

	// shotmode/shotparam capability according to B52 ISP, cfg & single/dual camera.
	// capture shotmode
	if ( pIspState && pIspState->iSupportedCaptureShotModeCnt )
	{
		pCameraCap->iSupportedCaptureShotModeCnt = pIspState->iSupportedCaptureShotModeCnt;
		for ( i = 0; i < pIspState->iSupportedCaptureShotModeCnt; i++ )
		{
			pCameraCap->eSupportedCaptureShotMode[i] = pIspState->astDfltCaptureShotModeData[i].stShotParam.eShotMode;
		}
	}
	else
	{
		pCameraCap->iSupportedCaptureShotModeCnt = 1;
		pCameraCap->eSupportedCaptureShotMode[0] = CAM_SHOTMODE_MANUAL;

		// update supported shot mode according to cfg file
	}

	_IspGetShotModeCap( hIspHandle, &pCameraCap->stSupportedCaptureShotParams, bIsBackSensor, CAM_TRUE );

	if ( pIspState && pIspState->iSupportedVideoShotModeCnt )
	{
		pCameraCap->iSupportedVideoShotModeCnt = pIspState->iSupportedVideoShotModeCnt;
		for ( i = 0; i < pIspState->iSupportedVideoShotModeCnt; i++ )
		{
			pCameraCap->eSupportedVideoShotMode[i] = pIspState->astDfltVideoShotModeData[i].stShotParam.eShotMode;
		}
	}
	else
	{
		pCameraCap->iSupportedVideoShotModeCnt = 1;
		pCameraCap->eSupportedVideoShotMode[0] = CAM_SHOTMODE_MANUAL;

		// update supported shot mode according to cfg file
	}

	_IspGetShotModeCap( hIspHandle, &pCameraCap->stSupportedVideoShotParams, bIsBackSensor, CAM_FALSE );

	return CAM_ERROR_NONE;
}

static CAM_Int32s _get_default_shot_mode_setting( _CAM_ShotModeConfig *pShotModeConfig, CAM_Int32s iShotModeCnt, CAM_Int32s iSensorID,
		CAM_ShotModeData *pDfltShotModeSetting, CAM_Int32s *pCnt )
{
  /*to do*/

	for( j = 0; j < iShotModeCnt; i++, j++ )
	{
		//OEM customization values
		pDfltShotModeSetting[i].stShotParam.eShotMode  = pShotModeConfig[j].eShotMode;
		pDfltShotModeSetting[i].stShotParam.iFpsQ16    = (CAM_Int32s)( pShotModeConfig[j].fTargetFrameRate * 65536.0 );
		pDfltShotModeSetting[i].stShotParam.iEvCompQ16 = (CAM_Int32s)( pShotModeConfig[j].fDefaultEvComp * 65536.0 );

		pDfltShotModeSetting[i].stShotParam.eIsoMode        = pShotModeConfig[j].eDefaultIsoMode;
		pDfltShotModeSetting[i].stShotParam.eExpMeterMode   = pShotModeConfig[j].eDefaultExpMeterMode;
		pDfltShotModeSetting[i].stShotParam.eBandFilterMode = pShotModeConfig[j].eDefaultBdFltMode;
		pDfltShotModeSetting[i].stShotParam.eWBMode         = pShotModeConfig[j].eDefaultWBMode;

		pDfltShotModeSetting[i].stShotParam.eColorEffect   = pShotModeConfig[j].eDefaultColorEffect;
		pDfltShotModeSetting[i].stShotParam.eFaceEffect    = pShotModeConfig[j].eDefaultFaceEffect;
		pDfltShotModeSetting[i].stShotParam.eFlashMode     = bIsSptFlash ? pShotModeConfig[j].eDefaultFlashMode : CAM_FLASH_OFF;
		pDfltShotModeSetting[i].stShotParam.eFocusMode     = bIsSptFocus ? pShotModeConfig[j].eDefaultFocusMode : CAM_FOCUS_INFINITY;

    CELOG("cuizm== %s IN eFlashMode = %d, eFocusMode = %d", __FUNCTION__, pDfltShotModeSetting[i].stShotParam.eFlashMode,
        pDfltShotModeSetting[i].stShotParam.eFocusMode);

		pDfltShotModeSetting[i].stShotParam.eFocusZoneMode = bIsSptFocus ? pShotModeConfig[j].eDefaultFocusZoneMode : CAM_FOCUSZONEMODE_LIMIT;

		pDfltShotModeSetting[i].stShotParam.eStillSubMode       = pShotModeConfig[j].eDefaultStillSubMode;
		pDfltShotModeSetting[i].stShotParam.stStillSubModeParam = pShotModeConfig[j].stDefaultStillSubModeParam;
		pDfltShotModeSetting[i].stShotParam.eVideoSubMode       = pShotModeConfig[j].eDefaultVideoSubMode;

		pDfltShotModeSetting[i].stShotParam.iSaturation = pShotModeConfig[j].iDefaultSaturation;
		pDfltShotModeSetting[i].stShotParam.iBrightness = pShotModeConfig[j].iDefaultBrightness;
		pDfltShotModeSetting[i].stShotParam.iContrast   = pShotModeConfig[j].iDefaultContrast;
		pDfltShotModeSetting[i].stShotParam.iSharpness  = pShotModeConfig[j].iDefaultSharpness;
#if 0
		//max ExpTime for video & still
		pDfltShotModeSetting[i].uiMaxExpTimeVideo = (CAM_Int32u) (( 1000.f/pShotModeConfig[j].fAcceptableFrameRate + 0.5 ) * (1 << DxOISP_EXPOSURETIME_FRAC_PART));
		ASSERT( pDfltShotModeSetting[i].uiMaxExpTimeVideo > 0 );
		pDfltShotModeSetting[i].uiMaxExpTimeStill = (CAM_Int32u) (( pShotModeConfig[j].fMaxStillExposureTime + 0.5 )  * ( 1 << DxOISP_EXPOSURETIME_FRAC_PART));
#endif
		// init these params which are not in the OEM file to avoid un-expected exception
		pDfltShotModeSetting[i].stShotParam.eSensorRotation  = CAM_FLIPROTATE_NORMAL;
		pDfltShotModeSetting[i].stShotParam.iDigZoomQ16      = 1 << 16;;
		pDfltShotModeSetting[i].stShotParam.iOptZoomQ16      = 1 << 16;
		pDfltShotModeSetting[i].stShotParam.iFocusPos        = -1;
		pDfltShotModeSetting[i].stShotParam.iShutterSpeedQ16 = -1;
		pDfltShotModeSetting[i].stShotParam.iB52ExposureLineQ4 = -1;

		pDfltShotModeSetting[i].stShotParam.iFNumQ16         = -1;
		pDfltShotModeSetting[i].stShotParam.eExpMode         = -1;
		pDfltShotModeSetting[i].stShotParam.bAutoExpLock     = CAM_FALSE;
		pDfltShotModeSetting[i].stShotParam.bAWBLock         = CAM_FALSE;
		pDfltShotModeSetting[i].stShotParam.bPreviewAspectRatioOverFOV = CAM_TRUE;

		memset( &pDfltShotModeSetting[i].stShotParam.stExpMeterROI, 0, sizeof(CAM_MultiROI) );
		memset( &pDfltShotModeSetting[i].stShotParam.stFocusROI, 0, sizeof(CAM_MultiROI) );

		pDfltShotModeSetting[i].stShotParam.iB52SensorGainQ16       = -1;
		pDfltShotModeSetting[i].stShotParam.stB52AWBGains.iBGainQ7  = -1;
		pDfltShotModeSetting[i].stShotParam.stB52AWBGains.iGbGainQ7 = -1;
		pDfltShotModeSetting[i].stShotParam.stB52AWBGains.iGrGainQ7 = -1;
		pDfltShotModeSetting[i].stShotParam.stB52AWBGains.iRGainQ7  = -1;
	}

	if (iShotModeCnt > 0) *pCnt = i;
	return 0;

}


CAM_Error isp_get_shotparams( void *hIspHandle, CAM_ShotParam *pShotParam )
{
	LOG_E();
	_CAM_IspState    *pIspState = (_CAM_IspState*)hIspHandle;

	_CHECK_BAD_POINTER( hIspHandle );

	*pShotParam = pIspState->stCurrentShotParam;

	return CAM_ERROR_NONE;
}
