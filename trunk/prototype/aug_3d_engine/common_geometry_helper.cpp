
// AugEngine Includes
#include "common_geometry_helper.h"


// Singleton instance of the CommonGeometryHelper class
CommonGeometryHelper* CommonGeometryHelper::instance = NULL;

/// <summary> Default constructor for CommonGeometryHelper. </summary>
CommonGeometryHelper::CommonGeometryHelper() {
}

/// <summary> Destructor for CommonGeometryHelper. </summary>
CommonGeometryHelper::~CommonGeometryHelper() {
}

/// <summary> Draw a axis/jack (i.e., x, y, and z axes). </summary>
/// <param name="size">	The size of the axis/jack. </param>
/// <param name="lineWidth"> Width of the lines that make up the axis. </param>
/// <param name="numTicks"> 
/// The number of ticks along any given axis (e.g., a value of 10 would result in 10 ticks along +x,y,z axes
/// and 10 ticks along -x,y,z axes. A value of 0 ignores ticks altogether.
/// </param>
void CommonGeometryHelper::DrawAxisJack(float size, float lineWidth, size_t numTicks) {
    assert(size > 0.0f);
    assert(lineWidth > 0.0f);
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_HINT_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glScalef(size, size, size);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(lineWidth);
    glDisable(GL_LINE_STIPPLE);

    glBegin(GL_LINES);

    // +x-axis line
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    
    // +y-axis line
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    // +z-axis line
    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);

    glEnd();
    
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xFF00);
    glBegin(GL_LINES);
    
    // -x-axis line
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    
    // -y-axis line
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);

    // -z-axis line
    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);

    glEnd();
    glPopMatrix();

    // If there are ticks to draw then draw them along each axis as points
    if (numTicks > 0) {
        const int numTicksInt = static_cast<int>(numTicks);
        glColor4f(1, 1, 1, 1);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glPointSize(2.0f * lineWidth);
        glBegin(GL_POINTS);
        for (int i = -numTicksInt; i <= numTicksInt; i++) {
            if (i == 0) {
                break;
            }
            // -/+x-axis ticks
            glVertex3i(i, 0, 0);
            glVertex3i(-i, 0, 0);
            // -/+y-axis ticks
            glVertex3i(0, i, 0);
            glVertex3i(0, -i, 0);
            // -/+z-axis ticks
            glVertex3i(0, 0, i);
            glVertex3i(0, 0, -i);
        }
        glEnd();
    }

    glPopAttrib();

    augengine::debug_opengl_state();
}

