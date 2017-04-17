#include "stdafx.h"

static const char *
foo( const char **dirs, int &found )
{
	if( found >= 0 ) return dirs[found];

	int i;

	for( i=0; true; i++ )
	{
		if( !dirs[i] )
		{
			printf( "Could not find data directory\n" );
			system( "pause" );
			exit( 0 );
		}

		if( dir_exists( dirs[i] ) )
		{
			found = i;
			return dirs[found];
		}
	}

	return NULL;
}

const char *
data_root_prefix( void )
{
	static int found = -1;
	static const char *dirs[] =
	{
		"../../data",
		"../data",
		NULL,
	};

	return foo( dirs, found );
}

const char *
writeable_data_root_prefix( void )
{
	static int found = -1;
	static const char *dirs[] =
	{
		"P:/data",
		"../../data",
		"../data",
		NULL,
	};

	return foo( dirs, found );
}

const char *
sound_root_prefix( void )
{
	static int found = -1;
	static const char *dirs[] =
	{
		"../../data/sounds",
		"../data/sounds",
		"data/sounds",
	};

	return foo( dirs, found );
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf( "Hello world\n" );

	int i;

	scanf_s( "%d", &i );
	
	return i;
}