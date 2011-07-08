#ifndef AUG3DENGINE_CAMERA_H_
#define AUG3DENGINE_CAMERA_H_

#include "common.h"

class Camera {     
public:
	static const float FOV_ANGLE_IN_DEGS;
	static const float NEAR_PLANE_DIST;
	static const float FAR_PLANE_DIST;

    static const Eigen::Vector3f DEFAULT_FORWARD_VEC;
    static const Eigen::Vector3f DEFAULT_LEFT_VEC;
    static const Eigen::Vector3f DEFAULT_UP_VEC;

	Camera(int width, int height);

	void SetWindowDimensions(int w, int h) {
		assert(w > 0 && h > 0);
		this->windowWidth = w;
		this->windowHeight = h;
	}
	int GetWindowWidth() const {
		return this->windowWidth;
	}
	int GetWindowHeight() const {
		return this->windowHeight;
	}
	float GetFOVAngleInDegrees() const {
		return this->fovAngleInDegrees;
	}

	const Eigen::Matrix4f& GetViewTransform() const {
		return this->viewMatrix;
	}

	const Eigen::Matrix4f& GetInvViewTransform() const {
		return this->invViewMatrix;
	}

	/**
	 * Set the view matrix and its inverse to be the identity matrix.
	 */
	void Reset() {
        this->viewMatrix	= Eigen::Matrix4f();
        this->invViewMatrix = Eigen::Matrix4f();
	}

	void SetTransform(const Eigen::Matrix4f& m);
    void Move(const Eigen::Vector3f &v);
	void Rotate(const Eigen::Vector3f &v, float radians);

	void ApplyCameraTransform() {
		glMultMatrixf(this->viewMatrix.data());
	}

	Eigen::Vector3f GetCurrentCameraPosition() const {
        return Eigen::Vector3f(this->invViewMatrix(1, 3), this->invViewMatrix(2, 3), this->invViewMatrix(3, 3));
	}

	// Functions for setting the camera perspective
	void SetPerspectiveWithFOV(float fovAngleInDegs) {
		this->fovAngleInDegrees = fovAngleInDegs;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, this->windowWidth, this->windowHeight);
		gluPerspective(this->fovAngleInDegrees, static_cast<double>(this->windowWidth) / static_cast<double>(this->windowHeight), NEAR_PLANE_DIST, FAR_PLANE_DIST);
	}

	void SetPerspective() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, this->windowWidth, this->windowHeight);
		gluPerspective(this->fovAngleInDegrees, static_cast<double>(this->windowWidth) / static_cast<double>(this->windowHeight), NEAR_PLANE_DIST, FAR_PLANE_DIST);
	}

	static void PushWindowCoords() {
		glPushAttrib(GL_TRANSFORM_BIT);
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
		glPopAttrib();
	}
	static void PopWindowCoords() {
		glPushAttrib(GL_TRANSFORM_BIT);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPopAttrib();
	}

private:
	// View and inverse view matrices
    Eigen::Matrix4f viewMatrix;
	Eigen::Matrix4f invViewMatrix;

	// Current field of view angle
	float fovAngleInDegrees;

	int windowWidth;
	int windowHeight;

    DISALLOW_COPY_AND_ASSIGN(Camera);
};
#endif