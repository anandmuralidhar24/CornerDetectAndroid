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

#include "backTexture.h"
#include "myShader.h"

BackTexture::BackTexture(int width, int height) {

    shaderProgramID = LoadShaders("back.vsh", "back.fsh");
    textureSamplerLocation = GetUniformLocation(shaderProgramID, "textureSampler");
    vertexAttribute = GetAttributeLocation(shaderProgramID, "vertexPosition");

    const GLfloat vertices[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f };
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    this->width = width;
    this->height = height;
    glGenTextures(1, &textureNameInGL);
    glBindTexture(GL_TEXTURE_2D, textureNameInGL);
    // specify linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // create an empty texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    CheckGLError("BackTexture::BackTexture");
}

void BackTexture::Render() {

    // loading background texture
    glUseProgram(shaderProgramID);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(textureSamplerLocation, 0);
    glBindTexture( GL_TEXTURE_2D, textureNameInGL);

    glEnableVertexAttribArray(vertexAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw the quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CheckGLError("BackTexture::Render");
}

bool BackTexture::LoadBackImg(cv::Mat backImage) {

    if(backImage.rows!=height || backImage.cols!=width) {
        MyLOGE("Image size does not match texture dims");
        return false;
    }

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, textureNameInGL);
    // specify linear filtering
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // load the OpenCV Mat into GLES
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
                    backImage.data);
    CheckGLError("BackTexture::LoadBackImg");
    return true;
}

