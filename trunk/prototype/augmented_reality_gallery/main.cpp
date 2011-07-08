

#include <common.h>
#include <kinect_controller.h>

// AugEngine Includes
#include <aug_3d_engine/common.h>
#include <aug_3d_engine/augengine.h>
#include <aug_3d_engine/camera.h>
#include <aug_3d_engine/texture_2d.h>
#include <aug_3d_engine/common_geometry_helper.h>
#include <aug_3d_engine/cgfx_render_depth_geometry.h>

// TODO: Fix the upscaling - transforms are not working out right when the resolution of
// the window is different from that of the depth/colour textures
static const int INIT_WIDTH  = 640;//1024;
static const int INIT_HEIGHT = 480;//768;

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

KinectController* kinect = NULL;
int windowWidth;
int windowHeight;

CgFxRenderDepthGeometry* depthGeometryRenderEffect = NULL;

GLuint topographyDrawList = 0;

static const int NUM_HORIZ_VERTS = 640;
static const int NUM_VERT_VERTS  = 480;
static const float TRI_SIZE = 3.0f;

float rotateAmt = 0;

Camera camera(1,1);

void InitKinect() {
    kinect = KinectController::Build();
    if (kinect == NULL) {
        std::cerr << "Failed to initialize kinect." << std::endl;
        exit(-1);
    }

    depthGeometryRenderEffect = new CgFxRenderDepthGeometry(kinect->GetDepthTexture(),
        kinect->GetColourTexture(), kinect->GetNearDistanceInMillimeters() / 10.0f,
        kinect->GetFarDistanceInMillimeters() / 10.0f);

    size_t numHorizontalVerts = kinect->GetDepthTexture()->GetWidth();
    size_t numVerticalVerts   = kinect->GetDepthTexture()->GetHeight();
    assert(numHorizontalVerts > 0 && numVerticalVerts > 0);

    //static const float NEG_HALF_H = - LEVEL_H_SIZE * NUM_HORIZ_VERTS / 2.0f;
    //static const float NEG_HALF_V = - LEVEL_V_SIZE * NUM_VERT_VERTS / 2.0f;

    topographyDrawList = glGenLists(1);
    glNewList(topographyDrawList, GL_COMPILE);
    
    float currX = 0.0f;
    float currY = 0.0f;
    float tempXTexCoord = 0.0f;

    glNormal3f(0, 0, 1.0f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < numVerticalVerts-1; i++) {
        
        //glBegin(GL_TRIANGLE_STRIP);
        for (size_t j = 0; j < numHorizontalVerts; j++) {
            tempXTexCoord = static_cast<float>(j) / static_cast<float>(numHorizontalVerts-1);

            glTexCoord2f(tempXTexCoord, static_cast<float>(i+1) / static_cast<float>(numVerticalVerts-1));
            glVertex3f(currX, currY + TRI_SIZE, 0.0f);
            glTexCoord2f(tempXTexCoord, static_cast<float>(i) / static_cast<float>(numVerticalVerts-1));
            glVertex3f(currX, currY, 0.0f);

            currX += TRI_SIZE;
        }
        //glEnd();
        currY += TRI_SIZE;
        currX = 0;
    }
    glEnd();
    
    glEndList();

    augengine::debug_opengl_state();
}

void KillKinect() {
    assert(kinect != NULL);
    delete kinect;
    kinect = NULL;

    delete depthGeometryRenderEffect;
    depthGeometryRenderEffect = NULL;
}

// Resize And Initialize The GL Window
void ResizeGLScene(GLsizei width, GLsizei height) {
    if (width == 0) {
        width = 1;
    }
    if (height == 0) {
		height = 1;
	}

	glViewport(0,0,width,height);

    windowWidth  = width;
    windowHeight = height;
    camera.SetWindowDimensions(width, height);
}