/*
/// <summary> Draws a frustum using OpenGL. </summary>
/// <param name="frustumLinesColour"> Colour of the frustum lines. </param>
/// <param name="frustumQuadsColour"> Colour of the frustum quads/fill. </param>
/// <param name="nearTopLeft"> The near-plane's top left point. </param>
/// <param name="nearBottomLeft"> The near-plane's bottom left point. </param>
/// <param name="nearBottomRight"> The near-plane's bottom right point. </param>
/// <param name="nearTopRight">	The near-plane's top right point. </param>
/// <param name="farTopLeft"> The far-plane's top left point. </param>
/// <param name="farBottomLeft"> The far-plane's bottom left point. </param>
/// <param name="farBottomRight"> The far-plane's bottom right point. </param>
/// <param name="farTopRight"> The far-plane's top right point. </param>
void CommonGeometryHelper::DrawFrustum(const ColourRGBA& frustumLinesColour, const ColourRGBA& frustumQuadsColour,
                                       const Eigen::Vector3f& nearTopLeft, const Eigen::Vector3f& nearBottomLeft, 
                                       const Eigen::Vector3f& nearBottomRight, const Eigen::Vector3f& nearTopRight,
                                       const Eigen::Vector3f& farTopLeft, const Eigen::Vector3f& farBottomLeft,
                                       const Eigen::Vector3f& farBottomRight, const Eigen::Vector3f& farTopRight) const {

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT | GL_LINE_BIT);

    // Don't need to cull faces - we want to see the sides of the frustum on inside and out
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    // Do simple blending
    glEnable(GL_BLEND);         
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Since there's blending we don't want order of transparency issues
    glDepthMask(GL_FALSE);

    glColor4fv(frustumQuadsColour.data());
    glBegin(GL_QUADS);
    // Near plane quad ----------------------------
    glVertex3fv(nearTopLeft.data());
    glVertex3fv(nearBottomLeft.data());
    glVertex3fv(nearBottomRight.data());
    glVertex3fv(nearTopRight.data());
    // --------------------------------------------

    // Side quads ---------------------------------
    // Left side
    glVertex3fv(farTopLeft.data());
    glVertex3fv(farBottomLeft.data());
    glVertex3fv(nearBottomLeft.data());
    glVertex3fv(nearTopLeft.data());

    // Right side
    glVertex3fv(farTopRight.data());
    glVertex3fv(farBottomRight.data());
    glVertex3fv(nearBottomRight.data());
    glVertex3fv(nearTopRight.data());

    // Top side
    glVertex3fv(farTopLeft.data());
    glVertex3fv(nearTopLeft.data());
    glVertex3fv(nearTopRight.data());
    glVertex3fv(farTopRight.data());

    // Bottom side
    glVertex3fv(farBottomLeft.data());
    glVertex3fv(nearBottomLeft.data());
    glVertex3fv(nearBottomRight.data());
    glVertex3fv(farBottomRight.data());
    // --------------------------------------------

    // Far plane quad -----------------------------
    glVertex3fv(farTopLeft.data());
    glVertex3fv(farBottomLeft.data());
    glVertex3fv(farBottomRight.data());
    glVertex3fv(farTopRight.data());
    // --------------------------------------------
    glEnd();

    glDepthMask(GL_TRUE);

    // Draw the frustum as a set of blended quads and lines...
    glLineWidth(2.0f);
    glColor4fv(frustumLinesColour.data());
    glBegin(GL_LINES);

    // Near plane lines ---------------------------
    glVertex3fv(nearTopLeft.data());
    glVertex3fv(nearBottomLeft.data());

    glVertex3fv(nearBottomLeft.data());
    glVertex3fv(nearBottomRight.data());

    glVertex3fv(nearBottomRight.data());
    glVertex3fv(nearTopRight.data());

    glVertex3fv(nearTopRight.data());
    glVertex3fv(nearTopLeft.data());
    // --------------------------------------------

    // Joining lines from near to far plane -------
    glVertex3fv(nearTopLeft.data());
    glVertex3fv(farTopLeft.data());

    glVertex3fv(nearBottomLeft.data());
    glVertex3fv(farBottomLeft.data());

    glVertex3fv(nearBottomRight.data());
    glVertex3fv(farBottomRight.data());

    glVertex3fv(nearTopRight.data());
    glVertex3fv(farTopRight.data());
    // --------------------------------------------

    // Far plane lines ----------------------------
    glVertex3fv(farTopLeft.data());
    glVertex3fv(farBottomLeft.data());

    glVertex3fv(farBottomLeft.data());
    glVertex3fv(farBottomRight.data());

    glVertex3fv(farBottomRight.data());
    glVertex3fv(farTopRight.data());

    glVertex3fv(farTopRight.data());
    glVertex3fv(farTopLeft.data());
    // --------------------------------------------
    glEnd();

    glPopAttrib();

    augengine::debug_opengl_state();
}

/// <summary> Draw a vector as a directed line segment at some given point in 3D space. </summary>
/// <param name="position"> The position to start drawing the vector from. </param>
/// <param name="vector"> The vector to draw. </param>
/// <param name="lineWidth"> Width of the line that represents the vector. </param>
/// <param name="colour"> The colour of the line (and arrow) representing the vector. </param>
void CommonGeometryHelper::DrawVector(const Eigen::Vector3f& position, const Eigen::Vector3f& vector, 
                                      float lineWidth, const ColourRGBA& colour) {

    assert(lineWidth > 0.0f);
    assert(!vector.IsZero());

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
    glDisable(GL_LIGHTING);

    glLineWidth(lineWidth);

    glPushMatrix();
    glTranslatef(position[0], position[1], position[2]);

    glColor4fv(colour.data());
    glBegin(GL_LINES);
    // Draw the vector as a single line segment
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3fv(vector.data());
    glEnd();

    glPopMatrix();
    glPopAttrib();

    augengine::debug_opengl_state();
}
*/

