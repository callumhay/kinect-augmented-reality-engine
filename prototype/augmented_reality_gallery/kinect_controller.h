
#ifndef KINECT_CONTROLLER_H_
#define KINECT_CONTROLLER_H_

// AugEngine Includes
#include <common.h>
#include <aug_3d_engine/fbo.h>

// AugEngine Forward Declarations
class Texture2D;
class FBO;
class CgFxKinectColourToTexture;
class CgFxKinectDepthToTexture;

class KinectController {
public:
    static KinectController* Build();
    ~KinectController();

    void PollController();

    // Colour and depth query methods
    const Texture2D* GetDepthTexture() const;
    const Texture2D* GetColourTexture() const;

    float GetNearDistanceInMillimeters() const;
    float GetFarDistanceInMillimeters() const;

    // Skeletal data query methods
    const Texture2D* GetSkeletalDebugTexture() const;
    bool GetHandPos(float scaleX, float scaleY, Eigen::Vector3f& pos) const {

        for (int i = 0; i < NUI_SKELETON_COUNT; i++) {
            if (this->skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED) {
                float xPos, yPos;

                Vector4 handPos = this->skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT];
                
                // For some reason there are hiccups in the data, and occasionally a really odd number comes through where
                // the z value of the hand is around zero
                if (handPos.z <= FLT_EPSILON) {
                    return false;
                }
                NuiTransformSkeletonToDepthImageF(handPos, &xPos, &yPos);

                pos[0] = std::max<float>(0, std::min<float>(xPos*scaleX, scaleX));
                pos[1] = std::max<float>(0, std::min<float>(yPos*scaleY, scaleY));
                pos[2] = handPos.z * 100;

                return true;
            }
        }

        return false;
    }

private:
    KinectController();

    const NUI_IMAGE_FRAME* colourImageFrame;
    const NUI_IMAGE_FRAME* depthImageFrame;
    NUI_SKELETON_FRAME skeletonFrame;

    HANDLE depthStreamHandle;
    HANDLE colourStreamHandle;

    Texture2D* depthTexture;
    Texture2D* colourTexture;

    FBO* depthFBO;
    FBO* colourFBO;
    FBO* skeletonFBO;

    std::vector<float> depthBuffer;

    CgFxKinectColourToTexture* colourConverter;
    CgFxKinectDepthToTexture* depthConverter;

    float nearDistanceInMm; // The closest distance in mm that the kinect can record in its depth buffer
    float farDistanceInMm;  // The furthest distance in mm that the kinect can record in its depth buffer

    bool isCalibrating; // Whether or not we are currently calibrating the kinect

    void PollForColourFrameEvent();
    void PollForDepthFrameEvent();
    void PollForSkeletonFrameEvent();

    void DrawSkeletonDebugTexture();
    void DrawSkeleton(const NUI_SKELETON_DATA& skeleton);
    void DrawSkeletonSegments(const NUI_SKELETON_DATA& skeleton,
        const std::vector<Eigen::Vector2f>& skeletonPts, int numJoints, ...);

    DISALLOW_COPY_AND_ASSIGN(KinectController);
};

inline void KinectController::PollController() {
    this->PollForColourFrameEvent();
    this->PollForDepthFrameEvent();
    this->PollForSkeletonFrameEvent();
}

inline const Texture2D* KinectController::GetDepthTexture() const {
    return this->depthFBO->GetFBOTexture();
}

inline const Texture2D* KinectController::GetColourTexture() const {
    return this->colourFBO->GetFBOTexture();
}

inline float KinectController::GetNearDistanceInMillimeters() const {
    return this->nearDistanceInMm;
}

inline float KinectController::GetFarDistanceInMillimeters() const {
    return this->farDistanceInMm;
}

inline const Texture2D* KinectController::GetSkeletalDebugTexture() const {
    return this->skeletonFBO->GetFBOTexture();
}

#endif // KINECT_CONTROLLER_H_