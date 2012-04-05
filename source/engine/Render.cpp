#include "Render.h"
#include "Core.h"

CRender::CRender(CCore *pCore):
_pCore(pCore),
_c_fFov(100.f), _c_fZNear(0.1f), _c_fZFar(100.f)
{
	_pCore->pDMessageProc()->Add(&_s_MessageProc, (void*)this);
}

CRender::~CRender()
{
	_pCore->pDMessageProc()->Remove(&_s_MessageProc, (void*)this);
}

void CRender::_MessageProc(const TWinMessage &stMsg)
{
	if (stMsg.uiMsgType == WMT_SIZE || stMsg.uiMsgType == WMT_RESTORED)
	{
		int32 l, r, t, b;
		_pCore->GetWindow()->GetClientRect(l, r, t, b);
		_OnResize(r - l, b - t);
	}
}

void CRender::_OnResize(int width, int height)
{
 	if (height == 0) height = 1;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float
	aspect = (float)width/(float)height,
	y_max = _c_fZNear * tan(_c_fFov * M_PI / 360.f),
	y_min = -y_max,
	x_min = y_min * aspect,
	x_max = y_max * aspect;

	glFrustum(x_min, x_max, y_min, y_max, _c_fZNear, _c_fZFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CRender::SetDefaultStates()
{
	glClearColor(0.f, 0.f, 0.f, 0.f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.25f);

	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
}

void CALLBACK CRender::_s_MessageProc(void *pParametr, const TWinMessage &stMsg)
{
	((CRender*)pParametr)->_MessageProc(stMsg);
}
