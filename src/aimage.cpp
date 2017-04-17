#include "stdafx.h"

static U64 sTotalTexels;

Alist images;

static const char *mybasefilename( const char *filename )
{
	if( !filename ) return NULL;
	size_t n = strlen( filename );
	if( n>1 ) n--;
	while( filename[n-1] != '\\' && filename[n-1] != '/' )
	{
		n--;
	}

	return filename + n;
}

AImage::AImage( const char *_filename, bool _mipmap, U32 transparent_color )
{
	filename = strdup( _filename );
	basefilename = mybasefilename( filename );
	this->init( _mipmap, transparent_color );

	if( !filename )
	{
		printf( "No filename supplied to create AImage class\n" );
		return;
	}

	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
	if (fif == FIF_UNKNOWN)
	{
		printf( "Couldn't do a FreeImage operation on %s\n", filename );
		return;
	}

	nimages = 1;
	FIBITMAP *&bitmap = bitmaps[0];
	bitmap = FreeImage_Load(fif,filename);
	if (!bitmap)
	{
		printf( "Couldn't read bitmap for file %s\n", filename );
		return;
	}
	BYTE *data = FreeImage_GetBits(bitmap);
	if (!data) 
	{
		FreeImage_Unload(bitmap);       
		printf( "no data! filename %s\n", filename );
		return;
	}

	// get the size of our image
	int bpp    = FreeImage_GetBPP(bitmap)/8; // convert from bits to bytes, convert width from bytes in row, to just number of pixels in row
	width      = FreeImage_GetPitch(bitmap)/bpp;
	height     = FreeImage_GetHeight(bitmap);

	oo_width = 1.0f / width;
	oo_height= 1.0f / height;

	U08 *&texData = texDatas[0];

	// Create our texture data
	texData = (U08*) calloc( width*height, 4 );

	static U64 totalmem;

	if( !texData )
	{
		printf( "Allocated %llu already/couldn't alloc %d bytes for %s.\n", totalmem, width*height*4, filename );
		return;
	}
	else
	{
		totalmem += width * height * 4;
	}

	FreeImage_ConvertToRawBits(texData, bitmap, width*4, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, true );

	// texData now contains our image.  However we need to 
	// swap the red and blue color channels and flip the scanlines
	// in the vertical direction
	// This is because of the difference between how OpenGL and FreeImage
	// store image data internally.

	int linesize = width*4;

	U32 color;

	// flip red and blue color components
	// why this can't be done in the 5th, 6th, and 7th args to FreeImage_ConvertToRawBits .. I don't know
	for(int y=0; y < height; y++)
	{
		int pos = y*linesize;
		for(int x=0; x < width; x++)
		{
			int xpos = y*linesize + x*4;

			memcpy( &color, texData+xpos, 4 );

			if( ( color & 0xff000000 ) == 0 )
			{
				color = transparent_color & 0x00ffffff;
				memcpy( texData+xpos, &color, 4 );
			}

			unsigned char temp = texData[xpos];
			texData[xpos] = texData[xpos+2];
			texData[xpos+2] = temp;
		}       
	}

	texdef2d();

	if( 0 )
	{
		free( texData );
		texData = NULL;
	}

	sTotalTexels += width * height;

//	printf( "Total texels is %llu - %s is %u\n", sTotalTexels, _filename, width*height );

	images += this;
}

AImage::AImage( const char *cubemap_filenames[6], bool _mipmap, U32 transparent_color )
{
	this->init( _mipmap, transparent_color );

	cubemap = true;

	if( !( cubemap_filenames && 
		cubemap_filenames[0] && 
		cubemap_filenames[1] && 
		cubemap_filenames[2] && 
		cubemap_filenames[3] && 
		cubemap_filenames[4] && 
		cubemap_filenames[5] ) )
	{
		printf( "One or more cubemap filenames is missing\n" );
		return;
	}

	int i;

	for( i=0; i<6; i++ )
	{
		const char *filename = file_find( cubemap_filenames[i] );

		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
		if (fif == FIF_UNKNOWN)
		{
			printf( "Couldn't do a FreeImage operation on %s\n", filename );
			return;
		}

		FIBITMAP *&bitmap = bitmaps[i];
		
		bitmap = FreeImage_Load(fif,filename);
		if (!bitmap)
		{
			printf( "Couldn't read bitmap for file %s\n", filename );
			return;
		}
		BYTE *data = FreeImage_GetBits(bitmap);
		if (!data) 
		{
			FreeImage_Unload(bitmap);       
			printf( "no data! filename %s\n", filename );
			return;
		}

		// get the size of our image
		int bpp    = FreeImage_GetBPP(bitmap)/8; // convert from bits to bytes, convert width from bytes in row, to just number of pixels in row

		if( i==0 )
		{
			width = FreeImage_GetPitch(bitmap)/bpp;
			height = FreeImage_GetHeight(bitmap);
			oo_width = 1.0f / width;
			oo_height= 1.0f / height;
		}
		else
		{
			if( ( width != FreeImage_GetPitch(bitmap)/bpp ) || 
				( height != FreeImage_GetHeight(bitmap) ) )
			{
				printf( "texture size for all 6 textures must be the same!\n" );
				return;
			}
		}

		U08 *&texData = texDatas[i];

		// Create our texture data
		texData = (U08*) calloc( width*height, 4 );

		FreeImage_ConvertToRawBits(texData, bitmap, width*4, 32, 0, 0, 0, 1);

		// texData now contains our image.  However we need to 
		// swap the red and blue color channels and flip the scanlines
		// in the vertical direction
		// This is because of the difference between how OpenGL and FreeImage
		// store image data internally.

		int linesize = width*4;

		U32 color;

		// flip red and blue color components
		// why this can't be done in the 5th, 6th, and 7th args to FreeImage_ConvertToRawBits .. I don't know
		for(int y=0; y < height; y++)
		{
			int pos = y*linesize;
			for(int x=0; x < width; x++)
			{
				int xpos = y*linesize + x*4;

				memcpy( &color, texData+xpos, 4 );

				if( ( color & 0xff000000 ) == 0 )
				{
					color = transparent_color & 0x00ffffff;
					memcpy( texData+xpos, &color, 4 );
				}

				unsigned char temp = texData[xpos];
				texData[xpos] = texData[xpos+2];
				texData[xpos+2] = temp;
			}       
		}
	}

	texdefCubemap();

	images += this;
}

