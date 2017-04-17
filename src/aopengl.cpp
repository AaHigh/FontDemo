#include "stdafx.h"

#include <shellapi.h> // for command line parsing

/***********************************************
*                                              *
*    Jeff Molofee's Revised OpenGL Basecode    *
*  Huge Thanks To Maxwell Sayles & Peter Puck  *
*            http://nehe.gamedev.net           *
*                     2001                     *
*                                              *
***********************************************/

#include <Windowsx.h>

#include "aopengl.h"														// Header File For The NeHeGL Basecode

#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

static bool g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static bool g_createFullScreen;											// If TRUE, Then Create Fullscreen

void TerminateApplication (GL_Window* window)							// Terminate The Application
{
	PostMessage (window->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
	g_isProgramLooping = FALSE;											// Stop Looping Of The Program
}

void ToggleFullscreen (GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage (window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}

static int width;
static int height;

int get_portrait_width( void )
{
	return ( gDisplayPortrait >= 0 && gDisplayPortrait <= 1 ) ? height : width;
}

int get_portrait_height( void )
{
	return ( gDisplayPortrait >= 0 && gDisplayPortrait <= 1 ) ? width : height;
}

int get_landscape_width( void )
{
	return ( gDisplayPortrait >= 0 && gDisplayPortrait <= 1 ) ? width : height;
}

int get_landscape_height( void )
{
	return ( gDisplayPortrait >= 0 && gDisplayPortrait <= 1 ) ? height : width;
}

int get_width( void )
{
    return width;
}

int get_height( void )
{
    return height;
}

void ReshapeGL (int _width, int _height)									// Reshape The Window When It's Moved Or Resized
{
	width = _width;
	height = _height;

	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				// Reset The Current Viewport
	glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
	glLoadIdentity ();													// Reset The Projection Matrix
	gluPerspective (45.0f, (GLfloat)(width)/(GLfloat)(height),			// Calculate The Aspect Ratio Of The Window
					1.0f, 100.0f);
	glScalef( 1.0f, gPerspectiveAngleRatio, 1.0f );
	glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity ();													// Reset The Modelview Matrix
}

bool ChangeScreenResolution (int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory (&dmScreenSettings, sizeof (DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize				= sizeof (DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth		= width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight		= height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel		= bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings (&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}
	return TRUE;														// Display Change Was Successful, Return True
}

bool CreateWindowGL (GL_Window* window, bool noborders)									// This Code Creates Our OpenGL Window
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		window->init.bitsPerPixel,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	RECT windowRect = {0, 0, window->init.width, window->init.height};	// Define Our Window Coordinates

	Dinfo di = getDisplayInfo( gCenterScreenIndex );

	windowRect.bottom += di.rect.top + window->init.y0;
	windowRect.top += di.rect.top + window->init.y0;
	windowRect.left += di.rect.left + window->init.x0;
	windowRect.right += di.rect.left + window->init.x0;

	GLuint PixelFormat;													// Will Hold The Selected Pixel Format

	if (window->init.isFullScreen == TRUE)								// Fullscreen Requested, Try Changing Video Modes
	{
		if (ChangeScreenResolution (window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			MessageBox (HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							// Set isFullscreen To False (Windowed Mode)
		}
		else															// Otherwise (If Fullscreen Mode Was Successful)
		{
			ShowCursor (FALSE);											// Turn Off The Cursor
			windowStyle = WS_POPUP;										// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;						// Set The Extended Window Style To WS_EX_TOPMOST
		}																// (Top Window Covering Everything Else)
	}
	else																// If Fullscreen Was Not Selected
	{
		if( noborders || window->init.width >= getDisplayWidth() || window->init.height >= getDisplayHeight() )
		{
			windowStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		}
		// Adjust Window, Account For Window Borders
//		AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	// Create The OpenGL Window
	window->hWnd = CreateWindowEx (windowExtendedStyle,					// Extended Style
								   window->init.application->className,	// Class Name
								   window->init.title,					// Window Title
								   windowStyle,							// Window Style
								   windowRect.left, windowRect.top,		// Window X,Y Position
								   windowRect.right - windowRect.left,	// Window Width
								   windowRect.bottom - windowRect.top,	// Window Height
								   HWND_DESKTOP,						// Desktop Is Window's Parent
								   0,									// No Menu
								   window->init.application->hInstance, // Pass The Window Instance
								   window);

	if (window->hWnd == 0)												// Was Window Creation A Success?
	{
		return FALSE;													// If Not Return False
	}

	window->hDC = GetDC (window->hWnd);									// Grab A Device Context For This Window

	if (window->hDC == 0)												// Did We Get A Device Context?
	{
		// Failed
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	PixelFormat = ChoosePixelFormat (window->hDC, &pfd);				// Find A Compatible Pixel Format
	if (PixelFormat == 0)												// Did We Find A Compatible Format?
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	if (SetPixelFormat (window->hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	window->hRC = wglCreateContext (window->hDC);						// Try To Get A Rendering Context
	if (window->hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (wglMakeCurrent (window->hDC, window->hRC) == FALSE)
	{
		// Failed
		wglDeleteContext (window->hRC);									// Delete The Rendering Context
		window->hRC = 0;												// Zero The Rendering Context
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	ShowWindow (window->hWnd, SW_NORMAL);
	window->isVisible = TRUE;											// Set isVisible To True

    if( !( windowStyle & WS_CLIPCHILDREN ) )
    {
        ReshapeGL (window->init.width-16, window->init.height-45);				// Reshape Our GL Window
    }
    else
    {
        ReshapeGL (window->init.width, window->init.height);				// Reshape Our GL Window
    }

	ZeroMemory (window->keys, sizeof (Keys));							// Clear All Keys

	window->lastTickCount = timeGetTime();							// Get Tick Count

	setSync( true );

	return TRUE;														// Window Creating Was A Success
																		// Initialization Will Be Done In WM_CREATE
}

bool DestroyWindowGL (GL_Window* window)								// Destroy The OpenGL Window & Release Resources
{
	if (window->hWnd != 0)												// Does The Window Have A Handle?
	{	
		if (window->hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent (window->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (window->hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext (window->hRC);							// Release The Rendering Context
				window->hRC = 0;										// Zero The Rendering Context

			}
			ReleaseDC (window->hWnd, window->hDC);						// Release The Device Context
			window->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
	}

	if (window->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings (NULL,0);									// Switch Back To Desktop Resolution
		ShowCursor (TRUE);												// Show The Cursor
	}	
	return TRUE;														// Return True
}

static void
maintain_1080p_aspect( HWND hWnd, LONG wParam, RECT *r )
{
	RECT cr;
	GetClientRect( hWnd, &cr );
//	printf( "cr %d, %d, %d, %d\n", cr.left, cr.top, cr.right, cr.bottom );
//	printf( "rs %d, %d, %d, %d\n", r->left, r->top, r->right, r->bottom );
	int width = cr.right - cr.left;
	int height = cr.bottom - cr.top;
	F32 aspect = ((F32)width)/((F32)height);
	F32 goal_aspect = 1920.0f / 1080.0f;
	int newheight = int(((F32)width) / goal_aspect );
	int newwidth = int(((F32)height) * goal_aspect );
	switch( wParam )
	{
	case WMSZ_RIGHT:
	case WMSZ_LEFT:
		r->bottom = r->bottom + newheight - height;
		break;
	case WMSZ_BOTTOMRIGHT:
	case WMSZ_BOTTOMLEFT:
	case WMSZ_TOPLEFT:
	case WMSZ_TOPRIGHT:
		//					printf( "%d %d\n", newheight, height );
//		r->bottom = r->bottom + newheight - height;
//		r->right = r->right + newwidth - width;
		break;
	case WMSZ_TOP:
	case WMSZ_BOTTOM:
		r->right = r->right + newwidth - width;
		break;
	}
	if( cr.right - cr.left >= 1920 )
	{
		int extrax = ( ( r->right - r->left ) - ( cr.right - cr.left ) ) / 2;
		int extray_top = r->top - cr.top;
		int extray_bottom = ( r->bottom - r->top ) - ( cr.bottom - cr.top ) - extray_top;
		r->left = -extrax / 2;
		r->right = 1920 + extrax / 2;
		r->top = -extray_top;
		r->bottom = 1080 + extray_bottom;
	}
}

static HWND game_window;

HWND get_main_window( void )
{
	return game_window;
}

static bool sReturnFocusToHiddenWindow;

// Process Window Message Callbacks
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( !game_window && hWnd ) game_window = hWnd;
	// Get The Window Context
	GL_Window* window = (GL_Window*)(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (uMsg)														// Evaluate Window Message
	{
		case WM_SIZING:
			//maintain_1080p_aspect( hWnd, wParam, ( RECT * ) lParam );
			break;

		case WM_SYSCOMMAND:												// Intercept System Commands
		{
			switch (wParam)												// Check System Calls
			{
				case SC_SCREENSAVE:										// Screensaver Trying To Start?
				case SC_MONITORPOWER:									// Monitor Trying To Enter Powersave?
				return 0;												// Prevent From Happening
			}
			break;														// Exit
		}
		return 0;														// Return

		case WM_CREATE:													// Window Creation
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
			window = (GL_Window*)(creation->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(window));
		}
		return 0;														// Return

		case WM_CLOSE:													// Closing The Window
			TerminateApplication(window);								// Terminate The Application
		return 0;														// Return

		case WM_SIZE:													// Size Action Has Taken Place
			switch (wParam)												// Evaluate Size Action
			{
				case SIZE_MINIMIZED:									// Was Window Minimized?
					window->isVisible = FALSE;							// Set isVisible To False
				return 0;												// Return

				case SIZE_MAXIMIZED:									// Was Window Maximized?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return

				case SIZE_RESTORED:										// Was Window Restored?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return
			}
		break;															// Break

		case WM_KEYDOWN:												// Update Keyboard Buffers For Keys Pressed
//			printf( "keydown: %d\n", wParam );
			if ((wParam >= 0) && (wParam <= 255) )						// Is Key (wParam) In A Valid Range?
			{
				window->keys->keyDown [wParam] = TRUE;					// Set The Selected Key (wParam) To True
				window->keys->keyEdge [wParam] = TRUE;
//				return 0;												// Return
			}
		break;															// Break

		case WM_KEYUP:													// Update Keyboard Buffers For Keys Released
//			printf( "keyup: %d\n", wParam );
			if ((wParam >= 0) && (wParam <= 255) )						// Is Key (wParam) In A Valid Range?
			{
				if( !window->keys->keyDown[wParam] )
				{
					window->keys->keyEdge[wParam] = TRUE;
				}
				window->keys->keyDown [wParam] = FALSE;					// Set The Selected Key (wParam) To False
				window->keys->keyTrailingEdge[wParam] = TRUE;
//				return 0;												// Return
			}
		break;															// Break

		case WM_TOGGLEFULLSCREEN:										// Toggle FullScreen Mode On/Off
			g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
			PostMessage (hWnd, WM_QUIT, 0, 0);
		break;															// Break
	
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			if( gInputMode == INPUT_MODE_REGULAR_MOUSE )
			{
				static F32 lastx,lasty;
				U64 pid = 0xdeadbeef;
				U64 iid = 0xabcdabde;
				RECT wrect;
				GetClientRect(hWnd, &wrect);
				F32 x = F32(( GET_X_LPARAM( lParam ) - wrect.left ) * get_width() / ( wrect.right - wrect.left ));
				F32 y = F32(( GET_Y_LPARAM( lParam ) - wrect.top  ) * get_height() / ( wrect.bottom - wrect.top ));
				F32 dx = x - lastx;
				F32 dy = y - lasty;
//				printf( "x %f y %f dx %f dy %f\n", x, y, dx, dy );
				lastx = x;
				lasty = y;
			}
			break;

		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
			break;

		case WM_TOUCH:
			PlayerInput::TouchInput( hWnd, uMsg, wParam, lParam );
			break;

		case WM_INPUT:
			PlayerInput::RawInput( hWnd, uMsg, wParam, lParam );
			break;

		case WM_SETFOCUS:
		case WM_ACTIVATE:
            ReshapeGL( window->init.width, window->init.height );
			if( gInputMode != INPUT_MODE_REGULAR_MOUSE ) game_disable_regular_mouse_cursor();
            break;

		case WM_KILLFOCUS:
			game_enable_regular_mouse_cursor();
			break;

		case WM_WINDOWPOSCHANGED:
		case WM_MOVE:
		case WM_WINDOWPOSCHANGING:
		case WM_NCMOUSELEAVE:
		case WM_GETMINMAXINFO:
		case WM_CONTEXTMENU:
		case WM_NCCREATE:
		case WM_NCCALCSIZE:
		case WM_SHOWWINDOW:
		case WM_ACTIVATEAPP:
		case WM_NCACTIVATE:
		case WM_IME_SETCONTEXT:
		case WM_IME_NOTIFY:
		case WM_GETOBJECT:
		case WM_NCPAINT:
		case WM_ERASEBKGND:
		case WM_GETICON:
		case WM_PAINT:
		case WM_NCHITTEST:
		case WM_SETCURSOR:
		case WM_SYNCPAINT:
		case WM_SYSKEYDOWN:
			break;

		case WM_DEVICECHANGE:
			printf( "Received WM_DEVICECHANGE message - wParam is 0x%x\n", wParam );
			break;

		default:
			{
				volatile int a;
				a = 0;
			}
			printf( "Unknown WndProc event 0x%x\n", uMsg );
			break;
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);					// Pass Unhandled Messages To DefWindowProc
}

bool RegisterWindowClass (Application* application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
	// Register A Window Class
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory (&windowClass, sizeof (WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize			= sizeof (WNDCLASSEX);					// Size Of The windowClass Structure
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	windowClass.hInstance		= application->hInstance;				// Set The Instance
	windowClass.hbrBackground	= (HBRUSH)(NULL/*COLOR_APPWORKSPACE*/);			// Class Background Brush Color
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName	= application->className;				// Sets The Applications Classname
	if (RegisterClassEx (&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}

// Program Entry (WinMain)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application			application;									// Application Structure
	GL_Window			window;											// Window Structure
	Keys				keys;											// Key Structure
	bool				isMessagePumpActive;							// Message Pump Active?
	MSG					msg;											// Window Message Structure
	DWORD				tickCount;										// Used For The Tick Counter
	
	Game::loadOptions();
	bool disable_console = false;

	int argCount;
	LPWSTR *szArgList;
	szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);
	if( szArgList )
	{
		LocalFree(szArgList);
	}

//	perlmbed_init();

	// Fill Out Application Data
	application.className = "OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance

	// Fill Out Window
	ZeroMemory (&window, sizeof (GL_Window));							// Make Sure Memory Is Zeroed
	window.keys					= &keys;								// Window Key Structure
	window.init.application		= &application;							// Window Application
	window.init.title			= "Comcast";					// Window Title

	int dwidth = getDisplayWidth( gCenterScreenIndex );
	int dheight = getDisplayHeight( gCenterScreenIndex );

#if 1
	if( dheight >= 1080 && dwidth >= 1920 && !gStretchToFitLargerScreen )
	{
#if 0
		window.init.width = 1800;
		window.init.height = window.init.width*1080/1920;
#else
		if( gDisplayPortrait >= 0 && gDisplayPortrait <= 1 )
		{
			window.init.width = 1920;
			window.init.height = 1080;
		}
		else if( gDisplayPortrait == 2 )
		{
			window.init.height = getDisplayHeight( gCenterScreenIndex ) - 40;
			if( window.init.height > 1920 ) window.init.height = 1920;
			window.init.width = 1080 * window.init.height / 1920;
		}

		if( gWindowJustification == 0 ) window.init.x0 = 0;
		else if( gWindowJustification == 1 ) window.init.x0 = ( dwidth - window.init.width );
		else window.init.x0 = ( dwidth - window.init.width ) / 2;

		window.init.y0 = ( dheight - window.init.height ) / 2;
#endif
	}
	else
	{
		window.init.width = getDisplayWidth( gCenterScreenIndex );									// Window Width
		window.init.height = getDisplayHeight( gCenterScreenIndex );									// Window Height
		if( gDisplayPortrait == 2 )
		{
			U32 tmp = getDisplayHeight( gCenterScreenIndex ) - 40;
			window.init.height = tmp;
			window.init.width = ( tmp * 1080 ) / 1920;
		}

		if( gWindowJustification == 0 ) window.init.x0 = 0;
		else if( gWindowJustification == 1 ) window.init.x0 = ( dwidth - window.init.width );
		else window.init.x0 = ( dwidth - window.init.width ) / 2;
		window.init.y0 = 0;
	}
#else
//	Chip::setOption( CHIPS_USE_RAW_MOUSE_INPUT );
	window.init.width = 1920-640;
	window.init.height = 1080*window.init.width/1920;
#endif
	window.init.bitsPerPixel	= 16;									// Bits Per Pixel
	window.init.isFullScreen	= TRUE;									// Fullscreen? (Set To TRUE)

	ZeroMemory (&keys, sizeof (Keys));									// Zero keys Structure

	// Ask The User If They Want To Start In FullScreen Mode?
	if (1 || MessageBox (HWND_DESKTOP, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode
	}

	// Register A Class For Our Window To Use
	if (RegisterWindowClass (&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBox (HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														// Terminate Application
	}

	hidden_window_key_input_init();

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE
	g_createFullScreen = window.init.isFullScreen;						// g_createFullScreen Is Set To User Default

	if( !disable_console )
	{
		console_init();
	}

	while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		// Create A Window
		window.init.isFullScreen = g_createFullScreen;					// Set Init Param Of Window Creation To Fullscreen?
		if (CreateWindowGL (&window, true ) == TRUE)							// Was Window Creation Successful?
		{
			// At This Point We Should Have A Window That Is Setup To Render OpenGL
			if (game_initialize (&window, &keys) == FALSE)					// Call User Intialization
			{
				// Failure
				TerminateApplication (&window);							// Close Window, This Will Handle The Shutdown
			}
			else														// Otherwise (Start The Message Pump)
			{	// Initialize was a success
				window.lastTickCount = timeGetTime();
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					// Success Creating Window.  Check For Window Messages
					if (PeekMessage (&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT)						// Is The Message A WM_QUIT Message?
						{
							DispatchMessage (&msg);						// If Not, Dispatch The Message
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							isMessagePumpActive = FALSE;				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						if (window.isVisible == FALSE)					// If Window Is Not Visible
						{
							WaitMessage ();								// Application Is Minimized Wait For A Message
						}
						else											// If Window Is Visible
						{
							// Process Application Loop
							tickCount = timeGetTime();				// Get The Tick Count
							U32 dtms = tickCount - window.lastTickCount;
							static U32 lastdtms;
							if( dtms > 1000 )
							{
								dtms = lastdtms;
							}
							F32 speedk = 60.0f/F32(get_refresh_frequency());
							if( dtms > 33 )
							{
								U32 newdtms = U32( 16.6f * speedk );
								U32 dtmsdelta = dtms - newdtms;
								input_adjust_for_pause( dtmsdelta );
							}
							game_update( dtms );	// Update The Counter
							window.lastTickCount = tickCount;			// Set Last Count To Current Count
							if( !game_disable_drawing() )
							{
								draw_everything();									// Draw Our Scene
								SwapBuffers (window.hDC);					// Swap Buffers (Double Buffering)
							}
							game_reset_keyboard_input_edges();
							lastdtms = dtms;
						}
					}
				}														// Loop While isMessagePumpActive == TRUE
			}															// If (Initialize (...

			DestroyWindowGL (&window);									// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			MessageBox (HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)

	UnregisterClass (application.className, application.hInstance);		// UnRegister Window Class

//	perlmbed_cleanup();

	return 0;
}																		// End Of WinMain()
