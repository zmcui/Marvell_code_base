/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.ex.camera2.portability;

import android.annotation.TargetApi;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.SceneDetectionListener;
import android.hardware.Camera.IspInfoListener;
import android.hardware.Camera.OnZoomChangeListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.ShutterCallback;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.SurfaceHolder;

import com.android.ex.camera2.portability.debug.Log;

import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.StringTokenizer;

/**
 * A class to implement {@link CameraAgent} of the Android camera framework.
 */
class AndroidCameraAgentImpl extends CameraAgent {
    private static final Log.Tag TAG = new Log.Tag("AndCamAgntImp");

/*todo*/
        /**
         * This method does not deal with the API level check.  Everyone should
         * check first for supported operations before sending message to this handler.
         */
        @Override
        public void handleMessage(final Message msg) {
            super.handleMessage(msg);

            if (getCameraState().isInvalid()) {
                Log.v(TAG, "Skip handleMessage - action = '" + CameraActions.stringify(msg.what) + "'");
                return;
            }
            Log.v(TAG, "handleMessage - action = '" + CameraActions.stringify(msg.what) + "'");

            int cameraAction = msg.what;
            try {
                switch (cameraAction) {
                    case CameraActions.OPEN_CAMERA: {
                        final CameraOpenCallback openCallback = (CameraOpenCallback) msg.obj;
                        final int cameraId = msg.arg1;
                        if (mCameraState.getState() != AndroidCameraStateHolder.CAMERA_UNOPENED) {
                            openCallback.onDeviceOpenedAlready(cameraId, generateHistoryString(cameraId));
                            break;
                        }

                        Log.i(TAG, "Opening camera " + cameraId + " with camera1 API");
                        //mCamera = android.hardware.Camera.open(cameraId);
                        mCamera = Camera.openUninitialized();
                        int error = mCamera.cameraInitUnspecified(cameraId);
                        if (mCamera != null) {
                            mCameraId = cameraId;
                            mParameterCache = new ParametersCache(mCamera);

                            mCharacteristics =
                                    AndroidCameraDeviceInfo.create().getCharacteristics(cameraId);
                            mCapabilities = new AndroidCameraCapabilities(
                                    mParameterCache.getBlocking());

                            mCamera.setErrorCallback(this);

                            mCameraState.setState(AndroidCameraStateHolder.CAMERA_IDLE);
                            if (openCallback != null) {
                                CameraProxy cameraProxy = new AndroidCameraProxyImpl(
                                        mAgent, cameraId, mCamera, mCharacteristics, mCapabilities);
                                openCallback.onCameraOpened(cameraProxy);
                            }
                        } else {
                            if (openCallback != null) {
                                if (error == EBUSY) {
                                    openCallback.onDeviceUsedByOtherApplications(cameraId, generateHistoryString(cameraId));
                                } else {
                                    openCallback.onDeviceOpenFailure(cameraId, generateHistoryString(cameraId));
                                }
                            }
                        }
                        break;
                    }

                    case CameraActions.RELEASE: {
                        if (mCamera != null) {
                            mCamera.release();
                            mCameraState.setState(AndroidCameraStateHolder.CAMERA_UNOPENED);
                            mCamera = null;
                            mCameraId = -1;
                        } else {
                            Log.w(TAG, "Releasing camera without any camera opened.");
                        }
                        break;
                    }

                    case CameraActions.RECONNECT: {
                        final CameraOpenCallbackForward cbForward =
                                (CameraOpenCallbackForward) msg.obj;
                        final int cameraId = msg.arg1;
                        try {
                            mCamera.reconnect();
                        } catch (IOException ex) {
                            if (cbForward != null) {
                                cbForward.onReconnectionFailure(mAgent, generateHistoryString(mCameraId));
                            }
                            break;
                        }

                        mCameraState.setState(AndroidCameraStateHolder.CAMERA_IDLE);
                        if (cbForward != null) {
                            cbForward.onCameraOpened(
                                    new AndroidCameraProxyImpl(AndroidCameraAgentImpl.this,
                                            cameraId, mCamera, mCharacteristics, mCapabilities));
                        }
                        break;
                    }

                    case CameraActions.UNLOCK: {
                        mCamera.unlock();
                        mCameraState.setState(AndroidCameraStateHolder.CAMERA_UNLOCKED);
                        break;
                    }

                    case CameraActions.LOCK: {
                        mCamera.lock();
                        mCameraState.setState(AndroidCameraStateHolder.CAMERA_IDLE);
                        break;
                    }

                    // TODO: Lock the CameraSettings object's sizes
                    case CameraActions.SET_PREVIEW_TEXTURE_ASYNC: {
                        setPreviewTexture(msg.obj);
                        break;
                    }

                    case CameraActions.SET_PREVIEW_DISPLAY_ASYNC: {
                        try {
                            mCamera.setPreviewDisplay((SurfaceHolder) msg.obj);
                        } catch (IOException e) {
                            throw new RuntimeException(e);
                        }
                        break;
                    }

                    case CameraActions.START_PREVIEW_ASYNC: {
                        final CameraStartPreviewCallbackForward cbForward =
                            (CameraStartPreviewCallbackForward) msg.obj;
                        mCamera.startPreview();
                        if (cbForward != null) {
                            cbForward.onPreviewStarted();
                        }
                        break;
                    }

                    // TODO: Unlock the CameraSettings object's sizes
                    case CameraActions.STOP_PREVIEW: {
                        mCamera.stopPreview();
                        break;
                    }

                    case CameraActions.SET_PREVIEW_CALLBACK_WITH_BUFFER: {
                        mCamera.setPreviewCallbackWithBuffer((PreviewCallback) msg.obj);
                        break;
                    }

                    case CameraActions.SET_ONE_SHOT_PREVIEW_CALLBACK: {
                        mCamera.setOneShotPreviewCallback((PreviewCallback) msg.obj);
                        break;
                    }

                    case CameraActions.ADD_CALLBACK_BUFFER: {
                        mCamera.addCallbackBuffer((byte[]) msg.obj);
                        break;
                    }

                    case CameraActions.AUTO_FOCUS: {
                        if (mCancelAfPending > 0) {
                            Log.v(TAG, "handleMessage - Ignored AUTO_FOCUS because there was "
                                    + mCancelAfPending + " pending CANCEL_AUTO_FOCUS messages");
                            break; // ignore AF because a CANCEL_AF is queued after this
                        }
                        mCameraState.setState(AndroidCameraStateHolder.CAMERA_FOCUSING);
                        mCamera.autoFocus((AutoFocusCallback) msg.obj);
                        break;
                    }

                    case CameraActions.CANCEL_AUTO_FOCUS: {
                        // Ignore all AFs that were already queued until we see
                        // a CANCEL_AUTO_FOCUS_FINISH
                        mCancelAfPending++;
                        mCamera.cancelAutoFocus();
                        mCameraState.setState(AndroidCameraStateHolder.CAMERA_IDLE);
                        break;
                    }

                    case CameraActions.CANCEL_AUTO_FOCUS_FINISH: {
                        // Stop ignoring AUTO_FOCUS messages unless there are additional
                        // CANCEL_AUTO_FOCUSes that were added
                        mCancelAfPending--;
                        break;
                    }

                    case CameraActions.SET_AUTO_FOCUS_MOVE_CALLBACK: {
                        setAutoFocusMoveCallback(mCamera, msg.obj);
                        break;
                    }

                    case CameraActions.SET_DISPLAY_ORIENTATION: {
                        // Update preview orientation
                        mCamera.setDisplayOrientation(
                                mCharacteristics.getPreviewOrientation(msg.arg1));
                        // Only set the JPEG capture orientation if requested to do so; otherwise,
                        // capture in the sensor's physical orientation. (e.g., JPEG rotation is
                        // necessary in auto-rotate mode.
                        Parameters parameters = mParameterCache.getBlocking();
                        parameters.setRotation(
                                msg.arg2 > 0 ? mCharacteristics.getJpegOrientation(msg.arg1) : 0);
                        mCamera.setParameters(parameters);
                        mParameterCache.invalidate();
                        break;
                    }

                    case CameraActions.SET_JPEG_ORIENTATION: {
                        Parameters parameters = mParameterCache.getBlocking();
                        parameters.setRotation(msg.arg1);
                        mCamera.setParameters(parameters);
                        mParameterCache.invalidate();
                        break;
                    }

                    case CameraActions.SET_ZOOM_CHANGE_LISTENER: {
                        mCamera.setZoomChangeListener((OnZoomChangeListener) msg.obj);
                        break;
                    }

                    case CameraActions.SET_FACE_DETECTION_LISTENER: {
                        setFaceDetectionListener((FaceDetectionListener) msg.obj);
                        break;
                    }

                    case CameraActions.START_FACE_DETECTION: {
                        startFaceDetection();
                        break;
                    }

                    case CameraActions.STOP_FACE_DETECTION: {
                        stopFaceDetection();
                        break;
                    }

                    case CameraActions.SET_SCENE_DETECTION_LISTENER:
                        mCamera.setSceneDetectionListener(
                        (SceneDetectionListener) msg.obj);
                    return;

                    case CameraActions.SET_ISP_INFO_LISTENER:
                        mCamera.setIspInfoListener(
                        (IspInfoListener) msg.obj);
                    return;

                    case CameraActions.APPLY_SETTINGS: {
                        Parameters parameters = mParameterCache.getBlocking();
                        CameraSettings settings = (CameraSettings) msg.obj;
                        applySettingsToParameters(settings, parameters);
                        mCamera.setParameters(parameters);
                        mParameterCache.invalidate();
                        break;
                    }

                    case CameraActions.SET_PARAMETERS: {
                        Parameters parameters = mParameterCache.getBlocking();
                        parameters.unflatten((String) msg.obj);
                        mCamera.setParameters(parameters);
                        mParameterCache.invalidate();
                        break;
                    }

                    case CameraActions.GET_PARAMETERS: {
                        Parameters[] parametersHolder = (Parameters[]) msg.obj;
                        Parameters parameters = mParameterCache.getBlocking();
                        parametersHolder[0] = parameters;
                        break;
                    }

                    case CameraActions.SET_PREVIEW_CALLBACK: {
                        mCamera.setPreviewCallback((PreviewCallback) msg.obj);
                        break;
                    }

                    case CameraActions.ENABLE_SHUTTER_SOUND: {
                        enableShutterSound((msg.arg1 == 1) ? true : false);
                        break;
                    }

                    case CameraActions.REFRESH_PARAMETERS: {
                        mParameterCache.invalidate();;
                        break;
                    }

                    case CameraActions.CAPTURE_PHOTO: {
                        mCameraState.setState(AndroidCameraStateHolder.CAMERA_CAPTURING);
                        CaptureCallbacks captureCallbacks = (CaptureCallbacks) msg.obj;
                        mCamera.takePicture(
                                captureCallbacks.mShutter,
                                captureCallbacks.mRaw,
                                captureCallbacks.mPostView,
                                captureCallbacks.mJpeg);
                        break;
                    }

                    default: {
                        Log.e(TAG, "Invalid CameraProxy message=" + msg.what);
                    }
                }
            } catch (final RuntimeException ex) {
                int cameraState = mCameraState.getState();
                String errorContext = "CameraAction[" + CameraActions.stringify(cameraAction) +
                        "] at CameraState[" + cameraState + "]";
                Log.e(TAG, "RuntimeException during " + errorContext, ex);

                // Be conservative by invalidating both CameraAgent and CameraProxy objects.
                mCameraState.invalidate();

                if (mCamera != null) {
                    Log.i(TAG, "Release camera since mCamera is not null.");
                    try {
                        mCamera.release();
                    } catch (Exception e) {
                        Log.e(TAG, "Fail when calling Camera.release().", e);
                    } finally {
                        mCamera = null;
                    }
                }

                // Invoke error callback.
                if (msg.what == CameraActions.OPEN_CAMERA && mCamera == null) {
                    final int cameraId = msg.arg1;
                    if (msg.obj != null) {
                        ((CameraOpenCallback) msg.obj).onDeviceOpenFailure(
                                msg.arg1, generateHistoryString(cameraId));
                    }
                } else {
                    CameraExceptionHandler exceptionHandler = mAgent.getCameraExceptionHandler();
                    exceptionHandler.onCameraException(
                            ex, generateHistoryString(mCameraId), cameraAction, cameraState);
                }
            } finally {
                WaitDoneBundle.unblockSyncWaiters(msg);
            }
        }

/*todo*/

        @Override
        public boolean applySettings(CameraSettings settings) {
            return applySettingsHelper(settings, AndroidCameraStateHolder.CAMERA_IDLE |
                    AndroidCameraStateHolder.CAMERA_UNLOCKED);
        }

}
