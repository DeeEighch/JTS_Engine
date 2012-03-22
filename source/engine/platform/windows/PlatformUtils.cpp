#include "stdafx.h"

TWinMessage WinAPIMsgToEngMsg(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_PAINT:
		return TWinMessage(WMT_REDRAW);

	case WM_SHOWWINDOW:
		if (wParam == TRUE && lParam == 0)
			return TWinMessage(WMT_PRESENT);
		else
			return TWinMessage();

	case WM_CLOSE:
		return TWinMessage(WMT_CLOSE);

	case WM_CREATE:
		return TWinMessage(WMT_CREATE);

	case WM_DESTROY:
		return TWinMessage(WMT_DESTROY);

	case WM_SETFOCUS:
		return TWinMessage(WMT_ACTIVATED);

	case WM_KILLFOCUS:
		return TWinMessage(WMT_DEACTIVATED);

	case WM_MOVING:
		return TWinMessage(WMT_MOVE, ((RECT*)lParam)->left, ((RECT*)lParam)->top, (RECT*)lParam);

	case WM_SIZING:
		return TWinMessage(WMT_SIZE, ((RECT*)lParam)->right - ((RECT*)lParam)->left, ((RECT*)lParam)->bottom - ((RECT*)lParam)->top, (RECT*)lParam);

	case WM_SIZE:
		RECT r, rc;
		r.left = r.top = 0;
		r.right = LOWORD(lParam);
		r.bottom = HIWORD(lParam);
		
		if (wParam == SIZE_MINIMIZED)
			return TWinMessage(WMT_MINIMIZED, r.right, r.bottom, &r);					
		else
			if (wParam == SIZE_RESTORED)
				return TWinMessage(WMT_RESTORED, r.right, r.bottom, &r);			
			else
				return TWinMessage(WMT_SIZE, r.right, r.bottom, &r);

	case WM_KEYUP:
		return TWinMessage(WMT_KEY_UP, ASCIIKeyToEngKey((uchar)wParam));

	case WM_KEYDOWN:
		return TWinMessage(WMT_KEY_DOWN, ASCIIKeyToEngKey((uchar)wParam));

	case WM_CHAR:
		return TWinMessage(WMT_ENTER_CHAR, wParam);

	case WM_MOUSEMOVE:
		return TWinMessage(WMT_MOUSE_MOVE, LOWORD(lParam), HIWORD(lParam));

	case WM_LBUTTONDOWN:
		return TWinMessage(WMT_MOUSE_DOWN, 0);

	case WM_MBUTTONDOWN:
		return TWinMessage(WMT_MOUSE_DOWN, 3);

	case WM_RBUTTONDOWN:
		return TWinMessage(WMT_MOUSE_DOWN, 2);

	case WM_LBUTTONUP:
		return TWinMessage(WMT_MOUSE_UP, 0);

	case WM_MBUTTONUP:
		return TWinMessage(WMT_MOUSE_UP, 3);

	case WM_RBUTTONUP:
		return TWinMessage(WMT_MOUSE_UP, 2);

	case WM_MOUSEWHEEL: 
		int delta;
		delta = GET_WHEEL_DELTA_WPARAM(wParam);
		return TWinMessage(WMT_MOUSE_WHEEL, 0, 0, &delta);

	default: 
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		msg.lParam = lParam;
		msg.wParam = wParam;
		msg.message = Msg;
		return TWinMessage(WMT_UNKNOWN, 0, 0, &msg);
	}
}