/// <summary> 
/// Draw a fullscreen quad.
/// NOTE: If you want the quad to appear over everything / in a certain manor you
/// must set the depth/blend states yourself before a call to this function.
/// </summary>
void CommonGeometryHelper::DrawFullscreenQuad() const {
    glPushAttrib(GL_TRANSFORM_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);

    glDisable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glPolygonMode(GL_FRONT, GL_FILL);

    glMatrixMode(GL_MODELVIEW); 
    glPushMatrix(); 
    glLoadIdentity(); 

    glMatrixMode(GL_PROJECTION); 
    glPushMatrix(); 
    glLoadIdentity();

    glBegin(GL_QUADS); 
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glTexCoord2i(0, 0);
    glVertex3i(-1, -1, -1); 
    glTexCoord2i(1, 0);
    glVertex3i(1, -1, -1); 
    glTexCoord2i(1, 1);
    glVertex3i(1, 1, -1); 
    glTexCoord2i(0, 1);
    glVertex3i(-1, 1, -1);
    glEnd();   

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW); 
    glPopMatrix();

    glPopAttrib();

    augengine::debug_opengl_state();
}

/// <summary> 
/// Draw subscreen quad (i.e., a quad that takes up some portion of the screen space).
/// NOTE: If you want the quad to appear over everything / in a certain manor you
/// must set the depth/blend states yourself before a call to this function.
/// </summary>
/// <param name="bottomLeftCornerX"> The x-coordinate of the bottom left corner of the quad in screen space. </param>
/// <param name="bottomLeftCornerY"> The y-coordinate of the bottom left corner of the quad in screen space. </param>
/// <param name="pixelWidth"> Width of the quad in pixels. </param>
/// <param name="pixelHeight"> Height of the quad in pixels. </param>
/// <param name="windowWidth"> Width of the window. </param>
/// <param name="windowHeight"> Height of the window. </param>
void CommonGeometryHelper::DrawSubscreenQuad(size_t bottomLeftCornerX, size_t bottomLeftCornerY, size_t pixelWidth, 
                                             size_t pixelHeight, size_t windowWidth, size_t windowHeight) {

    glPushAttrib(GL_TRANSFORM_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);

    glDisable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT, GL_FILL);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION); 
    glPushMatrix(); 
    glLoadIdentity(); 
    gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);

    size_t maxX = bottomLeftCornerX + pixelWidth;
    size_t maxY = bottomLeftCornerY + pixelHeight;

    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); 
    glVertex2i(bottomLeftCornerX, bottomLeftCornerY);
    glTexCoord2i(1, 0); 
    glVertex2i(maxX, bottomLeftCornerY); 
    glTexCoord2i(1, 1); 
    glVertex2i(maxX, maxY);
    glTexCoord2i(0, 1); 
    glVertex2i(bottomLeftCornerX, maxY);
    glEnd();

    glPopMatrix(); 
    glMatrixMode(GL_MODELVIEW); 
    glPopMatrix();

    glPopAttrib();

    augengine::debug_opengl_state();
}

