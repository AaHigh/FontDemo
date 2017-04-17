#include "stdafx.h"

#define EXTRA_PIXEL_FETCH

#define AF_DEFAULT               0x00000000
#define AF_FIXED_WIDTH           0x00000001
#define AF_FIXED_WIDTH_NUMS_ONLY 0x00000002
#define AF_LOWER_CASE_ONLY       0x00000004
#define AF_NOMIPMAP              0x00000008
#define AF_SQUEEZE               0x00000010
#define AF_FULL_HEIGHT           0x00000020
#define AF_MULTIHEIGHT           0x00000040

#pragma warning(disable:4996)

typedef struct
{
	F32 hwidth;
	F32 hspacing;
	F32 xoffset;
	F32 yoffset;
	F32 hheight;
}
FontChar;

#define MAX_CHARS ('~'-' '+1)

typedef struct
{
	AImage *aimg;
	F32 oox;
	F32 ooy;
	F32 firstline;
	F32 height;
	F32 draw_height;
	F32 space_hwidth;
	F32 squeeze;
	U08 numrows;   // how many rows of text detected
	bool nearest;
	FontChar c[MAX_CHARS];
}
FontInternal;

typedef struct
{
	char *filename;
	char *clist;
	F32 firstline;
	F32 height;
	F32 extra_spacing;
	F32 extra_cr_spacing;
	F32 fixed_width;
	F32 space_width;
	F32 baseline_offset;
	U32 flags;
}
FontInit;

static FontInit font_init[] =
{
	{
		"fixedsys.png",
		NULL,
		54.0f, 18.0f, 0, 0, 0, 0, 0,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"arial24.png",
		NULL,
		10.0f, 30.0f, 1, 0, 0, 0, 0,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY,
	},
	{
		"arial48.png",
		NULL,
		18.0f, 60.0f, 1, 0, 0, 0, 0,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY,
	},
	{
		"century20.png",
		NULL,
		3.0f, 32.0f, -2, 0, 0, 0, 0,
		AF_NOMIPMAP,
	},
	{
	   	// These have no stroke or drop shadow
		// they are needed to do black numbers on light backgrounds
		"kozuka17nums.png",
		"0123456789",
		3.0f, 26.0f, 0, -2, 0, 0, 0,
		AF_NOMIPMAP
	},
	{
		"kozuka17a.png",
		NULL,
		1.0f, 32.0f, 0, -3, 0, 0, 0,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"kozuka24a.png",
		NULL,
		2.0f, 36.0f, -4.0, -4.0f, 0, 0, -5,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"kozuka28a.png",
		NULL,
		3.0f, 36.0f, -3.0, -6.0, 0, 0, -4,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"kozuka57a.png",
		NULL,
		3.0f, 72.0f, -5.0, -12.0, 0, 0, -6,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"lucidahand51.png",
		NULL,
		0.0f, 72.0f, -5.0f, -12.0, 0, 0, -1,
		0 // AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"currency.png",
		// a:british pound,b:p,c:yen,d:1/4,e:1/2,f:3/4,g:ce (euro obsolete),h:colon sign,i:Cruzeiro sig
		// j:French Franc,k:Lira Sign,l:Mill Sign,m:Naira Sign,n:peseta sign,o:Rupee Sign,p:Won Sign,q:New Sheqel Sign,r:Dong Sign
		// s:Euro Sign (one big C with =),t:Chinese Yuan (Hong Kong and Taiwan),u:Yuan (in China)
		"abcdefghijklmnopqrstu0123456789.-,$",
		-2.0f, 36.0f, 0, 0, 0, 0, 0,
		AF_NOMIPMAP | AF_FIXED_WIDTH_NUMS_ONLY,
	},
	{
		"led.png",
		"abcdefghijklmnopqrstuvwxyz_/ -,0123456789_:.\"'",
		1.0f, 29.0f, 4, 0, 0, 0, 0,
	   	AF_LOWER_CASE_ONLY,
	},
	{
		"silkscreen.png",
		"!\"#$%&'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\\]^_`{|}~",
		2, 10, 0, 0, 0, 4, 0,
		AF_NOMIPMAP | AF_LOWER_CASE_ONLY
	},
	{
		"silkscreen_noglow.png",
		"!\"#$%&'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\\]^_`{|}~",
		3, 8, 1, 0, 0, 4, 0,
		AF_NOMIPMAP | AF_LOWER_CASE_ONLY
	},
	{
		"arial_numbers.png",
		"$0123456789",
		4.0f, 19.0f, -2, 0, 0, 0, 0,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"nmk18.png",
		"!\"#$%&'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\\]^_`{|}~",
		1.0f, 0, 0, 0, 0, 0, -5,
		AF_NOMIPMAP | AF_LOWER_CASE_ONLY // | AF_FIXED_WIDTH_NUMS_ONLY
	},
	{
		"rounded_line24.png",
		NULL,
		3, 28, 0, 0, 0, 0, -5,
		AF_NOMIPMAP // | AF_FIXED_WIDTH_NUMS_ONLY
	},
};

