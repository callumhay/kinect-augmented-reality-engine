#ifndef AUG3DENGINE_CGFXRENDERDEPTHGEOMETRY_H_
#define AUG3DENGINE_CGFXRENDERDEPTHGEOMETRY_H_

// AugEngine Includes
#include "common.h"
#include "cgfx_shader.h"

class Texture2D;
class Camera;

class CgFxRenderDepthGeometry : public CgFxShader {
public:
    static const char* GEOMETRY_ONLY_TECHNIQUE_NAME;
    static const char* SHADED_GEOMETRY_TECHNIQUE_NAME;

    CgFxRenderDepthGeometry(const Texture2D* depthTexture, const Texture2D* colourTexture,
                            float nearDistInCm, float farDistInCm);
    ~CgFxRenderDepthGeometry();

	void Draw(const Camera& camera, GLuint displayListID);

    void SetLightPosition(const Eigen::Vector3f& pos);
    
protected:
    void SetupParameterHandles();

private:
    const Texture2D* depthTexture;
    const Texture2D* colourTexture;

    float nearDistInCm;
    float farDistInCm;

    CGparameter wvpMatrixParam;
    CGparameter worldMatrixParam;
    CGparameter worldITMatrixParam;
    CGparameter viewInvMatrixParam;
    CGparameter nearDistanceParam;
    CGparameter distanceDiffParam;
    CGparameter depthSamplerParam;
    CGparameter colourSamplerParam;

    CGparameter keyLightPosParam;
    CGparameter keyLightColourParam;
    CGparameter keyLightAttenParam;
    CGparameter shininessParam;

    void SetupBeforePasses(const Camera& camera);
    void DrawPass(CGpass pass, GLuint displayListID);

    DISALLOW_COPY_AND_ASSIGN(CgFxRenderDepthGeometry);
};

inline void CgFxRenderDepthGeometry::Draw(const Camera& camera, GLuint displayListID) {
	this->SetupBeforePasses(camera);
	
	// Draw each pass of this effect
	CGpass currPass = cgGetFirstPass(this->currTechnique);
	while (currPass) {
		this->DrawPass(currPass, displayListID);
		currPass = cgGetNextPass(currPass);
	}
}

inline void CgFxRenderDepthGeometry::SetLightPosition(const Eigen::Vector3f& pos) {
    cgSetParameter3f(this->keyLightPosParam, pos.x(), pos.y(), pos.z());
}

inline void CgFxRenderDepthGeometry::DrawPass(CGpass pass, GLuint displayListID) {
	cgSetPassState(pass);
	glCallList(displayListID);
	cgResetPassState(pass);
}

#endif // AUG3DENGINE_CGFXRENDERDEPTHGEOMETRY_H_