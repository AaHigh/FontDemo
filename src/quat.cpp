#include "stdafx.h"

void lerp( F32 v1[3], F32 v2[3], F32 dst[3], F32 alpha )
{
	dst[0] = v1[0] + ( v2[0] - v1[0] ) * alpha;
	dst[1] = v1[1] + ( v2[1] - v1[1] ) * alpha;
	dst[2] = v1[2] + ( v2[2] - v1[2] ) * alpha;
}

void slerp(const F32 q1[4], const F32 q2[4], F32 qr[4], F32 alpha) 
{
	F32 dotproduct = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];
	F32 theta, st, sut, sout, coeff1, coeff2;

	// algorithm adapted from Shoemake's paper
    alpha = alpha / 2.0f;

    if( dotproduct >= 0.99999f ) // same quaternion in q1 and q2
    {
        qr[0] = q1[0];
        qr[1] = q1[1];
        qr[2] = q1[2];
        qr[3] = q1[3];
        return;
    }
	theta = (F32) acos(dotproduct);
    if (theta < 0.0f) theta=-theta;
	
	st = sinf(theta);
	sut = sinf(alpha*theta);
	sout = sinf((1-alpha)*theta);
	coeff1 = sout/st;
	coeff2 = sut/st;

//	printf( "c1: %.2f c2: %.2f\n", coeff1, coeff2 );
	qr[0] = coeff1*q1[0] + coeff2*q2[0];
	qr[1] = coeff1*q1[1] + coeff2*q2[1];
	qr[2] = coeff1*q1[2] + coeff2*q2[2];
	qr[3] = coeff1*q1[3] + coeff2*q2[3];

    norm4f( qr );
}

F32
sinify( F32 value )
{
	if( value <= 0.0f ) return 0.0f;
	else if( value >= 1.0f ) return 1.0f;
	else return ( 1.0f - cosf( value * M_PI ) ) * 0.5f;
}

F32
spinify( F32 value )
{
	F32 midpoint = 0.9f;
	if( value <= 0.0f ) return 0.0f;
	return sinify( value * value );
}

void
cross( F32 dst[3], F32 a[3], F32 b[3] )
{
	F32 tmp[3];

	tmp[0] = a[1] * b[2] - a[2] * b[1];
	tmp[1] = a[2] * b[0] - a[0] * b[2];
	tmp[2] = a[0] * b[1] - a[1] * b[0];

	dst[0] = tmp[0];
	dst[1] = tmp[1];
	dst[2] = tmp[2];
}

U64 GetUnixTime()
{
    SYSTEMTIME sysUnixEpoch;
    sysUnixEpoch.wYear = 1970;
    sysUnixEpoch.wMonth = 1;
    sysUnixEpoch.wDayOfWeek = 4;
    sysUnixEpoch.wDay = 1;
    sysUnixEpoch.wHour = 0;
    sysUnixEpoch.wMinute = 0;
    sysUnixEpoch.wSecond = 0;
    sysUnixEpoch.wMilliseconds = 0;

    FILETIME unixEpoch;
    SystemTimeToFileTime(&sysUnixEpoch, &unixEpoch);

    ULARGE_INTEGER unixEpochValue;
    unixEpochValue.HighPart = unixEpoch.dwHighDateTime;
    unixEpochValue.LowPart = unixEpoch.dwLowDateTime;

    FILETIME systemTime;
    GetSystemTimeAsFileTime(&systemTime);

    ULARGE_INTEGER systemTimeValue;
    systemTimeValue.HighPart = systemTime.dwHighDateTime;
    systemTimeValue.LowPart = systemTime.dwLowDateTime;

    U64 diffHundredNanos = systemTimeValue.QuadPart - unixEpochValue.QuadPart;

	return diffHundredNanos / 10000;
}

void
rand_init( void )
{
	srand( int( GetUnixTime() ) );
}

U64
rand64( void )
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	return gen();
}