static U32
font_init_checksum( U32 cksum, FontInit *fi )
{
	char *filename = fi->filename;
	char *clist = fi->clist;

	FontInit tmp = *fi;
	tmp.filename = NULL;
	tmp.clist = NULL;

	cksum = cksum_data( cksum, &tmp, sizeof( tmp ) );
	cksum = cksum_data( cksum, filename, filename ? strlen( filename ) : 0 );
	cksum = cksum_data( cksum, clist, clist ? strlen( clist ) : 0 );

	return cksum;
}

static FontInternal finternal[AF_NUM_FONTS];

#define MAX_FONT_ROWS 20

typedef struct
{
	float firstline;
	float height;
}
Row;

// Does this one row fit using firstline and height
static int fits( Row *row, F32 firstline, F32 height )
{
	while( firstline+height <= row->firstline ) firstline+=height;
	return ( row->firstline + row->height <= firstline+height );
}

static F32 howgood( Row *rows, F32 firstline, F32 height )
{
	int good = 0;

	for( ; rows && rows->height; rows++ )
	{
		if( fits( rows, firstline, height ) ) good++;
	}

	return (F32) good;
}

#if 0
#define DBG printf
static int debug_font = AF_SPACERANGER400;
#else
#define DBG no_dbg
static int debug_font =
// AF_FIXEDSYS;
//	AF_ARIAL24;
//	AF_ARIAL48;
//	AF_CENTURY20;
//	AF_KOZUKA17NUMS;
//	AF_KOZUKA17;
//	AF_KOZUKA24;
//	AF_KOZUKA28;
//	AF_KOZUKA57;
//	AF_CURRENCY;
//	AF_LED;
//	AF_NMK18;
-1;
#endif

static void no_dbg(const char *s,...)
{
	// do NOTHING
}

static inline F32 center_row( FontInternal *fi, int row )
{
	F32 rval = fi->firstline + fi->height * (((F32)row)+0.5f);
	F32 frac = rval - (int)rval;

	if( frac > 0.45 && frac < 0.55 ) return rval;
	return ((F32)(int)(rval-0.5f)) + 0.5f;
}


const char *
find_font_image_file( const char *filename )
{
	static char result[256];

	sprintf( result, "%s/fonts/%s", data_root_prefix(), filename );

	return result;
}

Afont::Afont( )
{
	memset( this, 0, sizeof( Afont ) );
	sanity();
}

Afont::~Afont()
{
}

