#ifndef __AIMAGE_H__
#define __AIMAGE_H__

#include "freeimage.h"

class AImage
{
public:
	AImage( const char *filename, bool mipmap=false, U32 transparent_color = 0x00000000 );
	AImage( const char *cubemap_filenames[6], bool mipmap=false, U32 transparent_color = 0x00000000 );
	~AImage();

private:
	void init( bool mipmap, U32 &transparent_color );

	int width;
	int height;

	F32 oo_width;
	F32 oo_height;

	U32 glname;

	int nimages;
	FIBITMAP *bitmaps[6];  // Freeimage structure
	U08      *(texDatas[6]); // Texture data in format needed by opengl
	
	char     *filename;
	const char *basefilename;

	bool	  mipmap;
	bool      cubemap;

public:
	void texdef2d( void );
	void texdefCubemap( void );
	void unload( void );

	const char *getFilename( void ) const { return filename; }

	int getWidth( void ) { return width; }
	int getHeight( void ) { return height; }
	F32 getWidthf( void ) { return F32(width); }
	F32 getHeightf( void ) { return F32(height); }


	U32 getColor( int x, int y );
	void setColor( int x, int y, U32 color );

	int getGLName( void ) { return glname; }
	void bindtex( void );

	void adjustHSV( F32 hue, F32 sat, F32 val );

	static int getVersion( void );

	static AImage *findExisting( const char *filename );
};

#endif/* !__AIMAGE_H__ */