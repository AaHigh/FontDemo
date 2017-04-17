#include "stdafx.h"

GL_Window*	g_window;
Keys*		g_keys;
F32			gFrameRate = 60.0f;

static bool disable_drawing;
static bool sUseCheatResult;
static int sFullScreenIndex = -1;

static const int SAVE_FILE_VERSION_NUMBER = 1;

bool
game_disable_drawing( void )
{
	return disable_drawing;
}

void
game_draw_debug_collision( void )
{
}

static AImage *sComcastLogo;

static int sAudioSelected = -1;

void
draw_runtime( void )
{
	Afont f;
	U32 ms = msec();

	f.font( AF_CURRENCY );
	f.pos( get_width(), get_height() - 12 );
	f.just( AF_RIGHT );
	static int hours,minutes,seconds,milliseconds;

	static U32 stamp;
	U32 dtms = ms - stamp;
	stamp = ms;

	milliseconds += dtms;
	seconds += milliseconds / 1000;
	milliseconds = milliseconds % 1000;
	minutes += seconds / 60;
	seconds = seconds % 60;
	hours += minutes / 60;
	minutes = minutes % 60;

	f.printf( "%02d-%02d-%02d-%03d", hours, minutes, seconds, milliseconds % 1000 );
}

void
game_draw( void )
{
	if( disable_drawing ) return;
	static int game = -1;
	static AA *aa;
	bool useAA = false; // msec() & 512;

	draw_runtime();

	if( useAA && !aa )
	{
		aa = new AA();
	}

	if( useAA )
	{
		aa->setTarget();
	}

	Afont f;
	U32 stamp = timeGetTime();
	
	setupLighting();

	UseShader( SHADER_NONE );
	glEnable( GL_TEXTURE_2D );

	menu_draw();

	input_draw();

	if( useAA )
	{
		aa->copyToBB();
	}

//	f.pos( 10, 10 );

//	f.printf( "Drawing took %u msec\n", timeGetTime() - stamp );
}

bool game_initialize(GL_Window* window, Keys* keys)					// Any GL Init Code & User Initialiazation Goes Here
{
	g_window	= window;
	g_keys		= keys;

	init();

	// Start Of User Initialization
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);						// Black Background
	glClearDepth (1.0f);										// Depth Buffer Setup
	glEnable (GL_DEPTH_TEST);									// Enable Depth Testing
	glShadeModel (GL_SMOOTH);									// Select Smooth Shading
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate

	GLenum err = glewInit();
	colorshader_init();

	glTexEnvf( GL_TEXTURE_ENV, GL_ALPHA_SCALE, 2.0f );

	return TRUE;												// Return TRUE (Initialization Successful)
}

static RECT saveclip;

void game_disable_regular_mouse_cursor( void )
{
	HWND hWnd = get_main_window();
	if( !hWnd ) return;
	RECT wrect;
	GetWindowRect(hWnd, &wrect);

	// restrict the actual cursor to a small are up top
	RECT rect = { wrect.left+50,wrect.top+50,wrect.left+55,wrect.top+55 };
	ShowCursor( false );
	if( !saveclip.bottom )
	{
		GetClipCursor(&saveclip); 
	}
	ClipCursor( &rect );
}

void game_enable_regular_mouse_cursor( void )
{
	ShowCursor( true );
	if( saveclip.bottom || saveclip.top )
	{
		ClipCursor( &saveclip );
	}
}

static int strncasecmp( const char *str1, const char *str2, int n )
{
	assert( str1 && str2 );

	char *dup1 = ( char * ) alloca( n + 1 );
	char *dup2 = ( char * ) alloca( n + 1 );

	int i;

	for( i=0; i<n; i++ )
	{
		dup1[i] = tolower( str1[i] );
		if( !str1[i] ) break;
	}

	for( i=0; i<n; i++ )
	{
		dup2[i] = tolower( str2[i] );
		if( !str2[i] ) break;
	}

	return strncmp( dup1, dup2, n );
}

static U08 num1,num2;

static void game_calc_framerate( U32 ms )
{
	static int count240;
	static int count120;
	static int count60;
	static int count30;

	if( ms < 6 )
	{
		count30 = 0;
		count60 = 0;
		count120 = 0;
		if( count240 >= 10 ) gFrameRate = 240.0f;
		else count240++;
	}
	else if( ms < 10 )
	{
		count30 = 0;
		count60 = 0;
		count240 = 0;
		if( count120 >= 10 || gFrameRate > 120 ) gFrameRate = 120.0f;
		else count120++;
	}
	else if( ms < 20 )
	{
		count30 = 0;
		count120 = 0;
		count240 = 0;
		if( count60 >= 10 || gFrameRate > 60.0f ) gFrameRate = 60.0f;
		else count60++;
	}
	else if( ms < 36 )
	{
		count60 = 0;
		count120 = 0;
		count240 = 0;
		if( count30 >= 10 || gFrameRate > 30.0f ) gFrameRate = 30.0f;
		else count30++;
	}
}

static bool playing( void )
{
	return 0;
}