static void afont_getsize( int fid )
{
	FontInternal *fi = &finternal[fid];
	const char *filename = find_font_image_file( font_init[fid].filename );
	const FontInit *finit = &font_init[fid];
	S32 i,j,x,y;
	AImage *bm;
	bool empty = 0;
	bool prev_empty = 0;
	int secondline = -1;
	int first = 0;
	int maxheight = 1;
	Row row[MAX_FONT_ROWS] = {0};
	int ri=0;
	int trail = 0;
	int vblank;
	int maxvblank = 0;
	int firstline = -1;

	if( !fi ) return;

	bm = new AImage( filename );

	if( !bm ) return;

	x = bm->getWidth();
	y = bm->getHeight();

	if( ( x & (x-1) ) || ( y & (y-1) ) )
	{
		DBG("Could not load bitmap from font because image not a power of 2");
		return;
	}

	fi->oox = 1.0f / ((F32)x);
	fi->ooy = 1.0f / ((F32)y);

	DBG("afont_init: font texture size is %dx%d\n", x, y );

	for( j=0; j<y; j++ )
	{
		empty = 1;
		for( i=0; i<x; i++ )
		{
			U32 color = bm->getColor( i, j );
            if( color & 0xff000000 )
			{
//				if( fid == debug_font ) DBG("o");
				if( firstline<0 && j>1 )
				{
					firstline = j-1; // the first line is transparent
				}
				empty = 0;
			}
			else
			{
//				if( fid == debug_font ) DBG("x");
			}
		}

//		if( fid == debug_font ) DBG("\n");

		if( j>0 && !empty && prev_empty )
		{
//			DBG("push %d %d\n",j,j-first);
			vblank = j - trail;
			if( vblank > maxvblank ) maxvblank = vblank;
//			DBG( "vblank %d\n", vblank );
			first = j;
		}
		if( j>0 && empty && !prev_empty )
		{
			row[ri].firstline = (float)first;
			row[ri].height = (float)(j-first);
			if( row[ri].height > maxheight )
			{
				maxheight = (int)row[ri].height;
			}

			ri++;
//			DBG("pop height %d\n",j-first);
			trail = j;
		}

		prev_empty = empty;
		if( fid == debug_font ) DBG("%d:%d\n", j, empty );
	}

	F32 totalheight = 0;

	// Show some debug info relating where the pixel data is located
	for( i=0; i<ri; i++ )
	{
		if( fid == debug_font )
		{
			DBG("Row %d from %.1f to %.1f height: %.1f (delta %.1f from first)\n", i,
					row[i].firstline, row[i].firstline+row[i].height-1.0f, row[i].height, row[i].firstline - (F32)firstline );
		}

		if( i>0 )
		{
//			DBG(" avg height %5.2f\n", (F32)( row[i].firstline - firstline ) / (F32) i );
			totalheight += ((F32)( row[i].firstline - firstline ) / (F32) i );
		}
	}

	F32 height = (F32) ( (int) (( totalheight / (F32)(ri-1) ) + 0.5f ) );

	U32 bestheightcount = 0;

	// figure out some defaults if not already adjusted by programmer
	if( 1 && finit->height != 0.0f )
	{
		height = fi->height = finit->height;
		firstline = (int) ( fi->firstline = finit->firstline );
	}
	else
	{
		fi->firstline = ((F32)firstline);
		fi->height = ((F32)height);
		for( i=0; i<4; i++ )
		{
			F32 h;
			F32 heighttotal=0;
			U32 heightcount=0;

			for( h=height-1.0f; h<height+1.0f; h+=0.1f )
			{
				if( h < 1 ) continue;
				F32 good = howgood( row, firstline-((F32)i), h );
	//			DBG( "howgood %f firstline: %d height: %f\n", good, firstline-i, h );

				if( good >= ri )
				{
	//				DBG( "Got them all!\n" );
					heighttotal+=h;
					heightcount++;
				}
			}

			if( heightcount > bestheightcount )
			{
				F32 avgheight = (heighttotal / (F32)heightcount);
	//			DBG( "This was good: %d %f\n", firstline-i, avgheight );
				bestheightcount = heightcount;
				fi->firstline = ((F32)(firstline-i));
				fi->height = ((F32)avgheight);
			}
		}
	}

	fi->draw_height = (F32)maxheight;

	if( finit->flags & AF_FULL_HEIGHT ) fi->draw_height = (F32)fi->height;

//	DBG("firstline: %d\n", firstline );

	if( fid == debug_font )
	{
		DBG("fi->height = %f fi->draw_height = %f\n", fi->height, fi->draw_height );
	}

	char c = ' ';
	if( !finit->clist ) c++;

	Row *r;
	F32 max_hwidth = 0.0;
	F32 max_num_hwidth = 0.0;

	for( r=row; r<&row[ri]; r++ )
	{
		int firstcolumn = -1;

//		DBG( "Inspecting the row:\n" );

		int found_firstcolumn = 0;

		for( i=0; i<x; i++ )
		{
			int transparent_column = 1;

			for( j=(int)r->firstline; j<r->firstline+r->height; j++ )
			{
				U32 color = bm->getColor( i, j );
				if( color & 0xff000000 )
				{
					if( !found_firstcolumn ) firstcolumn = i;
					found_firstcolumn = 1;
					transparent_column = 0;
					break;
				}
			}

			if( ( transparent_column || (i==x-1) ) && firstcolumn >= 0 )
			{
				FontChar *ch = &(fi->c[c-' ']);
				char savec = 0; // this initial value is never needed but gets rid of compiler warning

				if( finit->clist )
				{
					savec = c;
					c = finit->clist[c-' '];
					if( !c ) goto done;
					if( c < ' ' || c > '~' )
					{
						DBG( "This shit is just wrong, man\n" );
					}
					if( fid == debug_font ) DBG( "xform from %c to %c\n", savec, c );
					ch = &fi->c[c-' '];
				}

				if( finit->fixed_width > 0 )
				{
					ch->hwidth = ( finit->fixed_width - 2 ) * 0.5f;
					ch->xoffset = ( ((F32)firstcolumn) / finit->fixed_width );
					ch->xoffset = ( (int) ( ch->xoffset + 0.5f ) ) + 0.5f;
					ch->xoffset *= finit->fixed_width;
				}
				else
				{
					ch->hwidth = ch->hspacing = ((i-firstcolumn) * 0.5f);
					ch->xoffset = (((i-1)+firstcolumn) * 0.5f);
				}

#ifdef EXTRA_PIXEL_FETCH
				ch->hwidth += 0.5f;
#endif

				if( finit->flags & AF_MULTIHEIGHT )
				{
					ch->yoffset = ( r->firstline + r->height * 0.5f );
					ch->hheight = r->height * 0.5f;
				}
				else
				{
					ch->yoffset = center_row( fi, r-row );
					ch->hheight = fi->draw_height * 0.5f;
				}

				if( ch->hwidth > max_hwidth ) max_hwidth = ch->hwidth;
				if( c >= '0' && c <= '9' && ch->hwidth > max_num_hwidth ) max_num_hwidth = ch->hwidth;

				if( fid == debug_font )
				{
					DBG( "Found character %c centered at %f,%f with width %f\n", c, ch->xoffset, ch->yoffset, ch->hwidth * 2.0f );
					DBG( "Trying %f for vertical offset\n", ch->yoffset );
					DBG( "xofffset %f fi->firstline %f fi->height %f ch->yoffset %f ch->hwidth %f\n",
							ch->xoffset, fi->firstline, fi->height, ch->yoffset, ch->hwidth );
				}

				transparent_column = 1;
				found_firstcolumn = 0;
				firstcolumn = -1;

				if( finit->clist ) c = savec;

				c++;

				if( c > '~' ) goto done;
			}
		}
	}

done:

	if( finit->flags & AF_SQUEEZE )
	{
		fi->squeeze = 0.5f;
	}
	else
	{
		fi->squeeze = 0.0f;
	}

	if( finit->space_width > 0 )
	{
		fi->space_hwidth = finit->space_width * 0.5f;
	}

	if( finit->flags & AF_FIXED_WIDTH_NUMS_ONLY )
	{
		printf( "Fond fid %d\n", fid );
		for( c='0'; c<='9'; c++ )
		{
			FontChar *ch = &(fi->c[c-' ']);

			if( fid == debug_font )
				DBG("max_num_hwidth = %f ch->hspacing = %f\n", max_num_hwidth, ch->hspacing );

			ch->hspacing = max_num_hwidth;
		}

		if( finit->space_width == 0 )
		{
			fi->space_hwidth = fi->c['\"'-' '].hspacing + finit->extra_spacing; // take space used by ' and use for space
			if( fi->space_hwidth < 4 ) fi->space_hwidth = 4;
		}
	}
	else if( finit->flags & AF_FIXED_WIDTH )
	{
		for( i=0; i<MAX_CHARS; i++ )
		{
			FontChar *ch = &(fi->c[i]);

			ch->hspacing = max_hwidth;
		}

		if( finit->space_width == 0 )
		{
			fi->space_hwidth = max_hwidth + finit->extra_spacing;
		}
	}
	else if( finit->space_width == 0 )
	{
		fi->space_hwidth = fi->c['\"'-' '].hspacing + finit->extra_spacing; // take space used by ' and use for space
		if( fi->space_hwidth < 4 ) fi->space_hwidth = 4;
	}

	if( finit->flags & AF_LOWER_CASE_ONLY )
	{
		char uc,lc;

		for( uc='A',lc='a'; lc<='z'; uc++,lc++ )
		{
			FontChar *uch = &(fi->c[uc-' ']);
			FontChar *lch = &(fi->c[lc-' ']);

			if( uch->hwidth == 0.0 )
			{
				*uch = *lch;
				if( fid == debug_font )
					DBG("%c copied to %c\n", lc,uc );
			}
			else if( lch->hwidth == 0.0 )
			{
				*lch = *uch;
				if( fid == debug_font )
					DBG("%c copied to %c\n", uc,lc );
			}
		}
	}

	return;
}

