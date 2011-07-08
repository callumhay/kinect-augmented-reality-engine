
// AugEngine Includes
#include "common.h"
#include "resource_manager.h"

// Singleton instance of the ResourceManager
ResourceManager* ResourceManager::instance = NULL;

/// <summary> Default constructor for ResourceManager. </summary>
ResourceManager::ResourceManager() {
}

/// <summary> Destructor for ResourceManager. </summary>
ResourceManager::~ResourceManager() {
	// Destroy the cg context
	cgDestroyContext(this->cgContext);
	this->cgContext = NULL;
}

/// <summary>
/// Get the given cgfx effect file as a cgeffect and load it into the resource
/// manager. If the effect file has already been loaded the effect will simply be given.
/// </summary>
/// <param name="filepath"> The CgFx filepath. </param>
/// <param name="effect">
/// [in,out] The CGeffect associated with the given effect file on successful load.
/// </param>
/// <param name="techniques"> [in,out] The CgFx techniques for the effect. </param>
bool ResourceManager::GetCgFxResource(const std::string &filepath, CGeffect &effect,
                                      std::map<std::string, CGtechnique> &techniques) {

	// Try to find the effect in the loaded effects first...
	std::map<std::string, CGeffect>::iterator loadedEffectIter = this->loadedEffects.find(filepath);
	bool needToReadFromFile = loadedEffectIter == this->loadedEffects.end();

	if (needToReadFromFile) {
		long fileBufferLength;
		char* fileBuffer = this->FilepathToMemoryBuffer(filepath, fileBufferLength);
		if (fileBuffer == NULL) {
			debug_output("Effect file not found: " << filepath);
			return false;
		}

		// Load the effect file using physfs and create the effect using the Cg runtime
		effect = cgCreateEffect(this->cgContext, fileBuffer, NULL);
        augengine::debug_cg_state();

		// Clean-up the buffer and file handle
		delete[] fileBuffer;
		fileBuffer = NULL;

		// Add the effect as a resource
		assert(effect != NULL);
		this->numRefPerEffect[effect] = 1;
		this->loadedEffects[filepath] = effect;

		// Load all the techniques for the effect as well
		this->LoadEffectTechniques(effect, techniques);
		this->loadedEffectTechniques[effect] = techniques;
	}
	else {
		// Load the effect
		effect = loadedEffectIter->second;
		assert(effect != NULL);

		// Make sure techniques exist for the effect as well
		std::map<CGeffect, std::map<std::string, CGtechnique> >::iterator techniqueIter = this->loadedEffectTechniques.find(effect);
		assert(techniqueIter != this->loadedEffectTechniques.end());
		techniques = techniqueIter->second;
		assert(techniques.size() > 0);
		assert(this->numRefPerEffect.find(effect) != this->numRefPerEffect.end());

		// Increment the number of references to the effect
		this->numRefPerEffect[effect]++;
	}

    return true;
}


/// <summary>
/// Release an effect resource and its techniques from the manager if
/// the number of references has reached zero.
/// </summary>
/// <param name="effect">
/// [in,out] The CGeffect resource to release from this manager.
/// </param>
/// <returns> true if it succeeds, false if it fails. </returns>
bool ResourceManager::ReleaseCgFxResource(CGeffect& effect) {
	assert(effect != NULL);

	// Find the effect resource
	std::map<std::string, CGeffect>::iterator effectResourceIter = this->loadedEffects.end();
	for (std::map<std::string, CGeffect>::iterator iter = this->loadedEffects.begin(); iter != this->loadedEffects.end(); ++iter) {
		if (iter->second == effect) {
			effectResourceIter = iter;
		}
	}

	if (effectResourceIter == this->loadedEffects.end()) {
		return false;
	}

	// Check the number of references if we have reached the last reference then we delete the effect
	std::map<CGeffect, unsigned int>::iterator numRefIter = this->numRefPerEffect.find(effect);
	assert(numRefIter != this->numRefPerEffect.end());
	this->numRefPerEffect[effect]--;

	if (this->numRefPerEffect[effect] == 0) {
		// No more references: delete the effect resource and its associated techniques
		CGeffect effectResource = effectResourceIter->second;
		assert(effectResource != NULL);

		std::map<CGeffect, std::map<std::string, CGtechnique> >::iterator techniquesIter = this->loadedEffectTechniques.find(effectResource);
		assert(techniquesIter != this->loadedEffectTechniques.end());
		std::map<std::string, CGtechnique>& techniques = techniquesIter->second;
		assert(techniques.size() > 0);

		cgDestroyEffect(effectResource);
        augengine::debug_cg_state();

		this->loadedEffects.erase(effectResourceIter);
		this->loadedEffectTechniques.erase(techniquesIter);
		this->numRefPerEffect.erase(numRefIter);	
	}
	
	effect = NULL;
	return true;
}