void EngMsgToWinAPIMsg(const TWinMessage &msg, UINT &Msg, WPARAM &wParam, LPARAM &lParam)
{
	switch (msg.uiMsgType)
	{
	case WMT_REDRAW:
		Msg = WM_PAINT;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_PRESENT:
		Msg = WM_SHOWWINDOW;
		wParam = TRUE;
		lParam = 0;
		break;

	case WMT_CLOSE:
		Msg = WM_PAINT;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_CREATE:
		Msg = WM_CREATE;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_DESTROY:
		Msg = WM_DESTROY;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_ACTIVATED:
		Msg = WM_SETFOCUS;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_DEACTIVATED:
		Msg = WM_KILLFOCUS;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_MINIMIZED:
		Msg = WM_SIZE;
		wParam = SIZE_MINIMIZED;
		lParam = MAKELPARAM((WORD)msg.ui32Param1, (WORD)msg.ui32Param2);
		break;

	case WMT_MOVE:
		Msg = WM_MOVING;
		wParam = 0;
		if (msg.pParam3)
			lParam = LPARAM(msg.pParam3);
		else
			lParam = 0;
		break;

	case WMT_RESTORED:
	case WMT_SIZE:
		Msg = WM_SIZE;
		wParam = SIZE_RESTORED;
		lParam = MAKELPARAM((WORD)msg.ui32Param1, (WORD)msg.ui32Param2);
		break;

	case WMT_KEY_UP:
		Msg = WM_KEYUP;
		wParam = EngKeyToASCIIKey(msg.ui32Param1);
		lParam = 0;
		break;

	case WMT_KEY_DOWN:
		Msg = WM_KEYDOWN;
		wParam = EngKeyToASCIIKey(msg.ui32Param1);
		lParam = 0;
		break;

	case WMT_ENTER_CHAR:
		Msg = WM_CHAR;
		wParam = msg.ui32Param1;
		lParam = 0;
		break;

	case WMT_MOUSE_MOVE:
		Msg = WM_MOUSEMOVE;
		wParam = 0;
		lParam = MAKELPARAM((WORD)msg.ui32Param1, (WORD)msg.ui32Param2);
		break;

	case WMT_MOUSE_DOWN:
		if (msg.ui32Param1 == 0)
			Msg = WM_LBUTTONDOWN;
		else
			if (msg.ui32Param1 == 2)
					Msg = WM_RBUTTONDOWN;
				else
					Msg = WM_MBUTTONDOWN;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_MOUSE_UP:
		if (msg.ui32Param1 == 0)
			Msg = WM_LBUTTONUP;
		else
			if (msg.ui32Param1 == 2)
					Msg = WM_RBUTTONUP;
				else
					Msg = WM_MBUTTONUP;
		wParam = 0;
		lParam = 0;
		break;

	case WMT_MOUSE_WHEEL:
		Msg = WM_MOUSEWHEEL;
		if (msg.pParam3)
			wParam = MAKEWPARAM(0,*(int*)msg.pParam3);
		else
			wParam = 0;
		lParam = 0;
		break;
	}
}

uint64 GetPerfTimer()
{
	LARGE_INTEGER perf_counter, perf_freq;
	QueryPerformanceFrequency(&perf_freq);
	QueryPerformanceCounter(&perf_counter);
	return (1000000*perf_counter.QuadPart/perf_freq.QuadPart);
}

void GetLocalTimaAndDate(TSysTimeAndDate &time)
{
	SYSTEMTIME t;
	GetLocalTime(&t);
	time.ui16Year = t.wYear;
	time.ui16Month = t.wMonth;
	time.ui16Day = t.wDay;
	time.ui16Hour = t.wHour;
	time.ui16Minute = t.wMinute;
	time.ui16Second = t.wSecond;
	time.ui16Milliseconds = t.wMilliseconds;
}

void ShowModalUserAlert(const char *pcTxt, const char *pcCaption)
{
	MessageBoxA(NULL, pcTxt, pcCaption, MB_OK | MB_ICONSTOP | MB_SETFOREGROUND | MB_SYSTEMMODAL);
}

void GetDisplaySize(uint &width, uint &height)
{
	HDC desktop_dc	= GetDC(GetDesktopWindow());
	width	= GetDeviceCaps(desktop_dc, HORZRES);
	height	= GetDeviceCaps(desktop_dc, VERTRES);
	ReleaseDC(GetDesktopWindow(), desktop_dc);
}

void Terminate()
{
	TerminateProcess(GetCurrentProcess(), 2);
}