// All Setup For OpenGL Goes Here
void InitGL() {
    augengine::Init();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
    glDisable(GL_LIGHTING);
}

float temp = -80;
float multiplier = -1;

// Here's Where We Do All The Drawing
void DrawGLScene() {
    // Poll the kinect controller and get the colour and depth textures from it
    kinect->PollController();
    const Texture2D* colourTex          = kinect->GetColourTexture();
    const Texture2D* depthTex           = kinect->GetDepthTexture();
    const Texture2D* skeletonDebugTex   = kinect->GetSkeletalDebugTexture();

    size_t depthTexWidth  = depthTex->GetWidth();
    size_t depthTexHeight = depthTex->GetHeight();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float nearDist = (kinect->GetNearDistanceInMillimeters() / 10.0f) - 10;
    float farDist  = (kinect->GetFarDistanceInMillimeters()  / 10.0f) + 10;
    assert(nearDist > 0);
    assert(farDist > nearDist);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    camera.ApplyCameraTransform();

#define ORTHO_MODE

    glPushAttrib(GL_CURRENT_BIT);
#ifdef ORTHO_MODE
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, windowWidth, windowHeight);
    glOrtho(0.0, depthTexWidth*TRI_SIZE, 0.0, depthTexHeight*TRI_SIZE, nearDist, farDist);
 
    // Draw depth topography geometry
	glMatrixMode(GL_MODELVIEW);
    depthGeometryRenderEffect->SetTechnique(CgFxRenderDepthGeometry::GEOMETRY_ONLY_TECHNIQUE_NAME);
    depthGeometryRenderEffect->Draw(camera, topographyDrawList);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Draw a fullscreen quad of the coloured scene to lay over the geometry
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    colourTex->RenderToFullscreenQuad();
    glPopAttrib();

#else
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity(); 
    gluPerspective(43.0f, (GLfloat)windowWidth/(GLfloat)windowHeight, 1.0f, 2000.0f);
 
    // Draw colour buffer textured topology geometry
	glMatrixMode(GL_MODELVIEW);
    glTranslatef(-0.5f*depthTexWidth*TRI_SIZE, -0.5f*depthTexHeight*TRI_SIZE, 0);
    depthGeometryRenderEffect->Draw(topographyDrawList);
#endif
    glPopAttrib();

    
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, windowWidth, windowHeight);
    glOrtho(0.0, depthTexWidth*TRI_SIZE, 0.0, depthTexHeight*TRI_SIZE, nearDist, farDist);

    glMatrixMode(GL_MODELVIEW);

    // We need to bring the hand position into the orthographic space
    Eigen::Vector3f handPos;
    bool foundHandPos = kinect->GetHandPos(depthTexWidth*TRI_SIZE, depthTexHeight*TRI_SIZE, handPos);
    if (foundHandPos) {
        handPos -= Eigen::Vector3f(depthTexWidth*TRI_SIZE/2, depthTexHeight*TRI_SIZE/2, 0);
        handPos = Eigen::AngleAxisf(static_cast<float>(M_PI), Eigen::Vector3f(0,0,1)) * handPos;
        handPos += Eigen::Vector3f(depthTexWidth*TRI_SIZE/2, depthTexHeight*TRI_SIZE/2, 0);
        handPos[2] *= -1;

        depthGeometryRenderEffect->SetLightPosition(handPos);
    }

    // Draw the virtual light buffer by blending the colours it generates onto the current framebuffer
    depthGeometryRenderEffect->SetTechnique(CgFxRenderDepthGeometry::SHADED_GEOMETRY_TECHNIQUE_NAME);
    
    depthGeometryRenderEffect->Draw(camera, topographyDrawList);

    glPopAttrib();

    /*
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
    
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT, GL_FILL);

    // TODO: Draw all virtual objects into the world
    // Setup the camera's projection for drawing the virtual world - we need to make sure
    // the projection and the transforms match those of the depth/colour camera so that the objects
    // are properly placed into the real world portrayed in the captured camera image
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, windowWidth, windowHeight);
    gluPerspective(43.0f, windowWidth/windowHeight, nearDist, farDist);

    if (temp < -200) {
        multiplier = 1;
    }
    if (temp > -80) {
        multiplier = -1;
    }
    temp += multiplier * 1;

    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0, 0, temp);
    glColor4f(1,1, 1, 1);
    //CommonGeometryHelper::GetInstance()->DrawBox(Eigen::Vector3f(15, 15, 3), 1, false);

    CommonGeometryHelper::GetInstance()->DrawSphere(5, 20, 10);
    glPopAttrib();
    */



    glPushAttrib(GL_CURRENT_BIT);
    // Draw any debug textures as subscreen quads
    glColor4f(1,1,1,1);
    int currX = 10;
    colourTex->RenderToSubscreenQuad(currX, 10, windowWidth/8, windowHeight/8);
    currX += 10 + windowWidth/8;
    depthTex->RenderToSubscreenQuad(currX, 10, windowWidth/8, windowHeight/8);
    currX += 10 + windowWidth/8;
    skeletonDebugTex->RenderToSubscreenQuad(currX, 10, windowWidth/8, windowHeight/8);
    glPopAttrib();
}