#define FONT_CACHE_VERSION 2

static U32 checksum( void )
{
	int i,n;
	U32 cksum;

	n = sizeof( font_init ) / sizeof( font_init[0] );

    cksum = FONT_CACHE_VERSION + AImage::getVersion();
	
	for( i=0; i<n; i++ )
	{
		const char *filename = find_font_image_file( font_init[i].filename );
		if( file_exists( filename ) )
		{
			cksum = cksum_file( cksum, filename );
			printf( "cksum %s -> %x\n", filename, cksum );
		}
		cksum = font_init_checksum( cksum, &font_init[i] );
		if( font_init[i].clist )
		{
			cksum = cksum_data( cksum, font_init[i].clist, strlen( font_init[i].clist ) );
		}
	}

	return cksum;
}

static void write_font_cache( void )
{
	int i,n;

	n = sizeof( font_init ) / sizeof( font_init[0] );

	FILE *f = fopen( "cache.bin", "wb" );

	if( !f ) return;

	U32 cksum = checksum();

	fwrite( &cksum, sizeof(cksum), 1, f );

	for( i=0; i<n; i++ )
	{
		void *tmp = finternal[i].aimg;
		finternal[i].aimg = NULL; // just to be safe, don't write the pointer data .. it's useless in a file

		fwrite( &finternal[i], sizeof( finternal[0] ), 1, f );

		finternal[i].aimg = (AImage *)tmp;
	}

	fclose( f );
}

static int read_font_cache( void )
{
	if( debug_font >= 0 ) return 0;

	int n;

	n = sizeof( font_init ) / sizeof( font_init[0] );

	FILE *f = fopen( "cache.bin", "rb" );

	if( !f ) return 0;

	U32 cksum = 0;

	fread( &cksum, sizeof(cksum), 1, f );

	if( cksum != checksum() ) return 0;

	fread( &finternal[0], sizeof( finternal[0] ), n, f );

	fclose( f );

	return 1;
}

static void afont_load( void )
{
	int i,n;
	int cache_good;

	n = sizeof( font_init ) / sizeof( font_init[0] );

	U32 ms = timeGetTime();

	cache_good = read_font_cache();
//	cache_good = 0; // uncomment this to do debug testing

	for( i=0; i<n; i++ )
	{
		finternal[i].aimg = NULL;

		if( !cache_good )
		{
			finternal[i].aimg = new AImage( find_font_image_file ( font_init[i].filename) );

			afont_getsize( i );

			if( font_init[i].height > 0 )
			{
				finternal[i].height = font_init[i].height;
			}
			if( font_init[i].firstline > 0 )
			{
				finternal[i].firstline = font_init[i].firstline;
			}
		}
//		DBG( "Loaded font %s\n", font_init[i].filename );

		if( font_init[i].height == 0 && font_init[i].firstline == 0 )
		{
			DBG( "NOTICE: Regarding font %s:\n", font_init[i].filename );
			DBG( "automatic algorithm hopefully got close with\n  firstline=%f and height=%f\n", finternal[i].firstline, finternal[i].height );
			DBG( "please see afont.c and search for font_init to figure\nout how to tune these to better values if text is off vertically\n\n" );
		}
	}

	{
		U32 delta = timeGetTime() - ms;

		printf( "Took %dmsec to read fonts (%f seconds)\n", delta, (F32)msecs_to_secs(delta) );
	}

	if( !cache_good )
	{
		cache_good = 1; //
		write_font_cache();
	}
}


void afont_init( void )
{
	afont_load();
}

