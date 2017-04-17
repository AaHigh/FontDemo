#include "stdafx.h"

#include "shlobj.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

#define DBG
#pragma warning(disable:4996)

U32 cksum_composite( U32 cksum1, U32 cksum2 )
{   
    if( cksum1 == 0 ) return cksum2;

	U32 cksum[2];
	cksum[0] = cksum1;
	cksum[1] = cksum2;
	return cksum_data( 0, cksum, sizeof(cksum) );
}

U32 cksum_data( const void *_data, size_t size )
{   
    U32 i;
    if( !_data || !size ) return 0;
    U32 cksum = 0xabdefe12;
    char *data = (char *)_data;

    for( i=0; i<size; i++ )
    {
        cksum = ( cksum << 7 ) | (( cksum & 0xfe000000 ) >> 25 );
        cksum ^= (*data);
        data++;
    }

	return cksum;
}


U32 cksum_data( U32 incoming_cksum, const void *_data, size_t size )
{   
    return cksum_composite( incoming_cksum, cksum_data( _data, size ) );
}

U32 file_get_cksum( const char* p_name ) // Uses a combination of filename and modification time of the file
{
	U32 cksum = file_get_mtime( p_name );

	cksum = cksum_data( cksum, p_name, strlen( p_name ) ); 

    size_t size = file_get_size( p_name );

	cksum = cksum_data( cksum, &size, sizeof(size) ); 

	return cksum;
}

U32 cksum_file( U32 cksum, const char *filename )
{
	if( strstr( filename, "MadScience72" ) )
	{
		printf( "HUH?" );
	}
	U32 rval = file_get_cksum( filename );
	printf( "File %s cksum %x\n", filename, rval );
	return cksum_composite( cksum, rval );
}

int dir_create( const char *path )
{
    return SHCreateDirectoryEx( NULL, path, NULL );
}

bool dir_exists( const char *p_dirname )
{
	struct stat st;

	int rval;

	rval = stat( p_dirname, &st );

	if( rval == -1 ) // some error?
	{
		return 0;
	}

	if( st.st_mode & S_IFDIR )
	{
		return 1;
	}

	return 0;
}

const char *
file_find( const char *filename )
{
	if( !filename ) return NULL;
	static char result[256];

	strcpy( result, filename );

	if( !file_exists( result ) )
	{
		sprintf( result, "%s/%s", data_root_prefix(), filename );
	}

	if( !file_exists( result ) )
	{
		sprintf( result, "%s/%s/%s", data_root_prefix(), "illustrator/", filename );
	}

	if( !file_exists( result ) )
	{
		sprintf( result, "%s/%s/%s", data_root_prefix(), "../../../gamedesign/illustrator/", filename );
	}

	if( !file_exists( result ) )
	{
		printf( "Warning could not find file %s\n", filename );
		sprintf( result, "%s/%s", data_root_prefix(), filename ); // default location to create files
	}

	return result;
}

const char *
writeable_file_find( const char *filename )
{
	if( !filename ) return NULL;
	static char result[256];

	sprintf( result, "%s/%s", writeable_data_root_prefix(), filename );

	if( !file_exists( result ) )
	{
		return file_find( filename );
	}

	return result;
}

bool file_exists(const char* p_filename)
{
	bool	b_exists = FALSE;

	return ( -1 != file_get_size(p_filename) );
}

U32 file_get_mtime( const char* p_name )
{
	struct stat st;

	if( !p_name || !p_name[0] ) return 0;

	stat( p_name, &st );

	return (U32)st.st_mtime;
}

int file_get_size( const char* p_name, S32* p_size )
{
	// passes back the size of a file in bytes, 0 on failure
	// returns TRUE on success, FALSE on failure (file not found most likely)
	FILE*	p_file = NULL;
	int size;

	if( !p_size ) p_size = &size;
	if( !p_name ) return -1;

	*p_size = 0;

	p_file = fopen(p_name, "rb");
	if (p_file != NULL)
	{
		if (fseek(p_file, 0, SEEK_END) == 0)
		{
			*p_size = ftell(p_file);
		}
		else
		{
			DBG("error seeking in file: %s\n", p_name);
			*p_size = 0;
		}
		fclose(p_file);
	}
	else
	{
		DBG("file not found: %s\n", p_name);
		return -1;
	}

	return *p_size;
}

