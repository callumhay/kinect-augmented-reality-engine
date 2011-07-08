#ifndef AUG3DENGINE_FBO_H_
#define AUG3DENGINE_FBO_H_

// AugEngine Includes
#include "common.h"
#include "texture_2d.h"

class FBO {
public:
	enum FBOAttachments { NoAttachment = 0x00000000, DepthAttachment = 0x00000001 };

    static FBO* Build(int width, int height, int attachments,
        const Texture::TextureFilterType& filter, GLint internalTexFormat);
	~FBO();

	const Texture2D* GetFBOTexture() const;
	void BindFBO() const;
	void UnbindFBO() const;

	void BindDepthRenderBuffer();

private:
    FBO();

	GLuint fboID;        // OGL Framebuffer object ID
	GLuint depthBuffID;  // OGL depth renderbuffer object ID
	Texture2D* fboTex;   // Texture for holding the frame buffer data (render-to-texture)

	static bool CheckFBOStatus();

    DISALLOW_COPY_AND_ASSIGN(FBO);
};

inline const Texture2D* FBO::GetFBOTexture() const { 
	return this->fboTex; 
}

inline void FBO::BindFBO() const {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, this->fboID);
}

inline void FBO::UnbindFBO() const {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    if (this->fboTex->IsMipmappedFilter()) {
        this->fboTex->BindTexture();
        this->fboTex->GenerateMipmaps();
        this->fboTex->UnbindTexture();
    }
}

inline void FBO::BindDepthRenderBuffer() {
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, this->depthBuffID);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_RENDERBUFFER_EXT, this->depthBuffID);
}

#endif // AUG3DENGINE_FBO_H_