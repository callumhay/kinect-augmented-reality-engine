
// AugEngine Includes
#include "cgfx_kinect_colour_to_texture.h"
#include "common_geometry_helper.h"

const char* CgFxKinectColourToTexture::DEFAULT_TECHNIQUE_NAME = "KinectColourConverter";

CgFxKinectColourToTexture::CgFxKinectColourToTexture(FBO* resultFBO, Texture2D* kinectColourTexture) :
CgFxPostProcessingShader("../resources/shaders/kinect_colour_to_texture.cgfx"),
kinectColourTexture(kinectColourTexture), resultFBO(resultFBO), kinectColourSamplerParam(NULL) {
    
    assert(resultFBO != NULL);
    assert(kinectColourTexture != NULL);

    bool success = this->SetTechnique(CgFxKinectColourToTexture::DEFAULT_TECHNIQUE_NAME);
    assert(success);
    this->SetupParameterHandles();
}

CgFxKinectColourToTexture::~CgFxKinectColourToTexture() {
}

void CgFxKinectColourToTexture::SetupParameterHandles() {
    this->kinectColourSamplerParam = cgGetNamedEffectParameter(this->cgEffect, "KinectColourSampler");
    assert(this->kinectColourSamplerParam != NULL);

    augengine::debug_cg_state();
}

void CgFxKinectColourToTexture::Draw(int screenWidth, int screenHeight) {
    cgGLSetTextureParameter(this->kinectColourSamplerParam, this->kinectColourTexture->GetTextureID());
    
    this->resultFBO->BindFBO();
    CGpass currPass = cgGetFirstPass(this->currTechnique);
    cgSetPassState(currPass);
    CommonGeometryHelper::GetInstance()->DrawFullscreenQuad();
    cgResetPassState(currPass);
    this->resultFBO->UnbindFBO();

    augengine::debug_cg_state();
}