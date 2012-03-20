#include "Render.h"

#include "GL/glew.h"
#include "GL/wglew.h"

CRender::CRender(IEngineCore *pEngineCore, HWND hWnd):
_pEngineCore(pEngineCore),
_hWnd(hWnd),
_hDeviceContext(NULL),
_hRenderingContext(NULL)
{}

CRender::~CRender()
{
}

HRESULT CALLBACK CRender::Initialize()
{
	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		32,																// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		1,																// Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};
	
	_hDeviceContext = GetDC(_hWnd);
	if (!_hDeviceContext) 
	{
		_pEngineCore->AddToLog("Can't get window Draw Context.", true);	
		return E_FAIL;
	}
	
	GLuint pixel_format = ChoosePixelFormat(_hDeviceContext,&pfd);
	if (!pixel_format) 
	{
		_pEngineCore->AddToLog("Can't find a suitable PixelFormat.", true);	
		return E_FAIL;
	}
		
	if (!SetPixelFormat(_hDeviceContext,pixel_format,&pfd))
	{
		_pEngineCore->AddToLog("Can't set the PixelFormat.", true);	
		return E_FAIL;
	}
	
	_hRenderingContext = wglCreateContext(_hDeviceContext);
	if (!_hRenderingContext) 
	{
		_pEngineCore->AddToLog("Can't create a GL rendering context.", true);	
		return E_FAIL;
	}
	
	if (!wglMakeCurrent(_hDeviceContext,_hRenderingContext)) 
	{
		_pEngineCore->AddToLog("Can't activate the GL rendering context.", true);	
		return E_FAIL;	
	}
	
	glewInit();
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	
	
	return S_OK;
}

HRESULT CALLBACK CRender::Finalize()
{
	if (!wglMakeCurrent(NULL,NULL))
		return E_FAIL;
		
	if (_hRenderingContext) 
		wglDeleteContext(_hRenderingContext);
	
	if (_hDeviceContext)
		ReleaseDC(_hWnd,_hDeviceContext);
	
	return S_OK;
}

HRESULT CALLBACK CRender::StartFrame()
{
	if (!wglMakeCurrent(_hDeviceContext,_hRenderingContext)) 
	{
		_pEngineCore->AddToLog("Can't activate the GL rendering context.", true);	
		return E_FAIL;	
	}
	glClear(GL_COLOR_BUFFER_BIT);
	return S_OK;
}

HRESULT CALLBACK CRender::EndFrame()
{
	SwapBuffers(_hDeviceContext);
	return S_OK;
}
