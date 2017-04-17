#ifndef __QUAT_H__
#define __QUAT_H__

#ifndef RAD2DEG
#define RAD2DEG(x) ((x)*(180.0/M_PI))
#endif

#ifndef DEG2RAD
#define DEG2RAD(x) ((x)*(M_PI/180.0f))
#endif

struct KeyFrame
{
	U32 ms;
	F32 p[3];
	F32 q[4];
};

void lerp( F32 v1[3], F32 v2[3], F32 dst[3], F32 alpha );
void slerp( const F32 q1[4], const F32 q2[4], F32 qr[4], F32 alpha );
F32 sinify( F32 input );
F32 spinify( F32 input );
void cross( F32 dst[3], F32 a[3], F32 b[3] );
void rand_init( void );
U64 rand64( void ); // from 0 to 0xffffffffffffffff inclusive
U32 rand32( void ); // from 0 to 0xffffffff inclusive
F32 randf( void ); // from 0.0f to 1.0f inclusive
F64 randd( void ); // from 0.0 to 1.0 inclusive
int randi( int min, int max );
void randsphere( F32 dst[3] );
F32 veclength( F32 vec[3] );
F32 veclength2( F32 vec[3] );
F32 abdist( F32 a[3], F32 b[3] );
F32 abdist2( F32 a[3], F32 b[3] );
F32 abdist( F32 x1, F32 y1, F32 x2, F32 y2 );
F32 abdist2( F32 x1, F32 y1, F32 x2, F32 y2 );
F32 abdist( F32 x1, F32 y1, F32 z1, F32 x2, F32 y2, F32 z2 );
F32 abdist2( F32 x1, F32 y1, F32 z1, F32 x2, F32 y2, F32 z2 );
void scale3v( F32 dst[3], F32 src[3], F32 scale );
int sign( F32 value );
void norm2f( F32 v[2] );
void norm3f( F32 v[3] );
void norm4f( F32 v[4] );
void zero2f( F32 v[3] );
void zero3f( F32 v[3] );
void zero4f( F32 v[3] );
F32 dot2f( const F32 a[2], const F32 b[2] );
F32 dot3f( const F32 a[3], const F32 b[3] );
F32 dot4f( const F32 a[4], const F32 b[4] );
void quaternionShortestDist( const F32 q0[4], F32 q1[4] );
void quaternionToMatrix( const F32 q[4], F32 m[3][3] );
void axisAngleToMatrix( const F32 angle, const F32 x, const F32 y, const F32 z, F32 m[4][4] );
void matrixToQuaternion( const F32 a[4][4], F32 q[4] );
void copy4f( const F32 src[4], F32 dst[4] );
void copy3f( const F32 src[3], F32 dst[3] );
void copy3i( const int src[3], int dst[3] );
void copy4i( const int src[4], int dst[4] );
void copy3u( const U32 src[3], U32 dst[3] );
void copy4u( const U32 src[4], U32 dst[4] );
void add3f( const F32 a[3], const F32 b[3], F32 dst[3] );
void mymultmatrix( F32 dst[4][4], const F32 b[4][4] ); // a*=b
void mymultmatrix( F32 dst[4][4], const F32 a[4][4], const F32 b[4][4] ); // dst=a*b
void mult( F32 dst[3], F32 m[4][4], F32 src[3] );
void copymatrix( F32 dst[4][4], const F32 src[4][4] );
void mult( F32 a[4], F32 b[4], F32 dst[4] ); // Multiplying q1 with q2 applies the rotation q2 to q1
void axisAngleToQuaternion( F32 angle, F32 axis[3], F32 q[4] );
void quaternionToAxisAngle( F32 q[4], F32 *angle, F32 axis[3] );
void axisAngleToMatrix( F32 angle, F32 axis[3], F32 m[4][4] );
void quaternionToMatrix( const F32 q[4], F32 m[4][4] );
void identity( F32 m[4][4] );
void identity( F32 q[4] );
void world2local( const F32 world[3], const F32 matrix[4][4], F32 local_dst[3] );
void local2world( const F32 local[3], const F32 matrix[4][4], F32 world_dst[3] );
void rotatez( const F32 angle, const F32 vec[3], F32 dst[3] ); // rotate a point about the world z-axis (basic 2d rotation)
void mk_pos_mat( F32 m[4][4], F32 x, F32 y, F32 z );
void mk_rotatez_mat( F32 m[4][4], F32 angle, F32 x=0.0f, F32 y=0.0f, F32 z=0.0f );
void printmatrix( const F32 *m );
bool invert_matrix( const F32 src[4][4], F32 dst[4][4] );

#endif /* !__QUAT_H__ */