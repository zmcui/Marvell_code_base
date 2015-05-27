
    CAM_Error Engine::ceGetCurrentSceneModeCap(const CAM_DeviceHandle &handle,
            CameraSetting& setting)
{
        CAM_Error error = CAM_ERROR_NONE;
        String8 v = String8("");

        CAM_ShotParam tempShotParam;
        error = CAM_GetShootingParameters(handle, &tempShotParam);
        ASSERT_CAM_ERROR(error);

        CAM_ShotParam *pShotParam = &tempShotParam;

  /*to do*/

        // focus mode
        /*cuizm: first time, this para come from _set_sensor_id()*/
        CAM_FocusMode FocusMode = pShotParam->eFocusMode;
        v = String8("");
        v = setting.map_ce2andr(CameraSetting::map_focus,FocusMode);
        if( v != String8(""))
        {
            setting.set(CameraParameters::KEY_FOCUS_MODE,v.string());
            TRACE_CAP("%s:CE used focus mode:%s",__FUNCTION__,v.string());
        }


  /*to do*/

}
