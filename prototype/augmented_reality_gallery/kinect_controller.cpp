
// Augemented Gallery Includes
#include <kinect_controller.h>

// AugEngine Includes
#include <aug_3d_engine/camera.h>
#include <aug_3d_engine/texture_2d.h>
#include <aug_3d_engine/cgfx_kinect_colour_to_texture.h>
#include <aug_3d_engine/cgfx_kinect_depth_to_texture.h>

// OpenCV Includes
#include <opencv/cv.h>

static int BUILD_COUNT = 0;

static const float MIN_DISTANCE = 801;
static const float MAX_DISTANCE = 3975;
static const float DISTANCE_DIFF = MAX_DISTANCE - MIN_DISTANCE;

KinectController::KinectController() : depthStreamHandle(NULL), colourStreamHandle(NULL),
colourImageFrame(NULL), depthImageFrame(NULL), depthTexture(NULL), colourTexture(NULL),
depthFBO(NULL), colourFBO(NULL), skeletonFBO(NULL), colourConverter(NULL), depthConverter(NULL),
nearDistanceInMm(MIN_DISTANCE), farDistanceInMm(MAX_DISTANCE), isCalibrating(false) {
}

KinectController::~KinectController() {
    // Clean up any allocated frames
    if (this->colourImageFrame != NULL) {
        NuiImageStreamReleaseFrame(this->colourStreamHandle, this->colourImageFrame);
        this->colourImageFrame = NULL;
    }
    if (this->depthImageFrame != NULL) {
        NuiImageStreamReleaseFrame(this->depthStreamHandle, this->depthImageFrame);
        this->depthImageFrame = NULL;
    }

    // Clean up textures
    if (this->colourTexture != NULL) {
        delete this->colourTexture;
        this->colourTexture = NULL;
    }
    if (this->depthTexture != NULL) {
        delete this->depthTexture;
        this->depthTexture = NULL;
    }

    // Clean up FBOs
    if (this->depthFBO != NULL) {
        delete this->depthFBO;
        this->depthFBO = NULL;
    }
    if (this->colourFBO != NULL) {
        delete this->colourFBO;
        this->colourFBO = NULL;
    }
    if (this->skeletonFBO != NULL) {
        delete this->skeletonFBO;
        this->skeletonFBO = NULL;
    }

    // Clean up converter shaders
    if (this->colourConverter != NULL) {
        delete this->colourConverter;
        this->colourConverter = NULL;
    }
    if (this->depthConverter != NULL) {
        delete this->depthConverter;
        this->depthConverter = NULL;
    }

    // Shutdown the kinect API
    NuiShutdown();
}

