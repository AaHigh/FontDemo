#include "stdafx.h"

static const char *
find_shader_file( const char *filename )
{
	static char result[256];

	sprintf( result, "%s/shaders/%s", data_root_prefix(), filename );

	return result;
}

static char *GetShaderSource( char *shaderfilename )
{
	char *source;

	const char *filename = find_shader_file( shaderfilename );

	size_t sz = file_get_size( filename );

	source = (char *)calloc( sz + 1, 1 );

	if( file_read( (U8*)source, filename ) > sz )
	{
		printf( "Too many bytes in file error\n" );
	}

	return source;
}

static char *GetMultiTextureVertexShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "twotex.vrt" );

	return source;
}

static char *GetMultiTextureFragmentShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "twotex.frg" );

	return source;
}

static char *GetThreeColorVertexShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "multi3.vrt" );

	return source;
}

static char *GetThreeColorFragmentShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "multi3.frg" );

	return source;
}

static char *GetTwoColorVertexShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "multi.vrt" );

	return source;
}

static char *GetTwoColorFragmentShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "multi.frg" );

	return source;
}

static char *GetYUV420PVertexShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "yuv420p.vrt" );

	return source;
}

static char *GetYUV420PFragmentShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "yuv420p.frg" );

	return source;
}

static char *GetReflectVertexShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "reflect.vrt" );

	return source;
}

static char *GetReflectFragmentShaderSource( void )
{
	static char *source;

	if( source ) return source;

	source = GetShaderSource( "reflect.frg" );

	return source;
}

static void printlog(GLhandleARB obj, const char *msg)
{
	GLint blen = 0;   /* length of buffer to allocate */
	GLint slen = 0;   /* strlen actually written to buffer */
	GLcharARB *infoLog;

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
	if (blen > 1)
	{
		if (NULL == (infoLog = (GLcharARB *) alloca(blen)))
		{
			printf("printlog: Could not allocate buffer");
			return;
		}

		glGetInfoLogARB(obj, blen, &slen, infoLog);

		printf( "%s:\n%s\n", msg, infoLog );
	}
}

static GLenum my_multitexture_program;
static GLenum my_multitexture_vertex_shader;
static GLenum my_multitexture_fragment_shader;
static GLenum my_twocolor_program;
static GLenum my_twocolor_vertex_shader;
static GLenum my_twocolor_fragment_shader;
static GLenum my_yuv420p_program;
static GLenum my_yuv420p_vertex_shader;
static GLenum my_yuv420p_fragment_shader;
static GLenum my_threecolor_program;
static GLenum my_threecolor_vertex_shader;
static GLenum my_threecolor_fragment_shader;
static GLenum my_reflect_program;
static GLenum my_reflect_vertex_shader;
static GLenum my_reflect_fragment_shader;

#define TEST_SHADER_COMP(name) testShaderCompile( #name, name )

static void testShaderCompile( const char *shader_name, GLuint shader )
{
	GLint success = 0;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if( !success )
	{
		printf( "Shader %s did not succeed in compiling!\n", shader_name );
		system( "pause" );
		exit( 0 );
	}
}

static void CreateMultiTextureShader( void )
{
	const char *my_fragment_shader_source[2] = {0};
	const char *my_vertex_shader_source[2] = {0};

	// Get Vertex And Fragment Shader Sources
	my_fragment_shader_source[0] = GetMultiTextureFragmentShaderSource();
	my_vertex_shader_source[0] = GetMultiTextureVertexShaderSource();

	// Create Shader And Program Objects
	my_multitexture_program = glCreateProgramObjectARB();
	my_multitexture_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	my_multitexture_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	// Load Shader Sources
	glShaderSourceARB(my_multitexture_vertex_shader, 1, &my_vertex_shader_source[0], NULL);
	glShaderSourceARB(my_multitexture_fragment_shader, 1, &my_fragment_shader_source[0], NULL);

	// Compile The Shaders
	glCompileShaderARB(my_multitexture_vertex_shader);
	TEST_SHADER_COMP  (my_multitexture_vertex_shader);
	glCompileShaderARB(my_multitexture_fragment_shader);
	TEST_SHADER_COMP  (my_multitexture_fragment_shader);

	// Attach The Shader Objects To The Program Object
	glAttachObjectARB(my_multitexture_program, my_multitexture_vertex_shader);
	glAttachObjectARB(my_multitexture_program, my_multitexture_fragment_shader);

	// Link The Program Object
	glLinkProgramARB(my_multitexture_program);
}

