#ifndef AUG3DENGINE_CGFXKINECTDEPTHTOTEXTURE_H_
#define AUG3DENGINE_CGFXKINECTDEPTHTOTEXTURE_H_

// AugEngine Includes
#include "common.h"
#include "cgfx_post_processing_shader.h"
#include "texture_2d.h"
#include "fbo.h"

class CgFxKinectDepthToTexture : public CgFxPostProcessingShader {
public:
    CgFxKinectDepthToTexture(FBO* resultFBO, Texture2D* kinectDepthTexture);
    ~CgFxKinectDepthToTexture();

    void Draw();

protected:
    void SetupParameterHandles();

private:
    static const char* DEFAULT_TECHNIQUE_NAME;

    FBO* resultFBO;
    Texture2D* kinectDepthTexture;
    CGparameter kinectDepthSamplerParam;

    void Draw(int screenWidth, int screenHeight);

    DISALLOW_COPY_AND_ASSIGN(CgFxKinectDepthToTexture);
};

inline void CgFxKinectDepthToTexture::Draw() {
    this->Draw(this->resultFBO->GetFBOTexture()->GetWidth(), 
               this->resultFBO->GetFBOTexture()->GetHeight());
}


#endif // AUG3DENGINE_CGFXKINECTDEPTHTOTEXTURE_H_