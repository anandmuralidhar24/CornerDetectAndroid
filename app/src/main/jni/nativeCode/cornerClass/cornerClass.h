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

#ifndef CORNERCLASS_H
#define CORNERCLASS_H

#include "myLogger.h"
#include "myGLFunctions.h"
#include "myGLCamera.h"
#include "assimpLoader.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <mutex>
#include <backTexture.h>

class CornerClass {
public:
    CornerClass();
    ~CornerClass();
    void    PerformGLInits();
    void    Render();
    void    SetViewport(int width, int height);
    void    ProcessCameraImage(cv::Mat cameraRGBImage, int mPreview_width, int mPreview_height);
    void    SetCameraPreviewDims(int cameraPreviewWidth, int cameraPreviewHeight);

private:
    void    DetectAndHighlightCorners();

    bool    initsDone;
    int     screenWidth, screenHeight;
    cv::Mat cameraImageForBack;
    BackTexture * back;
    bool    newCameraImage;
    int     cameraPreviewWidth, cameraPreviewHeight;

    std::mutex cameraMutex;
    cv::Ptr<cv::Feature2D> cornerDetector;
    std::vector<cv::KeyPoint> keyPoints;
};

#endif //CORNERCLASS_H
