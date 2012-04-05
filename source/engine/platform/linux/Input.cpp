#include "Input.h"

CInput::CInput(CCore *pCore):
_pCore(pCore),
_bFocused(false),
_bIsTxtInput(false),
_pcBuffer(NULL),
_clInputTxt("")
{
	_pCore->pDMessageProc()->Add(&_s_MessageProc, (void*)this);
	_pCore->pDMLoopProc()->Add(&_s_Loop, (void*)this);
	_pCore->AddProcedure(EPT_PROCESS, &_s_Process, (void*)this);

    memset(_abKeys, 0, 256*sizeof(uint8));

	_stMsts.iDeltaX			= 0;
	_stMsts.iDeltaY			= 0;
	_stMsts.iX				= 0;
	_stMsts.iY				= 0;
	_stMsts.iDeltaWheel		= 0;
	_stMsts.bLeftButton		= false;
	_stMsts.bRightButton	= false;
	_stMsts.bMiddleButton	= false;

	_bExclusive				= false;
	_bHideCursor			= true;
	_bCurBeyond				= false;

    _pDisplay = XOpenDisplay(NULL);

    _pCore->GetWindow()->GetWindowHandle(_Win);

    Pixmap bitmap_no_data;
    XColor black;
    static char no_data[] = { 0,0,0,0,0,0,0,0 };
    black.red = black.green = black.blue = 0;

    bitmap_no_data = XCreateBitmapFromData(_pDisplay, _Win, no_data, 8, 8);
    _Cur = XCreatePixmapCursor(_pDisplay, bitmap_no_data, bitmap_no_data, &black, &black, 0, 0);

	_pCore->AddToLog("Input Subsystem initialized.");
}

CInput::~CInput()
{
    XFreeCursor(_pDisplay, _Cur);
    XCloseDisplay(_pDisplay);

	_pCore->pDMessageProc()->Remove(&_s_MessageProc, (void*)this);
	_pCore->pDMLoopProc()->Remove(&_s_Loop, (void*)this);
	_pCore->RemoveProcedure(EPT_PROCESS, &_s_Process, (void*)this);

	_pCore->AddToLog("Input Subsystem finalized.");
}

void CInput::_Loop()
{
	if(_bFocused && (_bExclusive  || _bCurBeyond))
	{
	    Window win; int x, y, t; uint mask;
	    XQueryPointer(_pDisplay, _Win, &win, &win, &x, &y, &t, &t, &mask);

		int32 i_win_left, i_win_right, i_win_top, i_win_bottom;

		_pCore->GetWindow()->GetClientRect(i_win_left, i_win_right, i_win_top, i_win_bottom);

        int new_x = x, new_y = y;

        bool flag = false;

		if (x >= i_win_right - 1)
		{
		    flag = true;

            if (_bCurBeyond)
            {
                new_x = i_win_left + 2;
                _uiOldMouseX = -(i_win_right - i_win_left - x);
            }
            else
            {
                new_x = i_win_right - 2;
                _uiOldMouseX = new_x;
            }
		}

		if (x <= i_win_left + 1)
		{
		    flag = true;

            if (_bCurBeyond)
            {
                new_x = i_win_right - 2;
                _uiOldMouseX = i_win_right + x - i_win_left;
            }
            else
            {
                new_x = i_win_left + 2;
                _uiOldMouseX = new_x;
            }
		}

		if (y >= i_win_bottom - 1)
		{
		    flag = true;

            if (_bCurBeyond)
            {
                new_y = i_win_top + 2;
                _uiOldMouseY = -(i_win_bottom - i_win_top - y);
            }
            else
            {
                new_y = i_win_bottom - 2;
                _uiOldMouseY = new_y;
            }
		}

        if (y <= i_win_top + 1)
		{
		    flag = true;

            if (_bCurBeyond)
            {
                new_y = i_win_bottom - 2;
                _uiOldMouseY = i_win_bottom + y - i_win_top;
            }
            else
            {
                new_y = i_win_top + 2;
                _uiOldMouseY = new_y;
            }
		}

        if (flag )
        {
            XWarpPointer(_pDisplay, None, None, 0, 0, 0, 0, new_x - x, new_y - y );

            _stMsts.iX = new_x;
            _stMsts.iY = new_y;
        }
	}
}

