#include "stdafx.h"

const F32 M_PI = 3.14159265358979323846f;
F32 gSinf[360];
F32 gCosf[360];
static F32 sSinCosInitted;

void draw_in_screen_coord( void )
{   
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,get_width(),0,get_height(),-1000.0,1000.0);
	glMatrixMode(GL_MODELVIEW);
}

void draw_in_shadow_coord( U32 w, U32 h )
{   
	F32 hw = F32(w)*0.5f;
	F32 hh = F32(h)*0.5f;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-hw,hw,hh,-hh,-1000.0,1000.0);
	glMatrixMode(GL_MODELVIEW);
}

void draw_in_portrait_screen_coord( void )
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,get_width(),0,get_height(),-1000.0,1000.0);
	if( gDisplayPortrait >= 0 && gDisplayPortrait <= 1 )
	{
		F32 hw = get_width()*0.5f;
		F32 hh = get_height()*0.5f;
		if( gDisplayPortrait == 1 )
		{
			glRotatef( -90.0f, 0, 0, 1 );
			glTranslatef( F32(-get_portrait_width()), 0, 0 );
		}
		else
		{
			glRotatef( 90.0f, 0, 0, 1 );
			glTranslatef( 0, F32(-get_portrait_height()), 0 );
		}
	}
	glMatrixMode(GL_MODELVIEW);
}

void draw_in_landscape_screen_coord( void )
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,get_width(),0,get_height(),-1000.0,1000.0);
	glMatrixMode(GL_MODELVIEW);
}

void draw_in_portrait_perspective_screen_coord( F32 cameradist, int _width, int _height )
{
	F32 width,height;

	if( _height < 1 || _width < 1 )
	{
		width = F32(get_portrait_width());
		height = F32(get_portrait_height());
	}
	else
	{
		width = F32(_width);
		height = F32(_height);
	}

	if( cameradist < 1.0f ) cameradist = height;

	F32 camangle = ( 180.0f / M_PI ) * atan( ( height * 0.5f ) / cameradist ) * 2.0f;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if( gDisplayPortrait != 2 )
	{
		glRotatef( gDisplayPortrait ? -90.0f : 90.0f, 0, 0, 1 );
	}
	// Our aspect ratio is fixed at 9.0f / 16.0f per HDTV aspect ratio 1080/1920
	gluPerspective( camangle, ( width / height ), 20.0f/2160.0f * width, 20000.0f/2160.0f * width );
	glTranslatef( -width*0.5f, -height*0.5f, -cameradist );
	glMatrixMode(GL_MODELVIEW);
}

void draw_in_landscape_perspective_screen_coord( F32 cameradist )
{
	if( cameradist < 1.0f ) cameradist = get_landscape_height() * 1.0f;

	F32 camangle = ( 180.0f / M_PI ) * atan( ( get_landscape_height() * 0.5f ) / cameradist ) * 2.0f;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if( gDisplayPortrait != 2 )
	{
		glRotatef( gDisplayPortrait ? 180.0f : 0.0f, 0, 0, 1 );
	}
	else
	{
		glRotatef( 90.0f, 0, 0, 1 );
	}
	// Our aspect ratio is fixed at 9.0f / 16.0f per HDTV aspect ratio 1080/1920
	gluPerspective( camangle, ( F32(get_landscape_width()) / F32(get_landscape_height()) ), 20.0f, 20000.0f );
	glTranslatef( F32(-get_landscape_width())*0.5f, F32(-get_landscape_height())*0.5f, -cameradist );
	glMatrixMode(GL_MODELVIEW);
}

static F32 sRotAxis[3] = {1,0,0};

void myDepthFunc( U32 mode )
{
    glDepthFunc( mode );
}

void myCullFace( U32 mode )
{
    glCullFace( mode );
}

// Construct a matrix to rotate about the center of a texture
// NOTE: this may be a little squirrelly depending on if textures start at top or bottom
void myTexRot( F32 angle, bool topbottom )
{
	F32 m[4][4];

	int ia = int( angle + 0.5f ) % 360;

	F32 ca = gCosf[ia];
	F32 sa = gSinf[ia];
	
	m[0][0] = ca;
	m[0][1] = sa;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] =-sa;
	m[1][1] = ca;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;

	m[3][0] = 0.5f - 0.5f * ca - 0.5f * sa;
	m[3][1] = 0.5f - 0.5f * sa + 0.5f * ca;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;

	glMatrixMode( GL_TEXTURE );
	glLoadMatrixf( &m[0][0] );
	glMatrixMode( GL_MODELVIEW );
}


