#include <JTS_Eng.h>

#ifdef PLATFORM_LINUX
#include <stdio.h>
#include <string.h>
#endif

#include <GL/gl.h> //Temp for testing purpose only!

JTS::IEngineCore *pEngineCore = NULL;
JTS::IInput *pInput = NULL;

JTS::uint32 ui32Counter = 0;
float x = 0.f;

void CALLBACK Init (void *pParametr)
{
	pEngineCore->GetInput(pInput);

	pInput->Configure((JTS::E_INPUT_CONFIGURATION_FLAGS)(JTS::ICF_EXCLUSIVE | JTS::ICF_CURSOR_BEYOND_SCREEN));
}

void CALLBACK Free (void *pParametr)
{

}

void CALLBACK Process (void *pParametr)
{
	ui32Counter++;

	bool pressed;

	pInput->GetKey(JTS::KEY_LEFT, pressed);
	if (pressed) x -= 0.05;

	pInput->GetKey(JTS::KEY_RIGHT, pressed);
	if (pressed) x += 0.05;

	pInput->GetKey(JTS::KEY_ESCAPE, pressed);
	if (pressed) pEngineCore->QuitEngine();
}

void CALLBACK Render (void *pParametr)
{
	//Temp for testing purpose only!
	glTranslatef(x, 0.f, -1.f);
	glRotatef(ui32Counter, 0.f, 0.f, 1.f);

	glEnableClientState(GL_VERTEX_ARRAY);
	static const float sc_quad[] = {-0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5};
	glVertexPointer(2, GL_FLOAT, 0, sc_quad);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	//
}

#ifdef PLATFORM_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char **argv)
#endif
{
	if (GetEngine(pEngineCore))
	{
		pEngineCore->AddProcedure(JTS::EPT_INIT, &Init);
		pEngineCore->AddProcedure(JTS::EPT_FREE, &Free);
		pEngineCore->AddProcedure(JTS::EPT_PROCESS, &Process);
		pEngineCore->AddProcedure(JTS::EPT_RENDER, &Render);

		JTS::E_ENGINE_INIT_FLAGS flags = JTS::EIF_DEFAULT;

		if(
#ifdef PLATFORM_WINDOWS
			MessageBoxA(NULL, "Would you like to run in FullScreen mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDYES
#else
            argc > 1 && (strcmp(argv[1], "--fscreen") == 0 || strcmp(argv[1], "-f") == 0)
#endif
			)
			flags = JTS::E_ENGINE_INIT_FLAGS(JTS::EIF_FULL_SCREEN | JTS::EIF_NATIVE_RESOLUTION);

		if (FAILED(pEngineCore->InitializeEngine(800, 600, "Jutos", flags)))
			pEngineCore->AddToLog("Failed to initialize engine!", true);

		FreeEngine();

		return 0;
	}
	else
	{
#ifdef PLATFORM_WINDOWS
		MessageBoxA(NULL, "Failed to load JTS engine!", "Jutos", MB_OK | MB_ICONSTOP | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#else
        fprintf(stderr, "Error: Failed to load JTS engine!\n");
#endif
	return 1;
	}
}