static void CreateTwoColorShader( void )
{
	const char *my_fragment_shader_source[2] = {0};
	const char *my_vertex_shader_source[2] = {0};

	// Get Vertex And Fragment Shader Sources
	my_fragment_shader_source[0] = GetTwoColorFragmentShaderSource();
	my_vertex_shader_source[0] = GetTwoColorVertexShaderSource();

	// Create Shader And Program Objects
	my_twocolor_program = glCreateProgramObjectARB();
	my_twocolor_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	my_twocolor_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	// Load Shader Sources
	glShaderSourceARB(my_twocolor_vertex_shader, 1, &my_vertex_shader_source[0], NULL);
	glShaderSourceARB(my_twocolor_fragment_shader, 1, &my_fragment_shader_source[0], NULL);

	// Compile The Shaders
	glCompileShaderARB(my_twocolor_vertex_shader);
	TEST_SHADER_COMP  (my_twocolor_vertex_shader);
	glCompileShaderARB(my_twocolor_fragment_shader);
	TEST_SHADER_COMP  (my_twocolor_fragment_shader);

	// Attach The Shader Objects To The Program Object
	glAttachObjectARB(my_twocolor_program, my_twocolor_vertex_shader);
	glAttachObjectARB(my_twocolor_program, my_twocolor_fragment_shader);

	// Link The Program Object
	glLinkProgramARB(my_twocolor_program);
}

static void CreateYUV420PShader( void )
{
	const char *my_fragment_shader_source[2] = {0};
	const char *my_vertex_shader_source[2] = {0};

	// Get Vertex And Fragment Shader Sources
	my_fragment_shader_source[0] = GetYUV420PFragmentShaderSource();
	my_vertex_shader_source[0] = GetYUV420PVertexShaderSource();

	// Create Shader And Program Objects
	my_yuv420p_program = glCreateProgramObjectARB();
	my_yuv420p_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	my_yuv420p_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	// Load Shader Sources
	glShaderSourceARB(my_yuv420p_vertex_shader, 1, &my_vertex_shader_source[0], NULL);
	glShaderSourceARB(my_yuv420p_fragment_shader, 1, &my_fragment_shader_source[0], NULL);

	// Compile The Shaders
	glCompileShaderARB(my_yuv420p_vertex_shader);
	TEST_SHADER_COMP  (my_yuv420p_vertex_shader);
	glCompileShaderARB(my_yuv420p_fragment_shader);
	TEST_SHADER_COMP  (my_yuv420p_fragment_shader);

	// Attach The Shader Objects To The Program Object
	glAttachObjectARB(my_yuv420p_program, my_yuv420p_vertex_shader);
	glAttachObjectARB(my_yuv420p_program, my_yuv420p_fragment_shader);

	// Link The Program Object
	glLinkProgramARB(my_yuv420p_program);
}

static void check_compile( GLenum vshader, GLenum fshader )
{
	GLint compiled1,compiled2;

	glGetObjectParameterivARB(my_threecolor_vertex_shader, GL_COMPILE_STATUS, &compiled1);
	glGetObjectParameterivARB(my_threecolor_fragment_shader, GL_COMPILE_STATUS, &compiled2);

	if( !compiled1 || !compiled2 )
	{
		printf( "Compile error\n" );
		Sleep(5000);
		exit(0);
	}
}

static void CreateThreeColorShader( void )
{
	const char *my_fragment_shader_source[2] = {0};
	const char *my_vertex_shader_source[2] = {0};

	// Get Vertex And Fragment Shader Sources
	my_fragment_shader_source[0] = GetThreeColorFragmentShaderSource();
	my_vertex_shader_source[0] = GetThreeColorVertexShaderSource();

	// Create Shader And Program Objects
	my_threecolor_program = glCreateProgramObjectARB();
	my_threecolor_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	my_threecolor_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	// Load Shader Sources
	glShaderSourceARB(my_threecolor_vertex_shader, 1, &my_vertex_shader_source[0], NULL);
	glShaderSourceARB(my_threecolor_fragment_shader, 1, &my_fragment_shader_source[0], NULL);

	// Compile The Shaders
	glCompileShaderARB(my_threecolor_vertex_shader);
	TEST_SHADER_COMP  (my_threecolor_vertex_shader);
	glCompileShaderARB(my_threecolor_fragment_shader);
	TEST_SHADER_COMP  (my_threecolor_fragment_shader);

	check_compile( my_threecolor_vertex_shader, my_threecolor_fragment_shader );

	// Attach The Shader Objects To The Program Object
	glAttachObjectARB(my_threecolor_program, my_threecolor_vertex_shader);
	glAttachObjectARB(my_threecolor_program, my_threecolor_fragment_shader);

	// Link The Program Object
	glLinkProgramARB(my_threecolor_program);
}

