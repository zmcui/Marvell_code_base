/*
 * (C) Copyright 2010 Marvell International Ltd.
 * All Rights Reserved
 *
 * MARVELL CONFIDENTIAL
 * Copyright 2008 ~ 2010 Marvell International Ltd All Rights Reserved.
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Marvell International Ltd or its
 * suppliers or licensors. Title to the Material remains with Marvell International Ltd
 * or its suppliers and licensors. The Material contains trade secrets and
 * proprietary and confidential information of Marvell or its suppliers and
 * licensors. The Material is protected by worldwide copyright and trade secret
 * laws and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Marvell's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by Marvell in writing.
 *
 */

namespace android {

    CAM_Error Engine::ceInit(CAM_DeviceHandle *pHandle,CAM_CameraCapability *pcamera_caps)
    {
        CAM_Error error = CAM_ERROR_NONE;
        int ret;
        CAM_DeviceHandle handle;
        CAM_PortCapability *pCap;

        error = CAM_GetHandle(&handle);
        *pHandle = handle;
        ASSERT_CAM_ERROR(error);

        const char* sensorname = mSetting.getSensorName();

        // select the proper sensor id
        int sensorid = mSetting.getEngineSensorID();
        error = CAM_SetSensorID(handle, sensorid);
        ASSERT_CAM_ERROR(error);
        TRACE_D("Current sensor index: %d - %s", sensorid, sensorname);

        error = CAM_QueryCameraCap(handle, sensorid, pcamera_caps);
        ASSERT_CAM_ERROR(error);
        error = CAM_SetEventHandler(handle, NotifyEvents, this);
        ASSERT_CAM_ERROR(error);

        //Init shotParameters.
        //ShotParam will be udpated only when scene mode is changed.
        CAM_ShotParam *pShotParam = mSetting.getCurrentShotParam();
        error = CAM_GetShootingParameters(handle, pShotParam);

        //Store the default focus zone mode and default meter mode here
        mDefaultFocusZoneMode = pShotParam->eFocusZoneMode;
        mDefaultMeterMode = pShotParam->eExpMeterMode;
        TRACE_D("%s: default Focus Zone mode is:  %d",__FUNCTION__, mDefaultFocusZoneMode);
        TRACE_D("%s: default Meter mode is:  %d",__FUNCTION__, mDefaultMeterMode);

        ASSERT_CAM_ERROR(error);

        return error;
    }

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

}; // namespace android
