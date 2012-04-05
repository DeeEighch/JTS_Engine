#include "MainWindow.h"
#include "..\..\..\build\Windows\resource.h"

CMainWindow::CMainWindow(IEngineCore *pEngineCore):
_pEngineCore(pEngineCore),
_hInst(GetModuleHandle(NULL)),
_c_uiMSAASamples(4u), _c_bVSync(true),
_hWnd(NULL),
_hDC(NULL),
_hRC(NULL),
_bIsLooping(false)
{}

CMainWindow::~CMainWindow()
{
	if (_hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
			_pEngineCore->AddToLog("Failed to release Device Context and Rendering Context.");

		if (!wglDeleteContext(_hRC))
			_pEngineCore->AddToLog("Failed to release Rendering Context.");
	}

	if (_hInst && UnregisterClass("JTSWindowClass", _hInst) == FALSE)
	{
		_hInst = NULL;
		_pEngineCore->AddToLog("Can't unregister window class.");
	}
	else
		_pEngineCore->AddToLog("Window closed properly.");
}

int CMainWindow::_wWinMain(HINSTANCE hInstance)
{
	_pEngineCore->AddToLog("**Entering main loop**");

	_bIsLooping = true;
	
	MSG st_msg = {0};

	while (_bIsLooping)
		if (PeekMessage(&st_msg, NULL, 0, 0, PM_REMOVE ))
		{
			if (WM_QUIT == st_msg.message) 
				_bIsLooping = false;
			else
			{   
				TranslateMessage(&st_msg);
				DispatchMessage (&st_msg);
			}
		}
		else
		{
	        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	        glLoadIdentity();

			_pDelMainLoop->Invoke();
	
			SwapBuffers(_hDC);
		}

	_pDelMessageProc->Invoke(TWinMessage(WMT_RELEASED));

	_pEngineCore->AddToLog("**Exiting main loop**");

	return (int) st_msg.wParam;
}

