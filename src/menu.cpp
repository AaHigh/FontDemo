#include "stdafx.h"

U32 gDisplayPortrait = 1;
F32 gVolume = 1.0f;
U32 gUsePerspective;
F32 gCameraDistance = 1900.0f;
F32 gPerspectiveCameraAngle = 53.0f;
F32 gPerspectiveAngleRatio = 0.78f;
F32 gViewAxisOffset;
U32 gPhysicsSpeed = 20;
U32 gGameWeArePlaying;
U32 gAllowQuickExit;
U32 gCenterScreenIndex;
U32 gScreenDiagonal = 46;
U32 gStretchToFitLargerScreen;
U32 gShowFrameDtms;
U32 gShowAudioWaveforms;
F32 gDebugScale = 1.0f;
F32 gDebugPos[2] = { 0.0f, 0.0f };
U32 gDebugSloMo = 1;
U32 gWindowJustification = 0;
U32 gDisableGitPull = 1;

F32 sMaxWidth;
F32 sMaxHeight;

enum MenuType
{
	MT_MENU,
	MT_STRING,
	MT_FLOAT,
	MT_INTEGER,
	MT_ENUM,
	MT_BOOLEAN,
	MT_ONOFF,
	MT_TOGGLE,
	MT_FILENAME,
};

struct EnumName
{
	int enum_id;
	const char *ename;
};

class Menu
{
public:
	Menu( MenuType _t, const char *_name, void *data=NULL, char *file_extension=NULL );
	~Menu();

private:
	MenuType t;
	const char *name;

	char *directory;
	char **filenames;

	union
	{
		void (*toggle)(void);
		U32 *bval;
		int *ival;
		U32 *eval;
		F32 *fval;
		char **cpp;
		void *data;
	} u;

	enum
	{
		MENU_MIN,
		MENU_MAX,
		MENU_INC,
		MENU_MINMAXINC,
	};

	union
	{
		F32 f[MENU_MINMAXINC];
		U32 u[MENU_MINMAXINC];
		S32 i[MENU_MINMAXINC];
	}
	minMaxInc;

	size_t str_alloc_len;

	int mSelected;
	Alist items;

public:
	void add( Menu *child );
	void addEnum( int enum_id, const char *enum_name );
	U32  getEnumVal( void );
	void setMinMaxInc( F32 min, F32 max, F32 inc=1.0f );
	void setMinMaxInc( U32 min, U32 max, U32 inc=1 );
	void up( void );
	void down( void );
	void right( void );
	void left( void );
	void increment( void );
	void decrement( void );
	void toggle( void );
	Menu *getChild( int index ) { return ( Menu * ) items[index]; }
	void setData( void *data );
	MenuType getType( void ) { return t; }
	int  getNumItems( void ) { return items; }

public:
	void update( void );
	void draw( F32 hpos, F32 vpos );
};

Menu::Menu( MenuType _t, const char *_name, void *_data, char *file_extension )
{
	t = _t;
	name = _name;

	memset( &minMaxInc.u, 0, sizeof( minMaxInc.u ) );
	filenames = NULL;

	setData( _data );
	str_alloc_len = 0;

	mSelected = 0;
}

Menu::~Menu()
{
	if( t == MT_STRING && str_alloc_len > 0 && u.cpp && *u.cpp )
	{
		free( *u.cpp );
		*u.cpp = NULL;
	}

	if( t == MT_ENUM )
	{
		int i;

		for( i=0; i<items; i++ )
		{
			EnumName *e = ( EnumName * ) items[i];
			delete e;
		}
	}

	if( directory )
	{
		free( directory );
		directory = NULL;
	}
	if( filenames )
	{
		free( filenames );
		filenames = NULL;
	}
}

void Menu::setMinMaxInc( F32 min, F32 max, F32 inc )
{
	if( t == MT_FLOAT )
	{
		minMaxInc.f[MENU_MIN] = min;
		minMaxInc.f[MENU_MAX] = max;
		minMaxInc.f[MENU_INC] = inc;
	}
	else if( t == MT_INTEGER )
	{
		minMaxInc.i[MENU_MIN] = int(min);
		minMaxInc.i[MENU_MAX] = int(max);
		minMaxInc.i[MENU_INC] = int(inc);
	}
}

