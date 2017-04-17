#include "stdafx.h"

#define ALIST_INITIAL_SIZE 1

Alist::Alist( void (*_remove_callback)( const void * ) )
{
	num_items = 0;
	num_alloced = 0;
	list = NULL;
	remove_callback = _remove_callback;
}

Alist::~Alist()
{
	removeAll();
	if( list )
	{
		free( list );
		list = NULL;
	}
	num_alloced = 0;
	remove_callback = NULL;
}

void
Alist::remove( int index )
{
	if( index < 0 || index >= num_items || num_items < 1 ) return;

	if( remove_callback ) remove_callback( list[index] );

	int i;

	for( i=index; i<num_items-1; i++ )
	{
		list[i] = list[i+1];
	}

	list[--num_items] = NULL;
}

void
Alist::removeAll( void )
{
	while( num_items ) remove( num_items-1 );
}

int
Alist::find( const void *const ptr )
{
	int i;
	const int &n = num_items;

	for( i=n-1; i>=0; --i )
	{
		if( list[i] == ptr ) return i;
	}

	return -1;
}

void
Alist::add( const void *const ptr )
{
	if( !list )
	{
		num_items = 0;
		num_alloced = ALIST_INITIAL_SIZE;
		size_t sz = sizeof( void * ) * num_alloced;
		list = ( void ** ) malloc( sz );
		memset( list, 0, sz );
	}
	else if( num_items >= num_alloced )
	{
		size_t old_sz = sizeof( void * ) * num_alloced;
		num_alloced *= 2;
		size_t new_sz = sizeof( void * ) * num_alloced;
		void **oldlist = list;
		list = ( void ** ) malloc( new_sz );
		memcpy( list, oldlist, old_sz );
		if( oldlist ) free( oldlist );
		memset( &list[num_items], 0, new_sz - old_sz );
	}

	list[num_items++] = ( void * ) ptr;
}

void
Alist::addFirst( const void *const ptr )
{
	if( !num_items )
	{
		add( ptr );
		return;
	}
	
	add( list[num_items-1] );

	int i;
	for( i=num_items-2; i>0; --i )
	{
		list[i] = list[i-1];
	}

	list[0] = ( void * ) ptr;
}

void
Alist::sort( int (*compare)( const void *_a, const void *_b ) )
{
	if( list ) qsort( list, num_items, sizeof( void * ), compare );
}
