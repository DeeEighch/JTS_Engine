#include "MainWindow.h"

CMainWindow::CMainWindow(IEngineCore *pEngineCore):
_pEngineCore(pEngineCore),
_hInst(GetModuleHandle(NULL)),
_hWnd(NULL),
_bIsLooping(false)
{}

CMainWindow::~CMainWindow()
{
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
			_pDelMainLoop->Invoke();

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
	wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(0);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = "JTSWindowClass";
	wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

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