void Menu::setMinMaxInc( U32 min, U32 max, U32 inc )
{
	if( t == MT_FLOAT )
	{
		minMaxInc.f[MENU_MIN] = F32(min);
		minMaxInc.f[MENU_MAX] = F32(max);
		minMaxInc.f[MENU_INC] = F32(inc);
	}
	else if( t == MT_INTEGER || t == MT_FILENAME )
	{
		minMaxInc.i[MENU_MIN] = min;
		minMaxInc.i[MENU_MAX] = max;
		minMaxInc.i[MENU_INC] = inc;
	}
}

void Menu::addEnum( int enum_id, const char *_enum_name )
{
	if( t != MT_ENUM ) return;

	EnumName *e = new EnumName;
	e->ename = _enum_name;
	e->enum_id = enum_id;

	if( u.eval && *u.eval == enum_id )
	{
		mSelected = items;
	}

	items += e;
}

U32 Menu::getEnumVal( void )
{
	if( t != MT_ENUM ) return 0;

	EnumName *e = ( EnumName * ) items[mSelected];
	return e ? e->enum_id : 0;
}

void Menu::right( void )
{
	increment();
}

void Menu::left( void )
{
	decrement();
}

void Menu::up( void )
{
	if( t == MT_ENUM )
	{
		decrement();
	}
	else
	{
		increment();
	}
}

void Menu::down( void )
{
	if( t == MT_ENUM )
	{
		increment();
	}
	else
	{
		decrement();
	}
}

void Menu::toggle( void )
{
	if( t == MT_TOGGLE )
	{
		u.toggle();
	}
	else if( t == MT_ONOFF || t == MT_BOOLEAN )
	{
		if( u.bval ) *u.bval = !(*u.bval);
	}
}

void Menu::increment( void )
{
	if( t == MT_ENUM )
	{
		if( mSelected == items-1 ) mSelected = 0;
		else mSelected++;
		if( u.eval ) *u.eval = getEnumVal();
	}
	else if( t == MT_FLOAT )
	{
		*u.fval += minMaxInc.f[MENU_INC];
		if( *u.fval > minMaxInc.f[MENU_MAX] )
		{
			*u.fval = minMaxInc.f[MENU_MAX];
		}
	}
	else if( t == MT_INTEGER || t == MT_FILENAME )
	{
		*u.ival += minMaxInc.i[MENU_INC];
		if( *u.ival > minMaxInc.i[MENU_MAX] )
		{
			*u.ival = minMaxInc.i[MENU_MAX];
		}
	}
}

void Menu::decrement( void )
{
	if( t == MT_ENUM )
	{
		if( mSelected == 0 )  mSelected = items-1;
		else mSelected--;
		if( u.eval ) *u.eval = getEnumVal();
	}
	else if( t == MT_FLOAT )
	{
		*u.fval -= minMaxInc.f[MENU_INC];
		if( *u.fval < minMaxInc.f[MENU_MIN] )
		{
			*u.fval = minMaxInc.f[MENU_MIN];
		}
	}
	else if( t == MT_INTEGER || t == MT_FILENAME )
	{
		*u.ival -= minMaxInc.i[MENU_INC];
		if( *u.ival < minMaxInc.i[MENU_MIN] )
		{
			*u.ival = minMaxInc.i[MENU_MIN];
		}
	}
}

void Menu::setData( void *_data )
{
	u.data = _data;
}

void Menu::add( Menu *child )
{
	items += child;
}

#define MAX_MENU_LEVELS 16

static Menu *sSelectedItem;

static int   sMenuLevel;
static Menu *sParentMenu[MAX_MENU_LEVELS];
static int   sParentItem[MAX_MENU_LEVELS];

static Menu *sCurrentMenu;
static int   sCurrentItem;

void Menu::update( void )
{
	int i;

	if( t == MT_ENUM )
	{
		EnumName *selected_en = ( EnumName * ) items[mSelected];
		int i;

		if( selected_en->enum_id != *u.eval )
		{
			for( i=0; i<items; i++ )
			{
				EnumName *en = ( EnumName * ) items[i];

				if( en->enum_id == *u.eval )
				{
					mSelected = i;
					break;
				}
			}
		}
	}
	else if( t == MT_MENU )
	{
		for( i=0; i<items; i++ )
		{
			Menu *child = ( Menu * ) items[i];
			child->update();
		}
	}
}

