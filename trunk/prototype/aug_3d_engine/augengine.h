#ifndef AUG3DENGINE_AUGENGINE_H_
#define AUG3DENGINE_AUGENGINE_H_

#include "common.h"
#include "resource_manager.h"
#include "common_geometry_helper.h"

namespace augengine {

/// <summary>
/// Initializes the AugEngine library.
/// NOTE: Please make sure the opengl context is established before calling this.
/// </summary>
inline bool Init() {
	// Load the OpenGL extension rangler (glew)
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error loading extensions: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

    // DevIL initialization
    ilInit();
    iluInit();

    // Initialize the resource manager - only do this AFTER we've initialized all the
    // 3rd party APIs/libraries
    ResourceManager* resourceMgr = ResourceManager::GetInstance();
    return (resourceMgr != NULL);
}

inline void Shutdown() {
    ResourceManager::DeleteInstance();
    CommonGeometryHelper::DeleteInstance();
}

};

#endif // AUG3DENGINE_AUGENGINE_H_