U32
rand32( void )
{
	std::random_device rd;
	std::mt19937 gen(rd());
	return gen();
}

F32
randf( void )
{
	const U32 rmax = ULONG_MAX >> 9;
	const U32 r = rand32() >> 9;
	return (F32(r)/F32(rmax));
}

F64
randd( void )
{
	const U64 rmax = ULLONG_MAX >> 12; // 11-bit exponent + 1-bit sign
	const U64 r = rand64() >> 12;
	return F64(r)/F64(rmax);
}

int
randi( int min, int max )
{
	if( min > max )
	{
		int tmp = min;
		min = max;
		max = tmp;
	}

	F64 d = randd();
	F64 drval = 0.5 + min + d * F64( max - min );
	int rval = int( drval );
	if( rval > max ) rval = max;
	return rval;
}

void
randsphere( F32 dst[3] )
{
	do
	{
		dst[0] = randf() * 2.0f - 1.0f;
		dst[1] = randf() * 2.0f - 1.0f;
		dst[2] = randf() * 2.0f - 1.0f;
	}
	while( dst[0] * dst[0] + dst[1] * dst[1] + dst[2] * dst[2] > 1.0f );
}

void
quaternionToMatrix( const F32 q[4], F32 m[3][3] )
{
	const F32 &x = q[0];
	const F32 &y = q[1];
	const F32 &z = q[2];
	const F32 &w = q[3];

	const F32 t = 2.0f;
	const F32 l = 1.0f;

	m[0][0] = l - t * y * y - t * z * z;
	m[0][1] =     t * x * y - t * z * w;
	m[0][2] =     t * x * z + t * y * w;

	m[1][0] =     t * x * y + t * z * w;
	m[1][1] = l - t * x * x - t * z * z;
	m[1][2] =     t * y * z - t * x * w;

	m[2][0] =     t * x * z - t * y * w;
	m[2][1] =     t * y * z + t * x * w;
	m[2][2] = l - t * x * x - t * y * y;
}

void
axisAngleToMatrix( F32 angle, F32 x, F32 y, F32 z, F32 m[4][4] )
{
	if( angle == 0 || ( x * x + y * y + z * z ) < 0.000001f )
	{
		::identity( m );
		return;
	}

	F32 rad = angle * ( F32(M_PI) / 180.0f );
	F32 c = cosf( rad );
	F32 s = sinf( rad );
	F32 t = 1.0f - c;

	m[0][0] = t * x * x + c;
	m[0][1] = t * x * y - s * z;
	m[0][2] = t * x * z + s * y;

	m[1][0] = t * x * y + s * z;
	m[1][1] = t * y * y + c;
	m[1][2] = t * y * z - s * x;

	m[2][0] = t * x * z - s * y;
	m[2][1] = t * y * z + s * x;
	m[2][2] = t * z * z + c;
}

void
matrixToQuaternion( const F32 a[4][4], F32 q[4] )
{
	F32 &x = q[0];
	F32 &y = q[1];
	F32 &z = q[2];
	F32 &w = q[3];

	F32 trace = a[0][0] + a[1][1] + a[2][2];

	if( trace > 0 )
	{
		F32 s = 0.5f / sqrtf(trace+ 1.0f);
		w = 0.25f / s;
		x = ( a[2][1] - a[1][2] ) * s;
		y = ( a[0][2] - a[2][0] ) * s;
		z = ( a[1][0] - a[0][1] ) * s;
	}
	else
	{
		if ( a[0][0] > a[1][1] && a[0][0] > a[2][2] )
		{
			F32 s = 2.0f * sqrtf( 1.0f + a[0][0] - a[1][1] - a[2][2]);
			w = (a[2][1] - a[1][2] ) / s;
			x = 0.25f * s;
			y = (a[0][1] + a[1][0] ) / s;
			z = (a[0][2] + a[2][0] ) / s;
		}
		else if (a[1][1] > a[2][2])
		{
			F32 s = 2.0f * sqrtf( 1.0f + a[1][1] - a[0][0] - a[2][2]);
			w = (a[0][2] - a[2][0] ) / s;
			x = (a[0][1] + a[1][0] ) / s;
			y = 0.25f * s;
			z = (a[1][2] + a[2][1] ) / s;
		}
		else
		{
			F32 s = 2.0f * sqrtf( 1.0f + a[2][2] - a[0][0] - a[1][1] );
			w = (a[1][0] - a[0][1] ) / s;
			x = (a[0][2] + a[2][0] ) / s;
			y = (a[1][2] + a[2][1] ) / s;
			z = 0.25f * s;
		}
	}

	F32 size = x*x+y*y+z*z+w*w;
	if( size > 1.01 )
	{
		F32 oosize = 1.0f / sqrtf( size );
		x*=oosize;
		y*=oosize;
		z*=oosize;
		w*=oosize;
	}
}

