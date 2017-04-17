#ifndef __COLORSHADER_H__
#define __COLORSHADER_H__

enum SHADER_TYPE
{
	SHADER_NONE,
	SHADER_TWOCOLOR=1,
	SHADER_THREECOLOR,
	SHADER_STANDARD_TEXTURE, // Disable the shader, but enable texturing on texture unit 0
	SHADER_MULTITEXTURE,
	SHADER_REFLECT,
	SHADER_YUV420P,
};

void colorshader_init( void );
SHADER_TYPE UseShader( SHADER_TYPE, bool force=false ); // Return previous state of using shader

#endif // !__COLORSHADER_H__