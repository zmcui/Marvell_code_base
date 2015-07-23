/*
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

/**
*
* This file maps the Camera Hardware Interface to Camera Instance Implementation.
*
*/

#define LOG_TAG "CameraModule"

#include <utils/threads.h>
#include "CameraHardwareSmt.h"
#include "CameraHardwareDxO.h"
#include "cam_log_mrvl.h"

static android::CameraHardwareBase* gCameraHals[MAX_CAMERAS_SUPPORTED];
....

static struct hw_module_methods_t camera_module_methods = {
        open: camera_device_open
};

camera_module_t HAL_MODULE_INFO_SYM = {
    common: {
         tag: HARDWARE_MODULE_TAG,
         version_major: 1,
         version_minor: 0,
         id: CAMERA_HARDWARE_MODULE_ID,
         name: "MRVL CameraHal Module",
         author: "MRVL",
         methods: &camera_module_methods,
         dso: NULL, /* remove compilation warnings */
         reserved: {0}, /* remove compilation warnings */
    },
    get_number_of_cameras: camera_get_number_of_cameras,
    get_camera_info: camera_get_camera_info,
    set_callbacks: NULL,
    get_vendor_tag_ops: NULL,
    open_legacy: NULL,
    reserved:{0},
};

typedef struct mrvl_camera_device {
    camera_device_t base;
    /* specific "private" data can go here (base.priv) */
    int cameraid;
} mrvl_camera_device_t;


/*******************************************************************
 * implementation of camera_device_ops functions
 *******************************************************************/
....
void camera_set_callbacks(struct camera_device * device,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void *user)
{
    mrvl_camera_device_t* mrvl_dev = NULL;

    TRACE_V("%s", __FUNCTION__);

    if(!device)
        return;

    mrvl_dev = (mrvl_camera_device_t*) device;

    gCameraHals[mrvl_dev->cameraid]->setCallbacks(notify_cb, data_cb, data_cb_timestamp, get_memory, user);
}

....
/*******************************************************************
 * implementation of camera_module functions
 *******************************************************************/

/* open device handle to one of the cameras
 *
 * assume camera service will keep singleton of each camera
 * so this function will always only be called once per camera instance
 */

int camera_device_open(const hw_module_t* module, const char* name,
                hw_device_t** device)
{
    FUNC_TAG;

    int rv = 0;
    int cameraid;
    mrvl_camera_device_t* camera_device = NULL;
    camera_device_ops_t* camera_ops = NULL;
    android::CameraHardwareBase* camera = NULL;

    char value[PROPERTY_VALUE_MAX] = {0};
    property_get(PROP_MULTIOPEN, value, "0");
    if (0 == strcmp(value, "1"))
        gSupportMultiCameraOpen = true;

    android::Mutex::Autolock lock(gCameraHalDeviceLock);


    if (name != NULL) {
        cameraid = atoi(name);

        if(gCurrentOpenCameras == 1 && !gSupportMultiCameraOpen)
        {
            TRACE_E("already open one camera, not support open multi camera!");
            rv = -EUSERS;
            goto fail;
        }

        if(cameraid > gNum_Cameras || cameraid < 0)
        {
            TRACE_E("camera service provided cameraid out of bounds, "
                    "cameraid = %d, num supported = %d",
                    cameraid, gNum_Cameras);
            rv = -EINVAL;
            goto fail;
        }

        camera_device = (mrvl_camera_device_t*)malloc(sizeof(*camera_device));
        if(!camera_device)
        {
            TRACE_E("camera_device allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        camera_ops = (camera_device_ops_t*)malloc(sizeof(*camera_ops));
        if(!camera_ops)
        {
            TRACE_E("camera_ops allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        memset(camera_device, 0, sizeof(*camera_device));
        memset(camera_ops, 0, sizeof(*camera_ops));

        camera_device->base.common.tag = HARDWARE_DEVICE_TAG;
        camera_device->base.common.version = 0;
        camera_device->base.common.module = (hw_module_t *)(module);
        camera_device->base.common.close = camera_device_close;
        camera_device->base.ops = camera_ops;

        camera_ops->set_preview_window = camera_set_preview_window;
        camera_ops->set_callbacks = camera_set_callbacks;
        camera_ops->enable_msg_type = camera_enable_msg_type;
        camera_ops->disable_msg_type = camera_disable_msg_type;
        camera_ops->msg_type_enabled = camera_msg_type_enabled;
        camera_ops->start_preview = camera_start_preview;
        camera_ops->stop_preview = camera_stop_preview;
        camera_ops->preview_enabled = camera_preview_enabled;
        camera_ops->store_meta_data_in_buffers = camera_store_meta_data_in_buffers;
        camera_ops->start_recording = camera_start_recording;
        camera_ops->stop_recording = camera_stop_recording;
        camera_ops->recording_enabled = camera_recording_enabled;
        camera_ops->release_recording_frame = camera_release_recording_frame;
        camera_ops->auto_focus = camera_auto_focus;
        camera_ops->cancel_auto_focus = camera_cancel_auto_focus;
        camera_ops->take_picture = camera_take_picture;
        camera_ops->cancel_picture = camera_cancel_picture;
        camera_ops->set_parameters = camera_set_parameters;
        camera_ops->get_parameters = camera_get_parameters;
        camera_ops->put_parameters = camera_put_parameters;
        camera_ops->send_command = camera_send_command;
        camera_ops->release = camera_release;
        camera_ops->dump = camera_dump;

        *device = &camera_device->base.common;

        // -------- specific stuff --------

        if(gCameraHals[cameraid] != NULL)
        {
            TRACE_E("gCameraHals[cameraid] != NULL, previous camera %d not free!", cameraid);
            delete gCameraHals[cameraid];
        }

        TRACE_D("cameraid = %d, gCameraInfo[cameraid].ports = %d", cameraid, gCameraInfo[cameraid].ports);
        if (gCameraInfo[cameraid].ports == SINGLE_PORTCAM)
        {
            camera = new android::CameraHardwareSmt(cameraid);
        }
        else if (gCameraInfo[cameraid].ports >= MULTI_PORTCAM)
        {
            camera = new android::CameraHardwareDxO(cameraid);
        }
        else
        {
           TRACE_E("Camera has too many / too less ports, not supported!");
        }
        gCameraHals[cameraid] = camera;
        camera_device->cameraid = cameraid;
    }

    gCurrentOpenCameras++;
    return rv;

fail:
    if(camera_device) {
        free(camera_device);
        camera_device = NULL;
    }
    if(camera_ops) {
        free(camera_ops);
        camera_ops = NULL;
    }
    if(camera) {
        camera->release();
        camera = NULL;
    }
    *device = NULL;
    return rv;
}

int camera_get_number_of_cameras(void)
{
    FUNC_TAG;

    int num_cameras = android::HAL_getNumberOfCameras();
    gNum_Cameras = num_cameras;
    return num_cameras;
}

int camera_get_camera_info(int camera_id, struct camera_info *info)
{
    FUNC_TAG;

    android::mrvl_camera_info camera_info;
    android::HAL_getCameraInfo(camera_id, &camera_info);

    gCameraInfo[camera_id] = camera_info;

    info->facing = camera_info.base.facing;
    info->orientation = camera_info.base.orientation;

    return 0;
}


