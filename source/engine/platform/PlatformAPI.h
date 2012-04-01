#ifndef _PLATFORMAPI_H
#define _PLATFORMAPI_H

#include "JTS_Eng.h"
#include "FuncDelegate.h"

	// {0D3C63FA-D14B-4190-BFB6-92F611506171}
	static const GUID IID_IMainWindow = 
	{ 0xd3c63fa, 0xd14b, 0x4190, { 0xbf, 0xb6, 0x92, 0xf6, 0x11, 0x50, 0x61, 0x71 } };

	class IMainWindow
	{
	public:
		virtual HRESULT InitWindow(TProcDelegate *pDelMainLoop, TMsgProcDelegate *pDelMsgProc) = 0;
		virtual HRESULT ConfigureWindow(uint uiResX, uint uiResY, bool bFScreen) = 0;
		virtual HRESULT SetCaption(const char *pcTxt) = 0;
		virtual	HRESULT SendMessage(const TWinMessage &stMsg) = 0;
		virtual HRESULT GetWindowHandle(TWindowHandle& result) = 0;
		virtual HRESULT GetClientRect(int32 &left, int32 &right, int32 &top, int32 &bottom) = 0;
		virtual HRESULT BeginMainLoop() = 0;
		virtual HRESULT KillWindow() = 0;
		virtual HRESULT Free() = 0;
	};

	struct TSysTimeAndDate
	{
		uint16 ui16Year;
		uint16 ui16Month;
		uint16 ui16Day;
		uint16 ui16Hour;
		uint16 ui16Minute;
		uint16 ui16Second;
		uint16 ui16Milliseconds;
	};

#ifdef PLATFORM_WINDOWS
	TWinMessage WinAPIMsgToEngMsg(UINT Msg, WPARAM wParam, LPARAM lParam);
	void EngMsgToWinAPIMsg(const TWinMessage &msg, UINT &Msg, WPARAM &wParam, LPARAM &lParam);
#endif

	uint64 GetPerfTimer();
	void GetLocalTimaAndDate(TSysTimeAndDate &time);
	void GetDisplaySize(uint &width, uint &height);
	void ShowModalUserAlert(const char *pcTxt, const char *pcCaption);
	void Terminate();
	void ClipCursorRect(int32 left = -1, int32 right = -1, int32 top = -1, int32 bottom = -1);

#endif //_PLATFORMAPI_H