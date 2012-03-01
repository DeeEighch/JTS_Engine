#include "Core.h"
#include "MainWindow.h"

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
_bDoExit(false),
_uiProcessInterval(33)
{
	_clDelMLoop.Add(&_s_MainLoop, this);
	_clDelMProc.Add(&_s_MessageProc, this);

	_pMainWindow = new CMainWindow(this);
}

CCore::~CCore()
{
	_pMainWindow->Free();

	if (_clLogFile.is_open())
	{
		_clLogFile << "Log Closed.";
		_clLogFile.close();
	}
}

void CCore::_MainLoop()
{
	if (_bDoExit)
	{
		_pMainWindow->KillWindow();
		return;
	}

	uint32 time			= GetPerfTimer()/1000;
	uint32 time_delta	= time - _ui32TimeOld;

	bool flag = false;

	uint cycles_cnt = (uint)(time_delta/_uiProcessInterval);
		
	for (int i = 0; i < cycles_cnt; i++)
	{
		if (!_clDelProcess.IsNull()) 
			_clDelProcess.Invoke();

		flag = true;
	}

	if (flag)
		_ui32TimeOld = time - time_delta % _uiProcessInterval;
	
	_clDelRender.Invoke();
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
		if ( (eInitFlags & EIF_NATIVE_RESOLUTION) && (eInitFlags & EIF_FULL_SCREEN))
			GetDisplaySize(uiResX, uiResY);

		if (FAILED(_pMainWindow->ConfigureWindow(uiResX, uiResY, eInitFlags & EIF_FULL_SCREEN)))
			return E_ABORT;

		_pMainWindow->SetCaption(pcApplicationName);

		AddToLog("Engine initialized.");

		if (!_clDelInit.IsNull()) 
		{
			AddToLog("Calling user initialization procedure...");
			_clDelInit.Invoke();
			AddToLog("Done.");
		}

		_ui32TimeOld = GetPerfTimer()/1000 - _uiProcessInterval;

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