void game_update(DWORD milliseconds)								// Perform Motion Updates Here
{
	static U32 save;
	if( GetAsyncKeyState( VK_LCONTROL ) )
	{
#if 1
		if( save == 0 )
		{
			save = gPhysicsSpeed;
			gPhysicsSpeed = 1;
		}
#else
		milliseconds = 1000 / U32(gFrameRate);
		Sleep( 300 );
#endif
	}
	else if( save > 0 )
	{
		gPhysicsSpeed = save;
		save = 0;
	}

	game_calc_framerate( milliseconds );
	timers_update( milliseconds );

	setSwapInterval( 1 );

	if (g_keys->keyEdge[VK_ESCAPE] )					// Is ESC Being Pressed?
	{
		static U32 last_escape;
		static U32 msec_since_last_escape = 5000;
		if( gAllowQuickExit ) msec_since_last_escape = 500;

		if( menu_displayed() )
		{
			if( msec() - last_escape > 200 )
			{
				menu_escape();
			}
			last_escape = msec();
		}
		else if( !last_escape || ( msec() - last_escape > msec_since_last_escape ) )
		{
			TerminateApplication (g_window);						// Terminate The Program
		}
	}

	if (g_keys->keyEdge[VK_HOME] )
	{
		screengrab();
	}

	if( g_keys->keyEdge['1'] || g_keys->keyTrailingEdge['1'] )
	{
		input_button_addevent( BUTT_START, g_keys->keyDown['1'] );
	}


#if 0
	if (g_keys->keyDown [VK_F1] == TRUE)						// Is F1 Being Pressed?
	{
		static int sizeidx;
		F32 sizes[][2] =
		{
			{ 640, 360 },
			{ 960, 540 },
			{ 1920, 1080 },
		};

		printf( "Resize to %dx%d\n", sizes[sizeidx][0], sizes[sizeidx][1] );

//		ToggleFullscreen (g_window);							// Toggle Fullscreen Mode
	}
#endif

	int i;

	for( i=1; i<=6; i++ )
	{
		if( g_keys->keyEdge['0'+i] )
		{
			if( !num1 || !num2 )
			{
				num1 = num2 = i;
			}
			else
			{
				num1 = num2;
				num2 = i;
				sUseCheatResult = true;
			}
		}
	} 

	disable_drawing = ( 0 != g_keys->keyDown['D'] && g_keys->keyDown['8'] ); // must hold d key and the 8 key to disable drawing

	if( g_keys->keyEdge[VK_UP] )
	{
		menu_up();
	}
	if( g_keys->keyEdge[VK_DOWN] )
	{
		menu_down();
	}
	if( g_keys->keyEdge[VK_LEFT] )
	{
		menu_left();
	}
	if( g_keys->keyEdge[VK_RIGHT] )
	{
		menu_right();
	}
	if( g_keys->keyEdge[VK_RETURN] )
	{
		if( menu_displayed() )
		{
			menu_select();
		}
		else
		{
			menu_start();
		}
	}

	menu_update();
}

#define ru32(x) if( !strcmp(varname,#x) ) x=readU32(cp);
#define rs32(x) if( !strcmp(varname,#x) ) x=readS32(cp);
#define rf32(x) if( !strcmp(varname,#x) ) x=readF32(cp);

void Game::loadOptions( void )
{
	static bool loaded;
	U32 options = 0;

	if( loaded ) return;

	factory_defaults();

	FILE *f = fopen( file_find( "gameoptions.txt" ), "r" );

	while( f && !feof( f ) )
	{
		char buf[1024];

		fgets( buf, sizeof(buf)-1, f );
		const char *cp = buf;
		const char *varname = readWord( cp );
		skip(cp,'=');

		ru32(gShowAudioWaveforms);
		ru32(gPhysicsSpeed);
		ru32(gAllowQuickExit);
		ru32(gGameWeArePlaying);
		ru32(gDisplayPortrait);
		ru32(gUsePerspective);
		rf32(gCameraDistance);
		rf32(gPerspectiveCameraAngle);
		ru32(gCenterScreenIndex);
		rf32(gVolume);
		rf32(gPerspectiveAngleRatio);
		rf32(gViewAxisOffset);
		ru32(gScreenDiagonal);
		ru32(gStretchToFitLargerScreen);
		rf32(gDebugScale);
		rf32(gDebugPos[0]);
		rf32(gDebugPos[1]);
		ru32(gInputMode);
		ru32(gDebugSloMo);
		ru32(gWindowJustification);
	}

	if( f ) fclose( f );

	loaded = true;
}

#define wu32(x) fprintf(f,"%s = %u\n",#x,x)
#define ws32(x) fprintf(f,"%s = %d\n",#x,x)
#define wf32(x) fprintf(f,"%s = %.4f\n",#x,x)

void Game::saveOptions( void )
{
	U32 ver = SAVE_FILE_VERSION_NUMBER;

	FILE *f = fopen( file_find( "gameoptions.txt" ), "w" );

	F32 gDebugPos0 = gDebugPos[0];
	F32 gDebugPos1 = gDebugPos[1];

	wu32(gShowAudioWaveforms);
	wu32(gPhysicsSpeed);
	wu32(gAllowQuickExit);
	wu32(gGameWeArePlaying);
	wu32(gDisplayPortrait);
	wu32(gUsePerspective);
	wf32(gCameraDistance);
	wf32(gPerspectiveCameraAngle);
	wu32(gCenterScreenIndex);
	wf32(gVolume);
	wf32(gPerspectiveAngleRatio);
	wf32(gViewAxisOffset);
	wu32(gScreenDiagonal);
	wu32(gStretchToFitLargerScreen);
	wf32(gDebugScale);
	wf32(gDebugPos0);
	wf32(gDebugPos1);
	wu32(gInputMode);
	wu32(gDebugSloMo);
	wu32(gWindowJustification);

	fclose( f );
}

void game_reset_keyboard_input_edges( void )
{
	int i;

	for( i=0; i<256; i++ )
	{
		g_keys->keyTrailingEdge[i] = FALSE;
		g_keys->keyEdge[i] = FALSE;
	}
}

void game_select_channel( int channel_idx, bool fullscreen )
{
	if( channel_idx >= 0 && channel_idx <= 8 )
	{
		sAudioSelected = channel_idx + 1;
	}

	if( fullscreen )
	{
		sFullScreenIndex = channel_idx + 1;
	}
	else
	{
		sFullScreenIndex = -1;
	}
}

