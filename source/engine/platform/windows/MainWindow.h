#ifndef _WINDOWAPI_H
#define _WINDOWAPI_H

#include "Common.h"

class CMainWindow : public IMainWindow
{
	IEngineCore				*_pEngineCore;
	HINSTANCE				_hInst;
	HWND					_hWnd;
	HDC						_hDC;
	TProcDelegate			*_pDelMainLoop;
	TMsgProcDelegate		*_pDelMessageProc;

	bool					_bIsLooping;

	int WINAPI				_wWinMain(HINSTANCE hInstance);	
	static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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

#endif //_WINDOWAPI_H