static inline void draw_char( F32 htx, F32 hty, F32 tx, F32 ty, F32 hx, F32 hy, F32 x, F32 y, F32 z, U32 col, U32 col2 )
{
//	ty = 1.0f - ty;
//	hty = (-hty);

	glBegin( GL_QUAD_STRIP );
	glColor4ub( col>>24, (col&0xff0000)>>16, (col&0xff00)>>8, (col&0xff) );
	glSecondaryColor3ub( col2>>24, (col2&0xff0000)>>16, (col2&0xff00)>>8 );

	glTexCoord2f( tx-htx , ty-hty  );glVertex3f( x-hx, y+hy, z );
	glTexCoord2f( tx-htx , ty+hty  );glVertex3f( x-hx, y-hy, z );
	glTexCoord2f( tx+htx , ty-hty  );glVertex3f( x+hx, y+hy, z );
	glTexCoord2f( tx+htx , ty+hty  );glVertex3f( x+hx, y-hy, z );
	glEnd( );
}

void
Afont::sanity( void )
{
	if( fid >= AF_NUM_FONTS )
	{
		memset( this, 0, sizeof( Afont ) );
	}

	if( sx <= 0.0 || sx > 100.0f )
	{
		sx = 1.0f;
		sy = 1.0f;

		if( col == 0 )
		{
			col = 0xffffffff;
		}
	}

	if( !boxcolor ) boxcolor = 0x7f7f7f7f;
	if( !shader ) shader = SHADER_TWOCOLOR;
}

F32
Afont::hwidth_until_return( const char *c )
{
	F32 total = 0.0f;
	FontInternal *fi = &(finternal[fid]);
	FontInit *finit = &font_init[fid];

	while( c && *c && *c != '\n' )
	{
		if( *c == ' ' && fi->c[0].hwidth == 0.0 )
		{
			total += fi->space_hwidth;
		}
		else if( *c >= ' ' && *c <= '~' )
		{
			total += fi->c[*c-' '].hspacing;
			total += finit->extra_spacing * 0.5f;
		}

		c++;
	}

	return total * sx;
}

F32
Afont::height( void )
{
	FontInternal *fi = &(finternal[fid]);
	FontInit *finit = &font_init[fid];
	return (fi->draw_height+finit->extra_cr_spacing)*sy;
}

// NOTE: This should match the return value of afont_print .. so you need to
// maintain these two function simultaneously if there are changes
F32
Afont::length( const char *c )
{
	if( !c || !c[0] ) return 0.0f;
	F32 rval = 0.0f;

	FontInternal *fi = &(finternal[fid]);
	FontInit *finit = &font_init[fid];

    do
	{
		if( *c == ' ' && fi->c[0].hwidth == 0.0 )
		{
			F32 space = ( fi->space_hwidth + fi->space_hwidth ) * sx;
			rval += space;
		}
		else if( *c == '\n' )
		{
			// NOP -- this is actually more or less undefined
		}
		else if( *c >= ' ' && *c <= '~' )
		{
			FontChar *fc = &(fi->c[*c-' ']);
			F32 tmp;

			tmp = fc->hwidth * sx;
			rval += tmp;
			tmp = ( finit->extra_spacing + fc->hspacing + fc->hspacing-fc->hwidth ) * sx;
			rval += tmp;
		}
	}
	while( *(++c) );

	return rval;
}

F32 afont_length( Afont *f, const char *c )
{
	return f->length( c );
}

F32 afont_lengthf( Afont *f, const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return f->length( buf );
}

F32 Afont::lengthf( const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return this->length( buf );
}

F32
Afont::print( const char *c )
{
	if( dropshad )
	{
		U08 tmp = dropshad;
		F32 rval;
		dropshad = 0;
		rval = print_ds( c );
		dropshad = tmp;
		return rval;
	}

	if( drawbox )
	{
		F32 rval;
		drawbox = 0;
		rval = print_box( c );
		drawbox = 1;
		return rval;
	}

	if( !c || !c[0] ) return 0.0f;
	F32 rval = 0.0f;

	F32 savex = x;

	FontInternal *fi = &(finternal[fid]);
	FontInit *finit = &font_init[fid];
	U32	gl_name = 0;

	if( !fi->aimg )
	{
		fi->aimg = new AImage(find_font_image_file ( font_init[fid].filename) );
	}

	gl_name = fi->aimg->getGLName();
	glBindTexture( GL_TEXTURE_2D, gl_name);
	SHADER_TYPE use = UseShader( shader );

#if 0 // leave this here .. we may need it still, but for now the accuracy is such that the results are almost
	  // identical.  I want to know when the texels don't match the pixels properly
	// Special case to make a cleaner 1:1 texel to pixel mapping when we can
	if( sx == 1.0f && sy == 1.0 && gfx_get_width() == 640 && gfx_get_height() == 480 )
	{
		if( !fi->nearest )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			fi->nearest = 1;
		}
	}
	else
	{
		if( fi->nearest )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			fi->nearest = 0;
		}
	}