AImage::~AImage( )
{
	if( filename )
	{
		free( filename );
		filename = NULL;
	}

	unload();

	images -= this;
}

void
AImage::init( bool _mipmap, U32 &transparent_color )
{
	mipmap = _mipmap;
	cubemap = false;
	width = 0;
	height = 0;
	oo_width = 0.0f;
	oo_height = 0.0f;
	glname = 0;

	int i;

	for( i=0; i<6; i++ )
	{
		bitmaps[i] = NULL;
		texDatas[i] = NULL;
	}
   
	// Swap byte order
	transparent_color = ( ( transparent_color & 0x000000ff ) << 24 ) |
						( ( transparent_color & 0xffffff00 ) >>  8 );
}

void
AImage::unload( void )
{
	int i;

	for( i=5; i>=0; --i )
	{
		if( texDatas[i] )
		{
			free( texDatas[i] );
			texDatas[i] = NULL;
		}

		if( bitmaps[0] )
		{
			FreeImage_Unload(bitmaps[i]);
			bitmaps[i] = NULL;
		}
	}

	glDeleteTextures( 1, &glname );
}

void
AImage::texdef2d( void )
{
	glGenTextures( 1, &glname );
	glBindTexture( GL_TEXTURE_2D, glname );

	if( mipmap )
	{
		gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texDatas[0] );

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,width,height,0,GL_RGBA, GL_UNSIGNED_BYTE,texDatas[0]); 

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
	}
}

void
AImage::texdefCubemap( void )
{
	glGenTextures( 1, &glname );
	glBindTexture( GL_TEXTURE_CUBE_MAP, glname );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

	int i;

	for( i=0; i<6; i++ )
	{
		if( 0 && mipmap )
		{
			gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texDatas[i] );

			glTexParameteri(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);	// Linear Filtering
			glTexParameteri(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
		}
		else
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,width,height,0,GL_RGBA, GL_UNSIGNED_BYTE,texDatas[i]); 

//			glTexParameteri(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
//			glTexParameteri(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
		}
	}
}

U32
AImage::getColor( int x, int y )
{
	return ((U32*)texDatas[0])[ x + y * width ];
}

void
AImage::setColor( int x, int y, U32 color )
{
	((U32*)texDatas[0])[ x + y * width ] = color;
}
#define AIMAGE_VERSION 1

int
AImage::getVersion( void )
{
	const char *fiver;
	
	fiver = FreeImage_GetVersion();

    printf( "FreeImageVersion: %s\n", fiver );

	size_t len = strlen( fiver );
	char *cpy = ( char * ) alloca( len + 1 );
	strcpy( cpy, fiver );

	size_t i;
	int fimage_ver = 0;
	int aimage_mult = 1;
	for( i=0; i<len; i++ )
	{
		if( cpy[i] >= '0' && cpy[i] <= '9' )
		{
			fimage_ver = fimage_ver * 10 + cpy[i] - '0';
			aimage_mult *= 10;
		}
	}

	return AIMAGE_VERSION * aimage_mult + fimage_ver;
}

void
AImage::bindtex( void )
{
	glBindTexture( cubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, glname );
}

struct RGBColor
{
	U08 alpha;
	U08 blue;
	U08 green;
	U08 red;
};

static int kHueUndefined = -1;

