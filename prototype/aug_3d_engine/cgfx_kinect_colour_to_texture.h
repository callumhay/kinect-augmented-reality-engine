#ifndef AUG3DENGINE_CGFXKINECTCOLOURTOTEXTURE_H_
#define AUG3DENGINE_CGFXKINECTCOLOURTOTEXTURE_H_

// AugEngine Includes
#include "common.h"
#include "cgfx_post_processing_shader.h"
#include "texture_2d.h"
#include "fbo.h"

class CgFxKinectColourToTexture : public CgFxPostProcessingShader {
public:
    CgFxKinectColourToTexture(FBO* resultFBO, Texture2D* kinectColourTexture);
    ~CgFxKinectColourToTexture();

    void Draw();

protected:
    void SetupParameterHandles();

private:
    static const char* DEFAULT_TECHNIQUE_NAME;

    FBO* resultFBO;
    Texture2D* kinectColourTexture;
    CGparameter kinectColourSamplerParam;

    void Draw(int screenWidth, int screenHeight);

    DISALLOW_COPY_AND_ASSIGN(CgFxKinectColourToTexture);
};

inline void CgFxKinectColourToTexture::Draw() {
    this->Draw(this->resultFBO->GetFBOTexture()->GetWidth(), 
               this->resultFBO->GetFBOTexture()->GetHeight());
}

#endif // AUG3DENGINE_CGFXKINECTCOLOURTOTEXTURE_H_