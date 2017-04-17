#ifndef __GAME_H__
#define __GAME_H__

enum GAME_ENUM
{
	GAME_DEFAULT=0,

	NUM_GAMES,
};

class Game
{
public:
	Game();
	~Game();

public:
	static void loadOptions( void );
	static void saveOptions( void );
};

extern Keys *g_keys;

void game_draw_debug_collision( void );
void game_draw( void );
bool game_disable_drawing( void );

bool game_initialize (GL_Window* window, Keys* keys);	// Performs All Your Initialization
void game_disable_regular_mouse_cursor( void );
void game_enable_regular_mouse_cursor( void );

void game_deinitialize (void);							// Performs All Your DeInitialization

void game_update (DWORD milliseconds);					// Perform Motion Updates
void game_reset_keyboard_input_edges( void );

void game_update( void );

void game_select_channel( int channel_idx, bool fullscreen ); // Channel index from 0 through 8 for the 3x3 matrix -- just a temporary routine

GAME_ENUM game_playing( void );

extern F32 gFrameRate; // currently 60.0 and 120.0 are the only values on this variable

#endif/* !__GAME_H__ */
