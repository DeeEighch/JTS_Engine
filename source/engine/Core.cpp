#include "Core.h"
#include "MainWindow.h"
#include "Render.h"
#include "platform\windows\Input.h"

CCore *pCore = NULL;

bool GetEngine(IEngineCore *&pEngineCore)
{
	if (!pCore)
	{
		pCore = new CCore();
		pEngineCore = (IEngineCore *&)pCore;
		return true;
	}
	else
	{
		pEngineCore = (IEngineCore *&)pCore;
		return false;
	}
}

void FreeEngine()
{
	if (pCore)
	{
		delete pCore;
		pCore = NULL;
	}
}

CCore::CCore():
_pInput(NULL),
_bDoExit(false),
_uiProcessInterval(33)
{
	_clDelMLoop.Add(&_s_MainLoop, this);
	_clDelMProc.Add(&_s_MessageProc, this);

	_pMainWindow = new CMainWindow(this);
		
	_pRender = new CRender(this);
}

CCore::~CCore()
{
	_pRender->Finalize();
	
	if (_pInput) delete _pInput;

	_pMainWindow->Free();

	if (_clLogFile.is_open())
	{
		_clLogFile << "Log Closed.";
		_clLogFile.close();
	}
}

TWindowHandle CCore::GetWindowHandle() const
{
	if (!_pMainWindow)
		return NULL;

	TWindowHandle window_handle;
	_pMainWindow->GetWindowHandle(window_handle);

	return window_handle;
}

void CCore::_MainLoop()
{
	if (_bDoExit)
	{
		_pMainWindow->KillWindow();
		return;
	}

	uint64 time			= GetPerfTimer()/1000;
	uint64 time_delta	= time - _ui64TimeOld;

	bool flag = false;

	uint cycles_cnt = (uint)(time_delta/_uiProcessInterval);
		
	for (int i = 0; i < cycles_cnt; i++)
	{
		if (!_clDelProcess.IsNull()) 
			_clDelProcess.Invoke();

		flag = true;
	}

	if (flag)
		_ui64TimeOld = time - time_delta % _uiProcessInterval;
	
	_pRender->StartFrame();
	_clDelRender.Invoke();
	_pRender->EndFrame();
}

void CCore::_MessageProc(const TWinMessage &stMsg)
{
	switch (stMsg.uiMsgType)
	{
	case WMT_REDRAW:
		_clDelMLoop.Invoke();
		break;

	case WMT_CLOSE:
		_bDoExit = true;
		break;

	case WMT_DESTROY:

		AddToLog("Finalizing Engine...");

		if (!_clDelFree.IsNull()) 
		{
			AddToLog("Calling user finalization procedure...");
			_clDelFree.Invoke();
			AddToLog("Done.");
		}

		break;

	}
}

void CALLBACK CCore::_s_MainLoop(void *pParametr)
{
	((CCore*)pParametr)->_MainLoop();
}

void CALLBACK CCore::_s_MessageProc(void *pParametr, const TWinMessage &stMsg)
{
	((CCore*)pParametr)->_MessageProc(stMsg);
}

HRESULT CALLBACK CCore::InitializeEngine(uint uiResX, uint uiResY, const char* pcApplicationName, E_ENGINE_INIT_FLAGS eInitFlags)
{
	if (!(eInitFlags & EIF_NO_LOGGING))
	{
		_clLogFile.setf(ios_base::right, ios_base::adjustfield);
		_clLogFile.open("log.txt", ios::out|ios::trunc);

		TSysTimeAndDate time;
		GetLocalTimaAndDate(time);

		_clLogFile << "JTS Engine Log File" << endl;
		_clLogFile << "Log Started at " << time.ui16Day << "." << time.ui16Month << "." << time.ui16Year << "." << endl;
	}

	if (SUCCEEDED(_pMainWindow->InitWindow(&_clDelMLoop, &_clDelMProc)))
	{
		_pMainWindow->SetCaption(pcApplicationName);

		_pInput = new CInput(this);

		if ( (eInitFlags & EIF_NATIVE_RESOLUTION) && (eInitFlags & EIF_FULL_SCREEN))
			GetDisplaySize(uiResX, uiResY);

		if (!_pRender->Initialize())
			return E_ABORT;
			
		if (FAILED(_pMainWindow->ConfigureWindow(uiResX, uiResY, eInitFlags & EIF_FULL_SCREEN)))
			return E_ABORT;
		
		AddToLog("Engine initialized.");

		if (!_clDelInit.IsNull()) 
		{
			AddToLog("Calling user initialization procedure...");
			_clDelInit.Invoke();
			AddToLog("Done.");
		}

		_ui64TimeOld = GetPerfTimer()/1000 - _uiProcessInterval;

		return _pMainWindow->BeginMainLoop();
	}
	else
		return E_ABORT;
}

HRESULT CALLBACK CCore::QuitEngine()
{
	_bDoExit = true;
	return S_OK;
}

HRESULT CALLBACK CCore::SetProcessInterval(uint uiProcessInterval)
{
	_uiProcessInterval = uiProcessInterval;

	return S_OK;
}

HRESULT CALLBACK CCore::AddProcedure(E_ENGINE_PROCEDURE_TYPE eProcType, void (CALLBACK *pProc)(void *pParametr), void *pParametr)
{
	switch(eProcType)
	{
	case EPT_PROCESS:
		_clDelProcess.Add(pProc, pParametr);
		break;
	case EPT_RENDER:
		_clDelRender.Add(pProc, pParametr);
		break;
	case EPT_INIT:
		_clDelInit.Add(pProc, pParametr);
		break;
	case EPT_FREE:
		_clDelFree.Add(pProc, pParametr);
		break;
	default: return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT CALLBACK CCore::RemoveProcedure(E_ENGINE_PROCEDURE_TYPE eProcType, void (CALLBACK *pProc)(void *pParametr), void *pParametr)
{
	switch(eProcType)
	{
	case EPT_PROCESS:
		_clDelProcess.Remove(pProc, pParametr);
		break;
	case EPT_RENDER:
		_clDelRender.Remove(pProc, pParametr);
		break;
	case EPT_INIT:
		_clDelInit.Remove(pProc, pParametr);
		break;
	case EPT_FREE:
		_clDelFree.Remove(pProc, pParametr);
		break;
	default: return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT CALLBACK CCore::AddToLog(const char *pcTxt, bool bError)
{
	if (_clLogFile.is_open())
	{
		TSysTimeAndDate time;
		GetLocalTimaAndDate(time);
		_clLogFile.fill('0');
		_clLogFile.width(2);
		_clLogFile << time.ui16Hour << ":";
		_clLogFile.width(2);
		_clLogFile << time.ui16Minute << ":";
		_clLogFile.width(2);
		_clLogFile << time.ui16Second << ".";	
		_clLogFile.width(3);
		_clLogFile << time.ui16Milliseconds;
		_clLogFile.width(0);
		_clLogFile << " - ";

		_clLogFile << pcTxt << endl;

		if (bError)
		{
			_clLogFile.flush();
			ShowModalUserAlert(pcTxt, "JTS Engine");
			Terminate();
		}
	}
	return S_OK;
}

HRESULT CALLBACK CCore::GetInput(IInput *&pInput)
{
	if (!_pInput)
		return E_ABORT;
	else
	{
		pInput = _pInput;
		return S_OK;
	}
}