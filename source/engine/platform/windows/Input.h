#ifndef _INPUT_H
#define _INPUT_H

#include "Common.h"
#include "Core.h"

class CCore;

class CInput: public IInput
{
	CCore		*_pCore;
	HCURSOR		 _hCurNone;
	HWND		 _hWnd;

	bool		 _bExclusive;
	bool		 _bHideCursor;
	bool		 _bFocused;
	bool		 _bCurBeyond;

	bool		 _bIsTxtInput;
	char		*_pcBuffer;
	string		 _clInputTxt;
	uint		 _uiBufSize;

	bool		 _abKeys[256];
	TMouseStates _stMsts;
	int			 _uiOldMouseX, _uiOldMouseY;

	void		 _ClipCursor();
	void		 _MessageProc(const TWinMessage &stMsg);
	void		 _Loop();
	void		 _Process();

	static void CALLBACK _s_MessageProc(void *pParametr, const TWinMessage &stMsg);
	static void CALLBACK _s_Process(void *pParametr);
	static void CALLBACK _s_Loop(void *pParametr);

public:

	CInput(CCore *pCore);
	~CInput();

	HRESULT CALLBACK Configure(E_INPUT_CONFIGURATION_FLAGS eParams);
	HRESULT CALLBACK GetMouseStates(TMouseStates &stMs) const;
	HRESULT CALLBACK GetKey(E_KEYBOARD_KEY_CODES eKey, bool &bPressed) const;
	HRESULT CALLBACK BeginTextInput(char *cBuffer, uint uiBufferSize);
	HRESULT CALLBACK EndTextInput();	

	IJTS_BASE_IMPLEMENTATION(IInput);
};

#endif //_INPUT_H