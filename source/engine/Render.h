#ifndef _RENDER_H
#define _RENDER_H

#include "Common.h"

class CCore;

class CRender : public IRender
{
	CCore *_pCore;

	const float	_c_fFov, _c_fZNear, _c_fZFar;

	void _MessageProc(const TWinMessage &stMsg);
	void _OnResize(int width, int height);

	static void CALLBACK _s_MessageProc(void *pParametr, const TWinMessage &stMsg);

public:

	CRender(CCore *pCore);
	~CRender();

	void SetDefaultStates();

	IJTS_BASE_IMPLEMENTATION(IRender);
};

#endif //_RENDER_H
