#ifndef __DRAW_H__
#define __DRAW_H__

U32 ncolortime(F32 n); // normalized colortime -- lowest level func
U32 colortime(F32 period_in_secs, F32 normalized_offset=0.0f);
U32 colortime(U32 msec, F32 period_in_secs, F32 normalized_offset=0.0f);
U32 blendcolors( U32 color1, U32 color2, F32 blend ); // return color1 + ( color2 - color1 ) * blend for each component / byte
void myPushMatrix( void );
void myPopMatrix( void );

struct Dinfo // my display info .. one instance per screen
{
	bool primary;
	int width;
	int height;
	RECT rect;
};

class AABB // axis aligned bounding box - for debugging and trivial rejection
{
public:
	AABB()
	{
		reset();
	}

	void reset( void )
	{
		minx =  1.0e+37f;
		miny =  1.0e+37f;
		maxx = -1.0e+37f;
		maxy = -1.0e+37f;
	}

	void expand( F32 pt[2] )
	{
		if( pt[0] < minx ) minx = pt[0];
		if( pt[0] > maxx ) maxx = pt[0];
		if( pt[1] < miny ) miny = pt[1];
		if( pt[1] > maxy ) maxy = pt[1];
	}

	void expand( AABB *bbox )
	{
		if( bbox->minx < minx ) minx = bbox->minx;
		if( bbox->maxx > maxx ) maxx = bbox->maxx;
		if( bbox->miny < miny ) miny = bbox->miny;
		if( bbox->maxy > maxy ) maxy = bbox->maxy;
	}

	void expand( F32 radius )
	{
		minx -= radius;
		maxx += radius;
		miny -= radius;
		maxy += radius;
	}
	
	bool overlap( AABB *bbox )
	{
		if( bbox->maxx < minx ) return 0;
		if( bbox->minx > maxx ) return 0;
		if( bbox->maxy < miny ) return 0;
		if( bbox->miny > maxy ) return 0;
		return 1;
	}

	void draw( U32 color=0xffffffff );

public:
	F32 minx,miny;
	F32 maxx,maxy;
};

void draw_everything( void );

void draw_clear_buffers( bool inverted );
void draw_textured_frame( F32 x, F32 y, F32 hwidth, F32 hheight, U32 color, U32 col2=0, F32 z=0, F32 trot=0 );
void draw_textured_frame_pieces( F32 x, F32 y, F32 hwidth, F32 hheight, int hpiececount, int vpiececount, int hoff, int voff, U32 color, U32 col2=0, F32 z=0 );
void draw_textured_frame_pieces_heightfield( F32 x, F32 y, F32 hwidth, F32 hheight, int cols, int rows, int x0, int y0, U32 color, U32 col2, F32 z, F32 **heightfield=NULL );
void draw_textured_frame_decreasing_timer( F32 normalized_timer_remaining, F32 x, F32 y, F32 hwidth, F32 hheight, U32 color, U32 col2=0 );
void draw_untextured_frame( F32 x, F32 y, F32 hwidth, F32 hheight, U32 color );
void draw_in_screen_coord( void );
void draw_in_shadow_coord( U32 w, U32 h );
void draw_in_portrait_screen_coord( void );
void draw_in_landscape_screen_coord( void );
void draw_in_portrait_perspective_screen_coord( F32 camera_dist=0.0f, int width=0, int height=0 );
void draw_in_landscape_perspective_screen_coord( F32 camera_dist=0.0f );
void draw_in_external_camera_perspective( F32 aspect_ratio /* EG: 640.0/480.0f */ );

void get_camera_position( F32 dst[3] ); // this is undefineded when not in Perspective Camera mode
bool getSync( void );
void setSync( bool onoff );
void setSwapInterval( int interval );
int  getSwapInterval( void );
void myCullFace( U32 mode );
void myTexRot( F32 angle, bool topbottom=true );
void myDepthFunc( U32 mode );
void myMaterial( GLenum face, GLenum pname, U32 color, F32 colormod=1.0f, F32 alphamod=1.0f );
void setupLighting( void );
void defaultLightMaterial( void );
void bind_circle_texture( void );

void draw_init( void );

U32 getNumDisplays( void );
int getDisplayWidth( int display=-1 ); // defaults to primary display
int getDisplayHeight( int display=-1 );
void world2screen( F32 dst_screen[3], F32 src_world[3] );
void screen2world( F32 x, F32 y, F32 z, F32 &dst_x, F32 &dst_y );
const Dinfo &getDisplayInfo( int display );

void screengrab( void );
U32 get_refresh_frequency( void );

#endif /* !__DRAW_H__ */
