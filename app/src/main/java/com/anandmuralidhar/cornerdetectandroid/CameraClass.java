/*
 *    Copyright 2016 Anand Muralidhar
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package com.anandmuralidhar.cornerdetectandroid;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import java.io.IOException;
import java.util.List;

public class CameraClass{

    // Camera parameters
    private static int cameraIndex = Camera.CameraInfo.CAMERA_FACING_BACK;
    private int mPreviewWidth = 1280;
    private int mPreviewHeight = 720;

    private static Camera mCamera=null;
    private SurfaceTexture testSurfaceTexture;
    private byte mPreviewBuffer[] = null;
    private float cameraFOVLandscape=0, cameraFOVPortrait=0;
    private boolean isResolutionSupported = false;
    private static CameraHandlerThread mCameraThread = null;

    private native void sendCamImageToNative(byte [] data, int mPreviewHeight, int mPreviewWidth);

    public CameraClass() {

        InitializeCamera();

    }

    void InitializeCamera() {
        
        if (mCamera != null) {
            return;
        }
        
        try {

            if (mCameraThread == null) {
                mCameraThread = new CameraHandlerThread();
            }

            synchronized (mCameraThread) {
                mCameraThread.OpenCamera();
            }

        } catch (Exception e) {
        
            Log.e("CameraClass", "Unable to open camera!");
            System.err.println(e.getMessage());
            return;
        
        }
        Log.d("CameraClass", "Got a camera instance");

        SetCameraParams();
        SetCameraCallback();
    }

    /**
     * Camera is opened on separate thread so that UI thread is not stalled in its callbacks
     */
    private static class CameraHandlerThread extends HandlerThread {
        Handler mHandler = null;

        CameraHandlerThread() {
            super("CameraHandlerThread");
            start();
            mHandler = new Handler(getLooper());
        }

        synchronized void NotifyCameraOpened() {
            notify();
        }

        void OpenCamera() {
            mHandler.post(new Runnable() {
                
                @Override
                public void run() {
                    try {
                        mCamera = Camera.open(cameraIndex);
                    } catch (RuntimeException e) {
                        Log.e("CameraClass", "Camera failed to open: " + e.getLocalizedMessage());
                    }
                    NotifyCameraOpened();
                }
            });

            // wait to be notified once camera is opened since we setup its params next
            try {
                wait();
            }
            catch (InterruptedException e) {
                Log.e("CameraClass", "wait was interrupted");
            }
        }
    }

    /**
     * Camera callbacks will occur on same thread in which open() was called
     */
    private void SetCameraCallback() {

        //Setup callback with buffer
        if (mPreviewBuffer == null) {
            mPreviewBuffer = new byte[(int) (mPreviewWidth * mPreviewHeight * 1.5)];
        }
        mCamera.addCallbackBuffer(mPreviewBuffer);
        mCamera.setPreviewCallbackWithBuffer(MyPreviewCallback);
        try {
            testSurfaceTexture = new SurfaceTexture(10);
            mCamera.setPreviewTexture(testSurfaceTexture);
        } catch (IOException e1) {
            Log.e("CameraClass",e1.getMessage());
        }
    }

    /**
     * Only setup the preview resolution for callbacks
     */
    private void SetCameraParams() {

        Camera.Parameters param;
        param = mCamera.getParameters();

        //Check if preview supports 720p resolution
        List<Camera.Size> resolution_list = param.getSupportedPreviewSizes();
        for (Camera.Size resolution : resolution_list) {
            if (resolution.width == mPreviewWidth && resolution.height == mPreviewHeight) {
                isResolutionSupported = true;
                break;
            }
        }
        if (!isResolutionSupported) {
            Log.d("CameraClass", "Specified resolution not supported by camera!");
            return;
        }

        param.setPreviewSize(mPreviewWidth, mPreviewHeight);
        Log.d("CameraClass", "Setting preview resolution to " + param.getPreviewSize().width + "x" 
                + param.getPreviewSize().height);

        SaveCameraFOV(param);

        mCamera.setParameters(param);
    }


    private void SaveCameraFOV(Camera.Parameters param) {

        float fovXDeg = param.getHorizontalViewAngle();

        if (fovXDeg <= 0. || fovXDeg >= 180.) {
            cameraFOVPortrait = 45;
            Log.d("CameraClass", "InValid FOV Detected! Setting default FOV value");
        }

        cameraFOVPortrait = fovXDeg;//This value is being returned correctly on Nexus 5, MotoG2
        float cameraAspectRatio = (float)mPreviewWidth/mPreviewHeight;
        cameraFOVLandscape = (float)(2*Math.atan(Math.tan((double)Math.toRadians(
                (double)cameraFOVPortrait)/2) / cameraAspectRatio));
        cameraFOVLandscape = (float)Math.toDegrees(cameraFOVLandscape);

        Log.d("CameraClass", "FOV for Landscape " + cameraFOVLandscape);
    }

    public void StopCamera() {

        // stop preview and release camera
        if (mCamera != null) {
            Log.d("CameraClass", "Stopping and releasing Camera");
            mCamera.stopPreview();
            mCamera.setPreviewCallback(null);
            mCamera.release();
            mCamera = null;
        }

        // stop camera handler thread
        if(mCameraThread != null) {
            mCameraThread.quit();
            mCameraThread = null;
        }
    }


    public void StartCameraPreview() {

        try {
            Log.d("CameraClass","Starting camera preview");
            mCamera.startPreview();
        } catch (Exception e) {
            Log.d("CameraClass","Unable to start camera preview! " + e.getLocalizedMessage());
        }
    }


    private Camera.PreviewCallback MyPreviewCallback = new Camera.PreviewCallback() {

        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {

//            Log.d("CameraClass","MyPreviewCallback");

            sendCamImageToNative(data, mPreviewHeight, mPreviewWidth);

            //Return buffer to camera
            camera.addCallbackBuffer(data);
        }
    };

}