void RGB2HSV( U32 color, int *piOutHue, int *piOutSaturation, int *piOutValue)
{
	short iMax, iMin;
	struct RGBColor tmp;
	tmp.alpha = ( color & 0xff000000 ) >> 24;
	tmp.blue  = ( color & 0xff0000 ) >> 16;
	tmp.green = ( color & 0xff00 ) >> 8;
	tmp.red   = ( color & 0xff ) >> 0;
	RGBColor *prgbcIn = ( RGBColor * ) &tmp;
	
	// Calculate the value component
	if (prgbcIn->red > prgbcIn->green)
	{
		iMax = prgbcIn->red;
		iMin = prgbcIn->green;
	}
	else
	{
		iMin = prgbcIn->red;
		iMax = prgbcIn->green;
	}
	
	if (prgbcIn->blue > iMax)
	{
		iMax = prgbcIn->blue;
	}
	
	if (prgbcIn->blue < iMin)
	{
		iMin = prgbcIn->blue;
	}
	
	*piOutValue = iMax;
	
	// Calculate the saturation component
	if (iMax != 0)
	{
		*piOutSaturation = 255 *
			(iMax - iMin) / iMax;
	}
	else
	{
		*piOutSaturation = 0;
	}

	// Calculate the hue
	if (*piOutSaturation == 0)
	{
		*piOutHue = kHueUndefined;
	}
	else
	{
		F32 fHue;
		F32 fDelta;
		fDelta = F32( iMax - iMin );
		if (prgbcIn->red == iMax)
		{
			fHue = (F32)(prgbcIn->green - prgbcIn->blue) / fDelta;
		}
		else if (prgbcIn->green == iMax)
		{
			fHue = F32( 2.0 + (prgbcIn->blue - prgbcIn->red) / fDelta );
		}
		else
		{
			fHue = F32( 4.0 + (prgbcIn->red - prgbcIn->green) / fDelta );
		}

		fHue *= 60.0;

		if (fHue < 0)
			fHue += 360;

		*piOutHue = (short)fHue;
	}
}

U32 HSV2RGB( int iInHue, int iInSaturation, int iInValue, U08 alpha=0xff )
{
	short _red, _green, _blue;
	short *piRed = &_red;
	short *piGreen = &_green;
	short *piBlue = &_blue;

	if (iInSaturation == 0)
	{
		*piRed = iInValue;
		*piGreen = iInValue;
		*piBlue = iInValue;
	}
	else
	{
		int i;
		F32 f;
		F32 p,q,t;
		F32 fHue, fValue, fSaturation;

		if (iInHue == 360) iInHue = 0;

		fHue = (F32)iInHue / 60;

		i = int( fHue );
		f = fHue - (F32)i;

		fValue = (F32)iInValue / 255;
		fSaturation = (F32)iInSaturation / 255;

		p = fValue * (1.0f - fSaturation);
		q = fValue * (1.0f - (fSaturation * f));
		t = fValue * (1.0f - (fSaturation * (1.0f - f)));

		switch (i)
		{
		case 0:
			*piRed = int( fValue * 255 );
			*piGreen = int( t * 255 );
			*piBlue = int( p * 255 );
			break;
		case 1:
			*piRed = int( q * 255 );
			*piGreen = int( fValue * 255 );
			*piBlue = int( p * 255 );
			break;
		case 2:
			*piRed = int( p * 255 );
			*piGreen = int( fValue * 255 );
			*piBlue = int( t * 255 );
			break;
		case 3:
			*piRed = int( p * 255 );
			*piGreen = int( q * 255 );
			*piBlue = int( fValue * 255 );
			break;
		case 4:
			*piRed = int( t * 255 );
			*piGreen = int( p * 255 );
			*piBlue = int( fValue * 255 );
			break;
		case 5:
			*piRed = int( fValue * 255 );
			*piGreen = int( p * 255 );
			*piBlue = int( q * 255 );
			break;
		}
	}

    return ( alpha << 24 ) | ( _blue << 16 ) | ( _green << 8 ) | ( _red << 0 );
}

void
AImage::adjustHSV( F32 hue, F32 sat, F32 val )
{
	int x,y;
	int linesize = width*4;
	U08 *&texData = texDatas[0];
	U32 color;

	// flip red and blue color components
	// why this can't be done in the 5th, 6th, and 7th args to FreeImage_ConvertToRawBits .. I don't know
	for(y=0; y < height; y++)
	{
		int pos = y*linesize;

		for(x=0; x < width; x++)
		{
			int xpos = y*linesize + x*4;

			memcpy( &color, texData+xpos, 4 );

			int h,s,v;
			RGB2HSV( color, &h, &s, &v );
			h = ( h + int( hue * 360.0f ) ) % 360;

			s = s + int( sat * 65535.0f );
			if( s > 0xffff ) s = 0xffff;
			if( s < 0 ) s = 0;

			v = v + int( val * 65535.0f );
			if( v > 0xffff ) v = 0xffff;
			if( v < 0 ) v = 0;

			color = HSV2RGB( h, s, v );

			// alpha .. blue .. green .. red
			memcpy( texData + xpos, &color, 4 );
		}       
	}

	texdef2d();
}

AImage *
AImage::findExisting( const char *filename )
{
	int i;

	for( i=0; i<images; i++ )
	{
		AImage *img = ( AImage * ) images[i];

		if( !_stricmp( img->filename, filename ) ) return img;
	}

	for( i=0; i<images; i++ )
	{
		AImage *img = ( AImage * ) images[i];

		if( !_stricmp( img->basefilename, filename ) ) return img;
	}

	return NULL;
}