/// <summary> 
/// Draw a basic rectangular prism (i.e., box), centered at the current world translation. 
/// The origin of the box (in its local space) is at its center.
/// </summary>
/// <param name="size"> The size of the rectangular prism along the x, y and z axes, respectively; in world units. </param>
/// <param name="texTilingAmt">	The texture tiling amount on all sides of the rectangular prism. </param>
/// <param name="flipInsideOut"> true to flip the box inside out (i.e., show its interior walls and not its exterior). </param>
void CommonGeometryHelper::DrawBox(Eigen::Vector3f size, float texTilingAmt, bool flipInsideOut) {
    Eigen::Vector3f halfSizeVec = 0.5f * size;

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT);
    glEnable(GL_CULL_FACE);

    // If we're going to flip the box inside-out then we need to cull the front faces instead
    // and flip the normals so that the box gets lit on the inside
    float normalMultiplier = 1.0f;
    if (flipInsideOut) {
        glCullFace(GL_FRONT);
        normalMultiplier = -1.0f;
    }
    else {
        glCullFace(GL_BACK);
    }

	glBegin(GL_QUADS);

    // Front Face
    glNormal3f(0.0, 0.0, 1.0f * normalMultiplier);
    glTexCoord2d(0.0f, 0.0f);                        glVertex3f(-halfSizeVec[0], -halfSizeVec[1], halfSizeVec[2]);
    glTexCoord2d(texTilingAmt, 0.0f);                glVertex3f(halfSizeVec[0], -halfSizeVec[1],  halfSizeVec[2]);
    glTexCoord2d(texTilingAmt, 1.0f * texTilingAmt); glVertex3f(halfSizeVec[0],  halfSizeVec[1],  halfSizeVec[2]);
    glTexCoord2d(0.0f, texTilingAmt);                glVertex3f(-halfSizeVec[0], halfSizeVec[1],  halfSizeVec[2]);

    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f * normalMultiplier);
    glTexCoord2d(texTilingAmt, 0.0f);           glVertex3f(-halfSizeVec[0], -halfSizeVec[1], -halfSizeVec[2]);
    glTexCoord2d(texTilingAmt, texTilingAmt);   glVertex3f(-halfSizeVec[0], halfSizeVec[1], -halfSizeVec[2]);
    glTexCoord2d(0.0f, texTilingAmt);           glVertex3f(halfSizeVec[0],  halfSizeVec[1], -halfSizeVec[2]);
    glTexCoord2d(0.0f, 0.0f);                   glVertex3f(halfSizeVec[0], -halfSizeVec[1], -halfSizeVec[2]);

    // Top Face
    glNormal3f(0.0f, 1.0f * normalMultiplier, 0.0f);
    glTexCoord2d(0.0f, texTilingAmt);         glVertex3f(-halfSizeVec[0], halfSizeVec[1], -halfSizeVec[2]);
    glTexCoord2d(0.0f, 0.0f);                 glVertex3f(-halfSizeVec[0], halfSizeVec[1],  halfSizeVec[2]);
    glTexCoord2d(texTilingAmt, 0.0f);         glVertex3f(halfSizeVec[0],  halfSizeVec[1],  halfSizeVec[2]);
    glTexCoord2d(texTilingAmt, texTilingAmt); glVertex3f(halfSizeVec[0],  halfSizeVec[1], -halfSizeVec[2]);

	// Bottom Face
	glNormal3f(0.0f, -1.0f * normalMultiplier, 0.0f);
	glTexCoord2d(texTilingAmt, texTilingAmt); glVertex3f(-halfSizeVec[0], -halfSizeVec[1], -halfSizeVec[2]);
	glTexCoord2d(0.0f, texTilingAmt);         glVertex3f(halfSizeVec[0], -halfSizeVec[1], -halfSizeVec[2]);
	glTexCoord2d(0.0f, 0.0f);                 glVertex3f(halfSizeVec[0], -halfSizeVec[1],  halfSizeVec[2]);
	glTexCoord2d(texTilingAmt, 0.0f);         glVertex3f(-halfSizeVec[0], -halfSizeVec[1], halfSizeVec[2]);

	// Right face
	glNormal3f(1.0f * normalMultiplier, 0.0f, 0.0f);
	glTexCoord2d(texTilingAmt, 0.0f);         glVertex3f(halfSizeVec[0], -halfSizeVec[1], -halfSizeVec[2]);
	glTexCoord2d(texTilingAmt, texTilingAmt); glVertex3f(halfSizeVec[0],  halfSizeVec[1], -halfSizeVec[2]);
	glTexCoord2d(0.0f, texTilingAmt);         glVertex3f(halfSizeVec[0],  halfSizeVec[1],  halfSizeVec[2]);
	glTexCoord2d(0.0f, 0.0f);                 glVertex3f(halfSizeVec[0], -halfSizeVec[1],  halfSizeVec[2]);

	// Left Face
	glNormal3f(-1.0f * normalMultiplier, 0.0f, 0.0f);
	glTexCoord2d(0.0f, 0.0f);                 glVertex3f(-halfSizeVec[0], -halfSizeVec[1], -halfSizeVec[2]);
	glTexCoord2d(texTilingAmt, 0.0f);         glVertex3f(-halfSizeVec[0], -halfSizeVec[1],  halfSizeVec[2]);
	glTexCoord2d(texTilingAmt, texTilingAmt); glVertex3f(-halfSizeVec[0],  halfSizeVec[1],  halfSizeVec[2]);
	glTexCoord2d(0.0f, texTilingAmt);         glVertex3f(-halfSizeVec[0],  halfSizeVec[1], -halfSizeVec[2]);
    
    glEnd();
    glPopAttrib();

    augengine::debug_opengl_state();
}

