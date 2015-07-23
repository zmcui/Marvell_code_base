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

void CameraHardwareBase::setCallbacks(
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory, void *user)
{
    FUNC_TAG_E;
    Mutex::Autolock lock(mLock);

    mGetCameraMemory = get_memory;
    if(mMsg == NULL)
    {
        TRACE_E("Invalid mMsg");
        return;
    }

    mMsg->setCallbacks(notify_cb, data_cb, data_cb_timestamp, user);
    mCamPtr->setCallbacks(mMsg, get_memory, user);
    setFrameHandler();
    BufferHolder::setAllocator(get_memory);
    FUNC_TAG_X;
}

