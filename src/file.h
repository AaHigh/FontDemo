#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>

static const S32 FILE_MAX_PATHNAME_LEN = 256;

U32  cksum_composite( U32 cksum, U32 cksum2 );
U32  cksum_data( U32 incoming_cksum, const void *_data, size_t size );
U32  cksum_file( U32 cksum, const char *filename );
void cksum_dump( U32 cksum );

const char *file_find( const char *filename );
const char *writeable_file_find( const char *filename );
int  file_get_size(const char* p_name, S32 *p_size=NULL);
U32  file_get_mtime( const char* p_name);
U32  file_get_cksum( const char *filename );
size_t file_read(U8* p_dest, const char* p_name);
bool file_write(U8* p_src, const char* p_name, S32 size);
bool file_remove( const char *p_name );
bool file_exists(const char* p_filename);
bool dir_exists( const char *p_dirname );
int  dir_create( const char *p_dirname );
FILE* file_open_with_path( char* filename, char* filemode, int pathmode = 0755 );

bool file_cat(const char* p_dest, const char* p_front, const char* p_back);
char **dirlist( char *directory, const char *file_extension=NULL, int *nfiles=NULL );
void free_dirlist( char **&cpp ); // Free the memory resource associated with the return value from the dirlist funciton

#endif
