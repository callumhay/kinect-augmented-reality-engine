
// AugEngine Includes
#include "cgfx_render_depth_geometry.h"
#include "texture_2d.h"
#include "camera.h"

const char* CgFxRenderDepthGeometry::GEOMETRY_ONLY_TECHNIQUE_NAME   = "RenderDepthGeometryNoShading";
const char* CgFxRenderDepthGeometry::SHADED_GEOMETRY_TECHNIQUE_NAME = "RenderDepthGeometryWithShading";

CgFxRenderDepthGeometry::CgFxRenderDepthGeometry(const Texture2D* depthTexture,
                                                 const Texture2D* colourTexture,
                                                 float nearDistInCm, float farDistInCm) : 

CgFxShader("../resources/shaders/render_depth_geometry.cgfx"), depthTexture(depthTexture),
colourTexture(colourTexture), nearDistInCm(nearDistInCm), farDistInCm(farDistInCm),
wvpMatrixParam(NULL), worldMatrixParam(NULL), nearDistanceParam(NULL),
distanceDiffParam(NULL), depthSamplerParam(NULL) {

    bool success = this->SetTechnique(CgFxRenderDepthGeometry::GEOMETRY_ONLY_TECHNIQUE_NAME);
    assert(success);

    this->SetupParameterHandles();

    //cgSetParameter3f(this->keyLightColourParam, 1.0f, 1.0f, 1.0f);
    cgSetParameter3f(this->keyLightAttenParam,  0.0f, 0.01f, 0.0f);
    
    augengine::debug_cg_state();
}

CgFxRenderDepthGeometry::~CgFxRenderDepthGeometry() {
}

void CgFxRenderDepthGeometry::SetupParameterHandles() {
    this->wvpMatrixParam              = cgGetNamedEffectParameter(this->cgEffect, "WvpXf");
    this->worldMatrixParam            = cgGetNamedEffectParameter(this->cgEffect, "WorldXf");
    this->worldITMatrixParam          = cgGetNamedEffectParameter(this->cgEffect, "WorldITXf");
    this->viewInvMatrixParam          = cgGetNamedEffectParameter(this->cgEffect, "ViewIXf");

    this->nearDistanceParam           = cgGetNamedEffectParameter(this->cgEffect, "NearDistanceInCm");
    this->distanceDiffParam           = cgGetNamedEffectParameter(this->cgEffect, "DistanceDiffInCm");
    this->depthSamplerParam           = cgGetNamedEffectParameter(this->cgEffect, "DepthSampler");
    this->colourSamplerParam          = cgGetNamedEffectParameter(this->cgEffect, "ColourSampler");
    
    this->keyLightPosParam      = cgGetNamedEffectParameter(this->cgEffect, "KeyPointLightPos");
    this->keyLightColourParam   = cgGetNamedEffectParameter(this->cgEffect, "KeyPointLightColour");
    this->keyLightAttenParam    = cgGetNamedEffectParameter(this->cgEffect, "KeyPointLightAtten");
    this->shininessParam        = cgGetNamedEffectParameter(this->cgEffect, "Shininess");
    augengine::debug_cg_state();
}

void CgFxRenderDepthGeometry::SetupBeforePasses(const Camera& camera) {
    cgGLSetStateMatrixParameter(this->wvpMatrixParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetStateMatrixParameter(this->worldMatrixParam, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
    cgGLSetStateMatrixParameter(this->worldITMatrixParam, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);

	const Eigen::Matrix4f& invViewXf = camera.GetInvViewTransform();
	cgGLSetMatrixParameterfc(this->viewInvMatrixParam, invViewXf.data());

    cgGLSetTextureParameter(this->depthSamplerParam, this->depthTexture->GetTextureID());
    cgGLSetTextureParameter(this->colourSamplerParam, this->colourTexture->GetTextureID());

    cgSetParameter1f(this->nearDistanceParam, this->nearDistInCm);
    cgSetParameter1f(this->distanceDiffParam, this->farDistInCm - this->nearDistInCm);

    augengine::debug_cg_state();
}