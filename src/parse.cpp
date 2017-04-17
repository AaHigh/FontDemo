#include "stdafx.h"

void expect( const char *&p, char value )
{
	if( *p != value )
	{
		printf( "Expected %c character not found\n", value );
	}
	p++;
}

bool see( const char *str1, const char *str2 )
{
	size_t len = strlen( str2 );
	return !strncmp( str1, str2, len );
}

bool seei( const char *str1, const char *str2 )
{
	size_t len = strlen( str2 );
	return !_strnicmp( str1, str2, len );
}

void expect( const char *&p, const char *string, const char *string2 )
{
	size_t len = strlen( string );
	if( !see( p, string ) && ( !string2 || !see( p, string2 ) ) )
	{
		printf( "Expecting %s string", string );
		if( string2 ) printf( "or %s string", string2 );
		printf( "\n" );
	}
	else p += len;
}

void expecti( const char *&p, const char *string, const char *string2 )
{
	size_t len = strlen( string );
	if( !seei( p, string ) && ( !string2 || !seei( p, string2 ) ) )
	{
		printf( "Expecting %s string", string );
		if( string2 ) printf( "or %s string", string2 );
		printf( "\n" );
	}
	else p += len;
}

bool truefalse( const char *&p )
{
	if( !strncmp( p, "true", 4 ) ) return true;
	if( !strncmp( p, "false", 5 ) ) return false;
	if( *p == '0' ) return false;
	if( *p == '1' ) return true;
	return false;
}

bool iswhite( const char p )
{
	return p == 0 || p == ' ' || p == '\t' || p == '\r' || p == '\n';
}

void skip( const char *&p, int c )
{
	if( !c ) while( iswhite(*p) && *p ) p++;
	else
	{
		while( *p && *p != c ) p++;
		if( *p ) p++;
		skip(p);
	}
}

inline int noop_toupper(int c)
{
	return c;
}

int skip(const char *&p, const char *string,bool ignorecase)
{
	int(*tup)(int) = ignorecase ? toupper : noop_toupper;
	do
	{
		int i = 0;
		while (*p && (*tup)(*p) != (*tup)(string[i]))
		{
			p++;
		}
		const char *savep = p;
		while (*p && string[i] && (*tup)(*p) == (*tup)(string[i]))
		{
			p++;
			i++;
		}
		if (!string[i])
		{
			p = savep + i;
			return 1;
		}
		else if (*p && (*tup)(*p) != (*tup)(string[i]) )
		{
			i = 0;
			p = savep + 1;
		}
	}
	while (*p);

	return 0;
}

F32 readF32( const char *&p )
{
	F32 f = ( F32 ) atof( p );
	while( ( *p >= '0' && *p <= '9' ) || *p == '.' || *p == '-' || *p == '+' ) p++;
	skip( p );
	return f;
}

S32 readS32( const char *&cp )
{
	bool negative = false;
	S32 num = 0;

	while( iswhite( *cp ) ) cp++;
	if( *cp == '-' )
	{
		negative = true;
	}
	while( *cp >= '0' && *cp <= '9' )
	{
		num *= 10;
		num += *cp - '0';
		cp++;
	}

	skip(cp);

	if( negative ) num = (-(num));

	return num;
}

U32 readU32( const char *&cp )
{
	U32 num = 0;

	while( *cp >= '0' && *cp <= '9' )
	{
		num *= 10;
		num += *cp - '0';
		cp++;
	}

	skip(cp);

	return num;
}

U64 readU64( const char *&p )
{
	U64 u = 0;
	skip( p );
	while( ( *p >= '0' && *p <= '9' ) )
	{
		u = u * 10 + ( *p - '0' );
		p++;
	}
	skip( p );
	return u;
}

void skipq( const char *&p )
{
	skip( p, '\"' );
}

const char *readWord( const char *&p )
{
	static char _buffer[256];
	skip(p);
	int count = 0;
	char *buffer = _buffer;

	while( !iswhite(*p) && count < sizeof( _buffer ) )
	{
		*buffer++ = *p++;
		count++;
	}

	*buffer = 0;
	
	return _buffer;
}

void chop( char *cp )
{
	size_t i;
	size_t n = strlen( cp );
	for( i=n-1; i>=0; i-- )
	{
		if( cp[i] == '\n' || cp[i] == '\r' )
		{
			cp[i] = 0;
		}
		else
		{
			return;
		}
	}
}

