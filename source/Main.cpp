#include <JTS_Eng.h>

JTS::IEngineCore *pEngineCore = NULL;

void CALLBACK Init (void *pParametr)
{

}

void CALLBACK Free (void *pParametr)
{

}

void CALLBACK Process (void *pParametr)
{
	//pEngineCore->QuitEngine();
}

void CALLBACK Render (void *pParametr)
{

}

#ifdef PLATFORM_WINDOWS

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (GetEngine(pEngineCore))
	{
		pEngineCore->AddProcedure(JTS::EPT_INIT, &Init);
		pEngineCore->AddProcedure(JTS::EPT_FREE, &Free);
		pEngineCore->AddProcedure(JTS::EPT_PROCESS, &Process);
		pEngineCore->AddProcedure(JTS::EPT_RENDER, &Render);

		pEngineCore->InitializeEngine(800, 600, "Jutos", JTS::EIF_DEFAULT);

		FreeEngine();
		
		return 0;
	}
	else
		return 1;
}

#endif