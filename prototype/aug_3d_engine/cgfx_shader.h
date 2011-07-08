#ifndef AUG3DENGINE_CGFXSHADER_H_
#define AUG3DENGINE_CGFXSHADER_H_

// AugEngine Includes
#include "common.h"
#include "resource_manager.h"

class CgFxShader {
public:
    CgFxShader(const std::string& effectPath);
    virtual ~CgFxShader();

    bool SetTechnique(const std::string& techniqueName);
    bool Reload();

protected:
    std::string effectPath;
	CGeffect cgEffect;                              // The Cg Effect pointer
	CGtechnique currTechnique;                      // The current technique
	std::map<std::string, CGtechnique> techniques;  // The set of all techniques for this effect

    virtual void SetupParameterHandles() = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(CgFxShader);
};

inline CgFxShader::CgFxShader(const std::string& effectPath) : effectPath(effectPath),
cgEffect(NULL), currTechnique(NULL) {

    // Load the effect and its techniques from file
    ResourceManager::GetInstance()->GetCgFxResource(this->effectPath, this->cgEffect, this->techniques);
    assert(this->cgEffect != NULL);
    assert(this->techniques.size() > 0);

    augengine::debug_cg_state();
}

inline CgFxShader::~CgFxShader() {
    ResourceManager::GetInstance()->ReleaseCgFxResource(this->cgEffect);
}

inline bool CgFxShader::SetTechnique(const std::string& techniqueName) {
    std::map<std::string, CGtechnique>::const_iterator findIter = this->techniques.find(techniqueName);
    if (findIter == this->techniques.end()) {
        debug_output("No matching Cg Technique found: " << techniqueName);
        return false;
    }

    CGtechnique temp = findIter->second;
	if (cgValidateTechnique(temp) == CG_FALSE) {
		debug_output("Invalid Cg Technique found: " << techniqueName);
        return false;
	}

	this->currTechnique = temp;
    
    return true;
}

inline bool CgFxShader::Reload() {
    if (this->cgEffect != NULL) {
        ResourceManager::GetInstance()->ReleaseCgFxResource(this->cgEffect);
    }

    // Load the effect and its techniques from file
    bool success = ResourceManager::GetInstance()->GetCgFxResource(this->effectPath, this->cgEffect, this->techniques);
    assert(this->cgEffect != NULL);
    assert(this->techniques.size() > 0);
    
    augengine::debug_cg_state();
    
    this->SetupParameterHandles();
    return success;
}

#endif // AUG3DENGINE_CGFXSHADER_H_