static bool InitVSync( void )
{
	static bool initted;

	if( initted ) return wglSwapIntervalEXT != NULL;
	initted = 1;

	const char *extensions = (char*)glGetString(GL_EXTENSIONS);

	if( extensions && strstr( extensions, "WGL_EXT_swap_control" ) )
	{
		wglSwapIntervalEXT    = (PFNWGLSWAPINTERVALEXTPROC   ) wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");
	}
	else
	{
		printf( "WARNING: Could not find opengl extensions for WGL_EXT_swap_control!\n" );
		wglSwapIntervalEXT = NULL;
		wglGetSwapIntervalEXT = NULL;
	}

	return wglSwapIntervalEXT != NULL;
}

void myMaterial( GLenum face, GLenum pname, U32 color, F32 colormod, F32 alphamod )
{
	F32 p[4];
	static F32 ootff = 1.0f/255.0f;

	if( colormod == 1.0f && alphamod == 1.0f )
	{
		p[0] = F32((color&0xff000000)>>24);
		p[1] = F32((color&0xff0000)>>16);
		p[2] = F32((color&0xff00)>>8);
		p[3] = F32(color&0xff);
	}
	else
	{
		p[0] = (((color&0xff000000)>>24)*colormod);
		p[1] = (((color&0xff0000)>>16)*colormod);
		p[2] = (((color&0xff00)>>8)*colormod);
		p[3] = ((color&0xff)*alphamod);
	}

	p[0]*=ootff;
	p[1]*=ootff;
	p[2]*=ootff;
	p[3]*=ootff;

	glMaterialfv( face, pname, p );
}

