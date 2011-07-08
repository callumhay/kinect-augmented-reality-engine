
// AugEngine Includes
#include "fbo.h"

FBO::FBO() : fboID(0), depthBuffID(0), fboTex(NULL) {
}

FBO::~FBO() {
	// Clean-up all FBO and renderbuffer objects in OGL
	glDeleteFramebuffersEXT(1,  &this->fboID);
	if (this->depthBuffID != 0) {
		glDeleteRenderbuffersEXT(1, &this->depthBuffID);
	}

	// Clean-up the FBO texture
    if (this->fboTex != NULL) {
	    delete this->fboTex;
	    this->fboTex = NULL;
    }
}

FBO* FBO::Build(int width, int height, int attachments, 
                const Texture::TextureFilterType& filter,
                GLint internalTexFormat) {
    std::auto_ptr<FBO> fbo(new FBO());
    
    // Generate the framebuffer object
	glGenFramebuffersEXT(1, &fbo->fboID);
	assert(fbo->fboID != 0);
	
	// Set up the texture to render the framebuffer into
    fbo->fboTex = Texture2D::CreateEmptyTexture(width, height, filter, internalTexFormat);
    if (fbo->fboTex == NULL) {
        return NULL;
    }
    fbo->fboTex->SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	// Bind this FBO for setup
	fbo->BindFBO();

	// Create the depth render buffer if requested...
	if ((attachments & DepthAttachment) == DepthAttachment) {
		glGenRenderbuffersEXT(1, &fbo->depthBuffID);
		assert(fbo->depthBuffID != 0);

		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo->depthBuffID);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->depthBuffID);
	}

	// Bind the given texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
        fbo->fboTex->GetTextureType(), fbo->fboTex->GetTextureID(), 0);

	// Unbind the FBO for now
	fbo->UnbindFBO();

    // Make sure everything is OK before returning the new FBO object
    augengine::debug_opengl_state();	
    if (!FBO::CheckFBOStatus()) {
        return NULL;
    }

    return fbo.release();
}

/// <summary> Checks the status of the frame buffer object and reports any errors. </summary>
/// <returns> true if valid/OK status, false if bad/ERROR status. </returns>
bool FBO::CheckFBOStatus() {
    augengine::debug_opengl_state();

	int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		switch (status) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
				debug_output("Framebuffer Object error detected: Incomplete attachment.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				debug_output("Framebuffer Object error detected: Incomplete dimensions.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				debug_output("Framebuffer Object error detected: Incomplete draw buffer.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				debug_output("Framebuffer Object error detected: Incomplete formats.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT:
				debug_output("Framebuffer Object error detected: Incomplete layer count.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT:
				debug_output("Framebuffer Object error detected: Incomplete layer targets.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				debug_output("Framebuffer Object error detected: Incomplete, missing attachment.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
				debug_output("Framebuffer Object error detected: Incomplete multisample.");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
				debug_output("Framebuffer Object error detected: Incomplete read buffer.");
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
				debug_output("Framebuffer Object error detected: Framebuffer unsupported.");
				break;
			default:
				debug_output("Framebuffer Object error detected: Unknown Error");
				break;
		}
		return false;
	}

	return true;
}