/// <summary> 
/// Draws a basic sphere with the given tesselation parameters. 
/// This will draw a sphere oriented such that the stacks are along the y-axis - i.e., y is up.
/// The origin of the sphere (in its local space) is at its center.
/// Face winding is Counter-Clockwise.
/// </summary>
/// <param name="radius"> The sphere radius. </param>
/// <param name="slices"> The slices (tesselation along the longitude of the sphere). </param>
/// <param name="stacks"> The stacks (tesselation along the latitude of the sphere). </param>
void CommonGeometryHelper::DrawSphere(float radius, int slices, int stacks) {
    // Make sure the given parameters are valid
    if (slices <= 0 || stacks <= 0) {
        assert(false);
        return;
    }

    // Compute the circle table
    std::vector<float> sinTable1, sinTable2;
    std::vector<float> cosTable1, cosTable2;
    CommonGeometryHelper::BuildCircleTable(sinTable1, cosTable1, slices);
    CommonGeometryHelper::BuildCircleTable(sinTable2, cosTable2, stacks * 2);

    // Temporary variables used to adjust z and radius as stacks are drawn.
    float z0, z1;
    float r0, r1;

    // The top stack is covered with a triangle fan
    z0 = 1.0f;
    z1 = cosTable2[1];
    r0 = 0.0f;
    r1 = sinTable2[1];

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, radius, 0.0f);
    for (int j = static_cast<int>(slices); j >= 0; j--) {       
        glNormal3d(cosTable1[j] * r1, z1, sinTable1[j] * r1);
        glVertex3d(cosTable1[j] * r1 * radius, z1 * radius, sinTable1[j] * r1 * radius);
    }
    glEnd();

    // Cover each stack with a quad strip, except the top and bottom stacks
    for (int i = 1; i < stacks-1; i++) {
        z0 = z1; 
        z1 = cosTable2[i+1];
        r0 = r1; 
        r1 = sinTable2[i+1];

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            glNormal3f(cosTable1[j] * r1, z1, sinTable1[j] * r1);
            glVertex3f(cosTable1[j] * r1 * radius, z1 * radius, sinTable1[j] * r1 * radius);
            glNormal3f(cosTable1[j] * r0, z0, sinTable1[j] * r0);
            glVertex3f(cosTable1[j] * r0 * radius, z0 * radius, sinTable1[j] * r0 * radius);
        }
        glEnd();
    }

    // The bottom stack is covered with a triangle fan
    z0 = z1;
    r0 = r1;

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, -radius, 0.0f);
    for (int j = 0; j <= slices; j++) {
        glNormal3f(cosTable1[j] * r0, z0, sinTable1[j] * r0);
        glVertex3f(cosTable1[j] * r0 * radius, z0 * radius, sinTable1[j] * r0 * radius);
    }
    glEnd();
}

