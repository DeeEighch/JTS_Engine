#ifndef _STDAFX_H
#define _STDAFX_H

#include "JTS_Eng.h"

using namespace JTS;

#include <string>
#include <fstream>

using namespace std;

#define _USE_MATH_DEFINES
#include <math.h>

#include "PlatformAPI.h"
#include "FuncDelegate.h"
#include "Utils.h"

#include <glew.h>
#include <wglew.h>

#ifdef JTS_USE_COM

#define IJTS_BASE_IMPLEMENTATION(interface_name) \
	HRESULT CALLBACK GetGUID(GUID &guid)\
	{\
		guid = IID_##interface_name;\
		return S_OK;\
	}\
	HRESULT CALLBACK QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)\
	{\
		*ppvObject = NULL;\
		if(::memcmp(&riid,&__uuidof(IUnknown),sizeof(GUID)) == 0) \
		*ppvObject = static_cast<IUnknown *>(this);\
		else if(::memcmp(&riid,&IID_##interface_name,sizeof(GUID)) == 0) \
		*ppvObject = static_cast<interface_name *>(this);\
		else return E_NOINTERFACE;\
		return S_OK;\
	}\
	ULONG CALLBACK AddRef(){return 1;}\
	ULONG CALLBACK Release(){return 1;}

#define IJTS_BASE_IMPLEMENTATION1(interface1_name, interface2_name) \
	HRESULT CALLBACK GetGUID(GUID &guid)\
	{\
		guid = IID_##interface1_name;\
		return S_OK;\
	}\
	HRESULT CALLBACK QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)\
	{\
		*ppvObject = NULL;\
		if(::memcmp(&riid,&__uuidof(IUnknown),sizeof(GUID)) == 0) \
		*ppvObject = static_cast<IUnknown *>(this);\
		else if(::memcmp(&riid,&IID_##interface1_name,sizeof(GUID)) == 0) \
		*ppvObject = static_cast<interface1_name *>(this);\
		else if(::memcmp(&riid,&IID_##interface2_name,sizeof(GUID)) == 0) \
		*ppvObject = static_cast<interface2_name *>(this);\
		else return E_NOINTERFACE;\
		return S_OK;\
	}\
	ULONG CALLBACK AddRef(){return 1;}\
	ULONG CALLBACK Release(){return 1;}

#define IUNKNOWN_IMPLEMENTATION(interface_name) \
	HRESULT CALLBACK QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)\
	{\
		*ppvObject = NULL;\
		if(::memcmp(&riid,&__uuidof(IUnknown),sizeof(GUID)) == 0)\
		*ppvObject = static_cast<IUnknown *>(this);\
		else if(::memcmp(&riid,&__uuidof(interface_name),sizeof(GUID)) == 0)\
		{*ppvObject = static_cast<interface_name *>(this); AddRef();}\
		else return E_NOINTERFACE;\
		return S_OK;\
	}\
	ULONG CALLBACK AddRef() { return 1; }\
	ULONG CALLBACK Release() { return 1; }

#else
#define IJTS_BASE_IMPLEMENTATION(interface_name) \
	HRESULT CALLBACK GetGUID(GUID &guid)\
	{\
		guid = IID_##interface_name;\
		return S_OK;\
	}

#define IJTS_BASE_IMPLEMENTATION1(interface1_name, interface2_name) \
	HRESULT CALLBACK GetGUID(GUID &guid)\
	{\
		guid = IID_##interface1_name;\
		return S_OK;\
	}

#define IUNKNOWN_IMPLEMENTATION(interface_name) 
#endif

#endif //_STDAFX_H