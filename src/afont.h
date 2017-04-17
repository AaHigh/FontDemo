#ifndef __AFONT_H__
#define __AFONT_H__

#define AF_NMK24 AF_NMK18

typedef enum
{
	AF_FIXEDSYS=0,
	AF_ARIAL24,
	AF_ARIAL48,
	AF_CENTURY20,
	AF_KOZUKA17NUMS,
	AF_KOZUKA17,
	AF_KOZUKA24,
	AF_KOZUKA28,
	AF_KOZUKA57,
	AF_LUCIDAHAND51,
	AF_CURRENCY,
	AF_LED,
	AF_BIGLED,
	AF_SILKSCREEN,
	AF_SILKSCREEN_NOGLOW,
	AF_ARIALNUMS,
	AF_NMK18,
	AF_ROUNDED24,

	AF_NUM_FONTS
}
AF_FONT;

typedef enum
{
    AF_LEFT = 0,
    AF_CENTER,
    AF_RIGHT,
}
AF_JUSTIFICATION;

class Afont
{
public:
	Afont();
	~Afont();

private:
	void sanity( void );
	F32  hwidth_until_return( const char *c );

public:
	void box  ( bool on_off=1, U32 boxcolor=0x7f7f7f7f, U08 box_drop_shadow_dist=3 );
	void font ( int );
	void pos  ( F32 x, F32 y, F32 z=0 );
	void pos  ( int x, int y ) { pos( F32(x), F32(y) ); }
	void move ( F32 dx, F32 dy, F32 dz=0 );
	void color( U32 cpack=0xffffffff, U32 cpack2=0 );
	U32  getcolor( void ) { return col; }
	U32  getalpha( void ) { return col & 0xff; }
	void colortime( F32 period, F32 normalized_offset=0.0f );
	void scale( F32 x, F32 y=-1.0f );
	void just ( int );
	void snap ( bool snap );
	void shade( SHADER_TYPE _shader ) { shader = _shader; }

	F32 length   ( const char *str ); // does same as print without the printing
	F32 lengthf  ( const char *str, ... );
	F32 height   ( void );
	F32 print    ( const char *str );
	F32 print_ds ( const char *str ); // Drop shadow also applied
	F32 print_box( const char *str ); // Put box behind text
	F32 printf   ( const char *str, ... );
	F32 printf_ds( const char *str, ... );

	F32 box_print ( U32 box_color, const char *str );
	F32 box_printf( U32 box_color, const char *str, ... );

	F32 print_squeezed    ( F32 size, const char *str );
	F32 printf_squeezed   ( F32 size, const char *str, ... );
	F32 print_squeezed_ds ( F32 size, const char *str );
	F32 printf_squeezed_ds( F32 size, const char *str, ... );

	void bindtex( void );
	void charbox( int c, F32 *htx, F32 *hty, F32 *tx, F32 *ty );
	void drop( F32 drop=0.0f );
	void nodrop( void );

	F32 getx( void ) const { return x; }
	F32 gety( void ) const { return y; }
	F32 getsx( void ) const { return sx; }
	F32 getsy( void ) const { return sy; }

private:
	F32 x,y,z;  // afont_pos()
    F32 sx,sy;  // afont_scale()
	F32 squeeze;// internal state dependent on scale
	U32 col;    // afont_color()
	U32 col2;   // afont_color(col,col2);
    U08 fid;    // afont_font()
    U08 justification; // afont_just()
	U08 dropshad;   // afont_drop()
	U08 box_drop;
	bool drawbox;
	bool nosnap;
	U32 boxcolor;
	F32 boxborder;
	SHADER_TYPE shader;
};

void afont_init( void );

void afont_box      ( Afont *f, bool on_off=1, U32 boxcolor=0x7f7f7f7f, U08 box_drop_shadow_dist=3 );
void afont_font     ( Afont *f, int );
void afont_pos      ( Afont *f, F32 x, F32 y, F32 z=0 );
void afont_move     ( Afont *f, F32 dx, F32 dy, F32 dz=0 );
void afont_color    ( Afont *f, U32 cpack, U32 cpack2=0 );
void afont_colortime( Afont *f, F32 period, F32 normalized_offset );
void afont_scale    ( Afont *f, F32 x, F32 y=-1.0f );
void afont_just     ( Afont *f, int );
void afont_snap     ( Afont *f, bool snap );
F32  afont_length   ( Afont *f, const char *str ); // does same as print without the printing
F32  afont_lengthf  ( Afont *f, const char *str, ... );
F32  afont_print    ( Afont *f, const char *str );
F32  afont_print_ds ( Afont *f, const char *str ); // Drop shadow also applied
F32  afont_print_box( Afont *f, const char *str ); // Put box behind text
F32  afont_printf   ( Afont *f, const char *str, ... );
F32  afont_printf_ds( Afont *f, const char *str, ... );

F32  afont_box_print ( Afont *f, U32 box_color, const char *str );
F32  afont_box_printf( Afont *f, U32 box_color, const char *str, ... );

F32  afont_print_squeezed    ( Afont *f, F32 size, const char *str );
F32  afont_printf_squeezed   ( Afont *f, F32 size, const char *str, ... );
F32  afont_print_squeezed_ds ( Afont *f, F32 size, const char *str );
F32  afont_printf_squeezed_ds( Afont *f, F32 size, const char *str, ... );

// Expose some low-level functionality:
// Just does the glBindTexture() to allow using texels in other draw routines
void afont_bindtex( Afont *f );

// This goes along with the bindtex funciton above .. low level access to
// the texture coordinates for a character
void afont_charbox( Afont *f, int c, F32 *htx, F32 *hty, F32 *tx, F32 *ty );

// set drop shadow distances .. 0 dist == disable
void afont_drop ( Afont *f, F32 dist=0.0f );
void afont_nodrop( Afont *f );

void afont_test( void ); // for testing only
void afont_draw( void ); // for testing only

void afont_state_push( void );
void afont_state_pop( void );

#endif /* !__AFONT_H__ */