/// <summary> 
/// Draws a basic cylinder with the y-axis as the up axis, which the stacks tesselate along.
/// The center of the cylinder (in its local space) is the center of the bottom cap.
/// Face winding is Counter-Clockwise.
/// </summary>
/// <param name="topRadius"> The cylinder's top cap radius. </param>
/// <param name="bottomRadius"> The cylinder's bottom cap radius. </param>
/// <param name="height"> The height from bottom to top cap. </param>
/// <param name="slices"> The slices (tesselation around the circumference of the cylinder). </param>
/// <param name="stacks"> The stacks (tesselation along the height of the cylinder). </param>
/// <param name="drawTopCap"> true to draw the top cap. </param>
/// <param name="drawBottomCap"> true to draw the bottom cap. </param>
void CommonGeometryHelper::DrawCylinder(float topRadius, float bottomRadius, float height, 
                                        int slices, int stacks, bool drawTopCap, bool drawBottomCap) {
    // Make sure parameters are valid
    if (topRadius < 0.0f || bottomRadius < 0.0f || height <= 0.0f || slices < 3 ||
        stacks < 1) {

        assert(false);
        return;
    }
    
    // Pre-compute the circle values
    std::vector<float> sinTable;
    std::vector<float> cosTable;
    CommonGeometryHelper::BuildCircleTable(sinTable, cosTable, slices);

    // Temporary variables/values used for z and radius calculations when drawing stacks
    const float zStep = height / static_cast<float>(stacks);
    float z0, z1;

    // Cover the base/bottom and top
	if (drawBottomCap) {
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
        for (int j = 0; j <= slices; j++) {
		    glVertex3f(cosTable[j] * topRadius, 0.0f, sinTable[j] * topRadius);
        }
		glEnd();
    }

    if (drawTopCap) {
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, height, 0.0f);
        for (int j = slices; j >= 0; j--) {
		    glVertex3f(cosTable[j] * bottomRadius, height, sinTable[j] * bottomRadius);
        }
		glEnd();
	}

    // Draw the side of the cylinder
    z0 = 0.0;
    z1 = zStep;
	float alpha1;
	float alpha2;
	float alphaStep = 1.0f/stacks;

    for (int i = 1; i <= stacks; i++) {
        if (i == stacks) {
            z1 = height;
        }

		float radius1 = topRadius + static_cast<float>(i-1) * (bottomRadius - topRadius) / static_cast<float>(stacks);
		float radius2 = topRadius + static_cast<float>(i)   * (bottomRadius - topRadius) / static_cast<float>(stacks);

		float tv0 = 1.0f * static_cast<float>(i-1) / static_cast<float>(stacks);
		float tv1 = 1.0f * static_cast<float>(i)   / static_cast<float>(stacks);

		alpha1 = 1.0f - (i-1) * alphaStep;
		alpha2 = 1.0f - i * alphaStep;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            glNormal3f(cosTable[j], 0.0f, sinTable[j]);
			glTexCoord2f(-(static_cast<float>(j)/slices), tv0);
            glVertex3f(cosTable[j]*radius1, z0, sinTable[j]*radius1);
			glTexCoord2f(-(static_cast<float>(j)/slices), tv1);
            glVertex3f(cosTable[j]*radius2, z1, sinTable[j]*radius2);
        }
        glEnd();

        z0 = z1; 
        z1 += zStep;
    }
}

