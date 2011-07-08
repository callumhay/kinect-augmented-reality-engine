
// Windows include (MUST ALWAYS BE FIRST)
#define NOMINMAX 1
#include <windows.h>

// OpenGL Includes
//#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>

// Cg includes
#include <cg/cg.h>
#include <cg/cgGL.h>

// DevIL includes
#define ILUT_USE_OPENGL 1
#undef  ILUT_USE_WIN32
#include <il/il.h>
#include <il/ilu.h>
#include <il/ilut.h>

// Eigen includes
#include <Eigen/Dense>
#include <Eigen/Geometry>

// STL Includes
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <sstream>
#include <fstream>
#include <memory>
#include <algorithm>

// C/C++ Includes
#include <cstdlib>
#include <cfloat>
#include <ctime>
#include <cassert>
#include <cstdlib>
#include <cmath>

// Memory Leak Tracking Includes
#if defined(_MSC_VER) || defined(_WIN32)
#include <crtdbg.h>
#define CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC 
#endif

// Useful Macros
#define BUFFER_OFFSET(bytes) ((GLubyte*)NULL + (bytes))

#define UNUSED_PARAMETER(p) ((void)p)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)


#ifndef AUG3DENGINE_COMMON_H_
#define AUG3DENGINE_COMMON_H_

namespace augengine {

#ifdef NDEBUG
#define debug_output(s) ((void)0)
inline void debug_opengl_state() {}
inline void debug_cg_state() {}
#else

#define debug_output(s) (std::cout << s << std::endl)

/// <summary>
/// Used to check for errors in opengl.
/// This will print off the error in debug mode and pose an assertion.
/// </summary>
inline void debug_opengl_state() {
	GLenum glErr = glGetError();
	if (glErr == GL_NO_ERROR) {
		return;
	}

	debug_output("OpenGL Error: " << glErr);
	assert(false);
}

/// <summary>
/// Used to check for errors in the Cg runtime.
/// This will print off the error in debug mode and pose an assertion.
/// </summary>
inline void debug_cg_state() {
	CGerror error = CG_NO_ERROR;
	const char* errorStr = cgGetLastErrorString(&error);
	
	if (error != CG_NO_ERROR) {
		debug_output("Cg Error: " << error << " - " << errorStr);
		assert(false);
	}
}

#endif // NDEBUG

}; // namespace augengine

#endif // AUG3DENGINE_COMMON_H_