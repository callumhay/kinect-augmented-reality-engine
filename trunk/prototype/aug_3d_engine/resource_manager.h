#ifndef AUG3DENGINE_RESOURCEMANAGER_H_
#define AUG3DENGINE_RESOURCEMANAGER_H_

// AugEngine Includes
#include "common.h"

class ResourceManager {
public:
    static ResourceManager* GetInstance();
    static void DeleteInstance();

	// Effects Resource Functions
	bool GetCgFxResource(const std::string& filepath, CGeffect& effect,
        std::map<std::string, CGtechnique>& techniques);
	bool ReleaseCgFxResource(CGeffect& effect);

private:
    static ResourceManager* instance;
    static bool Init();
    static bool InitCgContext(ResourceManager* resourceMgr);

    ResourceManager();
    ~ResourceManager();

	// Cg Runtime objects and helper functions
	CGcontext cgContext;
	std::map<std::string, CGeffect> loadedEffects;                                  // Effects that are already loaded
	std::map<CGeffect, std::map<std::string, CGtechnique> > loadedEffectTechniques;	// Techniques associated with each effect
	std::map<CGeffect, unsigned int> numRefPerEffect;	

    

    static void LoadEffectTechniques(const CGeffect effect,
        std::map<std::string, CGtechnique>& techniques);
    static char* FilepathToMemoryBuffer(const std::string& filepath, long& length);

    DISALLOW_COPY_AND_ASSIGN(ResourceManager);
};

inline ResourceManager* ResourceManager::GetInstance() {
    if (ResourceManager::instance == NULL) {
        ResourceManager::Init();
    }
    return ResourceManager::instance;
}

inline void ResourceManager::DeleteInstance() {
    if (ResourceManager::instance != NULL) {
        delete ResourceManager::instance;
        ResourceManager::instance = NULL;
    }
}

#endif // AUG3DENGINE_RESOURCEMANAGER_H_