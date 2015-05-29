/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.camera;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.location.Location;
import android.media.AudioManager;
import android.media.CamcorderProfile;
import android.media.CameraProfile;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.provider.MediaStore;
import android.provider.MediaStore.MediaColumns;
import android.provider.MediaStore.Video;
import android.view.KeyEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.widget.Toast;
import android.hardware.Camera;
import android.widget.ImageView;

import com.android.camera.app.AppController;
import com.android.camera.app.CameraAppUI;
import com.android.camera.app.LocationManager;
import com.android.camera.app.MediaSaver;
import com.android.camera.app.MemoryManager;
import com.android.camera.app.MemoryManager.MemoryListener;
import com.android.camera.debug.Log;
import com.android.camera.exif.ExifInterface;
import com.android.camera.hardware.HardwareSpec;
import com.android.camera.hardware.HardwareSpecImpl;
import com.android.camera.module.ModuleController;
import com.android.camera.settings.CameraSettingsActivity;
import com.android.camera.settings.Keys;
import com.android.camera.settings.SettingsManager;
import com.android.camera.settings.SettingsUtil;
import com.android.camera.ui.TouchCoordinate;
import com.android.camera.util.ApiHelper;
import com.android.camera.util.Callback;
import com.android.camera.util.CameraUtil;
import com.android.camera.util.UsageStatistics;
import com.android.camera2.R;
import com.android.ex.camera2.portability.CameraAgent;
import com.android.ex.camera2.portability.CameraAgent.CameraPictureCallback;
import com.android.ex.camera2.portability.CameraAgent.CameraProxy;
import com.android.ex.camera2.portability.CameraCapabilities;
import com.android.ex.camera2.portability.CameraDeviceInfo.Characteristics;
import com.android.ex.camera2.portability.CameraSettings;
import com.android.ex.camera2.portability.Size;
import com.google.common.logging.eventprotos;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

public class VideoModule extends CameraModule
    implements ModuleController,
    VideoController,
    MemoryListener,
    MediaRecorder.OnErrorListener,
    CameraAgent.CameraPreviewDataCallback,
    SettingsManager.OnSettingChangedListener,
    MediaRecorder.OnInfoListener, FocusOverlayManager.Listener {

    private static final String VIDEO_MODULE_STRING_ID = "VideoModule";

    private static final Log.Tag TAG = new Log.Tag(VIDEO_MODULE_STRING_ID);

/*todo*/

    private final ButtonManager.ButtonCallback mFlashCallback =
        new ButtonManager.ButtonCallback() {
            @Override
            public void onStateChanged(int state) {
                // Update flash parameters.
                enableTorchMode(true);
            }
        };


    @Override
    public CameraAppUI.BottomBarUISpec getBottomBarSpec() {
        CameraAppUI.BottomBarUISpec bottomBarSpec = new CameraAppUI.BottomBarUISpec();

        bottomBarSpec.enableCamera = true;
        bottomBarSpec.cameraCallback = mCameraCallback;
        bottomBarSpec.enableTorchFlash = true;
        bottomBarSpec.flashCallback = mFlashCallback;
        bottomBarSpec.hideHdr = true;
        bottomBarSpec.enableGridLines = true;
        bottomBarSpec.enableWhiteBalance = true;
        bottomBarSpec.wbCallback = mWhiteBalanceCallback;
        bottomBarSpec.enableSceneMode = true;
        bottomBarSpec.sceneModeSetCallback =
                new CameraAppUI.BottomBarUISpec.SceneModeSetCallback() {
                @Override
                public void setScene(String value) {
                    setSceneMode(value);
                }
        };

        if (isVideoCaptureIntent()) {
            bottomBarSpec.showCancel = true;
            bottomBarSpec.cancelCallback = mCancelCallback;
            bottomBarSpec.showDone = true;
            bottomBarSpec.doneCallback = mDoneCallback;
            bottomBarSpec.showReview = true;
            bottomBarSpec.reviewCallback = mReviewCallback;
        }

        return bottomBarSpec;
    }

/*todo*/

    /**
     * Used to update the flash mode. Video mode can turn on the flash as torch
     * mode, which we would like to turn on and off when we switching in and
     * out to the preview.
     *
     * @param enable Whether torch mode can be enabled.
     */
    private void enableTorchMode(boolean enable) {
        if (mCameraSettings.getCurrentFlashMode() == null) {
            return;
        }

        SettingsManager settingsManager = mActivity.getSettingsManager();

        CameraCapabilities.Stringifier stringifier = mCameraCapabilities.getStringifier();
        CameraCapabilities.FlashMode flashMode;
        if (enable) {
            flashMode = stringifier
                .flashModeFromString(settingsManager.getString(mAppController.getCameraScope(),
                                                               Keys.KEY_VIDEOCAMERA_FLASH_MODE));
        } else {
            flashMode = CameraCapabilities.FlashMode.OFF;
        }
        if (mCameraCapabilities.supports(flashMode)) {
            mCameraSettings.setFlashMode(flashMode);
        }
        /* TODO: Find out how to deal with the following code piece:
        else {
            flashMode = mCameraSettings.getCurrentFlashMode();
            if (flashMode == null) {
                flashMode = mActivity.getString(
                        R.string.pref_camera_flashmode_no_flash);
                mParameters.setFlashMode(flashMode);
            }
        }*/
        if (mCameraDevice != null) {
            mCameraDevice.applySettings(mCameraSettings);
        }
        mUI.updateOnScreenIndicators(mCameraSettings);
    }

