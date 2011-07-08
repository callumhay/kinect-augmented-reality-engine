
// AugEngine Includes
#include "camera.h"

const float Camera::FOV_ANGLE_IN_DEGS	= 50.0f;
const float Camera::NEAR_PLANE_DIST		= 0.01f;
const float Camera::FAR_PLANE_DIST		= 300.0f;

const Eigen::Vector3f Camera::DEFAULT_FORWARD_VEC	= Eigen::Vector3f(0, 0, -1);
const Eigen::Vector3f Camera::DEFAULT_LEFT_VEC      = Eigen::Vector3f(-1, 0, 0);
const Eigen::Vector3f Camera::DEFAULT_UP_VEC        = Eigen::Vector3f(0, 1, 0);

Camera::Camera(int width, int height) : 
fovAngleInDegrees(FOV_ANGLE_IN_DEGS), windowWidth(width), windowHeight(height) {
    this->viewMatrix.setIdentity();
    this->invViewMatrix.setIdentity();
}

/**
 * Sets the camera transform based on the given m, m will become the
 * transformation to get from camera to world coordinates and the inverse
 * will become the view inverse matrix.
 */
void Camera::SetTransform(const Eigen::Matrix4f& m) {
	this->viewMatrix = m;
	this->invViewMatrix = m.inverse();
}

/**
 * Moves the camera along the given vector (in camera coords) without changing the view
 * direction or up direction.
 */
void Camera::Move(const Eigen::Vector3f &v) {
    Eigen::Matrix4f translationMat(Eigen::Affine3f(Eigen::Translation3f(v)).data());
    Eigen::Matrix4f invTranslationMat(Eigen::Affine3f(Eigen::Translation3f(-v)).data());
    
	this->viewMatrix    = invTranslationMat * this->viewMatrix;
	this->invViewMatrix	= this->invViewMatrix * translationMat;
}

/**
 * Rotates the camera along the given axis ('x', 'y' or 'z'), in camera coords.
 */
void Camera::Rotate(const Eigen::Vector3f &v, float radians) {
    Eigen::Affine3f t(Eigen::AngleAxisf(radians, v));
    Eigen::Affine3f it(Eigen::AngleAxisf(-radians, v));

    Eigen::Matrix4f rotation(t.data());
	Eigen::Matrix4f invRotation(it.data()); 

	this->viewMatrix    = invRotation * this->viewMatrix;
	this->invViewMatrix = this->invViewMatrix * rotation;
}