#endif

	F32 afhw = hwidth_until_return( c );

		 if( justification == AF_CENTER ) x -= afhw;
	else if( justification == AF_RIGHT  ) x -= afhw * 2.0f;

	if( !nosnap )
	{
#ifdef EXTRA_PIXEL_FETCH
		x = (F32)((int)(x-0.5f));
#else
		x = (F32)((int)(x-0.5f))+0.5f;
#endif
		y = (F32)((int)(y-0.5f))+0.5f;
	}

	x += finit->extra_spacing * 0.5f * sx;
	y += finit->baseline_offset * sy;

    do
	{
		if( *c == ' ' && fi->c[0].hwidth == 0.0 )
		{
			F32 space = ( fi->space_hwidth + fi->space_hwidth ) * sx;
			x += space;
			rval += space;
		}
		else if( *c == '\n' )
		{
			y -= (fi->draw_height+finit->extra_cr_spacing)*sy;
			x = savex;

			afhw = hwidth_until_return( c+1 );

			     if( justification == AF_CENTER ) x -= afhw;
			else if( justification == AF_RIGHT  ) x -= afhw * 2.0f;

			if( !nosnap )
			{
				x = (F32)((int)(x-0.5f))+0.5f;
				y = (F32)((int)(y-0.5f))+0.5f;
			}
		}
		else if( *c >= ' ' && *c <= '~' )
		{
			FontChar *fc = &(fi->c[*c-' ']);
			F32 tmp;

			tmp = fc->hwidth * sx;
			x += tmp;
			rval += tmp;

			draw_char( (fc->hwidth-squeeze)*fi->oox, fc->hheight*fi->ooy,
					   fc->xoffset*fi->oox, fc->yoffset*fi->ooy,
					   fc->hwidth*sx, fc->hheight*sy, x, y, z,
					   col, col2 );

			tmp = ( finit->extra_spacing + fc->hspacing + fc->hspacing-fc->hwidth ) * sx;
//			tmp = ( finit->extra_spacing + fc->hwidth) * sx;
			x += tmp;
			rval += tmp;

//			x = (F32) ((int) ( x + 0.5f ));
		}
	}
	while( *(++c) );

	y -= finit->baseline_offset * sy;

	if( justification == AF_RIGHT ) x -= afhw * 2.0f;

	UseShader( use );

	return rval;
}

F32 afont_print( Afont *f, const char *c )
{
	return f->print( c );
}

void
Afont::bindtex( void )
{
	U32 gl_name = 0;

	FontInternal *fi = &(finternal[fid]);

	gl_name = fi->aimg->getGLName();
	glBindTexture( GL_TEXTURE_2D, gl_name);
}

void afont_bindtex( Afont *f )
{
	f->bindtex();
}

// Return the s and t center and half width/height for the requested character
void
Afont::charbox( int c, F32 *htx, F32 *hty, F32 *tx, F32 *ty )
{
	FontInternal *fi = &(finternal[fid]);
	if( c >= ' ' && c <= '~' )
	{
		FontChar *fc = &(fi->c[c-' ']);

		if( htx ) *htx = (fc->hwidth-squeeze)*fi->oox;
		if( hty ) *hty =  fc->hheight*fi->ooy;
		if( tx  ) *tx  =  fc->xoffset*fi->oox;
	    if( ty  ) *ty  =  fc->yoffset*fi->ooy;
	}
}

void afont_charbox( Afont *f, int c, F32 *htx, F32 *hty, F32 *tx, F32 *ty )
{
	f->charbox( c, htx, hty, tx, ty );
}

void
Afont::snap( bool snap )
{
	nosnap = !snap;
}

void afont_snap( Afont *f, bool snap )
{
	f->snap( snap );
}

F32
Afont::print_ds( const char *str )
{
	FontInternal *fi = &finternal[fid];
	U32 colorsave = col;
	U32 colorsave2 = col2;
	F32 xsave,ysave;

	xsave = x;
	ysave = y;

	col = ( ( col & 0xff ) >> 2 );
	F32 tmp = dropshad ? ((F32)dropshad) : ( sx * fi->space_hwidth * 0.25f );
	x += tmp;
	y -= tmp;

	print( str );

	x = xsave;
	y = ysave;
	col = colorsave;
	col2 = colorsave2;

	return print( str );
}

F32
Afont::printf_ds( const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return print_ds( buf );
}

F32 afont_print_ds( Afont *f, const char *str )
{
	return f->print_ds( str );
}

F32
Afont::print_box( const char *str )
{
	FontInternal *fi = &finternal[fid];
	F32 len = this->length( str );
	F32 height = fi->draw_height;

	F32 boxwidth = len + height;
	F32 boxheight = height * 1.2f;
	F32 _x,_y;

	_y = y;

	if( justification == AF_CENTER ) _x = x;
	else if( justification == AF_LEFT ) _x = x + len * 0.5f;
	else if( justification == AF_RIGHT ) _x = x - len * 0.5f;
	else
	{
		_x = 0.f;
		DBG("afont_print_box: illegal justification");
	}

#if 0
	if( box_drop )
	{
		draw_transparent_box_ds( boxwidth, boxheight, _x, _y, boxcolor );
	}
	else
	{
		draw_transparent_box( boxwidth, boxheight, _x, _y, boxcolor );
	}
#endif

	bool tmp = drawbox;
	drawbox = 0;
	F32 rval = print( str );
	drawbox = tmp;
	return rval;
}

F32 afont_print_box( Afont *f, const char *str )
{
	return f->print_box( str );
}

