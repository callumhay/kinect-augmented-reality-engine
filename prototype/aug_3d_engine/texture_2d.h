
#ifndef __TEXTURE2D_H__
#define __TEXTURE2D_H__

// AugEngine Includes
#include "common.h"
#include "texture.h"
#include "common_geometry_helper.h"

// Wraps a OpenGL 2D texture, takes care of texture ID and stuff like that
class Texture2D : public Texture {
public:
	virtual ~Texture2D();
	
	void RenderToFullscreenQuad() const;
    void RenderToSubscreenQuad(int x, int y, int width, int height) const;
    void SetBuffer(const GLenum& format, const GLenum& type, const GLvoid* buffer);

    void SetWrapMode(GLint sWrapMode, GLint tWrapMode);

	// Creator methods
	static Texture2D* CreateTexture2DFromBuffer(unsigned char* fileBuffer, long fileBufferLength, TextureFilterType texFilter);
	static Texture2D* CreateTexture2DFromImgFile(const std::string& filepath, TextureFilterType texFilter);
	static Texture2D* CreateEmptyTexture(int width, int height, Texture::TextureFilterType filter, GLint internalFormat);

private:
	Texture2D(TextureFilterType texFilter);
    DISALLOW_COPY_AND_ASSIGN(Texture2D);
};

inline void Texture2D::RenderToFullscreenQuad() const {
	this->BindTexture();
    CommonGeometryHelper::GetInstance()->DrawFullscreenQuad();
	this->UnbindTexture();
    augengine::debug_opengl_state();
}

inline void Texture2D::RenderToSubscreenQuad(int x, int y, int width, int height) const {
	this->BindTexture();
    CommonGeometryHelper::GetInstance()->DrawSubscreenQuad(x, y, width, height, this->GetWidth(), this->GetHeight());
	this->UnbindTexture();
    augengine::debug_opengl_state();
}

inline void Texture2D::SetWrapMode(GLint sWrapMode, GLint tWrapMode) {
    this->BindTexture();
    glTexParameteri(this->textureType, GL_TEXTURE_WRAP_S, sWrapMode);
	glTexParameteri(this->textureType, GL_TEXTURE_WRAP_T, tWrapMode);
    this->UnbindTexture();
}

#endif