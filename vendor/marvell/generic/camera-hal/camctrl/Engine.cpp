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

....

    //keep event handler function concise, or else it may affect preview performance.
    void Engine::NotifyEvents(CAM_EventId eEventId,void* param,void *pUserData)
    {
        Engine* self=static_cast<Engine*>(pUserData);
        TRACE_V("%s:eEventId=%d,param=%d",__FUNCTION__,eEventId, (int)param);
        CAM_Error error = CAM_ERROR_NONE;
        CAM_CaptureState state;
        String8 sceneString;
        int *pBestIndex;
        if(self->mMsg != NULL)
        {
            switch ( eEventId )
            {
                case CAM_EVENT_SCENE_UPDATE:
                    TRACE_D("%s:CAM_EVENT_SCENE_UPDATE:%d",__FUNCTION__,(int)param);
                    sceneString = self->mSetting.map_ce2andr(CameraSetting::map_scenemode,(int)param);
                    memset(self->mFaceData.sceneMode, '\0', sizeof(self->mFaceData.sceneMode));
                    strcpy(self->mFaceData.sceneMode, sceneString.string());

                    if (self->mMsg->msgTypeEnabled(CAMERA_MSG_SCENE_UPDATE))
                    {
                        camera_memory_t* scene_detected = self->mGetCameraMemory(-1, 1, 1, NULL);

                        sp<MsgData> msg = new MsgData;
                        MsgData* ptr    = msg.get();
                        ptr->msgType    = CAMERA_MSG_SCENE_UPDATE;
                        ptr->ext1       = 0;
                        ptr->ext2       = 0;
                        ptr->index      = 0;
                        ptr->setListener(msgDone_Cb,scene_detected);
                        ptr->data       = scene_detected;
                        ptr->metadata   = &self->mFaceData;
                        self->mMsg->postMsg(msg);
                    }
                    break;
                case CAM_EVENT_FRAME_DROPPED:
                    break;
                case CAM_EVENT_STILL_SHUTTERCLOSED:
                    TRACE_D("%s:CAM_EVENT_STILL_SHUTTERCLOSED",__FUNCTION__);
                    if (self->mMsg->msgTypeEnabled(CAMERA_MSG_SHUTTER))
                    {
/*
                        int w,h;
                        CameraParameters param=self->getParameters();
                        param.getPreviewSize(&w, &h);
                        image_rect_type rawsize;
                        rawsize.width=w;rawsize.height=h;
                        {
                            sp<MsgData> msg = new MsgData;
                            MsgData* ptr    = msg.get();
                            ptr->msgType    = CAMERA_MSG_SHUTTER;
                            ptr->ext1       = (int32_t)(&rawsize);
                            ptr->ext2       = 0;
                            self->mMsg->postMsg(msg);
                        }
 */
                        self->mCancelInfiniteBurstLock.lock();
                        if ( ! self->bCancelInfiniteBurst )
                        {
                            sp<MsgData> msg = new MsgData;
                            MsgData* ptr    = msg.get();
                            ptr->msgType    = CAMERA_MSG_SHUTTER;
                            ptr->ext1       = 0;
                            ptr->ext2       = 0;
                            self->mMsg->postMsg(msg);
                            self->mShutterCbSent++;
                        }
                        self->mCancelInfiniteBurstLock.unlock();
                    }
                    break;
                case CAM_EVENT_OUTOF_FOCUS:
                    self->mAFStarted = false;
                    break;
                case CAM_EVENT_FOCUS_AUTO_START:
                    TRACE_D("%s:CAM_EVENT_FOCUS_AUTO_START",__FUNCTION__);
                    {
                        if (self->mMsg->msgTypeEnabled(CAMERA_MSG_FOCUS_MOVE))
                        {
                            sp<MsgData> msg = new MsgData;
                            MsgData* ptr    = msg.get();
                            ptr->msgType    = CAMERA_MSG_FOCUS_MOVE;
                            ptr->ext1       = true;
                            ptr->ext2       = 0;
                            self->mMsg->postMsg(msg);
                        }
                    }
                    break;
                case CAM_EVENT_FOCUS_AUTO_STOP:
                    self->mAFStarted = false;
                    TRACE_D("%s:CAM_EVENT_FOCUS_AUTO_STOP:%d",__FUNCTION__,(int)param);
#ifdef __ISP_PARAMETERS__
                    if(B52_TEST_PARAMETERS == self->mEnableIspParams)
                    {
                        self->mMsg->enableMsgType(CAMERA_MSG_ISP_INFO_FOR_CT);
                    }
#endif

                    if (self->mMsg->msgTypeEnabled(CAMERA_MSG_FOCUS_MOVE))
                    {
                        sp<MsgData> msg = new MsgData;
                        MsgData* ptr    = msg.get();
                        ptr->msgType    = CAMERA_MSG_FOCUS_MOVE;
                        ptr->ext1       = false;
                        ptr->ext2       = 0;
                        self->mMsg->postMsg(msg);
                    }
                    else
                    {
                        sp<MsgData> msg = new MsgData;
                        MsgData* ptr    = msg.get();
                        ptr->msgType    = CAMERA_MSG_FOCUS;
                        ptr->ext1       = (((int)param>0)?true:false);
                        ptr->ext2       = 0;
                        self->mMsg->postMsg(msg);
                        self->mFocusState = AUTO_FOCUS_STOP;
                    }
                    break;
                case CAM_EVENT_STILL_ALLPROCESSED:
                    break;
                case CAM_EVENT_FATALERROR:
                    error = CAM_GetState(self->mCEHandle, &state);
                    ASSERT_CAM_ERROR(error);
                    if (state != CAM_CAPTURESTATE_STILL) {
                        TRACE_E("%s:Received fatal event, not in CAM_CAPTURESTATE_STILL mode, ignore it",__FUNCTION__);
                        break;
                    }
                    else {
                        if (CAM_STILLSUBMODE_BURST != self->getStillSubMode(self->mSetting)) {
                            TRACE_E("%s:Received fatal event, not in CAM_STILLSUBMODE_BURST mode, ignore it",__FUNCTION__);
                            break;
                        }
                    }
                    TRACE_E("%s:Received fatal event, send raw image notify & empty jpeg, and then return to preview",__FUNCTION__);

                    if (self->mMsg->msgTypeEnabled(CAMERA_MSG_RAW_IMAGE))
                    {
                        sp<MsgData> msg = new MsgData;
                        MsgData* ptr    = msg.get();
                        ptr->msgType    = CAMERA_MSG_RAW_IMAGE;
                        ptr->index      = 0;
                        ptr->data       = self->mFailBuf->get_cammem_t();
                        self->mMsg->postMsg(msg);
                    }
                    else if (self->mMsg->msgTypeEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY))
                    {
                        sp<MsgData> msg = new MsgData;
                        MsgData* ptr    = msg.get();
                        ptr->msgType    = CAMERA_MSG_RAW_IMAGE_NOTIFY;
                        ptr->ext1       = 0;
                        ptr->ext2       = 0;
                        self->mMsg->postMsg(msg);
                    }

                    {
                        sp<MsgData> msg = new MsgData;
                        MsgData* ptr    = msg.get();
                        ptr->msgType    = CAMERA_MSG_COMPRESSED_IMAGE;
                        ptr->index      = 0;
                        ptr->flag       = MsgData::POST_FRONT;
                        ptr->data       = self->mFailBuf->get_cammem_t();
                        self->mMsg->postMsg(msg);
                    }

                    self->stopCapture();
                    self->unregisterStillBuffers();
                    break;
                case CAM_EVENT_FACE_UPDATE:
                    TRACE_V("%s:CAM_EVENT_FACE_UPDATE",__FUNCTION__);
                    if (self->bFaceDetectedEnabled == false) {
                        TRACE_V("Face has been disabled");
                    }
                    else {
                        if (self->mMsg->msgTypeEnabled(CAMERA_MSG_PREVIEW_METADATA))
                        {
                            CAM_FaceResult* pFaceResult;

                            pFaceResult = (CAM_FaceResult*)param;
                            if (!(self->parseFaceInfo(pFaceResult)))
                            {
                                return;
                            }
                            static bool hasFaceinfo = true;
                            if (pFaceResult->iFaceCnt > 0)
                            {
                                hasFaceinfo = true;
                            }

                            if(true == hasFaceinfo)
                            {
                                camera_memory_t* face_detected = self->mGetCameraMemory(-1, 1, 1, NULL);
                                {
                                    sp<MsgData> msg = new MsgData;
                                    MsgData* ptr    = msg.get();
                                    ptr->msgType    = CAMERA_MSG_PREVIEW_METADATA;
                                    ptr->data       = face_detected;
                                    ptr->index      = 0;
                                    ptr->metadata   = &self->mFaceData;
                                    ptr->setListener(msgDone_Cb,face_detected);
                                    self->mMsg->postMsg(msg);

                                    //only focus for the first face, so set focus ROI when the first face rect change a lot
                                    if (self->mFaceChanged[0])
                                        self->setFocusMeteringROIManual(pFaceResult);
                                }
                                TRACE_V("%s:Detected facenum = %d",__FUNCTION__,
                                        self->mFaceData.number_of_faces);
                            }
                            if(pFaceResult->iFaceCnt == 0)
                            {
                                hasFaceinfo = false;
                            }
                        }
                    }
                    break;
                case CAM_EVENT_BESTSHOT_INDEX:
                    TRACE_V("%s:CAM_EVENT_BESTSHOT_INDEX",__FUNCTION__);
                    pBestIndex = (int*)param;
                    self->parseBestShot(*pBestIndex);
                    break;
                default:
                    break;
            }
        }
    }


}; // namespace android