/// <summary> 
/// Draws a basic cone with 'y' as the up axis, which is also what the stacks tesselate along. 
/// The origin of the cone (in its local space) is the center of its base.
/// Face winding is Counter-Clockwise.
/// </summary>
/// <param name="baseRadius"> The base radius of the cone. </param>
/// <param name="height"> The height of the cone from base to apex. </param>
/// <param name="slices"> The slices (tesselation around the cone's circumference). </param>
/// <param name="stacks"> The stacks (tesselation along the height of the cone). </param>
void CommonGeometryHelper::DrawCone(float baseRadius, float height, int slices, int stacks) {
    // Make sure the given parameters are valid
    if (baseRadius <= 0.0f || height == 0.0f || slices <= 0 || stacks <= 0) {
        assert(false);
        return;
    }

    const float zStep = height / static_cast<float>(stacks);
    const float rStep = baseRadius / static_cast<float>(stacks);

    // Scaling factors for the vertex normals
    const float cosn = (height / sqrt(height * height + baseRadius * baseRadius));
    const float sinn = (baseRadius / sqrt(height * height + baseRadius * baseRadius));

    // Pre-compute the circle values
    std::vector<float> sinTable;
    std::vector<float> cosTable;
    CommonGeometryHelper::BuildCircleTable(sinTable, cosTable, slices);

    // Temporary variables for z and radius values as stacks are being drawn
    float z0, z1;
    float r0, r1;

    // Cover the circular base with a triangle fan
    z0 = 0.0;
    z1 = zStep;
    r0 = baseRadius;
    r1 = r0 - rStep;

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, z0, 0.0f);
    for (int j = 0; j <= slices; j++) {
        glVertex3f(cosTable[j] * r0, z0, sinTable[j] * r0);
    }
    glEnd();

    // Cover each stack with a quad strip, except the top stack
    for (int i = 0; i < stacks-1; i++) {
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            glNormal3f(cosTable[j] * sinn, cosn, sinTable[j] * sinn);
            glVertex3f(cosTable[j] * r0, z0, sinTable[j] * r0);
            glVertex3f(cosTable[j] * r1, z1, sinTable[j] * r1);
        }

        z0 = z1; z1 += zStep;
        r0 = r1; r1 -= rStep;
        glEnd();
    }

    // The top stack is covered with individual triangles
    glBegin(GL_TRIANGLES);
    glNormal3f(cosTable[0] * sinn, cosn, sinTable[0] * sinn);
    for (int j = 0; j < slices; j++) {
        glVertex3f(cosTable[j+0] * r0, z0, sinTable[j+0] * r0);
        glVertex3f(0, height, 0);
        glNormal3f(cosTable[j+1] * sinn, cosn, sinTable[j+1] * sinn);
        glVertex3f(cosTable[j+1] * r0, z0, sinTable[j+1] * r0);
    }
    glEnd();
}

/// <summary> Static, private helper function that builds a 'circle table' - 
/// a set of cos and sin that make up the construction of a discretized circle.
/// </summary>
/// <param name="sinTable">	[in,out] The sin value table (resulting table will be size numValues+1 - 
/// the last value is a repeat of the first). </param>
/// <param name="cosTable"> [in,out] The cos value table (resulting table will be size numValues+1 -
/// the last value is a repeat of the first). </param>
/// <param name="numValues"> The number of values/steps to break the circle up into. The sign
/// of this value will flip the circle direction (positive is counter clockwise, negative is clockwise). </param>
void CommonGeometryHelper::BuildCircleTable(std::vector<float>& sinTable, std::vector<float>& cosTable, int numValues) {
    // The Table size, the sign of n flips the circle direction
    const int size = abs(numValues);
    if (size <= 0) {
        assert(false);
        return;
    }

    // Determine the angle between samples
    const float angle = static_cast<float>(2 * M_PI) / static_cast<float>(numValues);

    // Allocate memory for n samples, plus duplicate of first entry at the end
    sinTable.reserve(size+1);
    cosTable.reserve(size+1);

    // Compute the cos and sin around the circle
    float currAngle;
    for (int i = 0; i < size; i++) {
        currAngle = angle * i;
        sinTable.push_back(sin(currAngle));
        cosTable.push_back(cos(currAngle));
    }

    // The last value is a duplicate of the first
    sinTable.push_back(sinTable.front());
    cosTable.push_back(cosTable.front());
}