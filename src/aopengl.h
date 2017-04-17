// - adapated from NeHeGL Header file sample

#ifndef GL_FRAMEWORK__INCLUDED
#define GL_FRAMEWORK__INCLUDED

#include <windows.h>								// Header File For Windows

typedef struct
{
	bool keyDown [256];
	bool keyEdge [256];
	bool keyTrailingEdge[256];
}
Keys;

typedef struct
{
	HINSTANCE		hInstance;						// Application Instance
	const char*		className;						// Application ClassName
}
Application;

typedef struct
{													// Window Creation Info
	Application*		application;				// Application Structure
	char*				title;						// Window Title
	int					width;						// Width
	int					height;						// Height
	int					x0;
	int					y0;
	int					bitsPerPixel;				// Bits Per Pixel
	bool				isFullScreen;				// FullScreen?
}
GL_WindowInit;

typedef struct
{
	Keys*				keys;						// Key Structure
	HWND				hWnd;						// Window Handle
	HDC					hDC;						// Device Context
	HGLRC				hRC;						// Rendering Context
	GL_WindowInit		init;						// Window Init
	bool				isVisible;					// Window Visible?
	DWORD				lastTickCount;				// Tick Counter
}
GL_Window;

void TerminateApplication (GL_Window* window);

void ToggleFullscreen (GL_Window* window);

HWND get_main_window( void );
int get_width( void );
int get_height( void );
int get_portrait_width( void ); // returns smaller of two width and height
int get_portrait_height( void ); // returns larger of two width and height
int get_landscape_width( void ); // returns larger of two width and height
int get_landscape_height( void ); // returns smaller of two width and height

LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif