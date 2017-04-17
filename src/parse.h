#ifndef __PARSE_H__
#define __PARSE_H__

void expect( const char *&p, char value );
bool see( const char *str1, const char *str2 );
bool seei( const char *str1, const char *str2 );
void expect( const char *&p, const char *string, const char *string2=NULL );
void expecti( const char *&p, const char *string, const char *string2=NULL );
bool truefalse( const char *&p );
bool iswhite( const char p );
void skip( const char *&p, int c=0 );
int skip(const char *&p, const char *string,bool ignorecase);
S32 readS32( const char *&p );
U32 readU32( const char *&p );
F32 readF32( const char *&p );
U64 readU64( const char *&p );
void skipq( const char *&p );
void chop( char *cp );
const char *readWord( const char *&p );

#endif /* !__PARSE_H__ */