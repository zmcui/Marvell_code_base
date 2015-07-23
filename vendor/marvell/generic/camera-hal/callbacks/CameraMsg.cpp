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
  ....
    void CameraMsg::setCallbacks(
            camera_notify_callback notify_cb,
            camera_data_callback data_cb,
            camera_data_timestamp_callback data_cb_timestamp,
            void *user)
    {
        FUNC_TAG;
        Mutex::Autolock lock(mMsgLock);

        mNotifyCb_master = notify_cb;
        mDataCb_master = data_cb;
        mDataCbTimestamp_master = data_cb_timestamp;
        mCallbackCookie_master = user;
    }

    status_t CameraMsg::postMsg(sp<MsgData> msg)
    {
        _showMsgInfo(msg);

#ifdef __DEBUG_WITHOUT_MSG_THREAD__
        return _postMsg(msg);
#endif

        int32_t msgType = msg.get()->msgType;
        if( !msgTypeEnabled( msgType )){
            TRACE_V("%s: Drop disabled msg: 0x%x", __FUNCTION__, msgType);
            return UNKNOWN_ERROR;
        }


        int flag = msg.get()->flag;
        if(flag == MsgData::POST_IMMEDIATELY){
            return _postMsg(msg);
        }

        Mutex::Autolock lock(mMsgLock);
        size_t length = mMsgReceived.size();
        if(length>10){
            TRACE_E("Too many pending msg...");
        }
        if(flag == MsgData::POST_FRONT){
            mMsgReceived.push_front(msg);
        }
        else{
            mMsgReceived.push_back(msg);
        }
        mMsgCond.signal();
        return NO_ERROR;
    }

    //call this with mMsgLock unlocked!
    status_t CameraMsg::_postMsg(sp<MsgData> msg)
    {
        TRACE_V("%s: post Msg out:",__FUNCTION__);
        _showMsgInfo(msg);
        const MsgData* ptr=msg.get();
        int32_t msgType = ptr->msgType;
        if( !msgTypeEnabled( msgType )){
            TRACE_V("%s: Drop disabled msg: 0x%x", __FUNCTION__, msgType);
            return NO_ERROR;
        }

        //extract the data parcel
        int32_t     ext1;
        int32_t     ext2;
        void*       user;

        nsecs_t     timestamp;
        camera_memory_t* data;
        unsigned int index;
        camera_frame_metadata_t* metadata;
        uint32_t    flag;

        switch(msgType){
            case CAMERA_MSG_PREVIEW_FRAME:
            case CAMERA_MSG_POSTVIEW_FRAME:
            case CAMERA_MSG_RAW_IMAGE:
            case CAMERA_MSG_COMPRESSED_IMAGE:
            case CAMERA_MSG_PREVIEW_METADATA:
            case CAMERA_MSG_SCENE_UPDATE:
            case CAMERA_MSG_ISP_INFO_FOR_CT:
                data = static_cast<camera_memory_t*>(ptr->data);
                metadata = static_cast<camera_frame_metadata_t *>(ptr->metadata);
                index = ptr->index;
                user = mCallbackCookie_master;
                if(mDataCb_master){
                    mDataCb_master(msgType, data, index, metadata, user);
                    TRACE_V("DataCB Done");
                }
                break;
            case CAMERA_MSG_VIDEO_FRAME:
                data = static_cast<camera_memory_t*>(ptr->data);
                index = ptr->index;
                timestamp = ptr->timestamp;
                user = mCallbackCookie_master;
                if(mDataCbTimestamp_master){
                    mDataCbTimestamp_master(timestamp, msgType, data, index, user);
                    TRACE_V("DataCbTimestamp Done");
                }
                break;
            default:
                ext1 = ptr->ext1;
                ext2 = ptr->ext2;
                user = mCallbackCookie_master;
                if(mNotifyCb_master){
                    mNotifyCb_master(msgType, ext1, ext2, user);
                    TRACE_V("NotifyCb Done");
                }
                break;
        }
        TRACE_V("msg done");
        return NO_ERROR;
    }
}
