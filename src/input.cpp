#include "stdafx.h"

U32 gInputMode;
static U32 sLastMovement;
static U32 sPausedAmount;

static bool CALLBACK _terminateAppEnum( HWND hwnd, LPARAM lParam )
{
	DWORD dwID;

	GetWindowThreadProcessId( hwnd, &dwID );

	if( dwID == ( DWORD ) lParam )
	{
		PostMessage( hwnd, WM_CLOSE, 0, 0 );
	}

	return TRUE;
}

struct Nugget
{
	U64 usec;
	U32 msec;
	U08 data;
};

const int sNumNuggets = 1024;
static int sNuggetHead;
static int sNuggetTail;
static Nugget sNuggets[sNumNuggets];

static U32 _pc2ms( U64 val ) // NOTE: this function is not yet tested and has not worked yet
{
	static F64 pff64;
	static U64 pfu64;

	if( pff64 == 0.0 ) pff64 = F64(qpf());
	if( pfu64 == 0 ) pfu64 = qpf();

	static U64 qpc0ms;

	if( !qpc0ms )
	{
		qpc0ms = qpc();
		qpc0ms -= U64((F64(rtmsec())/1000.0)*pff64);
	}

	U64 delta_qpc = val - qpc0ms;
	return U32( ( delta_qpc * 1000ULL ) / pfu64 );
}

static void _stuff( U08 data )
{
	Nugget &n = sNuggets[sNuggetHead];
	n.data = data;
	n.usec = rtusec();
	n.msec = rtmsec();
	sNuggetHead++;
	if( sNuggetHead >= sNumNuggets ) sNuggetHead = 0;
	if( sNuggetHead == sNuggetTail )
	{
		printf( "NUGGET FIFO FULL!\n" );
	}
}

static bool _grab( U32 *msec, U64 *usec, U08 *data )
{
	if( sNuggetHead == sNuggetTail ) return false;
	
	Nugget &n = sNuggets[sNuggetTail];
	*msec = n.msec;
	*data = n.data;
	*usec = n.usec;
	sNuggetTail++;
	if( sNuggetTail >= sNumNuggets ) sNuggetTail = 0;

	return true;
}

void input_init( void )
{
	static RAWINPUTDEVICE *devs;
	static RAWINPUTDEVICELIST *dList;
	U32 numDevices = 0;
	int ndevs = 0;

	GetRawInputDeviceList( NULL, &numDevices, sizeof( RAWINPUTDEVICELIST ) );
	printf( "There are %d raw input devices attached to the system\n", numDevices );
	if( !dList ) dList = (RAWINPUTDEVICELIST *) calloc( numDevices, sizeof( RAWINPUTDEVICELIST ) );
	if( !devs ) devs = (RAWINPUTDEVICE *) calloc( numDevices, sizeof( RAWINPUTDEVICE ) );

	GetRawInputDeviceList( dList, &numDevices, sizeof( RAWINPUTDEVICELIST ) );

	if( 1 )
	{
		int i;
		const char *tname[] = { "MOUSE", "KEYBOARD", "HID" };

		for( i=0; i<int(numDevices); i++ )
		{
			printf( "raw input type %s\n", tname[dList[i].dwType] ); // , dList[i].hDevice );
			RID_DEVICE_INFO info = {0};
			char name[1024] = {0};
	
			U32 di_size = info.cbSize = sizeof( RID_DEVICE_INFO );
			GetRawInputDeviceInfo( dList[i].hDevice, RIDI_DEVICEINFO, &info, &di_size );

			U32 nm_size = sizeof( name ) - 1;
			GetRawInputDeviceInfo( dList[i].hDevice, RIDI_DEVICENAME, name, &nm_size );

			printf( "device name: %s\n", name );
			if( info.dwType == RIM_TYPEMOUSE )
			{
				printf( "mouse id:%d numbut:%d srate:%d hashw:%d\n",
					info.mouse.dwId,
					info.mouse.dwNumberOfButtons,
					info.mouse.dwSampleRate,
					info.mouse.fHasHorizontalWheel );

				static bool mouseregistered;
				if(	!mouseregistered )
				{
					// Register for mouse raw input
					devs[ndevs].usUsagePage = 1;
					devs[ndevs].usUsage = 2;
					devs[ndevs].dwFlags = 0;
					devs[ndevs].hwndTarget=NULL;
					ndevs++;
					mouseregistered = true;
				}
			}
			else if( info.dwType == RIM_TYPEHID )
			{
				if( info.hid.dwVendorId == 0x0801 )
				{
					printf( "Found MagTek card reader\n" );
				}
				printf( "HID vid: %08x, pid: %08x, ver: %d usagepage: %d usage: %d\n",
					info.hid.dwVendorId, info.hid.dwProductId, info.hid.dwVendorId, info.hid.usUsagePage, info.hid.usUsage );

				devs[ndevs].usUsagePage = info.hid.usUsagePage;
				devs[ndevs].usUsage = info.hid.usUsage;
				devs[ndevs].dwFlags = 0;
				devs[ndevs].hwndTarget = NULL;
				ndevs++;
			}
			else if( info.dwType == RIM_TYPEKEYBOARD )
			{
				printf( "keyboard type: %d subtype: %d mode: %d funckeys: %d indicators: %d totalkeys: %d\n",
					info.keyboard.dwType, info.keyboard.dwSubType, info.keyboard.dwKeyboardMode, info.keyboard.dwNumberOfFunctionKeys, info.keyboard.dwNumberOfIndicators, info.keyboard.dwNumberOfKeysTotal );

				static bool keyboardregistered;

				if( !keyboardregistered )
				{
					devs[ndevs].usUsagePage = 1;
					devs[ndevs].usUsage = 6;
					devs[ndevs].dwFlags = 0;
					devs[ndevs].hwndTarget = NULL;
					ndevs++;
					keyboardregistered = true;
				}
			}
			else
			{
				printf( "Unknown device type\n" );
			}
		}
	}

	if( ndevs ) RegisterRawInputDevices( devs, ndevs, sizeof( RAWINPUTDEVICE ) );
	RegisterTouchWindow( get_main_window(), TWF_WANTPALM );
}