size_t file_read(U8* p_dest, const char* p_name)
{
	FILE*	p_file = NULL;
	bool	b_ok = FALSE;
	size_t	num_bytes = 0;
	size_t	bytes_read = 0;
	
	if( !p_name ) return 0;
	if( !p_dest ) return 0;

	p_file = fopen(p_name, "rb");
	if (p_file != NULL)
	{
		if (fseek(p_file, 0, SEEK_END) == 0)
		{
			num_bytes = ftell(p_file);

			if (fseek(p_file, 0, SEEK_SET) == 0)
			{
				bytes_read = fread(p_dest, sizeof(U8), num_bytes, p_file);
			}
			else
			{
				DBG("error seeking in file: %s\n", p_name);
				return 0;
			}
		}
		else
		{
			DBG("error seeking in file: %s\n", p_name);
			return 0;
		}
		fclose(p_file);
	}
	else
	{
		DBG("file not found: %s\n", p_name);
	}

	return (bytes_read);
}

bool file_remove( const char *p_name )
{
	// write size bytes from p_src to file p_name
	// returns TRUE is successful, FALSE otherwise
	FILE *	p_file = NULL;
	size_t	bytes_written = 0;
	bool	b_ok = FALSE;

	if( !p_name ) return 0;

	int rval = unlink( p_name );

	b_ok = ( rval == 0 );

	return (b_ok);
}

bool file_write(U8* p_src, const char* p_name, S32 num_bytes)
{
	// write size bytes from p_src to file p_name
	// returns TRUE is successful, FALSE otherwise
	FILE *	p_file = NULL;
	size_t	bytes_written = 0;
	bool	b_ok = FALSE;

	if( !p_name ) return 0;
	if( !p_src ) return 0;
	if( !num_bytes ) return 0;

	p_file = fopen(p_name, "wb");
	if (p_file != NULL)
	{
		bytes_written = fwrite(p_src, sizeof(U8), num_bytes, p_file);
		
		if (bytes_written >= 0)
		{
			b_ok = TRUE;
		}

		fclose(p_file);
	}
	else
	{
		DBG("unable to create file: %s\n", p_name);
	}

	return (b_ok);
}

int _compare_filenames( const void *_a, const void *_b )
{
	int i;
	char *a = *( char ** ) _a;
	char *b = *( char ** ) _b;

	return strcmp( a, b );
	for( i=0; ;i++ )
	{
		if( !a[i] && !b[i] ) return 0;
		else if( a[i] != b[i] ) return a[i]-b[i];
	}

	return 0;
}

char **dirlist( char *dirname, const char *file_extension, int *nfiles )
{
	size_t xlen = file_extension ? strlen( file_extension ) : 0;
	
	DIR *dir = NULL;

	dir = opendir( dirname );

	if( !dir ) return NULL;

	struct dirent *ent = NULL;

	Alist files;

	do
	{
		ent = readdir( dir );
		if( !ent ) break;
		size_t len = strlen( ent->d_name );
		if( !strcmp( ent->d_name, "." ) || !strcmp( ent->d_name, ".." ) ) continue;
		if( len < xlen ) continue;
		else if( file_extension && strcmp( &ent->d_name[len-xlen], file_extension ) ) continue;
		files += ( void * ) strdup( ent->d_name );
	}
	while( ent );

	char **rval = ( char ** ) malloc( ( int( files ) + 1 ) * sizeof( const char * ) );

	int i;

	if( nfiles ) *nfiles = int( files );

	files.sort( _compare_filenames );

	for( i=0; i<files; i++ )
	{
		rval[i] = ( char * ) files[i];
//		printf( "%s\n", rval[i] );
	}

	rval[i] = NULL;

	return rval;
}

void free_dirlist( char **&cpp )
{
	int i = 0;

	while( cpp[i] )
	{
		free( cpp[i] );
		cpp[i++] = NULL;
	}

	free( cpp );

	cpp = NULL;
}