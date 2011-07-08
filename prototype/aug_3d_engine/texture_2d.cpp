
// AugEngine Includes
#include "texture_2d.h"

using namespace augengine;

// Default constructor for 2D textures
Texture2D::Texture2D(TextureFilterType texFilter) : Texture(texFilter, GL_TEXTURE_2D) {
}

Texture2D::~Texture2D() {
}

void Texture2D::SetBuffer(const GLenum& format, const GLenum& type, const GLvoid* buffer) {
    this->BindTexture();
    glTexImage2D(this->textureType, 0, this->internalFormat, this->width, this->height,
                 0, format, type, buffer);
    if (this->IsMipmappedFilter(this->texFilter)) {
        this->GenerateMipmaps();
    }
    this->UnbindTexture();
    augengine::debug_opengl_state();
}

Texture2D* Texture2D::CreateEmptyTexture(int width, int height, 
                                         Texture::TextureFilterType filter,
                                         GLint internalFormat) {
	int textureType = GL_TEXTURE_2D;

	glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
	Texture2D* newTex = new Texture2D(filter);
	newTex->textureType = textureType;
	
	glEnable(newTex->textureType);
	glGenTextures(1, &newTex->texID);
	if (newTex->texID == 0) {
		delete newTex;
		return NULL;
	}

	newTex->width  = width;
	newTex->height = height;
    newTex->internalFormat = internalFormat;
	
	newTex->BindTexture();
    if (internalFormat == GL_DEPTH_COMPONENT) {
        glTexImage2D(newTex->textureType, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    }
    else {
	    glTexImage2D(newTex->textureType, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
	Texture::SetFilteringParams(newTex->texFilter, newTex->textureType);
	
	// If it's mipmapped then we should generate the mipmaps
	if (Texture::IsMipmappedFilter(newTex->texFilter)) {
		glGenerateMipmapEXT(newTex->textureType);
	}
	newTex->UnbindTexture();

	glPopAttrib();
	debug_opengl_state();

	return newTex;
}

Texture2D* Texture2D::CreateTexture2DFromBuffer(unsigned char* fileBuffer, long fileBufferLength, TextureFilterType texFilter) {
	glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
	
	Texture2D* newTex = new Texture2D(texFilter);
	if (!newTex->Load2DOr1DTextureFromBuffer(fileBuffer, fileBufferLength, texFilter)) {
		delete newTex;
		newTex = NULL;
	}

	glPopAttrib();
	debug_opengl_state();

	return newTex;
}

/**
 * Static creator for making a 2D texture from a given file path to an image file.
 * Returns: 2D Texture with given image, NULL otherwise.
 */
Texture2D* Texture2D::CreateTexture2DFromImgFile(const std::string& filepath, TextureFilterType texFilter) {
	glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
	
	Texture2D* newTex = new Texture2D(texFilter);
	if (!newTex->Load2DOr1DTextureFromImg(filepath, texFilter)) {
		delete newTex;
		newTex = NULL;
	}

	glPopAttrib();
	debug_opengl_state();

	return newTex;
}