/// <summary> Initializes the resource manager singleton object. </summary>
/// <returns>
/// true if it succeeds (singleton will be created), 
/// false if it fails (singleton will be NULL).
/// </returns>
bool ResourceManager::Init() {
    #define CLEAN_UP_AND_RETURN_ERR(r) delete r; r = NULL; return false
    
    ResourceManager* resourceMgr = new ResourceManager();
    
    // Make sure we can properly setup/initialize all the members of the
    // resource manager, if not then we fail to create the singleton
    
    // Initialize the Cg context
    bool success = ResourceManager::InitCgContext(resourceMgr);
    if (!success) {
        CLEAN_UP_AND_RETURN_ERR(resourceMgr);
    }

    ResourceManager::instance = resourceMgr;
    return true;

    #undef CLEAN_UP_AND_RETURN_ERR
}

/// <summary>  Private helper function for initializing the Cg context. </summary>
/// <param name="resourceMgr"> [in,out] The resource manager to initialize. </param>
/// <returns> true if it succeeds, false if it fails. </returns>
bool ResourceManager::InitCgContext(ResourceManager* resourceMgr) {
	assert(resourceMgr->cgContext != NULL);

	// Load the cg context and check for error
	resourceMgr->cgContext = cgCreateContext();
	CGerror error = CG_NO_ERROR;
	const char* errorStr = cgGetLastErrorString(&error);
	if (error != CG_NO_ERROR) {
        debug_output(errorStr);
        return false;
    }

	// Register OGL states for the context and allow texture mgmt
	cgGLRegisterStates(resourceMgr->cgContext);
	cgGLSetManageTextureParameters(resourceMgr->cgContext, CG_TRUE);
	
    errorStr = cgGetLastErrorString(&error);
	if (error != CG_NO_ERROR) {
        debug_output(errorStr);
        return false;
    }

    return true;
}


/// <summary>
/// Private helper function that loads the techniques for the given effect into the given
/// map of techniques - these are hashed using their name as the key.
/// </summary>
/// <param name="effect"> The CGeffect to load techniques from. </param>
/// <param name="techniques"> [in,out] The resulting CgFx techniques for the effect. </param>
void ResourceManager::LoadEffectTechniques(const CGeffect effect,
                                           std::map<std::string, CGtechnique>& techniques) {

	assert(effect != NULL);

	// Obtain all the techniques associated with this effect
	CGtechnique currTechnique = cgGetFirstTechnique(effect);
	while (currTechnique) {
		const char* techniqueName = cgGetTechniqueName(currTechnique);
		if (cgValidateTechnique(currTechnique) == CG_FALSE) {
			debug_output("Could not validate Cg technique " << techniqueName);
			assert(false);
			currTechnique = cgGetNextTechnique(currTechnique);
			continue;
		}
		techniques[std::string(techniqueName)] = currTechnique;
		currTechnique = cgGetNextTechnique(currTechnique);
	}

	assert(techniques.size() != 0);
    augengine::debug_cg_state();
}

char* ResourceManager::FilepathToMemoryBuffer(const std::string& filepath, long& length) {
	std::ifstream iStream(filepath.c_str(), std::ios::binary);
	
	// If the file was not found then the stream will simply be closed and 
	// the file system will fall back to loading from the zip file
	if (iStream.is_open()) {
		iStream.seekg(0, std::ios::end);
		length = static_cast<long>(iStream.tellg());
		iStream.seekg(0, std::ios::beg);

		char* fileBuffer = new char[length+1];
		fileBuffer[length] = '\0';

		iStream.read(fileBuffer, length);
		iStream.close();

		return fileBuffer;
	}

    return NULL;
}