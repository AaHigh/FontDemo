#ifndef __FSAA_H__
#define __FSAA_H__

class AA
{
public:
	AA( U32 width=0, U32 height=0 );
	~AA();

private:
	GLuint back;
	GLuint depth;
	GLuint tex;
	GLuint fbo;
	GLuint width;
	GLuint height;

public:
	void setTarget( void );
	void copyToBB( void );
};

#endif /* !__FSAA_H__ */