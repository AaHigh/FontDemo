#ifndef __CONSOLE_H__
#define __CONSOLE_H__

void	console_init(void);
void console_cleanup(void);
int  console_active(void);
void console_pos( int x, int y, int width=640, int height=480 );


#endif