void copy4f( const F32 src[4], F32 dst[4] )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void copy3i( const int src[3], int dst[3] )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void copy4i( const int src[4], int dst[4] )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void copy3u( const U32 src[3], U32 dst[3] )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void copy4u( const U32 src[4], U32 dst[4] )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void copy3f( const F32 src[3], F32 dst[3] )
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void add3f( const F32 a[3], const F32 b[3], F32 dst[3] )
{
	F32 x,y,z;

	x = a[0] + b[0];
	y = a[1] + b[1];
	z = a[2] + b[2];

	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}

// Multiplying q1 with q2 applies the rotation q2 to q1
void mult( F32 a[4], F32 b[4], F32 dst[4] )
{
#if 0
	F32 m1[4][4];
	F32 m2[4][4];
	quaternionToMatrix( a, m1 );
	quaternionToMatrix( b, m2 );
	mymultmatrix( m1, m2 );
	matrixToQuaternion( m1, dst );
#else
	F32 tmp[4];

	tmp[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
	tmp[1] = a[3] * b[1] + a[1] * b[3] + a[2] * b[0] - a[0] * b[2];
	tmp[2] = a[3] * b[2] + a[2] * b[3] + a[0] * b[1] - a[1] * b[0];
	tmp[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];

	dst[0] = tmp[0];
	dst[1] = tmp[1];
	dst[2] = tmp[2];
	dst[3] = tmp[3];

	if( 0 && dst[0] * dst[0] + dst[1] * dst[1] + dst[2] * dst[2] < 0.0000001f )
	{
		dst[0] = 1.0f;
		dst[1] = 0.0f;
		dst[2] = 0.0f;
		dst[3] = 0.0f;
	}
#if 0
Quaternion Quaternion::operator* (const Quaternion &rq) const
	// the constructor takes its arguments as (x, y, z, w)
	return Quaternion(w * rq.x + x * rq.w + y * rq.z - z * rq.y,
	                  w * rq.y + y * rq.w + z * rq.x - x * rq.z,
	                  w * rq.z + z * rq.w + x * rq.y - y * rq.x,
	                  w * rq.w - x * rq.x - y * rq.y - z * rq.z);
#endif
#endif
}

void
axisAngleToMatrix( F32 angle, F32 axis[3], F32 m[4][4] )
{
	F32 q[4];
	axisAngleToQuaternion( angle, axis, q );
	identity( m );
	quaternionToMatrix( q, m );
}

void
axisAngleToQuaternion( F32 angle, F32 axis[3], F32 q[4] )
{
#if 1
	F32 dist2 = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2];
	if( dist2 < 0.99f || dist2 > 1.01f )
	{
		printf( "Non-normalized axis!\n" );
	}
#endif

	if( 0 && -0.000001 < angle && angle < 0.000001 )
	{
		q[0] = 1.0f;
		q[1] = 0.0f;
		q[2] = 0.0f;
		q[3] = 0.0f;
	}
	else
	{
		F32 ao2 = ( F32(M_PI) / 180.0f ) * angle * 0.5f;
		F32 sao2 = sinf( ao2 );

		q[0] = axis[0] * sao2;
		q[1] = axis[1] * sao2;
		q[2] = axis[2] * sao2;
		q[3] = cosf(ao2);
	}
}

