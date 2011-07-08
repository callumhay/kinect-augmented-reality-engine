#ifndef AUG3DENGINE_CGFXPOSTPROCESSINGSHADER_H_
#define AUG3DENGINE_CGFXPOSTPROCESSINGSHADER_H_

// AugEngine Includes
#include "common.h"
#include "cgfx_shader.h"

class CgFxPostProcessingShader : public CgFxShader {
public:
    CgFxPostProcessingShader(const std::string& effectPath);
    virtual ~CgFxPostProcessingShader();

    virtual void Draw(int screenWidth, int screenHeight) = 0;

protected:
    virtual void SetupParameterHandles() = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(CgFxPostProcessingShader);
};

inline CgFxPostProcessingShader::CgFxPostProcessingShader(const std::string& effectPath) :
CgFxShader(effectPath) {
}

inline CgFxPostProcessingShader::~CgFxPostProcessingShader() {
}


#endif // AUG3DENGINE_CGFXPOSTPROCESSINGSHADER_H_