LRESULT CALLBACK CMainWindow::_s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CMainWindow *this_ptr = (CMainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (this_ptr)
	{
		if (message == WM_DESTROY)
			PostQuitMessage(0);

		this_ptr->_pDelMessageProc->Invoke(WinAPIMsgToEngMsg(message, wParam, lParam));

		if ((message == WM_SYSCOMMAND && (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)) || 
			message == WM_CLOSE)
			return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT CMainWindow::InitWindow(TProcDelegate *pDelMainLoop, TMsgProcDelegate *pDelMsgProc)
{
	_pDelMainLoop		= pDelMainLoop;
	_pDelMessageProc	= pDelMsgProc;

	WNDCLASSEX wcex;
	wcex.cbSize 		= sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc    = (WNDPROC)CMainWindow::_s_WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = _hInst;
	wcex.hIcon          = LoadIcon(_hInst, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(0);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = "JTSWindowClass";
	wcex.hIconSm        = LoadIcon(_hInst, MAKEINTRESOURCE(IDI_ICON1));

	if (RegisterClassEx(&wcex) == FALSE)
	{
		_pEngineCore->AddToLog("Couldn't register window class!", true);
		return E_FAIL;
	}

	_hWnd = CreateWindowExA(WS_EX_APPWINDOW, "JTSWindowClass", "JTS Application", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 320, 240, NULL, NULL, _hInst, NULL);

	if (!_hWnd)
	{
		_hWnd = NULL;
		_pEngineCore->AddToLog("Failed to create window.", true);
		return E_FAIL;
	}

	if (!(_hDC = GetDC(_hWnd)))
	{
		_pEngineCore->AddToLog("Can't get window Draw Context.", true);
		return E_FAIL;
	}

	SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	uint msaa_samples = _c_uiMSAASamples;

	PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,											// Select Our Color Bits
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
			_pEngineCore->AddToLog("Error(s) while performing OpenGL MSAA preinit routine.");
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
					WGL_COLOR_BITS_ARB, 32,
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
					_pEngineCore->AddToLog(("Can't find suitable PixelFormat with required MSAA "+IntToStr(msaa_samples)+"X support.").c_str());
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
			_pEngineCore->AddToLog("Can't free resources after performing OpenGL MSAA preinit routine.");
	}

	if (pixel_format == NULL && !(pixel_format = ChoosePixelFormat(_hDC, &pfd)))
	{
		_pEngineCore->AddToLog("Can't find a suitable PixelFormat.", true);
		return E_ABORT;
	}

	if (!SetPixelFormat(_hDC, pixel_format, &pfd))
	{
		_pEngineCore->AddToLog("Can't set PixelFormat.", true);
		return E_ABORT;
	}

	if (!(_hRC = wglCreateContext(_hDC)))
	{
		_pEngineCore->AddToLog("Can't create GL Rendering Context.", true);
		return E_ABORT;
	}

	if(!wglMakeCurrent(_hDC, _hRC))
	{
		_pEngineCore->AddToLog("Can't active GL Rendering Context.", true);
		return E_ABORT;
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
		}

		_pEngineCore->AddToLog(("Can't initialize OpenGL Extension Wrangler. " + cause).c_str(), true);

		return E_ABORT;
	}

	if (WGLEW_EXT_swap_control && _c_bVSync)
		wglSwapIntervalEXT(1);

	_pEngineCore->AddToLog("Window created successfully.");

	return S_OK;
}

HRESULT CMainWindow::SetCaption(const char *pcTxt)
{
	SetWindowText(_hWnd, pcTxt);
	return S_OK;
}

HRESULT CMainWindow::SendMessage(const TWinMessage &stMsg)
{
	UINT msg; WPARAM wparam; LPARAM lparam;
	EngMsgToWinAPIMsg(stMsg, msg, wparam, lparam);
	::SendMessage(_hWnd, msg, wparam, lparam);
	return S_OK;
}

HRESULT CMainWindow::GetWindowHandle(TWindowHandle& result)
{
	result = _hWnd;
	return S_OK;
}

HRESULT CMainWindow::GetClientRect(int32 &left, int32 &right, int32 &top, int32 &bottom)
{
	if (!_hWnd)
		return E_FAIL;

	RECT rect;
	::GetClientRect(_hWnd, &rect);

	POINT lt, rb;
	
	lt.x = rect.left;
	lt.y = rect.top;
	rb.x = rect.right;
	rb.y = rect.bottom;

	ClientToScreen(_hWnd, &lt);
	ClientToScreen(_hWnd, &rb);

	left = lt.x;
	right = rb.x;
	top = lt.y;
	bottom = rb.y;

	return S_OK;
}

HRESULT CMainWindow::BeginMainLoop()
{
	return _wWinMain(GetModuleHandle(NULL)) != -1 ? S_OK : E_FAIL;
}

HRESULT CMainWindow::KillWindow()
{
	if (_hDC && !ReleaseDC(_hWnd,_hDC))
		_pEngineCore->AddToLog("Failed to release Device Context.");

	if (DestroyWindow(_hWnd) == FALSE)
	{
		_pEngineCore->AddToLog("Can't destroy window.", true);
		return E_ABORT;
	}

	return S_OK;
}

HRESULT CMainWindow::ConfigureWindow(uint uiResX, uint uiResY, bool bFScreen)
{
	DWORD dw_style		= WS_VISIBLE;
	DWORD dw_style_ex	= WS_EX_APPWINDOW;

	if (bFScreen)	
		dw_style |= WS_POPUP;
	else
		dw_style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	if (SetWindowLong(_hWnd, GWL_EXSTYLE, dw_style_ex) == 0)
	{
		_pEngineCore->AddToLog("Can't change window styleEx.", true);
		return E_ABORT;
	}

	if (SetWindowLong(_hWnd, GWL_STYLE, dw_style) == 0)
	{
		_pEngineCore->AddToLog("Can't change window style.", true);
		return E_ABORT;
	}

	uint desktop_width = 0, desktop_height = 0;

	RECT rc = {0, 0, uiResX, uiResY};
	int	 top_x = 0, top_y = 0;

	AdjustWindowRectEx(&rc, dw_style , FALSE, dw_style_ex);

	if(!bFScreen)
	{
		GetDisplaySize(desktop_width, desktop_height);

		top_x = (int)(desktop_width - (rc.right - rc.left))/2, 
		top_y = (int)(desktop_height - (rc.bottom - rc.top))/2;

		if (top_x < 0) top_x = 0;
		if (top_y < 0) top_y = 0;
	}

	SetWindowPos(_hWnd, HWND_TOP, top_x, top_y, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);

	SetCursorPos(top_x + (rc.right - rc.left)/2, top_y + (rc.bottom - rc.top)/2);
		
	if (IsWindowVisible(_hWnd) == FALSE)
		ShowWindow(_hWnd, SW_SHOWNA);

	SetForegroundWindow(_hWnd);

	return S_OK;
}

HRESULT CMainWindow::Free()
{
	delete this;
	return S_OK;
}