void
defaultLightMaterial( void )
{
	F32 lcol[4] = { 1.0f, 0.1f, 1.0f, 1.0f };
	F32 ldif[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	F32 lamb[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	glLightfv( GL_LIGHT0, GL_AMBIENT,  lamb );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  ldif );
	glLightfv( GL_LIGHT0, GL_SPECULAR, lcol );
	glLightf( GL_LIGHT0, GL_SHININESS, 1.0f );
}

void
setupLighting( void )
{
	static bool light_initted;

	myCullFace( GL_BACK );

	if( !light_initted )
	{	
		light_initted = true;

		F32 lpos[4] = { 1920.0f, 1080.0f*0.33f, 1000.0f, 1.0f };

		glEnable( GL_LIGHT0 );
		glEnable( GL_LIGHTING );
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

		defaultLightMaterial();

		glLightfv( GL_LIGHT0, GL_POSITION, lpos );
	}

	glShadeModel( GL_SMOOTH );
	glEnable( GL_COLOR_MATERIAL );
	glEnable( GL_LIGHTING );
}

static int sSwapInterval = 1;

bool getSync( void )
{
	return ( InitVSync() ? wglGetSwapIntervalEXT() > 0 : false );
}

void setSync( bool onoff )
{
	if( InitVSync() ) wglSwapIntervalEXT( onoff ? sSwapInterval : 0 );
}

void setSwapInterval( int interval )
{
	if( interval != sSwapInterval )
	{
		sSwapInterval = interval;
		if( InitVSync() ) wglSwapIntervalEXT( sSwapInterval );
	}
}

int getSwapInterval( void )
{
	return sSwapInterval;
}

static F32 sProjectionMatrix[4][4];
static F32 sInverseProjectionMatrix[4][4];

static F32 sMinz = 200.0f;
static F32 sMaxz = 20000.0f;
static F32 sZeroScreen;

void world2screen( F32 dst[3], F32 src[3] )
{
	F32 (&m)[4][4] = sProjectionMatrix;

	F32 _dst[3];
	mult( _dst, m, src );
	dst[0] = ( ( _dst[0] + 1 ) * 0.5f ) * get_width();
	dst[1] = ( ( _dst[1] + 1 ) * 0.5f ) * get_height();
	dst[2] = _dst[2];
}

static U32 secs_to_usecs( F32 secs )
{
	return (U32) ( secs * 1000000.0f );
}

U32 ncolortime(F32 n)
{
	n += 0.39f;

	if( n > 1 || n < 0 )
	{
		n = n - int(n);
	}

	n *= ( M_PI * 2.0f );

	F32 s = 0.5f+0.5f*sinf(n);
	F32 c = 0.5f+0.5f*cosf(n);

	U32 red = (U32)(s*255.0);
	U32 grn = (U32)(c*255.0);
	U32 blu = (U32)((1.0f-s)*255.0);

	return (red<<24) + (grn<<16) + (blu<<8) + 0xff;
}


U32 colortime(U32 msec, F32 period_in_secs, F32 normalized_offset)
{
	F32 angle = (((F32)(msec))/1000.0f);
//	printf( "%f %3.1f\n", angle, sinf(angle) );
	angle += normalized_offset;
	angle *= ( 1.0f / period_in_secs );
//	printf( "%d %d %d\n", red,grn,blu );

	return ncolortime(angle);
}

U32 blendcolors( U32 color1, U32 color2, F32 alpha ) // return color1 + ( color2 - color1 ) * alpha for each component
{
	S16 r1 = S16( ( color1 & 0xff000000 ) >> 24 );
	S16 r2 = S16( ( color2 & 0xff000000 ) >> 24 );
	S16 g1 = S16( ( color1 & 0x00ff0000 ) >> 16 );
	S16 g2 = S16( ( color2 & 0x00ff0000 ) >> 16 );
	S16 b1 = S16( ( color1 & 0x0000ff00 ) >>  8 );
	S16 b2 = S16( ( color2 & 0x0000ff00 ) >>  8 );
	S16 a1 = S16( ( color1 & 0x000000ff ) );
	S16 a2 = S16( ( color2 & 0x000000ff ) );

	return
		( ( U32( r1 + S16( F32( r2 - r1 ) * alpha ) ) ) << 24 ) |
		( ( U32( g1 + S16( F32( g2 - g1 ) * alpha ) ) ) << 16 ) |
		( ( U32( b1 + S16( F32( b2 - b1 ) * alpha ) ) ) <<  8 ) |
		( ( U32( a1 + S16( F32( a2 - a1 ) * alpha ) ) ) );
}

U32 colortime(F32 period_in_secs, F32 normalized_offset)
{
	return colortime( msec(), period_in_secs, normalized_offset );
}

void draw_textured_frame_decreasing_timer( F32 normalized_percent, F32 x, F32 y, F32 hwidth, F32 hheight, U32 color, U32 col2 )
{
	F32 &np = normalized_percent; // from 0 for none to 1.0 for full square

	if( np <= 0.0f ) return;
	else if( np == 1.0f )
	{
		draw_textured_frame( x, y, hwidth, hheight, color, col2 );
		return;
	}

	glBegin( GL_TRIANGLE_FAN );
	glColor4ub( color>>24, (color&0xff0000)>>16, (color&0xff00)>>8, (color&0xff) );
	if( col2 ) glSecondaryColor3ub( col2>>24, (col2&0xff0000)>>16, (col2&0xff00)>>8 );
	glNormal3f( 0, 0, 1 );
	glTexCoord2f( 0.5f, 0.5f ); glVertex3f( x, y, 0 );
	F32 tx=0,ty=0;
	if( hwidth < 0 ) hwidth = (-(hwidth)),tx=1.0f;
	if( hheight < 0 ) hheight = (-(hheight)),ty=1.0f;

	F32 theta = 2.0f * M_PI * normalized_percent;
	F32 sa = sinf( theta );
	F32 ca = cosf( theta );
	F32 asa = ( sa < 0 ? -sa : sa );
	F32 aca = ( ca < 0 ? -ca : ca );

	// clamp to the bounding rectangle so that the magnitude larger of the two is always one
	if( asa > aca )
	{
		sa *= 1.0f / asa;
		ca *= 1.0f / asa;
	}
	else
	{
		sa *= 1.0f / aca;
		ca *= 1.0f / aca;
	}

	F32 x1 = x + sa * hwidth;
	F32 y1 = y + ca * hheight;

	glTexCoord2f( 0.5f + sa * 0.5f, 0.5f - ca * 0.5f ); glVertex3f( x1, y1, 0 );

	if( np > 0.875f ) { glTexCoord2f( tx, ty );glVertex3f( x-hwidth, y+hheight, 0 ); }
	if( np > 0.625f ) { glTexCoord2f( tx, 1-ty );glVertex3f( x-hwidth, y-hheight, 0 ); }
	if( np > 0.375f ) { glTexCoord2f( 1-tx, 1-ty );glVertex3f( x+hwidth, y-hheight, 0 ); }
	if( np > 0.125f ) { glTexCoord2f( 1-tx, ty );glVertex3f( x+hwidth, y+hheight, 0 ); }
	glTexCoord2f( 0.5f, ty ); glVertex3f( x, y+hheight, 0 );

	glEnd();
}

void draw_textured_frame( F32 x, F32 y, F32 hwidth, F32 hheight, U32 color, U32 col2, F32 z, F32 rot )
{
	F32 f[4];
	F32 v[4]={-1,-1,1,1};

	if( rot == 0 )
	{
		f[0] = 0;
		f[1] = 0;
		f[2] = 1;
		f[3] = 1;
	}
	else if( rot == 90 )
	{
		f[0] = 0;
		f[1] = 1;
		f[2] = 1;
		f[3] = 0;
	}
	else if( rot == 180 )
	{
		f[0] = 1;
		f[1] = 1;
		f[2] = 0;
		f[3] = 0;
	}
	else if( rot == 270 )
	{
		f[0] = 1;
		f[1] = 0;
		f[2] = 0;
		f[3] = 1;
	}
	else
	{
		const int off = 128;
		F32 sos = gSinf[45];
		F32 sos2 = sos * 2;
		int ia = int( rot + 0.5f );

		f[0] = ( gSinf[ ( ia +  45 + off ) % 360 ] + sos ) / sos2;
		f[1] = ( gSinf[ ( ia + 135 + off ) % 360 ] + sos ) / sos2;
		f[2] = ( gSinf[ ( ia + 225 + off ) % 360 ] + sos ) / sos2;
		f[3] = ( gSinf[ ( ia + 315 + off ) % 360 ] + sos ) / sos2;
	}

	glBegin( GL_TRIANGLE_FAN );
	glColor4ub( color>>24, (color&0xff0000)>>16, (color&0xff00)>>8, (color&0xff) );
	if( col2 ) glSecondaryColor3ub( col2>>24, (col2&0xff0000)>>16, (col2&0xff00)>>8 );
	glNormal3f( 0, 0, 1 );
	U08 xo=1;
	U08 yo=2;
	U08 vxo=1;
	U08 vyo=0;
	if( hwidth < 0 ) hwidth = (-(hwidth)),xo=3;
	if( hheight < 0 ) hheight = (-(hheight)),yo=0;

	glTexCoord2f( f[xo&3], f[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z ); xo++; yo++; vxo++; vyo++;
	glTexCoord2f( f[xo&3], f[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z ); xo++; yo++; vxo++; vyo++;
	glTexCoord2f( f[xo&3], f[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z ); xo++; yo++; vxo++; vyo++;
	glTexCoord2f( f[xo&3], f[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z );
	glEnd();
}

void draw_textured_frame_pieces_heightfield( F32 x, F32 y, F32 hwidth, F32 hheight, int cols, int rows, int x0, int y0, U32 color, U32 col2, F32 z, F32 **heightfield )
{
	F32 xf[4];
	F32 yf[4];
	F32 v[4]={-1,-1,1,1};
	int o[4]={0,0,1,1};
	F32 oow = 1.0f / F32( cols );
	F32 ooh = 1.0f / F32( rows );
	F32 nx0 = oow * F32( x0 );
	F32 ny0 = ooh * F32( y0 );

	xf[0] = nx0;
	xf[1] = nx0;
	xf[2] = nx0+oow;
	xf[3] = nx0+oow;
		
	yf[0] = ny0;
	yf[1] = ny0;
	yf[2] = ny0+ooh;
	yf[3] = ny0+ooh;

	glBegin( GL_TRIANGLE_FAN );
	glColor4ub( color>>24, (color&0xff0000)>>16, (color&0xff00)>>8, (color&0xff) );
	if( col2 ) glSecondaryColor3ub( col2>>24, (col2&0xff0000)>>16, (col2&0xff00)>>8 );
	glNormal3f( 0, 0, 1 );
	U08 xo=1;
	U08 yo=2;
	U08 vxo=1;
	U08 vyo=0;

	hwidth *= oow;
	hheight *= ooh;
	x += hwidth * 2.0f * ( F32( x0 ) - F32(cols-1) * 0.5f);
	y -= hheight * 2.0f * ( F32( y0 ) - F32(rows-1) * 0.5f);

	if( heightfield )
	{
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, heightfield[y0+!o[vyo&3]][x0+o[vxo&3]] ); xo++; yo++; vxo++; vyo++;
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, heightfield[y0+!o[vyo&3]][x0+o[vxo&3]] ); xo++; yo++; vxo++; vyo++;
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, heightfield[y0+!o[vyo&3]][x0+o[vxo&3]] ); xo++; yo++; vxo++; vyo++;
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, heightfield[y0+!o[vyo&3]][x0+o[vxo&3]] );
	}
	else
	{
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z ); xo++; yo++; vxo++; vyo++;
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z ); xo++; yo++; vxo++; vyo++;
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z ); xo++; yo++; vxo++; vyo++;
		glTexCoord2f( xf[xo&3], yf[yo&3] );glVertex3f( x+v[vxo&3]*hwidth, y+v[vyo&3]*hheight, z );
	}
	glEnd();
}

