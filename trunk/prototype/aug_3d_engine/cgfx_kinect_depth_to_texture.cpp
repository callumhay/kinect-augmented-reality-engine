
// AugEngine Includes
#include "cgfx_kinect_depth_to_texture.h"
#include "common_geometry_helper.h"

const char* CgFxKinectDepthToTexture::DEFAULT_TECHNIQUE_NAME = "KinectDepthConverter";

CgFxKinectDepthToTexture::CgFxKinectDepthToTexture(FBO* resultFBO, Texture2D* kinectDepthTexture) :
CgFxPostProcessingShader("../resources/shaders/kinect_depth_to_texture.cgfx"),
kinectDepthTexture(kinectDepthTexture), resultFBO(resultFBO), kinectDepthSamplerParam(NULL) {
    
    assert(resultFBO != NULL);
    assert(kinectDepthTexture != NULL);

    bool success = this->SetTechnique(CgFxKinectDepthToTexture::DEFAULT_TECHNIQUE_NAME);
    assert(success);
    this->SetupParameterHandles();
}

CgFxKinectDepthToTexture::~CgFxKinectDepthToTexture() {
}

void CgFxKinectDepthToTexture::SetupParameterHandles() {
    this->kinectDepthSamplerParam = cgGetNamedEffectParameter(this->cgEffect, "KinectDepthSampler");
    assert(this->kinectDepthSamplerParam != NULL);

    augengine::debug_cg_state();
}

void CgFxKinectDepthToTexture::Draw(int screenWidth, int screenHeight) {
    cgGLSetTextureParameter(this->kinectDepthSamplerParam, this->kinectDepthTexture->GetTextureID());
    
    this->resultFBO->BindFBO();
    CGpass currPass = cgGetFirstPass(this->currTechnique);
    cgSetPassState(currPass);
    CommonGeometryHelper::GetInstance()->DrawFullscreenQuad();
    cgResetPassState(currPass);
    this->resultFBO->UnbindFBO();
}