F32
Afont::printf( const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return print( buf );
}

F32 afont_printf( Afont *f, const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return f->print( buf );
}

F32 afont_printf_ds( Afont *f, const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return f->print_ds( buf );
}

void
Afont::scale( F32 _sx, F32 _sy )
{
	if( _sy < 0 ) _sy = _sx;

	sx = _sx;
	sy = _sy;

	FontInternal *fi = &finternal[fid];

	// For the LED font and other fonts that butt up side to side, this makes sure there is no space between letters
	// when applying scale
	if( fi->squeeze != 0.0 && ( sx != 1.0 || sy != 1.0 ) )
	{
		F32 absdiff = 1.0f - sx;
		if( absdiff < 0 ) absdiff = (-(absdiff));
		if( absdiff > 1 ) absdiff = 1;

		squeeze = fi->squeeze * absdiff;
	}
	else
	{
		squeeze = 0.0;
	}
}

void afont_scale( Afont *f, F32 sx, F32 sy )
{
	f->scale( sx, sy );
}

void
Afont::just( int just )
{
	if( just >= AF_LEFT && just <= AF_RIGHT )
	{
		justification = just;
	}
	else
	{
		justification = AF_LEFT;
	}
}

void afont_just( Afont *f, int just )
{
	f->just( just );
}

void
Afont::color( U32 color, U32 color2 )
{
	col = color;
	col2 = color2;
}

void afont_color( Afont *f, U32 color, U32 color2 )
{
	f->color( color, color2 );
}

void
Afont::colortime( F32 period, F32 normalized_offset )
{
	color( ::colortime( period, normalized_offset ) );
}

void afont_colortime( Afont *f, F32 period, F32 normalized_offset )
{
	f->colortime( period, normalized_offset );
}

void
Afont::font( int _fid )
{
	fid = _fid;
}

void afont_font( Afont *f, int fid )
{
	f->font( fid );
}

void
Afont::box( bool on_off, U32 boxcolor, U08 box_drop_shadow_dist )
{
	drawbox = on_off;
	boxcolor = boxcolor;
	box_drop = box_drop_shadow_dist;
}

void afont_box( Afont *f, bool on_off, U32 boxcolor, U08 box_drop_shadow_dist )
{
	f->box( on_off, boxcolor, box_drop_shadow_dist );
}

void
Afont::pos( F32 _x, F32 _y, F32 _z )
{
	x = _x + 0.5f;
	y = _y + 0.5f;
	z = _z;
}

void afont_pos( Afont *f, F32 x, F32 y, F32 z )
{
	f->pos( x, y, z );
}

void
Afont::move( F32 _x, F32 _y, F32 _z )
{
	x += _x;
	y += _y;
	z += _z;
}

void afont_move( Afont *f, F32 x, F32 y, F32 z )
{
	f->move( x, y, z );
}

