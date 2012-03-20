#ifndef _RENDER_H
#define _RENDER_H

#include "stdafx.h"

class CRender : public IRender
{
	IEngineCore		*_pEngineCore;
	HWND			_hWnd;
	HDC 			_hDeviceContext;
	HGLRC			_hRenderingContext;
	
public:

	CRender(IEngineCore *pEngineCore, HWND hWnd);
	~CRender();
	
	HRESULT CALLBACK Initialize();
	HRESULT CALLBACK Finalize();
	HRESULT CALLBACK StartFrame();
	HRESULT CALLBACK EndFrame();
	
	IJTS_BASE_IMPLEMENTATION(IRender);
};

#endif //_RENDER_H