void CInput::_Process()
{
	_stMsts.iDeltaX = _stMsts.iX - _uiOldMouseX;
	_stMsts.iDeltaY = _stMsts.iY - _uiOldMouseY;
	_uiOldMouseX = _stMsts.iX;
	_uiOldMouseY = _stMsts.iY;
	_stMsts.iDeltaWheel = 0;
}

void CInput::_MessageProc(const TWinMessage &stMsg)
{
	switch(stMsg.uiMsgType)
	{
		case WMT_CLOSE:
			if(!_bHideCursor)
                XUndefineCursor(_pDisplay, _Win);
			break;

		case WMT_DEACTIVATED:
			if (!_bFocused)
				break;
		    memset(_abKeys, 0, 256*sizeof(uint8));

			_stMsts.bLeftButton		= false;
			_stMsts.bRightButton	= false;
			_stMsts.bMiddleButton	= false;
			_bFocused				= false;
			break;

		case WMT_ACTIVATED:
			if (_bFocused)
				break;
			_bFocused = true;
			break;

		case WMT_KEY_DOWN:
			_abKeys[stMsg.ui32Param1] = true;
			break;

		case WMT_KEY_UP:
			_abKeys[stMsg.ui32Param1] = false;
			break;

		case WMT_MOUSE_MOVE:
            _Loop();
			_stMsts.iX = stMsg.ui32Param1;
			_stMsts.iY = stMsg.ui32Param1;
			break;

		case WMT_MOUSE_DOWN:
			if (stMsg.ui32Param1 == 0)
				_stMsts.bLeftButton = true;
			else
				if (stMsg.ui32Param1 == 1)
					_stMsts.bRightButton = true;
				else
					_stMsts.bMiddleButton = true;
			break;

		case WMT_MOUSE_UP:
			if (stMsg.ui32Param1 == 0)
				_stMsts.bLeftButton = false;
			else
				if (stMsg.ui32Param1 == 1)
					_stMsts.bRightButton = false;
				else
					_stMsts.bMiddleButton = false;
			break;

		case WMT_MOUSE_WHEEL:
			_stMsts.iDeltaWheel = *((int*)stMsg.pParam3);
			break;

		case WMT_ENTER_CHAR:
			if (_bIsTxtInput)
			{
				if (stMsg.ui32Param1 > 31)
					_clInputTxt += (char)stMsg.ui32Param1;
				else
					if (stMsg.ui32Param1 == 8 && _clInputTxt.length() > 0)
						_clInputTxt.erase(_clInputTxt.length() - 1, 1);

				if (_uiBufSize > _clInputTxt.size())
					strcpy(_pcBuffer, _clInputTxt.c_str());
				else
					EndTextInput();
			}
			break;
        default: break;
	}
}

HRESULT CALLBACK CInput::Configure(E_INPUT_CONFIGURATION_FLAGS eParams)
{
	_bExclusive	= (eParams & ICF_EXCLUSIVE)!=0;

	_bCurBeyond = (eParams & ICF_CURSOR_BEYOND_SCREEN)!=0;

	_bHideCursor = (eParams & ICF_HIDE_CURSOR)!=0;

	if (_bHideCursor)
		XDefineCursor(_pDisplay, _Win, _Cur);
	else
		XUndefineCursor(_pDisplay, _Win);

	return S_OK;
}

HRESULT CALLBACK CInput::GetMouseStates(TMouseStates &stMs) const
{
	stMs = _stMsts;
	return S_OK;
}

HRESULT CALLBACK CInput::GetKey(E_KEYBOARD_KEY_CODES eKey, bool &bPressed) const
{
	bPressed = _abKeys[eKey];
	return S_OK;
}

HRESULT CALLBACK CInput::BeginTextInput(char *cBuffer, uint uiBufferSize)
{
	_pcBuffer		= cBuffer;
	_uiBufSize		= uiBufferSize;
	_bIsTxtInput	= true;
	_clInputTxt		= "";
	strcpy(_pcBuffer, "");
	return S_OK;
}

HRESULT CALLBACK CInput::EndTextInput()
{
	_bIsTxtInput = false;
	return S_OK;
}

void CALLBACK CInput::_s_MessageProc(void *pParametr, const TWinMessage &stMsg)
{
	((CInput*)pParametr)->_MessageProc(stMsg);
}

void CALLBACK CInput::_s_Loop(void *pParametr)
{
	((CInput*)pParametr)->_Loop();
}

void CALLBACK CInput::_s_Process(void *pParametr)
{
	((CInput*)pParametr)->_Process();
}
