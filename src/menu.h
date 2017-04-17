#ifndef __MENU_H__
#define __MENU_H__

extern U32 gDisplayPortrait;
extern F32 gVolume; // 0.0f to 1.0f
extern U32 gUsePerspective; // 0 == off, 1 == on, 2 == inverted perspective
extern F32 gCameraDistance;
extern F32 gPerspectiveCameraAngle;
extern F32 gPerspectiveAngleRatio;
extern F32 gViewAxisOffset;
extern U32 gPhysicsSpeed;
extern U32 gGameWeArePlaying;
extern U32 gAllowQuickExit;
extern U32 gCenterScreenIndex;  // which "windows" screen to draw the center screen
extern U32 gScreenDiagonal; // size in inches of the diagonal of the screen
extern U32 gStretchToFitLargerScreen;
extern U32 gShowFrameDtms;
extern U32 gShowAudioWaveforms;
extern F32 gDebugScale;
extern F32 gDebugPos[2];
extern U32 gDebugSloMo;
extern U32 gWindowJustification;
extern U32 gDisableGitPull;

bool menu_displayed( void );

void menu_start( void );
void menu_draw( void );
void menu_update( void );
void menu_up( void );
void menu_down( void );
void menu_right( void );
void menu_left( void );
void menu_select( void );
void menu_escape( void );

void factory_defaults( void );

inline const char *gameName( void ) { return "x9120"; }

#endif /* !__MENU_H__ */