static void input_remove_callback( const void *_input )
{
	PlayerInput *input = ( PlayerInput * ) _input;
	if( input ) delete input;
}

Alist inputs( input_remove_callback );

PlayerInput::PlayerInput( U64 _input_id, INPUT_TYPE itype )
{
	t = itype;
	unused = false;
	showcursor = true;
	input_id = _input_id;

	F32 brpos[3];

	brpos[0] = 1920 / 2;
	brpos[1] = 0;

	x = brpos[0];
	y = brpos[1];

	memset( irot, 0, sizeof( irot ) );
	
	buttons = 0;

	r.top = 0;
	r.bottom = get_height();
	r.left = 0;
	r.right = get_width();

	hscroll = 0;
	vscroll = 0;

	memset( mb, 0, sizeof( mb ) );

	inputs += this;
}

PlayerInput::~PlayerInput()
{
}

PlayerInput *PlayerInput::getUnused( U64 new_input_id )
{
	int i;
	int uidx = -1;

	for( i=0; i<inputs; i++ )
	{
		PlayerInput *pi = ( PlayerInput * ) inputs[i];
		if( pi->unused )
		{
			pi->input_id = new_input_id;
			pi->unused = false;
			return pi;
		}
	}

	return NULL;
}

PlayerInput *PlayerInput::get( U64 input_id )
{
	int i;
	int uidx = -1;

	for( i=0; i<inputs; i++ )
	{
		PlayerInput *pi = ( PlayerInput * ) inputs[i];

		if( pi->input_id == input_id ) return pi;
	}

	return NULL;
}

static bool sDisableTransforms = true;

U32
PlayerInput::deliverRawData( U32 ms, int _dx, int _dy, U32 _buttons, bool _showcursor, bool from_network )
{
	U32 rval = 0;
	bool moved = false;

	_dy = (-(_dy));

	dx = F32(_dx);
	dy = F32(_dy);
	buttons = _buttons;
	int i;
	for( i=0; i<sizeof(mb); i++ )
	{
		if( buttons & (1<<(i<<1)) ) mb[i] = 1;
		else if( buttons & (2<<(i<<1)) ) mb[i] = 0;
	}
	if( mb[5] )
	{
		vscroll += ( S32(buttons) >> 16 );
		moved = true;
	}
	if( mb[6] )
	{
		hscroll += ( S32(buttons) >> 16 );
		moved = true;
	}
	
	if( dx != 0 || dy != 0 ) moved = true;

	F32 tdx = dx;
	F32 tdy = dy;

	// perform per-player rotations
	if( gInputMode == INPUT_MODE_MULTI_MOUSE && !sDisableTransforms )
	{
		tdx = irot[0][0] * dx + irot[1][0] * dy;
		tdy = irot[0][1] * dx + irot[1][1] * dy;
	}

	dx = tdx;
	dy = tdy;

	x += dx;
	y += dy;

	if( x < r.left ) x = F32(r.left);
	if( x > r.right ) x = F32(r.right);
	if( y < r.top ) y = F32(r.top);
	if( y > r.bottom ) y = F32(r.bottom);

	F32 tx = x;
	F32 ty = y;

	return rval;
}

