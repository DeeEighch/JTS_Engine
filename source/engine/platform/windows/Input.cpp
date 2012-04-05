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

	ZeroMemory(_abKeys, 256);

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

	_pCore->GetWindow()->GetWindowHandle(_hWnd);

	BYTE ANDmaskIcon[128], XORmaskIcon[128]; 

	for (int i = 0; i < 128; i++)
	{
		ANDmaskIcon[i] = 0xFF;
		XORmaskIcon[i] = 0x00;
	}

	_hCurNone = CreateCursor(GetModuleHandle(NULL), 1, 1, 32, 32, ANDmaskIcon, XORmaskIcon);

	if(!_hCurNone)
		_pCore->AddToLog("Can't create blank cursor.");

	_pCore->AddToLog("Input Subsystem initialized.");
}

CInput::~CInput()
{
	if(_hCurNone)
		if(0 == DestroyCursor(_hCurNone))
			_pCore->AddToLog("Can't destroy blank cursor.");

	if(_bExclusive)
		ClipCursor(NULL);

	_pCore->pDMessageProc()->Remove(&_s_MessageProc, (void*)this);
	_pCore->pDMLoopProc()->Remove(&_s_Loop, (void*)this);
	_pCore->RemoveProcedure(EPT_PROCESS, &_s_Process, (void*)this);
	
	_pCore->AddToLog("Input Subsystem finalized.");
}

void CInput::_Loop()
{
	if(_bExclusive && _bFocused && _bCurBeyond)
	{
		POINT p;
		GetCursorPos(&p);

		int32 i_win_left, i_win_right, i_win_top, i_win_bottom;
		
		_pCore->GetWindow()->GetClientRect(i_win_left, i_win_right, i_win_top, i_win_bottom);

		bool flag = false;

		if (p.x >= i_win_right - 1)
		{
				flag = true;
				p.x = i_win_left + 1;
				_uiOldMouseX = -(i_win_right - i_win_left - _stMsts.iX);
		}

		if (p.x <= i_win_left)
		{
				flag = true;
				p.x = i_win_right - 2;
				_uiOldMouseX = i_win_right + _stMsts.iX - i_win_left;
		}

		if (p.y >= i_win_bottom - 1)
		{
				flag = true;				
				p.y = i_win_top + 1;
				_uiOldMouseY = -(i_win_bottom - i_win_top - _stMsts.iY);
		}

		if (p.y <= i_win_top)
		{		
				flag = true;
				p.y = i_win_bottom - 2;
				_uiOldMouseY = i_win_bottom + _stMsts.iY - i_win_top;
		}

		if (flag)
		{
			SetCursorPos(p.x,p.y);
		
			ScreenToClient(_hWnd, &p);

			_stMsts.iX = p.x;
			_stMsts.iY = p.y;
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

void CInput::_ClipCursor()
{
	RECT rect;
	_pCore->GetWindow()->GetClientRect(rect.left, rect.right, rect.top, rect.bottom);
	ClipCursor(&rect); 
}

void CInput::_MessageProc(const TWinMessage &stMsg)
{
	switch(stMsg.uiMsgType)
	{
		case WMT_CLOSE:
			if(!_bHideCursor)
				SetClassLong(_hWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
			break;

		case WMT_DEACTIVATED:
			if (!_bFocused)
				break;
			ZeroMemory(_abKeys, 256);
			_stMsts.bLeftButton		= false;
			_stMsts.bRightButton	= false;
			_stMsts.bMiddleButton	= false;
			_bFocused				= false;
			if(_bExclusive)
				ClipCursor(NULL);
			break;

		case WMT_ACTIVATED:
			if (_bFocused)
				break;
			_bFocused = true;
			if (_bExclusive)
				_ClipCursor();
			break;

		case WMT_KEY_DOWN:
			_abKeys[stMsg.ui32Param1] = true;
			break;

		case WMT_KEY_UP:
			_abKeys[stMsg.ui32Param1] = false;
			break;

		case WMT_MOUSE_MOVE:	
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
	}
}

HRESULT CALLBACK CInput::Configure(E_INPUT_CONFIGURATION_FLAGS eParams)
{
	_bExclusive	= (eParams & ICF_EXCLUSIVE)!=0;

	_bCurBeyond = (eParams & ICF_CURSOR_BEYOND_SCREEN)!=0;

	_bHideCursor = (eParams & ICF_HIDE_CURSOR)!=0;

	if(_bExclusive)
		_ClipCursor();
	else
		ClipCursor(NULL);

	if(_bHideCursor)
		SetClassLong(_hWnd, GCLP_HCURSOR, (LONG)_hCurNone);   
	else
		SetClassLong(_hWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));   

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