// Properly Kill The Window
void KillGLWindow() {
	if (fullscreen) {
		ChangeDisplaySettings(NULL,0); // Switch Back To The Desktop
		ShowCursor(TRUE);              // Show Mouse Pointer
	}

    // Do We Have A Rendering Context?
	if (hRC) {
        // Are We Able To Release The DC And RC Contexts?
		if (!wglMakeCurrent(NULL, NULL)) {
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

        // Are We Able To Delete The RC?
		if (!wglDeleteContext(hRC)) {
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;
	}

    // Are We Able To Release The DC
	if (hDC && !ReleaseDC(hWnd,hDC)) {
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}

    // Are We Able To Destroy The Window?
	if (hWnd && !DestroyWindow(hWnd)) {
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}

    // Are We Able To Unregister Class
	if (!UnregisterClass("OpenGL",hInstance)) {
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
BOOL CreateGLWindow(char* title, int width, int height, bool fullscreenflag) {
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

    // Attempt To Register The Window Class
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
    // Attempt Fullscreen Mode?
	if (fullscreen) {
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= 24;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?",
                "NeHe GL", MB_YESNO|MB_ICONEXCLAMATION)==IDYES) {
				fullscreen=FALSE;
			}
			else {
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
	}

	if (fullscreen)	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else {
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		24,										    // Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		32,											// 32-bit Z-Buffer
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
    // Did We Get A Device Context?
	if (!(hDC=GetDC(hWnd))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

    // Did Windows Find A Matching Pixel Format?
	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	{
		KillGLWindow();
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

    // Are We Able To Set The Pixel Format?
	if(!SetPixelFormat(hDC,PixelFormat,&pfd)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

    // Are We Able To Get A Rendering Context?
	if (!(hRC=wglCreateContext(hDC))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

    // Try To Activate The Rendering Context
	if(!wglMakeCurrent(hDC,hRC)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window

    // Initialize the OpenGL state
	ResizeGLScene(width, height);
	InitGL();
    InitKinect();

	return TRUE;
}

int lastXCoord = -1;

LRESULT CALLBACK WndProc(HWND   hWnd,			// Handle For This Window
					     UINT   uMsg,			// Message For This Window
                         WPARAM	wParam,			// Additional Message Information
                         LPARAM lParam)			// Additional Message Information
{
    // Check For Windows Messages
	switch (uMsg) {
		case WM_ACTIVATE:
            // Check Minimization State
			if (!HIWORD(wParam)) {
				active = TRUE;
			}
			else {
				active = FALSE;
			}
			return 0;

		case WM_SYSCOMMAND:
			switch (wParam)	{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
			keys[wParam] = TRUE;
			return 0;

		case WM_KEYUP:
			keys[wParam] = FALSE;
			return 0;

        case WM_LBUTTONDOWN:
            lastXCoord = LOWORD(lParam);
            return 0;

        case WM_MOUSEMOVE:
            if (wParam == MK_LBUTTON) {
                rotateAmt += 0.01f * (LOWORD(lParam) - lastXCoord);
            }
            return 0;

		case WM_SIZE:
			ResizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

void RedirectIOToConsole() {

    int hConHandle;

    long lStdHandle;

    CONSOLE_SCREEN_BUFFER_INFO coninfo;

    FILE *fp;

    // allocate a console for this app

    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),

    &coninfo);

    coninfo.dwSize.Y = 500;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),

    coninfo.dwSize);

    // redirect unbuffered STDOUT to the console

    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "w" );

    *stdout = *fp;

    setvbuf( stdout, NULL, _IONBF, 0 );

    // redirect unbuffered STDIN to the console

    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);

    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "r" );

    *stdin = *fp;

    setvbuf( stdin, NULL, _IONBF, 0 );

    // redirect unbuffered STDERR to the console

    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);

    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "w" );

    *stderr = *fp;

    setvbuf( stderr, NULL, _IONBF, 0 );

    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog

    // point to console as well

    std::ios::sync_with_stdio();
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG msg;            // Windows Message Structure
	BOOL done = FALSE;  // Bool Variable To Exit Loop


    /*
	if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", 
        "Start FullScreen?", MB_YESNO|MB_ICONQUESTION)==IDNO) {
		fullscreen=FALSE;
	}
    */
    fullscreen=FALSE;

	// Create Our OpenGL Window
	if (!CreateGLWindow("NeHe's OpenGL Framework", INIT_WIDTH, INIT_HEIGHT, fullscreen)) {
        // Quit If Window Was Not Created
		return 0;
	}

    RedirectIOToConsole();
	while(!done) {
        // Is There A Message Waiting?
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
            // Have We Received A Quit Message?
			if (msg.message==WM_QUIT) {
				done = TRUE;
                continue;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
		if (active)	{
			if (keys[VK_ESCAPE]) {
				done = TRUE;
			}
			else {
				DrawGLScene();					// Draw The Scene
				SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
			}
		}

		if (keys[VK_F1]) {
			keys[VK_F1] = FALSE;
			KillGLWindow();
			fullscreen = !fullscreen;
			// Recreate Our OpenGL Window
			if (!CreateGLWindow("NeHe's OpenGL Framework", 640, 480, fullscreen)) {
				return 0;
			}
		}

        if (keys[VK_SUBTRACT]) {
            depthGeometryRenderEffect->Reload();
        }

        float multiplier = 1;
        if (keys[VK_SHIFT]) {
            multiplier = 10;
        }   

        if (keys[VK_UP]) {
            camera.Move(multiplier*1.0f * Camera::DEFAULT_FORWARD_VEC);
        }
        if (keys[VK_DOWN]) {
            camera.Move(multiplier*-1.0f * Camera::DEFAULT_FORWARD_VEC);
        }
        if (keys[VK_LEFT]) {
            camera.Move(multiplier*1.0f * Camera::DEFAULT_LEFT_VEC);
        }
        if (keys[VK_RIGHT]) {
            camera.Move(multiplier*-1.0f * Camera::DEFAULT_LEFT_VEC);
        }
        if (keys[VK_NUMPAD7]) {
            camera.Rotate(Camera::DEFAULT_UP_VEC, 0.01f);
        }
        if (keys[VK_NUMPAD9]) {
            camera.Rotate(Camera::DEFAULT_UP_VEC, -0.01f);
        }
	}

    // Clean up
	KillGLWindow();
    KillKinect();

    augengine::Shutdown();

	return (msg.wParam);
}
