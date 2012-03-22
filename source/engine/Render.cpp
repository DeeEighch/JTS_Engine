#include "Render.h"
#include "Core.h"

CRender::CRender(CCore *pCore):
_pCore(pCore),

_c_uiColorDepth(32u), _c_uiMSAASamples(4u),
_c_fFov(100.f), _c_fZNear(0.1f), _c_fZFar(100.f),
_c_bVSync(true),

_hWnd(NULL),
_hDC(NULL),
_hRC(NULL)
{
	_pCore->pDMessageProc()->Add(&_s_MessageProc, (void*)this);
}

CRender::~CRender()
{
	_pCore->pDMessageProc()->Remove(&_s_MessageProc, (void*)this);
}

void CRender::_MessageProc(const TWinMessage &stMsg)
{
	if (stMsg.uiMsgType == WMT_SIZE || stMsg.uiMsgType == WMT_RESTORED || stMsg.uiMsgType == WM_SIZING)
	{
		RECT rc;
		GetClientRect(_hWnd, &rc);
		_OnResize(rc.right - rc.left, rc.bottom - rc.top);
	}
}

bool CRender::Initialize()
{
	_hWnd = _pCore->GetWindowHandle();

	_hDC = GetDC(_hWnd);

	uint msaa_samples = _c_uiMSAASamples;

	PIXELFORMATDESCRIPTOR pfd=				
	{
		sizeof(PIXELFORMATDESCRIPTOR),				
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		_c_uiColorDepth,							// Select Our Color Bits
		0, 0, 0, 0, 0, 0,							// Per Color Bits and Shifts Ignored
		8,											// Alpha Bits
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 24 Bit Z-Buffer (Depth Buffer)  
		0,											// Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	int pixel_format = NULL;

	if (msaa_samples > 1)
	{
		HWND	temp_win_handle	= NULL;
		HDC		temp_win_dc		= NULL;
		HGLRC	temp_win_rc		= NULL;
		int temp_pixel_format	= NULL;

		if (
		!(temp_win_handle = CreateWindowEx(0, "STATIC", NULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL))||
		!(temp_win_dc = GetDC(temp_win_handle))||
		!(temp_pixel_format = ChoosePixelFormat(temp_win_dc, &pfd))||
		!SetPixelFormat(temp_win_dc, temp_pixel_format, &pfd)||
		!(temp_win_rc = wglCreateContext(temp_win_dc))||
		!wglMakeCurrent(temp_win_dc, temp_win_rc)
		)
		{
			msaa_samples = 1;
			_pCore->AddToLog("Error(s) while performing OpenGL MSAA preinit routine.");
		}
		else
		{		
			if (string((char*)glGetString(GL_EXTENSIONS)).find("ARB_multisample") != string::npos)
			{	
				int ia_attributes[] =
				{
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_COLOR_BITS_ARB, _c_uiColorDepth,
					WGL_ALPHA_BITS_ARB, 8,
					WGL_DEPTH_BITS_ARB, 24,
					WGL_STENCIL_BITS_ARB, 0,
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
					WGL_SAMPLES_ARB, msaa_samples,
					0,0
				};

				float	fa_attributes[] = {0,0};
				uint	formats_cnt;
				int		tmp_pixel_format;
				
				wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
				int valid = wglChoosePixelFormatARB(temp_win_dc, ia_attributes, fa_attributes, 1, &tmp_pixel_format, &formats_cnt);
				wglChoosePixelFormatARB = NULL;

				if (valid && formats_cnt >= 1)
					pixel_format = tmp_pixel_format;
				else
				{
					_pCore->AddToLog(("Can't find suitable PixelFormat with required MSAA "+IntToStr(msaa_samples)+"X support.").c_str());
					msaa_samples = 1;
				}
			}
			else
				msaa_samples = 1;
		}

		if (
		!wglMakeCurrent(NULL, NULL)||
		(temp_win_rc!=NULL&&!wglDeleteContext(temp_win_rc))||
		(temp_win_dc!=NULL&&!ReleaseDC(temp_win_handle, temp_win_dc))||
		(temp_win_handle!=NULL&&!DestroyWindow(temp_win_handle))
		)
			_pCore->AddToLog("Can't free resources after performing OpenGL MSAA preinit routine.");
	}

	if (pixel_format == NULL && !(pixel_format = ChoosePixelFormat(_hDC, &pfd)))
	{
		_pCore->AddToLog("Can't find a suitable PixelFormat.");
		return false;
	}

	if (!SetPixelFormat(_hDC, pixel_format, &pfd))		
	{
		_pCore->AddToLog("Can't set PixelFormat.");
		return false;
	}

	if (!(_hRC = wglCreateContext(_hDC)))				
	{
		_pCore->AddToLog("Can't create GL Rendering Context.");
		return false;
	}

	if(!wglMakeCurrent(_hDC, _hRC))
	{
		_pCore->AddToLog("Can't active GL Rendering Context.");
		return false;
	}

	GLenum glew_res = glewInit();

	if(glew_res != GLEW_OK)
	{
		string cause = "";
		
		switch(glew_res)
		{
			case GLEW_ERROR_NO_GL_VERSION:
				cause = "Reason: Can't get OpenGL version.";
				break;
			case GLEW_ERROR_GL_VERSION_10_ONLY:
				cause = "Reason: OpenGL 1.1 or higher required.";
				break;
			case GLEW_ERROR_GLX_VERSION_11_ONLY:
				cause = "Reason: OpenGL 1.2 or higher required.";
				break;
		}

		_pCore->AddToLog(("Can't initialize OpenGL Extension Wrangler. " + cause).c_str());

		return false;
	}

	wglMakeCurrent(_hDC, _hRC);

	if (WGLEW_EXT_swap_control && _c_bVSync)
		wglSwapIntervalEXT(1);

	_SetDefaultStates();

	_pCore->AddToLog("OpenGL rendering subsystem initialized.");

	return true;
}

void CRender::Finalize()
{
	if (_hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
			_pCore->AddToLog("Failed to release Device Context and Rendering Context.");

		if (!wglDeleteContext(_hRC))
			_pCore->AddToLog("Failed to release Rendering Context.");
		
		_hRC = NULL;
	}

	_pCore->AddToLog("OpenGL Rendering and Device contexts finalized.");
}

void CRender::StartFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();										
}

void CRender::EndFrame()
{
	SwapBuffers(_hDC);
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

void CRender::_SetDefaultStates()
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