void
quaternionToAxisAngle( F32 q[4], F32 *angle, F32 axis[3] )
{
	if( angle ) *angle = 2.0f * acosf( q[3] ) * F32( 180.0 / M_PI );
	F32 denom = sqrtf( 1.0f - q[3] * q[3] );
	if( denom > 0 )
	{
		F32 scale = 1.0f / denom;
		axis[0] = q[0] * scale;
		axis[1] = q[1] * scale;
		axis[2] = q[2] * scale;
	}
	else
	{
		if( angle ) *angle = 0.0f;
		axis[0] = 1.0f;
		axis[1] = 0.0f;
		axis[2] = 0.0f;
	}

	F32 axlen_squared = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2];

	if( axlen_squared > 1.0001 || axlen_squared < 0.9999f )
	{
		F32 dist = sqrtf( axlen_squared );
		F32 oodist = 1.0f / dist;
		axis[0] *= oodist;
		axis[1] *= oodist;
		axis[2] *= oodist;
		if( angle ) *angle *= dist;
	}
}

F32 veclength( F32 vec[3] )
{
    return sqrtf( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
}

F32 veclength2( F32 vec[3] )
{
    return ( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
}

F32 abdist( F32 a[3], F32 b[3] )
{
	F32 dvec[3] = { a[0] - b[0], a[1] - b[1], a[2] - b[2] };
	return veclength( dvec );
}

F32 abdist2( F32 a[3], F32 b[3] )
{
	F32 dvec[3] = { a[0] - b[0], a[1] - b[1], a[2] - b[2] };
	return veclength2( dvec );
}

F32 abdist( F32 x1, F32 y1, F32 z1, F32 x2, F32 y2, F32 z2 )
{
	F32 dvec[3] = { x1 - x2, y1 - y2, z1 - z2 };
	return veclength( dvec );
}

F32 abdist2( F32 x1, F32 y1, F32 z1, F32 x2, F32 y2, F32 z2 )
{
	F32 dvec[3] = { x1 - x2, y1 - y2, z1 - z2 };
	return veclength2( dvec );
}

F32 abdist( F32 x1, F32 y1, F32 x2, F32 y2 )
{
	F32 dvec[3] = { x1 - x2, y1 - y2, 0 };
	return veclength( dvec );
}

F32 abdist2( F32 x1, F32 y1, F32 x2, F32 y2 )
{
	F32 dvec[3] = { x1 - x2, y1 - y2, 0 };
	return veclength2( dvec );
}

void scale3v( F32 dst[3], F32 src[3], F32 scale )
{
	dst[0] = src[0] * scale;
	dst[1] = src[1] * scale;
	dst[2] = src[2] * scale;
}

int sign( F32 value )
{
	if( value == 0.0f ) return 0;
	else if( value > 0 ) return 1;
	else return -1;
}

void norm2f( F32 v[2] )
{
    F32 oodist = 1.0f / sqrtf( v[0] * v[0] + v[1] * v[1] );
    v[0] *= oodist;
    v[1] *= oodist;
}

void norm3f( F32 v[3] )
{
    F32 oodist = 1.0f / sqrtf( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
    v[0] *= oodist;
    v[1] *= oodist;
    v[2] *= oodist;
}

void norm4f( F32 v[4] )
{
    F32 oodist = 1.0f / sqrtf( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3] );
    v[0] *= oodist;
    v[1] *= oodist;
    v[2] *= oodist;
    v[3] *= oodist;
}

void zero2f( F32 v[2] )
{
	v[0] = 0;
	v[1] = 0;
}

void zero3f( F32 v[3] )
{
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
}

void zero4f( F32 v[4] )
{
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
	v[3] = 0;
}

F32 dot2f( const F32 a[2], const F32 b[2] )
{
	return a[0] * b[0] + a[1] * b[1];
}

F32 dot3f( const F32 a[3], const F32 b[3] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

F32 dot4f( const F32 a[4], const F32 b[4] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

void quaternionShortestDist( const F32 q0[4], F32 q1[4] )
{
	if( dot4f( q0, q1 ) < 0 )
	{
		q1[0] = -q1[0];
		q1[1] = -q1[1];
		q1[2] = -q1[2];
		q1[3] = -q1[3];
	}
}

void copymatrix( F32 dst[4][4], const F32 src[4][4] )
{
	dst[0][0] = src[0][0];
	dst[0][1] = src[0][1];
	dst[0][2] = src[0][2];
	dst[0][3] = src[0][3];

	dst[1][0] = src[1][0];
	dst[1][1] = src[1][1];
	dst[1][2] = src[1][2];
	dst[1][3] = src[1][3];

	dst[2][0] = src[2][0];
	dst[2][1] = src[2][1];
	dst[2][2] = src[2][2];
	dst[2][3] = src[2][3];

	dst[3][0] = src[3][0];
	dst[3][1] = src[3][1];
	dst[3][2] = src[3][2];
	dst[3][3] = src[3][3];
}

void mult( F32 dst[3], F32 m[4][4], F32 src[3] )
{
	F32 tmp[4];

	tmp[0] = m[0][0] * src[0] + m[1][0] * src[1] + m[2][0] * src[2] + m[3][0];
	tmp[1] = m[0][1] * src[0] + m[1][1] * src[1] + m[2][1] * src[2] + m[3][1];
	tmp[2] = m[0][2] * src[0] + m[1][2] * src[1] + m[2][2] * src[2] + m[3][2];
	tmp[3] = m[0][3] * src[0] + m[1][3] * src[1] + m[2][3] * src[2] + m[3][3];

	if( ( tmp[3] > 1.000001f || tmp[3] < 0.99999999f ) && tmp[3] > 0 )
	{
		F32 oom3 = 1.0f / tmp[3];
		dst[0] = tmp[0] * oom3;
		dst[1] = tmp[1] * oom3;
		dst[2] = tmp[2] * oom3;
	}
	else
	{
		dst[0] = tmp[0];
		dst[1] = tmp[1];
		dst[2] = tmp[2];
	}
}

void mymultmatrix( F32 a[4][4], F32 b[4][4] )
{
	F32 m[4][4];

	int i,j,k;

	for( i=0; i<4; i++ )
	{
		for( j=0; j<4; j++ )
		{
			m[i][j] = 0.0f;

			for( k=0; k<4; k++ )
			{
				m[i][j] += a[i][k] * b[k][j];
			}
		}
	}

	memcpy( a, m, sizeof( m ) );
}

void mymultmatrix( F32 _dst[4][4], const F32 a[4][4], const F32 b[4][4] )
{
	F32 tmp[4][4];
	F32 (*dst)[4][4];

	if( _dst != a && _dst != b )
	{
		dst = (F32(*)[4][4]) _dst;
	}
	else dst = &tmp;

	int i,j,k;

	for( i=0; i<4; i++ )
	{
		for( j=0; j<4; j++ )
		{
			(*dst)[i][j] = 0.0f;

			for( k=0; k<4; k++ )
			{
				(*dst)[i][j] += a[i][k] * b[k][j];
			}
		}
	}

	if( dst == &tmp )
	{
		memcpy( _dst, tmp, sizeof( tmp ) );
	}
}

void
quaternionToMatrix( const F32 q[4], F32 m[4][4] )
{
	const F32 &x = q[0];
	const F32 &y = q[1];
	const F32 &z = q[2];
	const F32 &w = q[3];

	const F32 t = 2.0f;
	const F32 l = 1.0f;

	m[0][0] = l - t * y * y - t * z * z;
	m[0][1] =     t * x * y - t * z * w;
	m[0][2] =     t * x * z + t * y * w;

	m[1][0] =     t * x * y + t * z * w;
	m[1][1] = l - t * x * x - t * z * z;
	m[1][2] =     t * y * z - t * x * w;

	m[2][0] =     t * x * z - t * y * w;
	m[2][1] =     t * y * z + t * x * w;
	m[2][2] = l - t * x * x - t * y * y;
}

void
identity( F32 m[4][4] )
{
	m[0][0] = 1.0f;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void
identity( F32 q[4] )
{
	q[0] = 0.0f;
	q[1] = 0.0f;
	q[2] = 0.0f;
	q[3] = 1.0f;
}

void
world2local( const F32 w[3], const F32 m[4][4], F32 l[3] )
{
	F32 x,y,z;

	x = w[0] - m[3][0];
	y = w[1] - m[3][1];
	z = w[2] - m[3][2];

	l[0] = x * m[0][0] + y * m[0][1] + z * m[0][2];
	l[1] = x * m[1][0] + y * m[1][1] + z * m[1][2];
	l[2] = x * m[2][0] + y * m[2][1] + z * m[2][2];
}

void
local2world( const F32 l[3], const F32 m[4][4], F32 w[3] )
{
	F32 x,y,z;

	x = m[3][0] + l[0] * m[0][0] + l[1] * m[1][0] + l[2] * m[2][0];
	y = m[3][1] + l[0] * m[0][1] + l[1] * m[1][1] + l[2] * m[2][1];
	z = m[3][2] + l[0] * m[0][2] + l[1] * m[1][2] + l[2] * m[2][2];

	w[0] = x;
	w[1] = y;
	w[2] = z;
}

void
rotatez( const F32 angle, const F32 p[3], F32 dst[3] )
{
	F32 theta = F32( M_PI * 2.0 ) * ( angle / 360.0f );
	F32 sa = sinf( theta );
	F32 ca = cosf( theta );

	F32 x,y,z;

	x =  p[0] * ca + p[1] * sa;
	y = -p[0] * sa + p[1] * ca;
	z =  p[2];

	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}

void mk_pos_mat( F32 m[4][4], F32 x, F32 y, F32 z )
{
	m[0][0] = 1;
	m[0][1] = 0;
	m[0][2] = 0;
	m[0][3] = 0;

	m[1][0] = 0;
	m[1][1] = 1;
	m[1][2] = 0;
	m[1][3] = 0;

	m[2][0] = 0;
	m[2][1] = 0;
	m[2][2] = 1;
	m[2][3] = 0;

	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
	m[3][3] = 1;
}

void
mk_rotatez_mat( F32 m[4][4], F32 angle, F32 x, F32 y, F32 z )
{
	F32 sa,ca;
	
	if( angle == 0.0f )
	{
		sa = 0;
		ca = 1;
	}
	else
	{
		F32 theta = F32( angle * M_PI / 180.0 );
		sa = sinf( theta );
		ca = cosf( theta );
	}

	m[0][0] = ca;
	m[0][1] = -sa;
	m[0][2] = 0;
	m[0][3] = 0;

	m[1][0] = sa;
	m[1][1] = ca;
	m[1][2] = 0;
	m[1][3] = 0;
	
	m[2][0] = 0;
	m[2][1] = 0;
	m[2][2] = 1;
	m[2][3] = 0;

	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
	m[3][3] = 1;
}

void
printmatrix( const F32 *_m )
{
	int i,j;
	F32 (*m)[4][4] = ( F32 (*)[4][4] ) _m;

	for( i=0; i<4; i++ )
	{
		for( j=0; j<4; j++ )
		{
			printf( "%.2f ", (*m)[i][j] );
		}
		printf( "\n" );
	}
}

static bool _invert_matrix(const F32 *m, F32 *invOut )
{
    F32 inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0f / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

bool invert_matrix( const F32 src[4][4], F32 dst[4][4] )
{
	return _invert_matrix( &src[0][0], &dst[0][0] );
}