#include "stdafx.h"

#ifdef WIN32

#include "console.h"
#include <conio.h>
#include <io.h>
#include <FCNTL.H>

int sPosX = 1920-640;
int sPosY = 0;
int sWidth = 640;
int sHeight = 1080;

static const WORD MAX_CONSOLE_LINES = 5000;

void RedirectIoToConsole()
{
#ifndef DEBUG_CONSOLE_DISABLE
	int hConHandle;	
	long lStdHandle;
	
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	
	FILE* fp = NULL;
	
	// allocate a console for this app	
	AllocConsole();

	// http://softwareforums.intel.com/ids/board/message?board.id=15&message.id=2971&page=1&view=by_threading
	// would like to see messages without mousing around each time I run
	{
		char name[256];
		HWND win;
		GetConsoleTitle( name, 255 );
		win = FindWindow( NULL, name );
		if( getNumDisplays() > 1 )
		{
			Dinfo di = getDisplayInfo( gCenterScreenIndex ? 0 : 1 );
			sPosY = di.rect.top;
			sPosX = di.rect.left;
		}
		else if( getNumDisplays() == 1 )
		{
			Dinfo di = getDisplayInfo( 0 );
			sPosY = 0;
			if( di.width == 3840 && gDisplayPortrait == 2 )
			{
				if( gWindowJustification == 0 ) sPosX = 1080;
				else if( gWindowJustification == 1 ) sPosX = di.width-1080-640;
				else sPosX = di.width - 640;
			}
			else
			{
				sPosX = di.width - 640;
			}
			sHeight = di.height-40;
		}
		SetWindowPos( win, HWND_TOP, sPosX, sPosY, sWidth, sHeight, 0 );
		ShowWindow( win, SW_SHOWNORMAL );
	}
	
	// set the screen buffer to be big enough to let us scroll text
	
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),	&coninfo);
	
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	
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
	
//	ios::sync_with_stdio();
	
#endif //DEBUG_CONSOLE_DISABLE
}
#endif // WIN32

static int sbLibDebugStarted = 0;

// functions
// library routines
void console_init(void)
{
#ifdef WIN32
	RedirectIoToConsole();
#endif

	sbLibDebugStarted = TRUE;
}

void console_pos( int x, int y, int width, int height )
{
	sPosX = x;
	sPosY = y;
	sWidth = width;
	sHeight = height;
}

void console_cleanup(void)
{
#ifndef DEBUG_CONSOLE_DISABLE
#ifdef WIN32
	FreeConsole();
#endif
#endif //DEBUG_CONSOLE_DISABLE
	sbLibDebugStarted = 0;
}

int console_started(void)
{
	return sbLibDebugStarted;
}