KinectController* KinectController::Build() {
    // Currently we only allow one controller to ever be built,
    // code refactoring is required otherwise!
    if (BUILD_COUNT != 0) {
        assert(false);
        return NULL;
    }

    HRESULT result = -1;

    // Attempt to initialize the kinect API
    result = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH | 
                           NUI_INITIALIZE_FLAG_USES_SKELETON);
    if (FAILED(result)) {
        return NULL;
    }

    // NOTE: for more than one sensor this app will need to use...
    // NuiDeviceCount, NuiCreateInstanceByIndex, ...
    
    std::auto_ptr<KinectController> newKinect(new KinectController());
    
    // Initialize the colour and depth buffers for the controller
    result = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,               // RGB32
                                NUI_IMAGE_RESOLUTION_640x480,
                                0, 2,
                                NULL,
                                &newKinect->colourStreamHandle);
    if (FAILED(result)) {
        std::cerr << "Failed to open colour image stream" << std::endl;
        return NULL;
    }

    static const NUI_IMAGE_RESOLUTION DEPTH_RESOLUTION = NUI_IMAGE_RESOLUTION_640x480;
    result = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH,
                                DEPTH_RESOLUTION,
                                0, 2,
                                NULL,
                                &newKinect->depthStreamHandle);
    if (FAILED(result)) {
        std::cerr << "Failed to open depth image stream" << std::endl;
        return NULL;
    }

    // Initialize the skeleton tracking
    result = NuiSkeletonTrackingEnable(NULL, 0);
    if (FAILED(result)) {
        std::cerr << "Failed to enable skeletal tracking" << std::endl;
        return NULL;
    }


    DWORD depthWidth, depthHeight;
    NuiImageResolutionToSize(DEPTH_RESOLUTION, depthWidth, depthHeight);
    newKinect->depthBuffer.resize(depthWidth*depthHeight);

    // Setup the textures that will hold the the images for depth and colour in the kinect
    // controller object...
    newKinect->colourTexture = Texture2D::CreateEmptyTexture(640, 480, Texture::Nearest, GL_RGBA8);
    newKinect->depthTexture  = Texture2D::CreateEmptyTexture(depthWidth, depthHeight, Texture::Nearest, GL_LUMINANCE);
    if (newKinect->colourTexture == NULL || newKinect->depthTexture == NULL) {
        std::cerr << "Failed to create colour/depth texture." << std::endl;
        return NULL;
    }


    // Setup the FBOs, these are used to convert the hardware buffers into something
    // that looks correct in OpenGL
    newKinect->colourFBO    = FBO::Build(640, 480, FBO::NoAttachment, Texture::Bilinear, GL_RGBA8);
    newKinect->depthFBO     = FBO::Build(depthWidth, depthHeight, FBO::NoAttachment, Texture::Bilinear, GL_LUMINANCE);
    newKinect->skeletonFBO  = FBO::Build(640, 480, FBO::DepthAttachment, Texture::Bilinear, GL_RGBA8);
    if (newKinect->colourFBO == NULL || newKinect->depthFBO == NULL) {
        std::cerr << "Failed to create colour/depth frame buffer objects." << std::endl;
        return NULL;
    }

    // Setup the conversion effects/shaders
    newKinect->colourConverter = new CgFxKinectColourToTexture(newKinect->colourFBO, newKinect->colourTexture);
    newKinect->depthConverter  = new CgFxKinectDepthToTexture(newKinect->depthFBO, newKinect->depthTexture);

    BUILD_COUNT++;
    return newKinect.release();
}

/// <summary> Poll the kinect device for an available colour frame. </summary>
void KinectController::PollForColourFrameEvent() {
    if (this->colourImageFrame != NULL) {
        NuiImageStreamReleaseFrame(this->colourStreamHandle, this->colourImageFrame);
        this->colourImageFrame = NULL;
    }

    HRESULT result = NuiImageStreamGetNextFrame(this->colourStreamHandle, 0, &this->colourImageFrame);
    if (FAILED(result)) {
        return;
    }

    NuiImageBuffer* colourImageBuffer = this->colourImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT lockedRect;
    colourImageBuffer->LockRect(0, &lockedRect, NULL, 0);
    if (lockedRect.Pitch != 0) {

        // The data from the buffer will be in the BGRA format and the image will be flipped
        this->colourTexture->SetBuffer(GL_BGRA, GL_UNSIGNED_BYTE, static_cast<BYTE*>(lockedRect.pBits));
        this->colourConverter->Draw();
    }
    else {
        debug_output("Colour buffer length of received texture is bogus.");
    }
    colourImageBuffer->UnlockRect(0);

    //std::cout << "Colour frame accquired." << std::endl;
}

/// <summary> Poll the kinect device for an available depth frame. </summary>
void KinectController::PollForDepthFrameEvent() {
    if (this->depthImageFrame != NULL) {
        NuiImageStreamReleaseFrame(this->depthStreamHandle, this->depthImageFrame);
        this->depthImageFrame = NULL;
    }

    HRESULT result = NuiImageStreamGetNextFrame(this->depthStreamHandle, 0, &this->depthImageFrame);
    if (FAILED(result)) {
        return;
    }

    NuiImageBuffer* depthImageBuffer = this->depthImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT lockedRect;
    depthImageBuffer->LockRect(0, &lockedRect, NULL, 0);
    if (lockedRect.Pitch != 0) {
        
        //int largestDistance  = INT_MIN; 
        //int smallestDistance = INT_MAX; 

        float distanceDiffInMm = this->farDistanceInMm - this->nearDistanceInMm;

        assert(!this->depthBuffer.empty());
        float* currDepthPtr = &this->depthBuffer[0];
        BYTE* buffer = (BYTE*)(lockedRect.pBits);
        BYTE b0, b1;
        for (size_t i = 0; i < this->depthBuffer.size(); i++) {
         
            b0 = *buffer;
            buffer++;
            b1 = *buffer;
            buffer++;

            int distance = (b0 | b1 << 8);
            //if (distance < 65535) {
            //    largestDistance = std::max<int>(largestDistance, distance);
            //}
            //if (distance > 0) {
            //    smallestDistance = std::min<int>(smallestDistance, distance);
            //}

            *currDepthPtr = static_cast<float>((BYTE)((255 * std::max<float>(distance - this->nearDistanceInMm, 0.0f) / distanceDiffInMm))) / 255.0f;
            currDepthPtr++;
        }
        //std::cout << "Largest Distance:  " << largestDistance << std::endl;
        //std::cout << "Smallest Distnace: " << smallestDistance << std::endl;

        this->depthTexture->SetBuffer(GL_LUMINANCE, GL_FLOAT, &this->depthBuffer[0]);
        this->depthConverter->Draw();
    }
    else {
        debug_output("Depth buffer length of received texture is bogus.");
    }
    depthImageBuffer->UnlockRect(0);

    //std::cout << "Depth frame accquired." << std::endl; 
}

void KinectController::PollForSkeletonFrameEvent() {

    HRESULT result = NuiSkeletonGetNextFrame(0, &this->skeletonFrame);
    if (FAILED(result)) {
        return;
    }

    NuiTransformSmooth(&this->skeletonFrame, NULL);
    this->DrawSkeletonDebugTexture();
}

void KinectController::DrawSkeletonDebugTexture() {
    glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_POINT_BIT);

    glDisable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    this->skeletonFBO->BindFBO();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float width  = static_cast<float>(this->skeletonFBO->GetFBOTexture()->GetWidth());
    float height = static_cast<float>(this->skeletonFBO->GetFBOTexture()->GetHeight());

    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    Camera::PushWindowCoords();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // We need to flip the points in the image so they properly correspond to having a bottom left
    // origin in OpenGL
    glTranslatef(width/2, height/2, 0);
    glRotatef(180, 0, 0, 1);
    glTranslatef(-width/2, -height/2, 0);

    for (int i = 0; i < NUI_SKELETON_COUNT; i++) {
        if (this->skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED) {
            this->DrawSkeleton(this->skeletonFrame.SkeletonData[i]);
        }
    }
    glPopMatrix();

    Camera::PopWindowCoords();

    this->skeletonFBO->UnbindFBO();

    glPopAttrib();
}

void KinectController::DrawSkeleton(const NUI_SKELETON_DATA& skeleton) {

    std::vector<Eigen::Vector2f> skeletonPts;
    skeletonPts.reserve(NUI_SKELETON_POSITION_COUNT);

    float scaleX = static_cast<float>(this->depthFBO->GetFBOTexture()->GetWidth());
    float scaleY = static_cast<float>(this->depthFBO->GetFBOTexture()->GetHeight());
    float xPos, yPos;
    for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
        NuiTransformSkeletonToDepthImageF(skeleton.SkeletonPositions[i], &xPos, &yPos);
        skeletonPts.push_back(Eigen::Vector2f(xPos*scaleX, yPos*scaleY));
    }

    // Draw the joints of the skeleton
    glPointSize(15.0f);
    glColor4f(1, 0, 0, 1);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < skeletonPts.size(); i++) {
        const Eigen::Vector2f& currPt = skeletonPts[i];
        glVertex2f(currPt.x(), currPt.y());
    }
    glEnd();

    // Draw the lines connecting the various joints of the skeleton
    this->DrawSkeletonSegments(skeleton, skeletonPts, 4, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_HEAD);
    this->DrawSkeletonSegments(skeleton, skeletonPts, 5, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);
    this->DrawSkeletonSegments(skeleton, skeletonPts, 5, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);
    this->DrawSkeletonSegments(skeleton, skeletonPts, 5, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);
    this->DrawSkeletonSegments(skeleton, skeletonPts, 5, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);
}

void KinectController::DrawSkeletonSegments(const NUI_SKELETON_DATA& skeleton,
                                            const std::vector<Eigen::Vector2f>& skeletonPts,
                                            int numJoints, ...) {
    va_list vl;
    va_start(vl, numJoints);

    glColor4f(0,1,0,1);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < numJoints; i++) {
        NUI_SKELETON_POSITION_INDEX jointIndex = va_arg(vl,NUI_SKELETON_POSITION_INDEX);
        const Eigen::Vector2f& currPt = skeletonPts[jointIndex];
        glVertex2f(currPt.x(), currPt.y());
    }
    glEnd();
}