U32
timeSinceLastMovement( void )
{
	return msec() - sLastMovement;
}

U32
PlayerInput::deliverTouchData( U32 msec, int abs_x, int abs_y, U32 buttons, bool _showcursor )
{
	U32 rval = 0;

	static int initted;
	static Dinfo di;

	if( !initted )
	{
		di = getDisplayInfo( gCenterScreenIndex );
	}

	abs_x -= di.rect.left;
	abs_y -= di.rect.top;

	abs_x = ( abs_x * 1920 ) / get_width();
	abs_y = ( abs_y * 1080 ) / get_height();

	abs_y = 1080 - abs_y;

	dx = abs_x - x;
	dy = abs_y - y;

//	printf( "deliverTouchData : %08x %08d %.1f %.1f %d %d %u\n", this, msec, x, y, abs_x, abs_y, buttons );
	rval = deliverRawData( msec, int(dx), int(-dy), buttons );
	showcursor = _showcursor;

	return rval;
}

AImage *cursor;

// position from the top left of the tip of the cursor
// positive x to the right positive y down
static F32 _cursor_tip_pos[2] = { 11.0f, 8.0f };

void
PlayerInput::draw( void )
{
	int i;
	Afont f;
	static F32 hw,hh;

	glDepthMask( GL_FALSE );
	glDisable( GL_DEPTH_TEST );

	if( !cursor )
	{
		cursor = new AImage( file_find( "arrow_cursor.png" ) );
		hw = cursor->getWidth() * 0.5f;
		hh = cursor->getHeight() * 0.5f;
	}

	glEnable( GL_TEXTURE_2D );

	for( i=0; i<inputs; i++ )
	{
		PlayerInput *pi = ( PlayerInput * ) inputs[i];

		if( !pi->showcursor ) continue;
		cursor->bindtex();
		glPushMatrix();
		glTranslatef( (F32)pi->getX(), (F32)pi->getY(), 0.0f );

		if( gInputMode == INPUT_MODE_MULTI_MOUSE && !sDisableTransforms )
		{
			F32 m[4][4] = {0};
			m[0][0] = pi->irot[0][0];
			m[0][1] = pi->irot[0][1];
			m[1][0] = pi->irot[1][0];
			m[1][1] = pi->irot[1][1];
			m[2][2] = m[3][3] = 1;
			glMultMatrixf( &m[0][0] );
		}

		glTranslatef( hw-_cursor_tip_pos[0], -hh+_cursor_tip_pos[1], 0.0f );
		draw_textured_frame( 0.0f, 0.0f, hw, hh, 0xffffffff );
//		f.pos(0,0);
//		f.printf( "%08x", pi->getButtons() );

		glPopMatrix();
	}
}

void PlayerInput::TouchInput( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( gInputMode != INPUT_MODE_MULTI_TOUCH && gInputMode != INPUT_MODE_MULTI_TOUCH_MULTI_MOUSE ) return;

	int ninputs = (int) wParam;
	TOUCHINPUT *input;

	input = ( TOUCHINPUT * ) alloca( sizeof( TOUCHINPUT ) * ninputs );
	GetTouchInputInfo( (HTOUCHINPUT) lParam, ninputs, input, sizeof( TOUCHINPUT ) );
	int i;

	for( i=0; i<ninputs; i++ )
	{
		TOUCHINPUT &in = input[i];

		U64 unique_id = in.dwID + 0x60000ULL;

		PlayerInput *pi = PlayerInput::get( unique_id );

		if( !pi )
		{
			static int unique_idx;
			pi = new PlayerInput( unique_id+unique_idx );
		}
		
		U32 buttons = 0;
		if( in.dwFlags & TOUCHEVENTF_DOWN ) buttons |= 1;
		if( in.dwFlags & TOUCHEVENTF_UP )   buttons |= 2;
//		if( in.dwFlags & TOUCHEVENTF_MOVE )

		pi->deliverTouchData( msec(), in.x/100, in.y/100, buttons );

		// in.cxContact, in.cyContact, in.dwExtraInfo, in.dwFlags, in.dwID, in.dwMask, in.dwTime, in.hSource, in.x, in.y
//		printf( "touch input %d: pinput 0x%x %d %d (%02x)\n",i,  pi, in.x, in.y, in.dwFlags );
	}

	CloseTouchInputHandle( (HTOUCHINPUT) lParam );
}

