#ifndef __TIMERS_H__
#define __TIMERS_H__

void timers_update( U32 delta_ms );

// This is used to sync the msec timers between networked machines
void setMsec( U32 ms );

U32 msec( void );
U32 rtmsec( void ); // this increments in units of 1ms
U64 rtusec( void );
U32 usec( void );
U32 rtms( void );
U64 qpc( void );
U64 qpf( void );

F32 usecs_to_secs( U64 usecs );
F32 msecs_to_secs( U32 msecs );

#endif/* !__TIMERS_H__ */
