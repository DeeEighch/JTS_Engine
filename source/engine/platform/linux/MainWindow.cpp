#include "MainWindow.h"

CMainWindow::CMainWindow(IEngineCore *pEngineCore):
_pEngineCore(pEngineCore),
_c_uiMSAASamples(4u), _c_bVSync(true),
_pDisplay(NULL), _glc(NULL), _bFScreen(false),
_bIsLooping(false)
{}

CMainWindow::~CMainWindow()
{
    if (_glc)
    {
        glXMakeCurrent(_pDisplay, None, NULL);
        glXDestroyContext(_pDisplay, _glc);
    }

    if (_pDisplay && _bFScreen)
    {
        XF86VidModeSwitchToMode(_pDisplay, _screen, &_deskMode);
        XF86VidModeSetViewPort(_pDisplay, _screen, 0, 0);
    }

    if (_pDisplay)
        XCloseDisplay(_pDisplay);
}

HRESULT CMainWindow::InitWindow(TProcDelegate *pDelMainLoop, TMsgProcDelegate *pDelMsgProc)
{
	_pDelMainLoop		= pDelMainLoop;
	_pDelMessageProc	= pDelMsgProc;

    _pDisplay = XOpenDisplay(NULL);

    if (_pDisplay == NULL)
    {
        _pEngineCore->AddToLog("Could not open display.", true);
        return E_ABORT;
    }

    int dummy;

    if (!glXQueryExtension(_pDisplay, &dummy, &dummy))
    {
        _pEngineCore->AddToLog("X server has no OpenGL GLX extension.", true);
        return E_ABORT;
    }

    if (_c_uiMSAASamples > 1)
    {
        static int dblMSAABuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, GLX_SAMPLE_BUFFERS_ARB, 1, GLX_SAMPLES_ARB, _c_uiMSAASamples, None};
        _pVisualInfo = glXChooseVisual(_pDisplay, DefaultScreen(_pDisplay), dblMSAABuf);
    }
    else
        _pVisualInfo = NULL;

    if (_pVisualInfo == NULL)
    {
      static int dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
      _pVisualInfo = glXChooseVisual(_pDisplay, DefaultScreen(_pDisplay), dblBuf);
      if (_pVisualInfo == NULL)
      {
        static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, None};
        _pVisualInfo = glXChooseVisual(_pDisplay, DefaultScreen(_pDisplay), snglBuf);
        if (_pVisualInfo == NULL)
        {
          _pEngineCore->AddToLog("No RGBA visual with depth buffer.", true);
          return E_ABORT;
        }
        _bDoubleBuffer = false;
      }
      else
        _bDoubleBuffer = true;
    }
    else
     _bDoubleBuffer = true;

    _glc = glXCreateContext(_pDisplay, _pVisualInfo, None, GL_TRUE);
    if (_glc == NULL)
    {
        _pEngineCore->AddToLog("Could not create rendering context.", true);
        return E_ABORT;
    }

    _winAttribs.colormap = XCreateColormap(_pDisplay, RootWindow(_pDisplay, _pVisualInfo->screen), _pVisualInfo->visual, AllocNone);
    _winAttribs.border_pixel = 0;
    _winAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | FocusChangeMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | KeymapStateMask;

    _Win = XCreateWindow(_pDisplay, RootWindow(_pDisplay, _pVisualInfo->screen), 0, 0, 320, 240, 0, _pVisualInfo->depth, InputOutput, _pVisualInfo->visual, CWBorderPixel | CWColormap | CWEventMask, &_winAttribs);

    Atom wmDelete = XInternAtom(_pDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(_pDisplay, _Win, &wmDelete, 1);

 	_pEngineCore->AddToLog("Window created successfully.");

	return S_OK;
}

HRESULT CMainWindow::SetCaption(const char *pcTxt)
{
    XStoreName(_pDisplay, _Win, pcTxt);
	return S_OK;
}

HRESULT CMainWindow::SendMessage(const TWinMessage &stMsg)
{
//To do in future or consider if is this function needed? May be just remove it?
	return S_FALSE;
}

HRESULT CMainWindow::GetWindowHandle(TWindowHandle& result)
{
    result = _Win;
	return S_OK;
}

HRESULT CMainWindow::GetClientRect(int32 &left, int32 &right, int32 &top, int32 &bottom)
{
    XWindowAttributes attribs;
    XGetWindowAttributes(_pDisplay, _Win, &attribs);

    int l, t; Window child;
    XTranslateCoordinates(_pDisplay, _Win, RootWindow(_pDisplay, _pVisualInfo->screen), attribs.x, attribs.y, &l, &t, &child);

    left = l;
    right = l + attribs.width;
    top = t;
    bottom = t + attribs.height;

	return S_OK;
}

