
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_CAMERA_HARDWARE_INTERFACE_H
#define ANDROID_HARDWARE_CAMERA_HARDWARE_INTERFACE_H

#include <binder/IMemory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/RefBase.h>
#include <ui/GraphicBuffer.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <system/window.h>
#include <hardware/camera.h>

namespace android {

typedef void (*notify_callback)(int32_t msgType,
                            int32_t ext1,
                            int32_t ext2,
                            void* user);

typedef void (*data_callback)(int32_t msgType,
                            const sp<IMemory> &dataPtr,
                            camera_frame_metadata_t *metadata,
                            void* user);

typedef void (*data_callback_timestamp)(nsecs_t timestamp,
                            int32_t msgType,
                            const sp<IMemory> &dataPtr,
                            void *user);

/**
 * CameraHardwareInterface.h defines the interface to the
 * camera hardware abstraction layer, used for setting and getting
 * parameters, live previewing, and taking pictures. It is used for
 * HAL devices with version CAMERA_DEVICE_API_VERSION_1_0 only.
 *
 * It is a referenced counted interface with RefBase as its base class.
 * CameraService calls openCameraHardware() to retrieve a strong pointer to the
 * instance of this interface and may be called multiple times. The
 * following steps describe a typical sequence:
 *
 *   -# After CameraService calls openCameraHardware(), getParameters() and
 *      setParameters() are used to initialize the camera instance.
 *   -# startPreview() is called.
 *
 * Prior to taking a picture, CameraService often calls autofocus(). When auto
 * focusing has completed, the camera instance sends a CAMERA_MSG_FOCUS notification,
 * which informs the application whether focusing was successful. The camera instance
 * only sends this message once and it is up  to the application to call autoFocus()
 * again if refocusing is desired.
 *
 * CameraService calls takePicture() to request the camera instance take a
 * picture. At this point, if a shutter, postview, raw, and/or compressed
 * callback is desired, the corresponding message must be enabled. Any memory
 * provided in a data callback must be copied if it's needed after returning.
 */

class CameraHardwareInterface : public virtual RefBase {
public:
    CameraHardwareInterface(const char *name)
    {
        mDevice = 0;
        mName = name;
    }

    ~CameraHardwareInterface()
    {
        ALOGI("Destroying camera %s", mName.string());
        if(mDevice) {
            int rc = mDevice->common.close(&mDevice->common);
            if (rc != OK)
                ALOGE("Could not close camera %s: %d", mName.string(), rc);
        }
    }

....

    /** Set the notification and data callbacks */
    void setCallbacks(notify_callback notify_cb,
                      data_callback data_cb,
                      data_callback_timestamp data_cb_timestamp,
                      void* user)
    {
        mNotifyCb = notify_cb;
        mDataCb = data_cb;
        mDataCbTimestamp = data_cb_timestamp;
        mCbUser = user;

        ALOGV("%s(%s)", __FUNCTION__, mName.string());

        if (mDevice->ops->set_callbacks) {
            mDevice->ops->set_callbacks(mDevice,
                                   __notify_cb,
                                   __data_cb,
                                   __data_cb_timestamp,
                                   __get_memory,
                                   this);
        }
    }

....
}
