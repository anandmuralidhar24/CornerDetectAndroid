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

#include "myShader.h"
#include "cornerClass.h"


#include "assimp/Importer.hpp"
#include <opencv2/opencv.hpp>
#include <myJNIHelper.h>

/**
 * Class constructor
 */
CornerClass::CornerClass() {

    MyLOGD("CornerClass::CornerClass");
    initsDone = false;

}

CornerClass::~CornerClass() {

    MyLOGD("CornerClass::CornerClass");
}

/**
 * Perform inits and load the triangle's vertices/colors to GLES
 */
void CornerClass::PerformGLInits() {

    MyLOGD("CornerClass::PerformGLInits");

    MyGLInits();
    back = new BackTexture(1280, 720);
    newCameraImage = false;
    cornerDetector = cv::ORB::create();
    CheckGLError("CornerClass::PerformGLInits");
    initsDone = true;
}


/**
 * Render to the display
 */
void CornerClass::Render() {

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cameraMutex.try_lock();
    if(newCameraImage) {
        back->LoadBackImg(cameraImageForBack);
    }
    newCameraImage = false;
    cameraMutex.unlock();
    back->Render();
    CheckGLError("CornerClass::Render");

}

/**
 * set the viewport, function is also called when user changes device orientation
 */
void CornerClass::SetViewport(int width, int height) {

    MyLOGD("viewport width x height: %d x %d ", width, height);
    screenHeight = height;
    screenWidth = width;
    glViewport(0, 0, width, height);
    CheckGLError("Cube::SetViewport");

}

void CornerClass::ProcessCameraImage(uchar *data, int mPreview_width, int mPreview_height) {

    if(mPreview_height != back->GetHeight() || mPreview_width != back->GetWidth()) {
        MyLOGE("Preview image dims do not match back texture size");
    }
    //YUV -> RGBA conversion
    cv::Mat _yuv(mPreview_height * 1.5, mPreview_width, CV_8UC1, data);
    cameraMutex.lock();
    cv::cvtColor(_yuv, cameraImageForBack, CV_YUV2RGB_NV21, 3);
    cv::flip(cameraImageForBack, cameraImageForBack, 0);

    cornerDetector->detect(cameraImageForBack, keyPoints);
    for(int i=0;i<keyPoints.size();i++){
            cv::circle(cameraImageForBack, keyPoints[i].pt, 5, cv::Scalar(0,255,0));
    }

    newCameraImage = true;
    cameraMutex.unlock();
//    cv::imwrite("/mnt/sdcard/Tryamble/cameraImageForBack.jpg", gCornerObject->cameraImageForBack);

}