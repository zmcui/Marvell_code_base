
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