void PlayerInput::RawInput( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( gInputMode != INPUT_MODE_MULTI_MOUSE && gInputMode != INPUT_MODE_MULTI_TOUCH_MULTI_MOUSE ) return;

	UINT dwSize;

    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, 
                    sizeof(RAWINPUTHEADER));
    
	LPBYTE lpb = ( LPBYTE ) alloca( dwSize );
    if (lpb == NULL) 
    {
        return;
    } 

    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, 
         sizeof(RAWINPUTHEADER)) != dwSize )
         OutputDebugString (TEXT("GetRawInputData doesn't return correct size !\n")); 

    RAWINPUT* raw = (RAWINPUT*)lpb;

	
	if( (raw->header.dwType == RIM_TYPEMOUSE) )
    {
		UINT di_size,rval;
		
		if( 0 ) 
		{
			RID_DEVICE_INFO info;
	
			di_size = sizeof( RID_DEVICE_INFO );
			rval = GetRawInputDeviceInfo( raw->header.hDevice, RIDI_DEVICEINFO, &info, &di_size );

			printf( "mouse id:%d numbut:%d srate:%d hashw:%d\n",
				info.mouse.dwId,
				info.mouse.dwNumberOfButtons,
				info.mouse.dwSampleRate,
				info.mouse.fHasHorizontalWheel );
		}

		U32 cksum = 0;
		
		{
			char name[1024];
			name[0] = 0;
			di_size = sizeof(name);
			rval =  GetRawInputDeviceInfo( raw->header.hDevice, RIDI_DEVICENAME, &name[0], &di_size );
			cksum = cksum_data( 0, name, strlen( name ) );
//			printf( "mouse name:%s nlen:%d cksum:%08x\n", name, di_size, cksum );
		}

		if( raw->data.mouse.usButtonData )
		{
			static int a;
			a = 3;
		}
		if( 0 ) printf("Mouse: dv=%x Flg=%08x But=%04x BFlg=%08x BtnDat=%08x RwBut=%08x dx=%04x dy=%04x xtra=%08x\n", 
			raw->header.hDevice,
            raw->data.mouse.usFlags, 
            raw->data.mouse.ulButtons, 
            raw->data.mouse.usButtonFlags, 
            raw->data.mouse.usButtonData, 
            raw->data.mouse.ulRawButtons, 
            raw->data.mouse.lLastX, 
            raw->data.mouse.lLastY, 
            raw->data.mouse.ulExtraInformation);
	
		U64 input_id = cksum;
		U64 player_id = 0; // Player::input_id_to_player_id( cksum );

		if( !player_id )
		{
			static volatile int a;
			a = 0;
		}
		PlayerInput *pi = PlayerInput::get( input_id );

		if( !pi )
		{
			RID_DEVICE_INFO info;
	
			di_size = sizeof( RID_DEVICE_INFO );
			rval = GetRawInputDeviceInfo( raw->header.hDevice, RIDI_DEVICEINFO, &info, &di_size );

			printf( "mouse id:%d numbut:%d srate:%d hashw:%d\n",
				info.mouse.dwId,
				info.mouse.dwNumberOfButtons,
				info.mouse.dwSampleRate,
				info.mouse.fHasHorizontalWheel );

			INPUT_TYPE t = INPUT_TYPE_MOUSE;

			pi = new PlayerInput( input_id, t );
		}

		pi->deliverRawData( msec(),
			raw->data.mouse.lLastX,
			raw->data.mouse.lLastY,
			raw->data.mouse.ulButtons );
    } 
	else if( (raw->header.dwType == RIM_TYPEHID) )
	{
//		printf( "RAW HID input received\n" );
//		printf( "data size: %d", raw->data.hid.dwSizeHid );

		int i;

		for( i=0; i<int(raw->data.hid.dwSizeHid); i++ )
		{
	//		if( !( i&31 ) ) printf( "\n" );
//			printf( "%02x ", raw->data.hid.bRawData[i] );
		}
//		printf( "\n" );
	}
	else if( (raw->header.dwType == RIM_TYPEKEYBOARD) )
	{
		static int lastvkey;
		static int lastflag;

//		printf( "Raw keyboard data received %d %x\n", raw->data.keyboard.Flags, raw->data.keyboard.VKey );

		if( 0 )
		{
			printf(" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n", 
				raw->data.keyboard.MakeCode, 
				raw->data.keyboard.Flags, 
				raw->data.keyboard.Reserved, 
				raw->data.keyboard.ExtraInformation, 
				raw->data.keyboard.Message, 
				raw->data.keyboard.VKey );
		}

		lastvkey = raw->data.keyboard.VKey;
		lastflag = raw->data.keyboard.Flags;
	}
}