void afont_state_push( void )
{
	draw_in_screen_coord();

	glPushMatrix();
	glLoadIdentity();

//	if( !get_rotate() )
//	{
//		glRotatef( 90.0f, 0.0f, 0.0f, 1.0f );
//		glTranslatef( 0.0f, (F32)(-(get_width())), 0.0f );
//	}

//	if( get_backglass_extra() )
//	{
//		glTranslated( 0, get_backglass_extra(), 0 );
//	}

 	glPushAttrib(GL_TEXTURE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 
	glDisable(GL_TEXTURE_2D); // This is correct .. state is disabled except during print
	glAlphaFunc(GL_NOTEQUAL,0);
	glEnable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDepthMask(0);
	glDisable(GL_DEPTH_TEST);
}

static Afont f;

static void _afont_test( int fid, const char *fontname )
{
	afont_font( &f, fid );
	afont_scale( &f, 2.0f );
	afont_just( &f, AF_CENTER );
	afont_printf( &f, "Test123.(%s)\nA\n", fontname );
	afont_scale( &f, 1.0f );
	afont_just( &f, AF_RIGHT );
	afont_color( &f, 0xff0000ff );
	afont_print( &f, "Right justified in red\n" );
	afont_just( &f, AF_LEFT );
	afont_color( &f, 0x00ff00ff );
	afont_scale( &f, 1.0f );
	afont_print( &f, "Left justified in green\n" );
	afont_scale( &f, 1.0f );
	afont_color( &f, 0xffffffff );
	afont_just( &f, AF_CENTER );
	afont_print( &f, "Double center test\nDouble center test\n" );
}

static void _afont_alphabet( int fid )
{
	F32 vert = 960 - 40;
	F32 horiz = 1080;
	Afont f;

	afont_font( &f, fid );
	afont_pos( &f, 10, vert );
	afont_scale( &f, 0.5f );
	f.colortime( 1.0f );

	{
		afont_pos( &f, 10, vert );
		afont_just( &f, AF_LEFT );
		afont_print( &f, "/.-,+*)('&%$#\"!\n" );
		afont_print( &f, "@?>=<;:9876543210\n" );
		afont_print( &f, "MLKJIHGFEDCBA\n" );
		afont_print( &f, "ZYXWVUTSRQPON\n" );
		afont_print( &f, "`_^]\\[\n" );
		afont_print( &f, "zyxwvutsrqpon\n" );
		afont_print( &f, "mlkjihgfedcba\n" );
		afont_print( &f, "~}|{\n" );

		afont_print( &f, "Hello World!\n" );

		afont_font( &f, fid );
		afont_pos( &f, horiz-10, vert );
		afont_just( &f, AF_RIGHT );
		afont_print( &f, "!\"#$%&'()*+,-./\n" );
		afont_print( &f, "0123456789:;<=>?@\n" );
		afont_print( &f, "ABCDEFGHIJKLM\n" );
		afont_print( &f, "NOPQRSTUVWXYZ\n" );
		afont_print( &f, "[\\]^_`\n" );
		afont_print( &f, "abcdefghijklm\n" );
		afont_print( &f, "nopqrstuvwxyz\n" );
		afont_print( &f, "{|}~\n" );

		afont_print( &f, "Hello World!\n" );
	}

	if( font_init[fid].clist )
	{
		FontInit *finit = &font_init[fid];
		int i,n = strlen( font_init[fid].clist );

		afont_pos( &f, horiz-10, vert + 40 );
		afont_just( &f, AF_RIGHT );

		for( i=n-1; i>=0; i-- )
		{
			afont_printf( &f, "%c", font_init[fid].clist[i] );
			if( f.getx() < 100 )
			{
				afont_pos( &f, 640-10, f.gety() - (finternal[fid].draw_height+finit->extra_cr_spacing)*f.getsy() );
			}
		}
	}

}

static void afont_test_ds( void )
{
	afont_font( &f, AF_KOZUKA24 );
	afont_scale( &f, 2.0f );
	afont_just( &f, AF_CENTER );
	afont_print_ds( &f, "DROPSHADOW" );
}

void afont_test( void )
{
	if( debug_font >= 0 )
	{
		_afont_alphabet( debug_font );
	}

//	return;

//	Afont f;
	afont_pos( &f, 10, 300 );
	afont_test_ds();
//	_afont_test( AF_VERDANA, "Verdana" );
//	_afont_test( AF_ARIAL48, "Arial48" );
	_afont_test( AF_FIXEDSYS, "Fixedsys" );
//	_afont_test( AF_LED, "LED 0123456789" );
//	_afont_test( AF_LED, "silkscreen_noglow" );
	_afont_test( AF_NMK18, "nmk24" );
}

void afont_state_pop( void )
{
	glPopAttrib();
	glPopMatrix();
}

void afont_draw( void )
{
	if( debug_font < 0 ) return;

	afont_state_push();
	afont_test();
	afont_state_pop();
}

void
Afont::drop( F32 _drop )
{
	if( !_drop )
	{
		FontInternal *fi = &finternal[fid];
		dropshad = (U08)( sx * fi->space_hwidth * 0.25f );
	}
	else
	{
		dropshad = (U08)_drop;
	}
}

void afont_drop( Afont *f, F32 drop )
{
	f->drop( drop );
}

void
Afont::nodrop( void )
{
	dropshad = 0;
}

void afont_nodrop( Afont *f )
{
	f->nodrop();
}

// If the number of pixels needed to print this exceeds a fixed amount,
// just squeeze that _____ in somehow
F32
Afont::print_squeezed( F32 size, const char *str )
{
	F32 len = this->length( str );
	F32 save_sx = sx;
	F32 rval = 0;

	if( len > 0 && len > size ) sx = sx * size / len;

	rval = print( str );

	sx = save_sx;

	return rval;
}

F32 afont_print_squeezed( Afont *f, F32 size, const char *str )
{
	return f->print_squeezed( size, str );
}

F32 afont_printf_squeezed( Afont *f, F32 size, const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return afont_print_squeezed( f, size, buf );
}

F32
Afont::print_squeezed_ds( F32 size, const char *str )
{
	F32 len = this->length( str );
	F32 save_sx = sx;
	F32 rval = 0;

	if( len > 0 && len > size ) sx = sx * size / len;

	rval = this->print_ds( str );

	sx = save_sx;

	return rval;
}

F32
Afont::printf_squeezed_ds( F32 size, const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return print_squeezed_ds( size, buf );
}


F32 afont_print_squeezed_ds( Afont *f, F32 size, const char *str )
{
	return f->print_squeezed_ds( size, str );
}

F32 afont_printf_squeezed_ds( Afont *f, F32 size, const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return afont_print_squeezed_ds( f, size, buf );
}

F32
Afont::box_print( U32 box_color, const char *str )
{
	F32 _x,_y;
	F32 _sx,_sy;
	FontInternal *fi = &finternal[fid];
	F32 len = this->length( str );

	_x = x;
	_y = y;

	_sx = len + sx * 6.0f;
    _sy = sy * ( fi->draw_height + 6.0f );

	     if( justification == AF_LEFT  ) _x += len * 0.5f;
	else if( justification == AF_RIGHT ) _x -= len * 0.5f;

#if 0
	draw_transparent_box( _sx, _sy, _x, _y, box_color );
#endif

	return this->print( str );
}

F32 afont_box_print( Afont *f, U32 box_color, const char *str )
{
	return f->box_print( box_color, str );
}

F32 afont_box_printf( Afont *f, U32 box_color, const char *str, ... )
{
	char buf[1024];
	va_list args;

	va_start(args, str);
	vsprintf(buf, str, args);
	va_end(args);

	return afont_box_print( f, box_color, buf );
}
