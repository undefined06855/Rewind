#include "CCRenderTexture.hpp"

// genuinely just copying cocos source into ccrendertexture begin and end fixes all
// issues i've had with it and mat's rendertexture (and i assume with level thumbs as well)
// taken from
// https://github.com/cocos2d/cocos2d-x/blob/cocos2d-x-2.2.3/cocos2dx/misc_nodes/CCRenderTexture.cpp

using namespace cocos2d;

void HookedCCRenderTexture::begin() {
    kmGLMatrixMode(KM_GL_PROJECTION);
	kmGLPushMatrix();
	kmGLMatrixMode(KM_GL_MODELVIEW);
    kmGLPushMatrix();
    
    CCDirector *director = CCDirector::sharedDirector();
    director->setProjection(director->getProjection());

#if CC_TARGET_PLATFORM == CC_PLATFORM_WP8
    kmMat4 modifiedProjection;
    kmGLGetMatrix(KM_GL_PROJECTION, &modifiedProjection);
    kmMat4Multiply(&modifiedProjection, CCEGLView::sharedOpenGLView()->getReverseOrientationMatrix(), &modifiedProjection);
    kmGLMatrixMode(KM_GL_PROJECTION);
    kmGLLoadMatrix(&modifiedProjection);
    kmGLMatrixMode(KM_GL_MODELVIEW);
#endif

    const CCSize& texSize = m_pTexture->getContentSizeInPixels();

    // Calculate the adjustment ratios based on the old and new projections
    CCSize size = director->getWinSizeInPixels();
    float widthRatio = size.width / texSize.width;
    float heightRatio = size.height / texSize.height;

    // Adjust the orthographic projection and viewport
    glViewport(0, 0, (GLsizei)texSize.width, (GLsizei)texSize.height);


    kmMat4 orthoMatrix;
    kmMat4OrthographicProjection(&orthoMatrix, (float)-1.0 / widthRatio,  (float)1.0 / widthRatio,
        (float)-1.0 / heightRatio, (float)1.0 / heightRatio, -1,1 );
    kmGLMultMatrix(&orthoMatrix);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_nOldFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_uFBO);
    
    /*  Certain Qualcomm Andreno gpu's will retain data in memory after a frame buffer switch which corrupts the render to the texture. The solution is to clear the frame buffer before rendering to the texture. However, calling glClear has the unintended result of clearing the current texture. Create a temporary texture to overcome this. At the end of CCRenderTexture::begin(), switch the attached texture to the second one, call glClear, and then switch back to the original texture. This solution is unnecessary for other devices as they don't have the same issue with switching frame buffers.
     */
    if (CCConfiguration::sharedConfiguration()->checkForGLExtension("GL_QCOM"))
    {
        // -- bind a temporary texture so we can clear the render buffer without losing our texture
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pTextureCopy->getName(), 0);
        CHECK_GL_ERROR_DEBUG();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pTexture->getName(), 0);
    }
}

void HookedCCRenderTexture::end() {
    CCDirector *director = CCDirector::sharedDirector();
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_nOldFBO);

    // restore viewport
    director->setViewport();

    kmGLMatrixMode(KM_GL_PROJECTION);
	kmGLPopMatrix();
	kmGLMatrixMode(KM_GL_MODELVIEW);
	kmGLPopMatrix();
}