void draw_textured_frame_pieces( F32 x, F32 y, F32 hwidth, F32 hheight, int cols, int rows, int x0, int y0, U32 color, U32 col2, F32 z )
{
	draw_textured_frame_pieces_heightfield( x, y, hwidth, hheight, cols, rows, x0, y0, color, col2, z );
}

void draw_untextured_frame( F32 x, F32 y, F32 hwidth, F32 hheight, U32 color )
{
	SHADER_TYPE use = UseShader( SHADER_NONE );
	glDisable( GL_TEXTURE_2D );
	glBegin( GL_TRIANGLE_FAN );
	glColor4ub( color>>24, (color&0xff0000)>>16, (color&0xff00)>>8, (color&0xff) );
	glNormal3f( 0, 0, 1 );
	glVertex3f( x-hwidth, y-hheight, 0 );
	glVertex3f( x+hwidth, y-hheight, 0 );
	glVertex3f( x+hwidth, y+hheight, 0 );
	glVertex3f( x-hwidth, y+hheight, 0 );
	glEnd();
	glEnable( GL_TEXTURE_2D );
	UseShader( use );
}

void draw_clear_buffers( bool inverted )
{
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );                 // required to make sure we clear the depth bits
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);	// Black Background
	glClearDepth ( inverted ? -20000.0f : 20000.0f);					// Depth Buffer Setup

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