void Menu::draw( F32 hpos, F32 vpos )
{
	Afont f;
	f.font( AF_CENTURY20 );
	f.pos( hpos, vpos );
	f.print( name );
	static F32 charwidth;
	
	if( !charwidth ) charwidth = f.length("0");

	if( f.getx() + charwidth > sMaxWidth ) sMaxWidth = f.getx() + charwidth;

	if( this == sSelectedItem )
	{
		f.print( " - " );

		if( t == MT_ENUM )
		{
			int i;

			for( i=0; i<items; i++ )
			{
				EnumName *en = ( EnumName * ) items[i];
				f.color( i == mSelected ? 0xff0000ff : 0xffffffff );
				F32 len = f.printf( "%s\n", en->ename );
				if( f.getx() + len + charwidth > sMaxWidth ) sMaxWidth = f.getx() + len + charwidth;
			}
		}
		else if( t == MT_FLOAT )
		{
			const F32 width = 400.0f;
			F32 min = minMaxInc.f[MENU_MIN];
			F32 max = minMaxInc.f[MENU_MAX];
			F32 inc = minMaxInc.f[MENU_INC];
			F32 val = *(u.fval);
			F32 range = max - min - inc;
			F32 side = 8.0f;

			f.just( AF_CENTER );
			F32 fl;

			vpos -= 24.0f;

			for( fl=min; fl<max; fl+=inc )
			{
				f.pos( hpos + side + ( width - side * 2.0f ) * ( ( fl - min ) / range ), vpos );
				f.print( ( fl < val ) ? "|" : "-" );
			}
			f.pos( hpos, vpos );
			f.print( "[" );
			f.pos( hpos + width, vpos );
			f.print( "]" );
			if( f.getx() + charwidth > sMaxWidth ) sMaxWidth = f.getx() + charwidth;
			f.pos( hpos + width * 0.5f, vpos - 24.0f );
			f.printf( "%0.3f", *(u.fval) );
		}
		else if( t == MT_INTEGER || t == MT_FILENAME )
		{
			const F32 width = 400.0f;
			int min = minMaxInc.i[MENU_MIN];
			int max = minMaxInc.i[MENU_MAX];
			int inc = minMaxInc.i[MENU_INC];
			int val = *(u.ival);
			int range = max - min - inc;
			F32 side = 8.0f;

			f.just( AF_CENTER );
			int il;

			vpos -= 24.0f;

			for( il=min; il<max; il+=inc )
			{
				f.pos( hpos + side + ( width - side * 2.0f ) * ( F32( il - min ) / F32(range) ), vpos );
				f.print( ( il < val ) ? "|" : "-" );
			}
			f.pos( hpos, vpos );
			f.print( "[" );
			f.pos( hpos + width, vpos );
			f.print( "]" );
			if( f.getx() + charwidth > sMaxWidth ) sMaxWidth = f.getx() + charwidth;
			f.pos( hpos + width * 0.5f, vpos - 24.0f );
		
			if( t == MT_INTEGER )
			{
				f.printf( "%d", *(u.ival) );
			}
			else
			{
				f.printf( "%s", filenames[ *(u.ival) ] );
			}
		}
	}
	else if( this == sCurrentMenu )
	{
		hpos += 24.0f;

		int i;

		for( i=0; i<items; i++ )
		{
			Menu *child = ( Menu * ) items[i];
			vpos -= 24.0f;

			if( i == sCurrentItem )
			{
				f.pos( hpos - 24, vpos );
				f.print( ">" );
			}
			child->draw( hpos, vpos );
			if( sSelectedItem == child ) break;
		}
	}
	else if( t == MT_BOOLEAN )
	{
		f.printf( " - %s", ( u.bval && *u.bval ) ? "true" : "false" );
	}
	else if( t == MT_ONOFF )
	{
		f.printf( " - %s", ( u.bval && *u.bval ) ? "on" : "off" );
	}
	else if( t == MT_TOGGLE )
	{
		f.printf( " - press enter to cycle possible settings" );
	}
	else if( t == MT_INTEGER )
	{
		if( u.ival ) f.printf( " - %d", *u.ival );
	}
	else if( t == MT_FLOAT )
	{
		if( u.fval ) f.printf( " - %.3f", *u.fval );
	}
	else if( t == MT_ENUM )
	{
		EnumName *e = ( EnumName * ) items[mSelected];

		f.printf( " - %s", e->ename );
	}
	else if( t == MT_FILENAME )
	{
		f.printf( " - %s", filenames[ *u.ival ] );
	}

	if( get_portrait_height() - f.gety() + f.height() > sMaxHeight ) sMaxHeight = get_portrait_height() - f.gety() + f.height();
	if( f.getx() + charwidth > sMaxWidth ) sMaxWidth = f.getx() + charwidth;
}