HRESULT CMainWindow::BeginMainLoop()
{
    _pEngineCore->AddToLog("**Entering main loop**");

    _bIsLooping = true;

    XEvent event;

    while (_bIsLooping)
    {
        while (XPending(_pDisplay) > 0)
        {
            XNextEvent(_pDisplay, &event);

            if (event.type == ClientMessage)
            {
                if (*XGetAtomName(_pDisplay, event.xclient.message_type) == *"WM_PROTOCOLS")
                    _pDelMessageProc->Invoke(TWinMessage(WMT_CLOSE));
            }
            else
                if (event.type ==  KeymapNotify)
                    XRefreshKeyboardMapping(&event.xmapping);
                else
                    _pDelMessageProc->Invoke(XEventToEngMsg(event));
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        _pDelMainLoop->Invoke();

        if (_bDoubleBuffer)
            glXSwapBuffers(_pDisplay, _Win);
        else
            glFlush();
    }

    _pDelMessageProc->Invoke(TWinMessage(WMT_RELEASED));

	_pEngineCore->AddToLog("**Exiting main loop**");

	return S_OK;
}

HRESULT CMainWindow::KillWindow()
{
    _bIsLooping = false;
	return S_OK;
}

HRESULT CMainWindow::ConfigureWindow(uint uiResX, uint uiResY, bool bFScreen)
{
    glXMakeCurrent(_pDisplay, None, NULL);

    if (bFScreen)
    {
        XDestroyWindow(_pDisplay, _Win);

        XF86VidModeModeInfo **modes;
        int mode_num;
        _screen = DefaultScreen(_pDisplay);

        XF86VidModeGetAllModeLines(_pDisplay, _screen, &mode_num, &modes);

        _deskMode = *modes[0];

        XF86VidModeSwitchToMode(_pDisplay, _screen, &_deskMode);
        XF86VidModeSetViewPort(_pDisplay, _screen, 0, 0);
        XFree(modes);

        _winAttribs.override_redirect = True;
        _Win = XCreateWindow(_pDisplay, RootWindow(_pDisplay, _pVisualInfo->screen),0, 0, uiResX, uiResY, 0, _pVisualInfo->depth, InputOutput, _pVisualInfo->visual, CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &_winAttribs);
        XWarpPointer(_pDisplay, None, _Win, 0, 0, 0, 0, 0, 0);
        XMapRaised(_pDisplay, _Win);
        XGrabKeyboard(_pDisplay, _Win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    }
    else
    {
        if (_bFScreen != bFScreen)
        {
            XDestroyWindow(_pDisplay, _Win);

            XF86VidModeSwitchToMode(_pDisplay, _screen, &_deskMode);
            XF86VidModeSetViewPort(_pDisplay, _screen, 0, 0);

            _Win = XCreateWindow(_pDisplay, RootWindow(_pDisplay, _pVisualInfo->screen), 0, 0, 320, 240, 0, _pVisualInfo->depth, InputOutput, _pVisualInfo->visual, CWBorderPixel | CWColormap | CWEventMask, &_winAttribs);
            Atom wmDelete = XInternAtom(_pDisplay, "WM_DELETE_WINDOW", True);
            XSetWMProtocols(_pDisplay, _Win, &wmDelete, 1);
        }

        uint screen_w, screen_h;
        GetDisplaySize(screen_w, screen_h);

        XSizeHints sizehints;
        sizehints.flags      = PPosition | PSize | PMinSize | PMaxSize;
        sizehints.min_width  = sizehints.max_width  = uiResX;
        sizehints.min_height = sizehints.max_height = uiResY;

        XSetWMNormalHints(_pDisplay, _Win, &sizehints);

        XMoveResizeWindow(_pDisplay, _Win, (screen_w - uiResX)/2, (screen_h - uiResY)/2, uiResX, uiResY);
    }

    _bFScreen = bFScreen;

    glXMakeCurrent(_pDisplay, _Win, _glc);

	GLenum glew_res = glewInit();

	if(glew_res != GLEW_OK)
	{
		string cause = "";

		switch(glew_res)
		{
			case GLEW_ERROR_NO_GL_VERSION:
				cause = "Reason: Can't get OpenGL version.";
				break;
			case GLEW_ERROR_GLX_VERSION_11_ONLY:
				cause = "Reason: OpenGL 1.2 or higher required.";
				break;
		}

		_pEngineCore->AddToLog(("Can't initialize OpenGL Extension Wrangler. " + cause).c_str());

		return E_ABORT;
	}

    if (GLXEW_EXT_swap_control && _c_bVSync)
          glXSwapIntervalEXT(_pDisplay, glXGetCurrentDrawable(), 1);

    XMapWindow(_pDisplay, _Win);

	return S_OK;
}

HRESULT CMainWindow::Free()
{
	delete this;
	return S_OK;
}