/*
                [ 2N/(R-L)    0      (R+L)/(R-L)      0      ] 
     P       =  [   0      2N/(T-B)  (T+B)/(T-B)      0      ] 
      Persp     [   0         0     -(F+N)/(F-N)  -2FN/(F-N) ] 
                [   0         0          -1           0      ] 

                [  2/(R-L)    0           0      -(R+L)/(R-L) ] 
     P       =  [   0       2/(T-B)       0      -(T+B)/(T-B) ] 
      Ortho     [   0         0        -2/(F-N)  -(F+N)/(F-N) ] 
                [   0         0           0           1       ] 

                [  2/(R-L)    0           0      -(R+L)/(R-L) ] 
     P       =  [   0      2N/(T-B)  (T+B)/(T-B)      0      ] 
      Hybrid    [   0         0     -(F+N)/(F-N)  -2FN/(F-N) ] 
                [   0         0          -1           0      ] 
*/

void draw_everything( void )
{
	static U32 stamp;
	U32 dtms = msec() - stamp;

	draw_clear_buffers( gUsePerspective == 2 );

	glAlphaFunc(GL_GREATER,0.0f);
	glEnable(GL_ALPHA_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    myDepthFunc( GL_LEQUAL );
	glDepthMask( GL_FALSE );
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	game_draw();

	afont_draw(); // uncomment to test out the font system

	glFlush ();													// Flush The GL Rendering Pipeline

	stamp = msec();

	if( gShowFrameDtms )
	{
		Afont f;
		f.pos( 20, 1080 - 20 );
		f.printf( "%.0f fps %d %02d",
			gFrameRate, getSwapInterval(), dtms );
	}
}

void screengrab( void )
{
    const char *grabfile;
    char filename[1024];

    int i;

	grabfile = "C:\\Screenshots\\comcast_grab";

    for( i=0; i<10000; i++ )
    {
        sprintf( filename, "%s_%04d.png", grabfile, i );
        if( !file_exists( filename ) ) break;
    }

    U32 *pixels = ( U32 * ) malloc( get_width() * get_height() * sizeof( U32 ) );

    FIBITMAP *bitmap = FreeImage_Allocate( get_width(), get_height(), 32 );

    glReadPixels( 0, 0, get_width(), get_height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels );

    int j;
    U32 *p = pixels;

    for( j=0; j<get_height(); j++ )
    {
        for( i=0; i<get_width(); i++ )
        {
            RGBQUAD color;
            color.rgbReserved = 0xff;
            color.rgbRed   = ((*p)>>0)&0xff;
            color.rgbGreen = ((*p)>>8)&0xff;
            color.rgbBlue  = ((*p)>>16)&0xff;
            FreeImage_SetPixelColor( bitmap, i, j, &color );
            p++;
        }
    }

    int rval = FreeImage_Save( FIF_PNG, bitmap, filename, 0 );
	if( !rval ) FreeImage_Save( FIF_PNG, bitmap, "C:\\comcast_screenshot.png", 0 );
}

#if 0
static bool tstate;

int texture( int enable )
{
	if( tstate == enable ) return enable;
	tstate = enable;
	if( enable ) glEnable( GL_TEXTURE_2D );
	else glDisable( GL_TEXTURE_2D );
	return !enable;
}
#endif

void AABB::draw( U32 color )
{
	glDisable( GL_TEXTURE_2D );
	glBegin( GL_LINE_LOOP );
	glColor4ub( color>>24, (color&0xff0000)>>16, (color&0xff00)>>8, (color&0xff) );
	glVertex2f( minx, miny );
	glVertex2f( minx, maxy );
	glVertex2f( maxx, maxy );
	glVertex2f( maxx, miny );
	glEnd();
	glEnable( GL_TEXTURE_2D );
}

static int nDisplays;

Alist displays;
int primary_display;

static BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT r, LPARAM dwData )
{
//	printf( "boo: %x %x\n%d %d %d %d\n", hMonitor, hdcMonitor,
//		r->left, r->right, r->bottom, r->top );

	MONITORINFO minfo;
	GetMonitorInfo( hMonitor, &minfo );

	Dinfo *i = new Dinfo;
	i->rect = *r;
	i->height = r->bottom - r->top;
	i->width = r->right - r->left;
	if( minfo.dwFlags & MONITORINFOF_PRIMARY )
	{
		primary_display = displays;
		i->primary = true;
	}
	else i->primary = false;

	displays += i;

	return true;
}

