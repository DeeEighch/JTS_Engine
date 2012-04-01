#ifndef _RENDER_H
#define _RENDER_H

#include "Common.h"

class CCore;

class CRender : public IRender
{
	CCore *_pCore;

	const uint	_c_uiColorDepth, _c_uiMSAASamples;
	const float	_c_fFov, _c_fZNear, _c_fZFar;
	const bool _c_bVSync;

	HWND	_hWnd;
	HDC		_hDC;
	HGLRC	_hRC;

	void _MessageProc(const TWinMessage &stMsg);
	void _SetDefaultStates();
	void _OnResize(int width, int height);

	static void CALLBACK _s_MessageProc(void *pParametr, const TWinMessage &stMsg);

public:

	CRender(CCore *pCore);
	~CRender();

	bool Initialize();
	void Finalize();
	void StartFrame();
	void EndFrame();

	IJTS_BASE_IMPLEMENTATION(IRender);
};

#endif //_RENDER_H