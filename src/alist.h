#ifndef __ALIST_H__
#define __ALIST_H__

class Alist
{
public:
	Alist( void (*remove_callback)( const void * ) = NULL );
	~Alist();

private:
	void **list;
	int num_alloced;
	int num_items;
	void (*remove_callback)( const void * );

	// let's keep this private and go with 100% operator interface
	void add( const void *const );
	void addFirst( const void *const ); // add to the beginning of the list instead of the end
	void remove( int index ); // note: shifts index+1 through num_items-1 to the next lower indices!
	void removeAll( void );

public:
	void sort( int (*compare)( const void *_a, const void *_b ) );
	operator int() const { return num_items; }
	inline void *operator []( const int index ) const { return ( index >= 0 && index < num_items ) ? list[index] : NULL; }
	inline void *get( const int index ) { return list[index]; }
	void **getList( void ) { return list; }
	void operator +=( const void *const ptr ) { add( ptr ); }
	void operator *=( const void *const ptr ) { addFirst( ptr ); }
	void operator -=( int index ) { remove( index ); }
	void operator -=( const void *const ptr ) { remove( find( ptr ) ); }
	void operator =(int param) { if( param == 0 ) removeAll(); } // list = 0 will remove all elements from the list

	int  find( const void *const ); // NOTE: returns -1 if cannot be found
};

#endif /*! __ALIST_H__ */