Menu *gMainMenu;
Menu *gSoundMenu;
Menu *gSoundVolume;
Menu *gInputMenu;
Menu *gVisualsMenu;
Menu *gDebugMenu;
Menu *gCommonGameMenu;
Menu *gInputMenuMode;

bool sDrawMenu;
bool sInitted;

static void menu_item_wrap( void )
{
	if( !sCurrentMenu )
	{
		sCurrentItem = 0;
		return;
	}

	if( sCurrentItem < 0 )
	{
		sCurrentItem = sCurrentMenu->getNumItems() - 1;
	}
	if( sCurrentItem >= sCurrentMenu->getNumItems() )
	{
		sCurrentItem = 0;
	}
}

void menu_right( void )
{
	if( sSelectedItem )
	{
		sSelectedItem->right();
	}
}

void menu_left( void )
{
	if( sSelectedItem )
	{
		sSelectedItem->left();
	}
}

void menu_up( void )
{
	if( sSelectedItem )
	{
		sSelectedItem->up();
	}
	else
	{
		sCurrentItem--;
		menu_item_wrap();
	}
}

void menu_down( void )
{
	if( sSelectedItem )
	{
		sSelectedItem->down();
	}
	else
	{
		sCurrentItem++;
		menu_item_wrap();
	}
}

void menu_select( void )
{
	if( sSelectedItem )
	{
		sSelectedItem = NULL;
	}
	else
	{
		if( sCurrentMenu->getChild( sCurrentItem ) ) sSelectedItem = sCurrentMenu->getChild( sCurrentItem );
		else return;

		if( sSelectedItem->getType() == MT_MENU )
		{
			sParentMenu[sMenuLevel] = sCurrentMenu;
			sParentItem[sMenuLevel] = sCurrentItem;
			sMenuLevel++;
			sCurrentMenu = sSelectedItem;
			sCurrentItem = 0;
			sSelectedItem = NULL;
		}
		else if( sSelectedItem->getType() == MT_ONOFF || sSelectedItem->getType() == MT_TOGGLE )
		{
			sSelectedItem->toggle();
			sSelectedItem = NULL;
		}
	}
}

bool menu_displayed( void )
{
	return sDrawMenu;
}

void menu_escape( void )
{
	if( sSelectedItem )
	{
		sSelectedItem = NULL;
	}
	else
	{
		if( sMenuLevel > 0 )
		{
			--sMenuLevel;
			sCurrentMenu = sParentMenu[sMenuLevel];
			sCurrentItem = sParentItem[sMenuLevel];
		}
		else
		{
			Game::saveOptions();
			sDrawMenu = false;
		}
	}
}