static KeyInput *ki;

DWORD WINAPI KeyInput::MyThreadFunction( LPVOID lpParam )
{
	if( ki )
	{
		printf( "Ki already exists\n" );
	}

	if( !ki ) ki = new KeyInput();

	MSG msg;

	while( 1 )
	{
		if(PeekMessage (&msg, NULL, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
#ifdef TWO_WINDOWS
			if( GetForegroundWindow() == get_main_window() )
			{
				printf( "Grabbing focus away from main window to the hidden input window\n" );
				SetForegroundWindow( hwnd );
			}
			else
#endif
				Sleep(1);
		}
	}

	return 0;
}

void hidden_window_key_input_init( void )
{
	static DWORD threadId;

	if( threadId )
	{
		printf( "Error: Thread already exists\n" );
	}

	CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            KeyInput::MyThreadFunction,       // thread function name
            0,          // argument to thread function 
            0,                      // use default creation flags 
            &threadId);
}

static LRESULT CALLBACK sMainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
    {
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_TOUCH:
		case WM_KEYDOWN:
		case WM_KEYUP:
			// just forward these messages over to the main window as they aren't time sensitive -- just the raw data is of concern to use the timing
			PostMessage( get_main_window(), msg, wParam, lParam );
			break;

		case WM_INPUT:
			PlayerInput::RawInput( hwnd, msg, wParam, lParam );
			break;

		default:
//			printf( "%-6dMSG %4x on win lParam = 0x%08x wParam = %d wnd = %d\n", msec(), msg, lParam, wParam, hwnd );
			break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND KeyInput::hwnd;
WNDCLASSEX KeyInput::wcx;

KeyInput::KeyInput()
{
	hwnd = 0;

	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_DBLCLKS;
	wcx.lpfnWndProc = sMainWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = 0;
	wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = (HBRUSH) (COLOR_WINDOW);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = "A";
	wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wcx))
	{
		printf( "Error\n" );
		exit(0);
	}

#ifdef TWO_WINDOWS
	openWindow();

	if (!hwnd)
	{
		printf( "Error\n" );
		exit(0);
	}

	ShowWindow(hwnd,SW_SHOW);
#endif
}

KeyInput::~KeyInput()
{
	CloseWindow( hwnd );
	hwnd = NULL;
}

void
KeyInput::openWindow( void )
{
	if( hwnd )
	{
		printf( "Error: window already opened\n" );
		CloseWindow( hwnd );
		hwnd = NULL;
	}

	hwnd = CreateWindowEx(0, "A", "B",
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, 1, 1, HWND_DESKTOP, 0, 0, 0 );
}

void
input_draw( void )
{
	draw_in_screen_coord();
}

static int sIndex;
static bool sStartPressed;
static bool sLeftPressed;
static bool sRightPressed;
static bool sUpPressed;
static bool sDownPressed;
static bool sForwardPressed;
static bool sBackPressed;
static bool sBut1Pressed;
static bool sBut2Pressed;

bool input_button_pressed( ButtonEnum b )
{
	if( b == BUTT_START ) return sStartPressed;
	else if( b == BUTT_MOVE_LEFT ) return sLeftPressed;
	else if( b == BUTT_MOVE_RIGHT ) return sRightPressed;
	else if( b == BUTT_MOVE_UP ) return sUpPressed;
	else if( b == BUTT_MOVE_DOWN ) return sDownPressed;
	else if( b == BUTT_MOVE_FORWARD ) return sForwardPressed;
	else if( b == BUTT_MOVE_BACKWARD ) return sBackPressed;
	else if( b == BUTT_BUTTON1 ) return sBut1Pressed;
	else if( b == BUTT_BUTTON2 ) return sBut2Pressed;
	return false;
}

void input_button_addevent( ButtonEnum b, bool onoff )
{
}

void input_adjust_for_pause( U32 dtms )
{
	sPausedAmount += dtms;
}

void
input_write( FILE *f )
{
}

void
input_reset( void )
{
}

