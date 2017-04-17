#include "stdafx.h"

static U32 sUsec;
static U32 sMsec;
static U32 sTick;
static U32 sRTClockDelta;
static U32 sLastDelta;
static U64 sQpcStart;

void timers_init( void )
{
}

void timers_update( U32 msec )
{
#if 0 // debugging
	{
		static U32 stamp;
		if( ::msec() - stamp > 10000 )
		{
			stamp = ::msec();
			printf( "1: %lld %d %d %d\n", rtusec(), rtmsec(), ::msec(), timeGetTime() );
		}
	}
#endif

	static bool avoided_zero;

	sUsec += msec * 1000;
	sMsec += msec;

	// zero is a sentinnel and starting value; we want to avoid it just in case after increment
	// NOTE: this has not produced a known bug .. and would require continuous
	// operation for 49.7 days to trigger any such bug .. this is just prevention
	// and/or preparation for 24/7 continuous operation without lots of checks like
	// this throughout the code that takes stamps and assumed a zero stamp to be
	// uninitialized
	if( !sMsec )
	{
		sUsec+=1000;
		sMsec++;
		avoided_zero = true;
	}
	else if( avoided_zero && msec > 1 && sMsec > 1 )
	{
		avoided_zero = false;
		sUsec-=1000;
		sMsec--;
	}

	if( !sRTClockDelta )
	{
		sRTClockDelta = timeGetTime() - sMsec;
	}
	
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter( &li );
		U64 tmp = U64(li.QuadPart) - ( sMsec * qpf() / 1000 );
		if( sQpcStart )
		{
			U64 delta = ( sQpcStart > tmp ) ? sQpcStart - tmp : tmp - sQpcStart;

			// In general this will only fire off if the CPU clock speed is changing enough for the time to be off by more than 1ms
			if( delta > qpf() / 500 )
			{
				sQpcStart = tmp;
			}
		}
		else
		{
			sQpcStart = tmp;
		}
	}

#if 0 // debugging
	{
		static U32 stamp;
		if( ::msec() - stamp > 10000 )
		{
			stamp = ::msec();
			printf( "2: %lld %d %d %d\n", rtusec(), rtmsec(), ::msec(), timeGetTime() );
		}
	}
#endif
}

void setMsec( U32 ms )
{
	sMsec = ms;
	sUsec = ms*1000;
	sRTClockDelta = timeGetTime() - ms;
}

U32 msec( void )
{
	return sMsec;
}

U64 qpc( void )
{
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	return U64(li.QuadPart) - sQpcStart;
}

U64 qpf( void )
{
	static LARGE_INTEGER li;
	QueryPerformanceFrequency( &li );
	return U64(li.QuadPart);
}

U32 rtmsec( void )
{
	static U32 stamp;

	if( !sRTClockDelta )
	{
		if( !stamp ) stamp = timeGetTime();

		return timeGetTime() - stamp;
	}
	else
	{
		return timeGetTime() - sRTClockDelta;
	}
}

U64 rtusec( void )
{
	return ( qpc() * 1000000 ) / qpf();
}

U32 usec( void )
{
	return sUsec;
}

F32 usecs_to_secs( U64 usecs )
{
	return (F32) ((F64)(S64)usecs / (F64)1000000);
}

F32 msecs_to_secs( U32 msecs )
{
	return (F32) ((F64)(S64)msecs / (F64)1000);
}