static void CreateReflectShader( void )
{
	GLint status = 0;
	const char *my_fragment_shader_source[2] = {0};
	const char *my_vertex_shader_source[2] = {0};

	// Get Vertex And Fragment Shader Sources
	my_fragment_shader_source[0] = GetReflectFragmentShaderSource();
	my_vertex_shader_source[0] = GetReflectVertexShaderSource();

	// Create Shader And Program Objects
	my_reflect_program = glCreateProgramObjectARB();
	my_reflect_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	my_reflect_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	// Load Shader Sources
	glShaderSourceARB(my_reflect_vertex_shader, 1, &my_vertex_shader_source[0], NULL);
	glShaderSourceARB(my_reflect_fragment_shader, 1, &my_fragment_shader_source[0], NULL);

	// Compile The Shaders
	glCompileShaderARB(my_reflect_vertex_shader);
	TEST_SHADER_COMP  (my_reflect_vertex_shader);
	glCompileShaderARB(my_reflect_fragment_shader);
	TEST_SHADER_COMP  (my_reflect_fragment_shader);

    glGetObjectParameterivARB(my_reflect_vertex_shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
    if(!status) printlog(my_reflect_vertex_shader, "reflection vertex shader compile error");

    glGetObjectParameterivARB(my_reflect_fragment_shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
    if(!status) printlog(my_reflect_fragment_shader, "reflection fragment shader compile error");

	// Attach The Shader Objects To The Program Object
	glAttachObjectARB(my_reflect_program, my_reflect_vertex_shader);
	glAttachObjectARB(my_reflect_program, my_reflect_fragment_shader);

	// Link The Program Object
	glLinkProgramARB(my_reflect_program);
}

void colorshader_init( void )
{
	CreateMultiTextureShader();
	CreateTwoColorShader();
	CreateYUV420PShader();
	CreateThreeColorShader();
	CreateReflectShader();
}

SHADER_TYPE UseShader( SHADER_TYPE use, bool force )
{
	static SHADER_TYPE u;
	if( use == u && !force ) return u;
	SHADER_TYPE prev = u;
	u = use;

	// Use The Program Object Instead Of Fixed Function OpenGL
	if( !use )
	{
		glUseProgramObjectARB( NULL );
		
		glActiveTexture( GL_TEXTURE2 );
		glDisable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE1 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glDisable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE0 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glDisable( GL_TEXTURE_2D );
	}
	else if( use == SHADER_THREECOLOR )
	{
		glUseProgramObjectARB( my_threecolor_program );

		glActiveTexture( GL_TEXTURE2 );
		glDisable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE1 );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );

		glActiveTexture( GL_TEXTURE0 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glEnable( GL_TEXTURE_2D );
	}
	else if( use == SHADER_STANDARD_TEXTURE )
	{
		glUseProgramObjectARB( NULL );
		
		glActiveTexture( GL_TEXTURE2 );
		glDisable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE1 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glDisable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE0 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glEnable( GL_TEXTURE_2D );
	}
	else if( use == SHADER_MULTITEXTURE )
	{
		glUseProgramObjectARB( my_multitexture_program );

		glActiveTexture( GL_TEXTURE2 );
		glDisable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE1 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glEnable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE0 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glEnable( GL_TEXTURE_2D );
	}
	else if( use == SHADER_YUV420P )
	{
		glUseProgramObjectARB( my_yuv420p_program );
		GLint y_tex = glGetUniformLocation( my_yuv420p_program, "s_texture_y" );
		GLint u_tex = glGetUniformLocation( my_yuv420p_program, "s_texture_u" );
		GLint v_tex = glGetUniformLocation( my_yuv420p_program, "s_texture_v" );

		if( -1 != y_tex && -1 != u_tex && -1 != v_tex )
		{
			glActiveTexture( GL_TEXTURE2 );
			glEnable( GL_TEXTURE_2D );
			glUniform1i( v_tex, 2 );

			glActiveTexture( GL_TEXTURE1 );
			glDisable( GL_TEXTURE_CUBE_MAP_ARB );
			glEnable( GL_TEXTURE_2D );
			glUniform1i( u_tex, 1 );

			glActiveTexture( GL_TEXTURE0 );
			glDisable( GL_TEXTURE_CUBE_MAP_ARB );
			glEnable( GL_TEXTURE_2D );
			glUniform1i( y_tex, 0 );
		}
		else
		{
			if( -1 == y_tex ) printf( "Cannot find Uniform Location for s_texture_y\n" );
			if( -1 == u_tex ) printf( "Cannot find Uniform Location for s_texture_u\n" );
			if( -1 == v_tex ) printf( "Cannot find Uniform Location for s_texture_v\n" );
		}
	}
	else if( use == SHADER_TWOCOLOR )
	{
		glUseProgramObjectARB( my_twocolor_program );

		glActiveTexture( GL_TEXTURE2 );
		glDisable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE1 );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );

		glActiveTexture( GL_TEXTURE0 );
		glDisable( GL_TEXTURE_CUBE_MAP_ARB );
		glEnable( GL_TEXTURE_2D );
	}
	else if( use == SHADER_REFLECT )
	{
		glUseProgramObjectARB( my_reflect_program );
		GLint textMap = glGetUniformLocation( my_reflect_program, "texMap" );
		GLint cubeMap = glGetUniformLocation( my_reflect_program, "cubeMap" );

		if( -1 != textMap && -1 != cubeMap )
		{
			glActiveTexture( GL_TEXTURE2 );
			glDisable( GL_TEXTURE_2D );

			glActiveTexture( GL_TEXTURE1 );
			glEnable( GL_TEXTURE_2D );
			glUniform1i( textMap, 1 );

			glActiveTexture( GL_TEXTURE0 );
			glEnable( GL_TEXTURE_CUBE_MAP );
			glUniform1i( cubeMap, 0 );
		}
		else
		{
			if( -1 == textMap ) printf( "Cannot find Uniform Location for texMap\n" );
			if( -1 == cubeMap ) printf( "Cannot find Uniform Location for cubeMap\n" );
		}
	}

	return prev;
}