static void menu_build( void )
{
	Menu *m = NULL;

	gDebugMenu = new Menu( MT_MENU, "Debug" );
	gDebugMenu->add( m = new Menu( MT_ONOFF, "show frame dtms", &gShowFrameDtms ) );
	gDebugMenu->add( m = new Menu( MT_ONOFF, "show audio waveforms", &gShowAudioWaveforms ) );
	gDebugMenu->add( m = new Menu( MT_FLOAT, "scale", &gDebugScale ) );
	m->setMinMaxInc( F32(0.1f), F32( 4.0f ), F32( 0.1f ) );
	gDebugMenu->add( m = new Menu( MT_FLOAT, "debug pos X", &gDebugPos[0] ) );
	m->setMinMaxInc( F32(-1024.0f), F32( 1024.0f ), F32( 1.0f ) );
	gDebugMenu->add( m = new Menu( MT_FLOAT, "debug pos Y", &gDebugPos[1] ) );
	m->setMinMaxInc( F32(-512.0f), F32( 512.0f ), F32( 1.0f ) );
	gDebugMenu->add( m = new Menu( MT_ENUM, "window justification", &gWindowJustification ) );
	m->addEnum( 0, "left" );
	m->addEnum( 1, "right" );
	m->addEnum( 2, "center" );
	gDebugMenu->add( m = new Menu( MT_ONOFF, "disable git pull", &gDisableGitPull ) );

	gVisualsMenu = new Menu( MT_MENU, "Visuals" );
	gVisualsMenu->add( m = new Menu( MT_ENUM, "Display Portrait", &gDisplayPortrait ) );
	m->addEnum( 0, "Landscape" );
	m->addEnum( 1, "Landscape 180" );
	m->addEnum( 2, "Portrait" );
	gVisualsMenu->add( m = new Menu( MT_INTEGER, "Screen index", &gCenterScreenIndex ) );
	m->setMinMaxInc( U32(0), U32(getNumDisplays()-1), U32(1) );
	gVisualsMenu->add( m = new Menu( MT_ENUM, "Use perspective", &gUsePerspective ) );
    m->addEnum( 0, "off" );
    m->addEnum( 1, "on" );
    m->addEnum( 2, "inverted perspective" );
	gVisualsMenu->add( m = new Menu( MT_FLOAT, "camera distance", &gCameraDistance ) ); m->setMinMaxInc( 200.0f, 40000.0f, 20.0f );
	gVisualsMenu->add( m = new Menu( MT_FLOAT, "camera angle", &gPerspectiveCameraAngle ) ); m->setMinMaxInc( 1.0f, 180.0f, 1.0f );
	gVisualsMenu->add( m = new Menu( MT_FLOAT, "camera angle ratio", &gPerspectiveAngleRatio ) ); m->setMinMaxInc( 0.01f, 2.0f, 0.01f );
	gVisualsMenu->add( m = new Menu( MT_FLOAT, "view axis offset", &gViewAxisOffset) ); m->setMinMaxInc( -2, 2, 0.02f );
	gVisualsMenu->add( m = new Menu( MT_INTEGER, "screen diagonal", &gScreenDiagonal ) ); m->setMinMaxInc( 21U, 100U, 1U );
	gVisualsMenu->add( m = new Menu( MT_ONOFF, "stretch fit", &gStretchToFitLargerScreen ) );

	gSoundMenu = new Menu( MT_MENU, "Sound menu" );
	gSoundVolume = new Menu( MT_FLOAT, "Sound volume", &gVolume );
	gSoundVolume->setMinMaxInc( 0.0f, 1.0f, 0.025f );
	gSoundVolume->setData( &gVolume );
	gSoundMenu->add( gSoundVolume );

    gInputMenu = new Menu( MT_MENU, "Input Menu" );

	gInputMenu->add( m = new Menu( MT_ONOFF, "Allow Quick Exit", &gAllowQuickExit ) );
	gInputMenu->add( m = new Menu( MT_ENUM, "Input mode", &gInputMode ) );
	m->addEnum( INPUT_MODE_REGULAR_MOUSE, "regular mouse" );
	m->addEnum( INPUT_MODE_MULTI_MOUSE, "multi-mouse" );
	m->addEnum( INPUT_MODE_MULTI_TOUCH_MULTI_MOUSE, "multi-touch + mouse" );
	m->addEnum( INPUT_MODE_MULTI_TOUCH, "multi-touch only" );

	gMainMenu = new Menu( MT_MENU, "Main menu" );

	gMainMenu->add( gSoundMenu );
	gMainMenu->add( gInputMenu );
	gMainMenu->add( gVisualsMenu );
	gMainMenu->add( gDebugMenu );

}

static void menu_init( void )
{
	menu_build();
}

void menu_start( void )
{
	if( !sInitted )
	{
		menu_init();
		sInitted = 1;
	}

	sCurrentMenu = gMainMenu;
	sSelectedItem = NULL;
	sMenuLevel = 0;
	sDrawMenu = true;
}

void menu_draw( void )
{
	glDisable( GL_DEPTH_TEST );
	draw_in_landscape_screen_coord();

	if( !sDrawMenu ) return;

	F32 h = get_portrait_height() - sMaxHeight * 0.5f;

	draw_untextured_frame( sMaxWidth * 0.5f, h, sMaxWidth * 0.5f + 4, sMaxHeight * 0.5f + 4, 0xff00003f );
	draw_untextured_frame( sMaxWidth * 0.5f, h, sMaxWidth * 0.5f, sMaxHeight * 0.5f, 0x00005f7f );

	sMaxWidth = 0;
	sMaxHeight = 0;

	if( sCurrentMenu ) sCurrentMenu->draw( 24.0f, get_height() - 24.0f );
}

void menu_update( void )
{
	if( sCurrentMenu ) sCurrentMenu->update();
}

void
factory_defaults( void )
{
	gUsePerspective = 1;
	gCameraDistance = 1900.0f;
	gPerspectiveCameraAngle = 53.0f;
	gPerspectiveAngleRatio = 0.78f;
	gViewAxisOffset = 0.0f;
	gCenterScreenIndex = 0;
	gScreenDiagonal = 39;
	gStretchToFitLargerScreen = 0;

	gVolume = 1.0f;
	gUsePerspective = 1;
	gCameraDistance = 1300.0f;
	gPhysicsSpeed = 17;
}