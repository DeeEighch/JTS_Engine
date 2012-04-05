#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include "Common.h"

class CMainWindow : public IMainWindow
{
	IEngineCore				*_pEngineCore;
	TProcDelegate			*_pDelMainLoop;
	TMsgProcDelegate		*_pDelMessageProc;

	const uint _c_uiMSAASamples;
	const bool _c_bVSync;

    Display *_pDisplay;
    XVisualInfo *_pVisualInfo;
    XSetWindowAttributes _winAttribs;
    Window _Win;
    GLXContext _glc;
    bool _bDoubleBuffer, _bFScreen;

    int _screen;
    XF86VidModeModeInfo _deskMode;

	bool _bIsLooping;

public:

	CMainWindow(IEngineCore *pEngineCore);
	~CMainWindow();

	HRESULT InitWindow(TProcDelegate *pDelMainLoop, TMsgProcDelegate *pDelMsgProc);
	HRESULT ConfigureWindow(uint uiResX, uint uiResY, bool bFScreen);
	HRESULT SendMessage(const TWinMessage &stMsg);
	HRESULT GetWindowHandle(TWindowHandle& result);
	HRESULT SetCaption(const char *pcTxt);
	HRESULT GetClientRect(int32 &left, int32 &right, int32 &top, int32 &bottom);
	HRESULT BeginMainLoop();
	HRESULT KillWindow();
	HRESULT Free();

};

#endif //_MAINWINDOW_H
