#ifndef __INPUT_H__
#define __INPUT_H__

enum
{
	INPUT_MODE_MULTI_MOUSE,
	INPUT_MODE_MULTI_TOUCH,
	INPUT_MODE_MULTI_TOUCH_MULTI_MOUSE,
	INPUT_MODE_REGULAR_MOUSE,
};

enum INPUT_TYPE
{
	INPUT_TYPE_MOUSE = 0,
};

class PlayerInput
{
public:
	PlayerInput( U64 input_id, INPUT_TYPE=INPUT_TYPE_MOUSE );
	~PlayerInput();

private:
	INPUT_TYPE t;
	U64 input_id;
	U64 player_id;
	RECT r; // rectangular region of allowable X/Y values
	F32 x,y;
	F32 dx,dy;
	F32 irot[2][2]; // 2d rotation matrix for player input
	bool showcursor;
	U32 buttons;
	bool mb[8];
	bool unused;
	U32 vscroll;
	U32 hscroll;

public:
	U32 getButtons( void ) { return buttons; }
	void markUnused( void ) { unused = true; }

public:
	U32 deliverRawData( U32 ms, int dx, int dy, U32 buttons, bool showcursor=true, bool from_network=false );
	U32 deliverTouchData( U32 ms, int x, int y, U32 buttons, bool showcursor=false );

	static PlayerInput *get( U64 input_id );
	static PlayerInput *getUnused( U64 new_input_id );
	static void draw( void ); // draw the cursors and what-not
	static void RawInput( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static void TouchInput( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	int getX() const { return int(x); }
	int getY() const { return int(y); }
	U32 getButton( int i ) const { return mb[i%3]; }
};

extern U32 gInputMode;

U32 timeSinceLastMovement( void );

void input_init( void );

void hidden_window_key_input_init( void );
void input_init( void );
void input_reset( void );
void input_draw( void ); // debug draw routines for high speed input

class KeyInput
{
public:
	KeyInput();
	~KeyInput();

	static DWORD WINAPI MyThreadFunction( LPVOID lpParam );

private:
	static void openWindow( void );
	static HWND hwnd;
	static WNDCLASSEX wcx;
};

enum ButtonEnum
{
	BUTT_MOVE_LEFT,
	BUTT_MOVE_RIGHT,
	BUTT_MOVE_UP,
	BUTT_MOVE_DOWN,
	BUTT_MOVE_FORWARD,
	BUTT_MOVE_BACKWARD,
	BUTT_START,
	BUTT_BUTTON1,
	BUTT_BUTTON2,
};

void input_advance( U32 ms );
bool input_button_pressed( ButtonEnum b );
void input_button_addevent( ButtonEnum b, bool onoff );
void input_adjust_for_pause( U32 dtms );

#define SPIN_MAXHIST 8

struct Input // input mechanism for four-way joystick with AB button arrangement
{
public:
	bool pleft,pright,pup,pdown,pbut1,pbut2;
	bool  left, right, up, down, but1, but2;
	bool  ledge, redge, uedge, dedge;
	bool ltedge,rtedge,utedge,dtedge;
	bool  b1edge, b2edge;
	bool b1tedge,b2tedge;
	bool edge,bedge;

	void zerostick( void )
	{
		left = right = up = down = false;
	}
	void update( void )
	{
		pleft = left;
		pright = right;
		pup = up;
		pdown = down;
		pbut1 = but1;
		pbut2 = but2;

		left =  input_button_pressed( BUTT_MOVE_LEFT );
		right = input_button_pressed( BUTT_MOVE_RIGHT );
		up =    input_button_pressed( BUTT_MOVE_UP );
		down =  input_button_pressed( BUTT_MOVE_DOWN );
		but1 =  input_button_pressed( BUTT_BUTTON1 );
		but2 =  input_button_pressed( BUTT_BUTTON2 );

		ledge = left && !pleft;
		redge = right && !pright;
		uedge = up && !pup;
		dedge = down && !pdown;

		ltedge = !left && pleft;
		rtedge = !right && pright;
		utedge = !up && pup;
		dtedge = !down && pdown;

		b1edge = but1 && !pbut1;
		b2edge = but2 && !pbut2;

		b1tedge = !but1 && pbut1;
		b2tedge = !but2 && pbut2;

		edge = ledge || redge || uedge || dedge;
		bedge = b1edge || b2edge;
	}
};

#endif /*! __INPUT_H__ */