void draw_init( void )
{
	if( !displays )
	{
		int rval = EnumDisplayMonitors( NULL, NULL, MonitorEnumProc, NULL );
	}

	if( !sSinCosInitted )
	{
		sSinCosInitted = true;
		int i;
		for( i=0; i<360; i++ )
		{
			gSinf[i] = F32(sin( F64( i ) * 2.0 * M_PI / 360.0 ));
			gCosf[i] = F32(cos( F64( i ) * 2.0 * M_PI / 360.0 ));
		}
	}
}

U32 getNumDisplays( void )
{
	draw_init();
	return displays;
}

int getDisplayWidth( int display )
{
	draw_init();
	if( display < 0 ) display = primary_display;
	Dinfo *i = ( Dinfo * ) displays[display];
	return i ? i->width : 0;
}

int getDisplayHeight( int display )
{
	draw_init();
	if( display < 0 ) display = primary_display;
	Dinfo *i = ( Dinfo * ) displays[display];
	return i ? i->height : 0;
}

const Dinfo &getDisplayInfo( int display )
{
	draw_init();
	Dinfo *i = ( Dinfo * ) displays[display];
	if( !i )
	{
		static Dinfo tmp;
		return tmp;
	}
	else
	{
		return *i;
	}
}

#undef glPushMatrix
#undef glPopMatrix

static int glLevel;

void myPushMatrix( void )
{
	glLevel++;
	if( 1 || glLevel > 2 )
	{
		printf( "glLevel increase %d\n", glLevel );
	}
	glPushMatrix();
}

void myPopMatrix( void )
{
	glLevel--;
	if( 1 || glLevel > 2 )
	{
		printf( "glLevel reduced %d\n", glLevel );
	}
	glPopMatrix();
}

static AImage *sCircle;

void bind_circle_texture( void )
{
	if( !sCircle )
	{
		sCircle = new AImage( file_find( "circle.png" ) );
	}

	sCircle->bindtex();
}

U32 get_refresh_frequency( void )
{
	DEVMODE lpDevMode;
	memset(&lpDevMode, 0, sizeof(DEVMODE));
	lpDevMode.dmSize = sizeof(DEVMODE);
	lpDevMode.dmDriverExtra = 0;

	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode) == 0)
	{
		gFrameRate = 60.0f;
	}

	return lpDevMode.dmDisplayFrequency;
}