#ifndef AUGENGINE_COMMONGEOMETRYHELPER_H_
#define AUGENGINE_COMMONGEOMETRYHELPER_H_

// AugEngine Includes
#include "common.h"

/// <summary>
/// Singleton class used to help draw common geometry objects using OpenGL.
/// These methods can be used anywhere to draw basic geometric objects for various purposes.
/// </summary>
class CommonGeometryHelper {
public:
    static CommonGeometryHelper* GetInstance();
    static void DeleteInstance();

    void DrawAxisJack(float size, float lineWidth, size_t numTicks);

    /*
    void DrawFrustum(const ColourRGBA& frustumLinesColour, const ColourRGBA& frustumQuadsColour,
        const Eigen::Vector3f& nearTopLeft, const Eigen::Vector3f& nearBottomLeft, 
                     const Eigen::Vector3f& nearBottomRight, const Eigen::Vector3f& nearTopRight,
                     const Eigen::Vector3f& farTopLeft, const Eigen::Vector3f& farBottomLeft,
                     const Eigen::Vector3f& farBottomRight, const Eigen::Vector3f& farTopRight) const;
    

    void DrawVector(const Eigen::Vector3f& position, const Eigen::Vector3f& vector, float lineWidth,
                    const ColourRGBA& colour);
    */

    void DrawFullscreenQuad() const;
    void DrawSubscreenQuad(size_t bottomLeftCornerX, size_t bottomLeftCornerY, size_t pixelWidth, size_t pixelHeight, 
                           size_t windowWidth, size_t windowHeight);

    void DrawBox(Eigen::Vector3f size, float texTilingAmt, bool flipInsideOut);
    void DrawCube(float size, float texTilingAmt);
    void DrawSphere(float radius, int slices, int stacks);
    void DrawCylinder(float topRadius, float bottomRadius, float height, int slices, int stacks, bool drawTopCap, bool drawBottomCap);
    void DrawCone(float baseRadius, float height, int slices, int stacks);

private:
    CommonGeometryHelper();
    ~CommonGeometryHelper();

    // Singleton instance of the common geometry helper
    static CommonGeometryHelper* instance;

    static void BuildCircleTable(std::vector<float>& sinTable, std::vector<float>& cosTable, int numValues);

    DISALLOW_COPY_AND_ASSIGN(CommonGeometryHelper);
};

/// <summary> Gets the singleton instance of the CommonGeometryHelper. </summary>
/// <returns> The singleton instance of the CommonGeometryHelper. </returns>
inline CommonGeometryHelper* CommonGeometryHelper::GetInstance() {
    if (CommonGeometryHelper::instance == NULL) {
        CommonGeometryHelper::instance = new CommonGeometryHelper();
    }
    return CommonGeometryHelper::instance;
}

/// <summary> Destroys the instance of the CommonGeometryHelper, called automatically at exit. </summary>
inline void CommonGeometryHelper::DeleteInstance() {
    if (CommonGeometryHelper::instance != NULL) {
        delete CommonGeometryHelper::instance;
        CommonGeometryHelper::instance = NULL;
    }
}

/// <summary> 
/// Draw a basic cube, centered at the current world translation. 
/// The origin of the cube (in its local space) is at its center.
/// </summary>
/// <param name="size"> The size of the cube in world units. </param>
/// <param name="texTilingAmt">	The texture tiling amount on all sides of the cube. </param>
inline void CommonGeometryHelper::DrawCube(float size, float texTilingAmt) {  
    this->DrawBox(Eigen::Vector3f(size, size, size), texTilingAmt, false);
}

#endif // AUGENGINE_COMMONGEOMETRYHELPER_H_
