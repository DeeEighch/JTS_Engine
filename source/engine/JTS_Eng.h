#ifndef _JTS_ENG_H
#define _JTS_ENG_H

#if defined(_WIN32) || defined(_WIN64)

//Platform Windows//

#include <Windows.h>

/** Internal engine define, shows that target platform is Windows.*/
#define PLATFORM_WINDOWS

/** If defined, all interfaces will be derived from IUnknown. */
#define JTS_USE_COM

/** If defined, all structures in header will be aligne by 1 byte. */
//#define STRUCT_ALIGNMENT_1

#elif defined(__linux__)

//Platform Linux//

#include <X11/Xlib.h>

/** Internal engine define, shows that target platform is Linux.*/
#define PLATFORM_LINUX

#else

//Unknown platform//
#error Unknown platform.

#endif

#ifndef PLATFORM_WINDOWS

#   define CALLBACK

	/** Engine interface unique identifier.
		Every engine interface must have it's own GUID.
	*/
	struct GUID
	{
		unsigned long int	Data1;
		unsigned short int	Data2;
		unsigned short int	Data3;
		unsigned char       Data4[8];
	};

	/** Default return type for all JTS methods.
		Every engine interface method returnes HRESULT as result.
		HRESULT must be one of these types S_OK, S_FALSE, E_FAIL, E_INVALIDARG or E_ABORT.
		\note Engine suppresses a lot of errors and keeps all things stable, so it is not necessary to check every methods result.
		\see SUCCEEDED, FAILED, CHECK_HR, PARANOIC_CHECK_HR, S_OK, S_FALSE, E_FAIL, E_INVALIDARG, E_ABORT
	*/
	typedef long int HRESULT;

//HRESULT return values//

/** Return value of HRESULT type. Method returns S_OK when all of its routines were done succesfully. */
#define S_OK			0

/** Return value of HRESULT type. Method returns S_FALSE when most of its routines were done succesfully but some minor errors occured. */
#define S_FALSE			1

/** Return value of HRESULT type. Method returns E_FAIL when unspecified error occured. */
#define E_FAIL			0x80004005

/** Return value of HRESULT type. Method returns E_INVALIDARG when one or more arguments are invalid. */
#define E_INVALIDARG	0x80070057

/** Return value of HRESULT type. Method returns E_ABORT when operation aborted due to errors of some other function. */
#define E_ABORT			0x80004004

/** Macros to check HRESULT value. Returns true if value is S_OK or S_FALSE and false otherwise.
	\param[in] hr HRESULT value to be checked.
*/
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

/** Macros to check HRESULT value. Returns true for any value except S_OK and S_FALSE.
	\param[in] hr HRESULT value to be checked.
*/
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#endif

namespace JTS
{

	//signed//
	typedef short int			int16;
	typedef long int			int32;
	typedef signed char			int8;
	typedef wchar_t				wchar;

	//unsigned//
	typedef unsigned int		uint;
	typedef unsigned short int	uint16;
	typedef unsigned long int	uint32;
	typedef unsigned char		uint8;
	typedef uint8				uchar;

	//int64//
#ifdef _MSC_VER
	typedef __int64				int64;
	typedef unsigned __int64	uint64;
#else
	typedef signed long long	int64;
	typedef unsigned long long	uint64;
#endif

#if defined(PLATFORM_WINDOWS)

	typedef HWND                TWindowHandle;

#elif defined(PLATFORM_LINUX)

    typedef Window              TWindowHandle;

#endif

//Engine Base interface//

	/** Engine base fundamental interface.
		Any engine interface must be derived from this interface.
		\attention On Windows platform IJTS_Base is derived from IUnknown for more flexibility and compatibility,
		but engine doesn't provides real COM technology. The reference counter is always 1,
		"Release" and "AddRef" methods are dummies, "QueryInterface" can return pointer only
		to IUnknown or to the last interface in the inheritance chain.
	*/
	class IJTS_Base
#if defined(PLATFORM_WINDOWS) && defined(JTS_USE_COM)
		: public IUnknown
#endif
	{
	public:
		/** Returns uniq identifier of the last interface in the inheritance chain.
			\param[out] guid Uniq interface identifier.
			\return Always returnes S_OK.
		 */
		virtual HRESULT CALLBACK GetGUID(GUID &guid) = 0;
	};

#if defined(STRUCT_ALIGNMENT_1) && defined(PLATFORM_WINDOWS)
#pragma pack( push, 1 )
#endif

	/** Flags of TWinMessage structure that determines type of the message.
	\see TWinMessage
	 */
	enum E_WINDOW_MESSAGE_TYPE
	{
		WMT_UNKNOWN = 0,/**< This type will be returned if there is no proper conversion from platform specific window event to engine window message. Message parameters have platform specific usage. \note For Windows pParam3 of TWinMessage structure will point to the MSG structure, for Linux will point to XEvent. */
		WMT_REDRAW,		/**< Message indicates that window must redraw itself. Message parameters are not used. */
		WMT_PRESENT,	/**< Message indicates that window is ready and now will be displayed to the user for the first time. Message parameters are not used. */
		WMT_CLOSE,		/**< Message indicates that window is ready to be destroyed and now will disappear from the screen. Message parameters are not used. */
		WMT_CREATE,		/**< Message indicates that window is being created. Message parameters are not used. */
		WMT_DESTROY,	/**< Message indicates that window is ready to be destroyed. Message parameters are not used. */
		WMT_RELEASED,	/**< Message indicates that window was destroyed and released successfully. Message parameters are not used. */
		WMT_ACTIVATED,	/**< Message indicates that window became a foreground window and get user input focus. Message parameters are not used. */
		WMT_DEACTIVATED,/**< Message indicates that window became a background window and lost user input focus. Message parameters are not used. */
		WMT_MINIMIZED,	/**< Message indicates that window was minimized. Message parameter ui32Param1 stores width of the window and ui32Param2 stores height. \note Under Windows pParam3 points to RECT structure with window size. */
		WMT_RESTORED,	/**< Message indicates that window was restored to its normal state. Message parameter ui32Param1 stores width of the window and ui32Param2 stores height. \note Under Windows pParam3 points to RECT structure with window size. */
		WMT_MOVE,		/**< Message indicates that window is being moved. Message parameter ui32Param1 stores x coordinate of upper left window corner and ui32Param2 stores y coordinate. \note Under Windows pParam3 points to RECT structure with window size. */
		WMT_SIZE,		/**< Message indicates that window is being sized. Message parameter ui32Param1 stores width of the window and ui32Param2 stores height. \note Under Windows pParam3 points to RECT structure with window size. */
		WMT_KEY_UP,		/**< Message indicates that the user has released some keyboard key. Message parameter ui32Param1 is the engine code of the key being released. \see E_KEYBOARD_KEY_CODES */
		WMT_KEY_DOWN,	/**< Message indicates that the user has pressed some keyboard key. Message parameter ui32Param1 is the engine code of the key being pressed. \see E_KEYBOARD_KEY_CODES */
		WMT_ENTER_CHAR, /**< Message indicates that the user has pressed some keyboard key. Message parameter ui32Param1 is the ASCII code of the key being pressed. */
		WMT_MOUSE_MOVE, /**< Message indicates that cursor is being moved within the window. Message parameter ui32Param1 stores x coordinate of cursor and ui32Param1 stores y coordinate. */
		WMT_MOUSE_DOWN, /**< Message indicates that the user has pressed mouse button. Message parameter ui32Param1 indicates what button was pressed: 0 - Left, 1 - Right, 2 - Middle */
		WMT_MOUSE_UP,	/**< Message indicates that the user has released mouse button. Message parameter ui32Param1 indicates what button was released: 0 - Left, 1 - Right, 2 - Middle */
		WMT_MOUSE_WHEEL /**< Message indicates that the user has rolled mouse wheel. Message parameter pParam3 points to integer (int) with mouse wheel delta value. */
	};

	/** Unified and platform independent window event type.
	 */
	struct TWinMessage
	{
		E_WINDOW_MESSAGE_TYPE
				uiMsgType;	/**< Window message type identifier. */
		uint32	ui32Param1;	/**< Message first parametr. */
		uint32	ui32Param2;	/**< Message second parametr. */
		void	*pParam3;	/**< Message third parametr. Points to specific message data. */

		TWinMessage():uiMsgType(WMT_UNKNOWN), ui32Param1(0), ui32Param2(0), pParam3(NULL){}
		TWinMessage(E_WINDOW_MESSAGE_TYPE msg, uint32 param1 = 0, uint32 param2 = 0, void *param3 = NULL):uiMsgType(msg), ui32Param1(param1), ui32Param2(param2), pParam3(param3){}
	};

#if defined(STRUCT_ALIGNMENT_1) && defined(PLATFORM_WINDOWS)
#pragma pack(pop)
#endif

	/** Engine initialization flags.
		\see IEngineCore::InitializeEngine
	 */
	enum E_ENGINE_INIT_FLAGS
	{
		EIF_DEFAULT				= 0x00000000,	/**< Use default settings. */
		EIF_FULL_SCREEN			= 0x00000001,	/**< Engine will run in full screen mode. */
		EIF_NATIVE_RESOLUTION	= 0x00000002,	/**< If EIF_FULL_SCREEN flag is set, engine will set native system screen resolution. */
		EIF_NO_LOGGING			= 0x00000004	/**< Means that engine logging will be turned off. */
	};

	/** Type of engine callbacks.
		IEngineCore can register calbacks of these types.
		\see IEngineCore::AddProcedure
	 */
	enum E_ENGINE_PROCEDURE_TYPE
	{
		EPT_PROCESS = 0,	/**< Procedure is called periodically(like on timer event). Interval of calling is set on engine initialization. In this procedure you should do any application computes. \see IEngineCore::InitializeEngine */
		EPT_RENDER,			/**< Procedure is called when engine decides to draw new frame. In this procedure you can call any rendering routines. */
		EPT_INIT,			/**< Procedure is called before engine will start its main loop. In this procedure you should load all resources needed by your application. */
		EPT_FREE			/**< Procedure is called before engine quits. In this procedure you should release all resources and free memmory. */
	};

	class IInput;

	// {111BB884-2BA6-4e84-95A5-5E4700309CBA}
	static const GUID IID_IEngineCore =
	{ 0x111bb884, 0x2ba6, 0x4e84, { 0x95, 0xa5, 0x5e, 0x47, 0x0, 0x30, 0x9c, 0xba } };

	/** Main engine interface.
	Pointer to this interface is retrieved directly from JTS library.
	\see GetEngine
	*/
	class IEngineCore : public IJTS_Base
	{
	public:
		/** Initialize engine and all of its subroutines. Also creates main engine window.
		 \param[in] uiResX Horizontal screen resolution.
		 \param[in] uiResY Vertical screen resolution.
		 \param[in] pcApplicationName Caption of main engine window.
		 \param[in] eInitFlags Special engine configuration flags. \see E_ENGINE_INIT_FLAGS
		 \return Common HRESULT return type.
		 */
		virtual HRESULT CALLBACK InitializeEngine(uint uiResX, uint uiResY, const char* pcApplicationName, E_ENGINE_INIT_FLAGS eInitFlags = EIF_DEFAULT ) = 0;
		/** Sets engine interval of calling EPT_PROCESS user routine.
		 \param[in] uiProcessInterval Interval in milliseconds.
		*/
		virtual HRESULT CALLBACK SetProcessInterval(uint uiProcessInterval) = 0;
		/** Add main system procedure for internal engine calls.
		 \param[in] eProcType Type of the procedure to be registered.
		 \param[in] pProc Pointer to the procedure with specified signature.
		 \param[in] pParametr Pointer to some data. This data will be send to registered procedure when engine calls it's delegate.
		*/
		virtual HRESULT CALLBACK AddProcedure(E_ENGINE_PROCEDURE_TYPE eProcType, void (CALLBACK *pProc)(void *pParametr), void *pParametr = NULL) = 0;
		/** Removes main system procedure for internal engine calls.
		 \param[in] eProcType Type of the procedure to be removed.
		 \param[in] pProc Pointer to the procedure with specified signature.
		 \param[in] pParametr Pointer to some data. Must be the same as was used to call AddProcedure.
		*/
		virtual HRESULT CALLBACK RemoveProcedure(E_ENGINE_PROCEDURE_TYPE eProcType, void (CALLBACK *pProc)(void *pParametr), void *pParametr = NULL) = 0;
		/** Closes main engine window and quits engine.
		*/
		virtual HRESULT CALLBACK QuitEngine() = 0;
		/** Writes string to log file.
		 \param[in] pcTxt Text to output.
		 \param[in] bError Is this an error and engine must be stopped?
		*/
		virtual	HRESULT CALLBACK AddToLog(const char *pcTxt, bool bError = false) = 0;
		/** Get Input sub system.
		 \param[out] pInput Pointer to IInput interface.
		*/
		virtual	HRESULT CALLBACK GetInput(IInput *&pInput) = 0;
	};

	// {2363509D-BFB4-448E-89EE-B1E8EFCC1D41}
	static const GUID IID_IRender =
	{ 0x2363509d, 0xbfb4, 0x448e, { 0x89, 0xee, 0xb1, 0xe8, 0xef, 0xcc, 0x1d, 0x41 } };

	/** Engine render interface.
	*/
	class IRender : public IJTS_Base
	{
	public:
	};

	/** Describes the state of the mouse.
		\see IInput
	*/
	struct TMouseStates
	{
		int	 iX;			/**< X coordinate of mouse pointer. */
		int	 iY;			/**< Y coordinate of mouse pointer. */
		int	 iDeltaX;		/**< The difference between the current and previous X coordinate value. */
		int	 iDeltaY;		/**< The difference between the current and previous Y coordinate value. */
		int	 iDeltaWheel;	/**< Mouse wheel offset. */
		bool bLeftButton;	/**< Is mouse left button pressed. */
		bool bRightButton;	/**< Is mouse right button pressed. */
		bool bMiddleButton; /**< Is mouse middle button pressed. */
	};

	/** Flags to configure user input behaviour.
	*/
	enum E_INPUT_CONFIGURATION_FLAGS
	{
		ICF_DEFAULT					= 0x00000000, /**< Use default settings. */
		ICF_EXCLUSIVE				= 0x00000001, /**< Mouse movment will be restricted by client area. */
		ICF_HIDE_CURSOR				= 0x00000002, /**< Mouse hardware cursor will be hidden. */
		ICF_CURSOR_BEYOND_SCREEN	= 0x00000004  /**< Mouse will move through window borders and then dropped to opposite border. */
	};

	enum E_KEYBOARD_KEY_CODES
#ifndef _MSC_VER
	 : uint8
#endif
	 ;

	// {64DAAF7F-F92C-425f-8B92-3BE40D8C6666}
	static const GUID IID_IInput =
	{ 0x64daaf7f, 0xf92c, 0x425f, { 0x8b, 0x92, 0x3b, 0xe4, 0xd, 0x8c, 0x66, 0x66 } };

	/** Engine input interface.
	*/
	class IInput : public IJTS_Base
	{
	public:
		/** Configures input subsystem behaviour.
		 \param[in] eFlags Input configuration flags.
		*/
		virtual HRESULT CALLBACK Configure(E_INPUT_CONFIGURATION_FLAGS eFlags = ICF_DEFAULT) = 0;
		/** Returnes the current state of the mouse.
		 \param[out] stMStates Struct with current mouse states.
		*/
		virtual HRESULT CALLBACK GetMouseStates(TMouseStates &stMStates) const = 0;
		/** Checks is the current key pressed.
		 \param[in] eKeyCode Name of the key to be checked.
		 \param[out] bPressed Is true when key is pressed.
		*/
		virtual HRESULT CALLBACK GetKey(E_KEYBOARD_KEY_CODES eKeyCode, bool &bPressed) const = 0;
		/** Start grabbing user keyboard text input.
		 \param[in] pcBuffer Allocated character buffer to store user input.
		 \param[in] uiBufferSize Size of the buffer.
		*/
		virtual HRESULT CALLBACK BeginTextInput(char* pcBuffer, uint uiBufferSize) = 0;
		/** Stop grabbing user input and copy result to buffer.
		*/
		virtual HRESULT CALLBACK EndTextInput() = 0;
	};

	/** Describes all keyboard key codes.
		\warning This is not common ASCII key codes!
	*/
	enum E_KEYBOARD_KEY_CODES
#ifndef _MSC_VER
	 : uint8
#endif
	{
		KEY_ESCAPE			= 0x01,
		KEY_TAB				= 0x0F,
		KEY_GRAVE			= 0x29,
		KEY_CAPSLOCK		= 0x3A,
		KEY_BACKSPACE		= 0x0E,
		KEY_RETURN			= 0x1C,
		KEY_SPACE			= 0x39,
		KEY_SLASH			= 0x35,
		KEY_BACKSLASH		= 0x2B,

		KEY_SYSRQ			= 0xB7,
		KEY_SCROLL			= 0x46,
		KEY_PAUSE			= 0xC5,

		KEY_INSERT			= 0xD2,
		KEY_DELETE			= 0xD3,
		KEY_HOME			= 0xC7,
		KEY_END				= 0xCF,
		KEY_PGUP			= 0xC9,
		KEY_PGDN			= 0xD1,

		KEY_LSHIFT			= 0x2A,
		KEY_RSHIFT			= 0x36,
		KEY_LALT			= 0x38,
		KEY_RALT			= 0xB8,
		KEY_LWIN_OR_CMD		= 0xDB,
		KEY_RWIN_OR_CMD		= 0xDC,
		KEY_LCONTROL		= 0x1D,
		KEY_RCONTROL		= 0x9D,

		KEY_UP				= 0xC8,
		KEY_RIGHT			= 0xCD,
		KEY_LEFT			= 0xCB,
		KEY_DOWN			= 0xD0,

		KEY_1				= 0x02,
		KEY_2				= 0x03,
		KEY_3				= 0x04,
		KEY_4				= 0x05,
		KEY_5				= 0x06,
		KEY_6				= 0x07,
		KEY_7				= 0x08,
		KEY_8				= 0x09,
		KEY_9				= 0x0A,
		KEY_0				= 0x0B,

		KEY_F1				= 0x3B,
		KEY_F2				= 0x3C,
		KEY_F3				= 0x3D,
		KEY_F4				= 0x3E,
		KEY_F5				= 0x3F,
		KEY_F6				= 0x40,
		KEY_F7				= 0x41,
		KEY_F8				= 0x42,
		KEY_F9				= 0x43,
		KEY_F10				= 0x44,
		KEY_F11				= 0x57,
		KEY_F12				= 0x58,

		KEY_Q				= 0x10,
		KEY_W				= 0x11,
		KEY_E				= 0x12,
		KEY_R				= 0x13,
		KEY_T				= 0x14,
		KEY_Y				= 0x15,
		KEY_U				= 0x16,
		KEY_I				= 0x17,
		KEY_O				= 0x18,
		KEY_P				= 0x19,
		KEY_A				= 0x1E,
		KEY_S				= 0x1F,
		KEY_D				= 0x20,
		KEY_F				= 0x21,
		KEY_G				= 0x22,
		KEY_H				= 0x23,
		KEY_J				= 0x24,
		KEY_K				= 0x25,
		KEY_L				= 0x26,
		KEY_Z				= 0x2C,
		KEY_X				= 0x2D,
		KEY_C				= 0x2E,
		KEY_V				= 0x2F,
		KEY_B				= 0x30,
		KEY_N				= 0x31,
		KEY_M				= 0x32,

		KEY_MINUS			= 0x0C,
		KEY_PLUS			= 0x0D,
		KEY_LBRACKET		= 0x1A,
		KEY_RBRACKET		= 0x1B,

		KEY_SEMICOLON		= 0x27,
		KEY_APOSTROPHE		= 0x28,

		KEY_COMMA			= 0x33,
		KEY_PERIOD			= 0x34,

		KEY_NUMPAD0			= 0x52,
		KEY_NUMPAD1			= 0x4F,
		KEY_NUMPAD2			= 0x50,
		KEY_NUMPAD3			= 0x51,
		KEY_NUMPAD4			= 0x4B,
		KEY_NUMPAD5			= 0x4C,
		KEY_NUMPAD6			= 0x4D,
		KEY_NUMPAD7			= 0x47,
		KEY_NUMPAD8			= 0x48,
		KEY_NUMPAD9			= 0x49,
		KEY_NUMPADPERIOD	= 0x53,
		KEY_NUMPADENTER		= 0x9C,
		KEY_NUMPADSTAR		= 0x37,
		KEY_NUMPADPLUS		= 0x4E,
		KEY_NUMPADMINUS		= 0x4A,
		KEY_NUMPADSLASH		= 0xB5,
		KEY_NUMLOCK			= 0x45,
	};
}

	/** Returns pointer to main engine class. If class is not created, also creates it.
		\param[out] pEngineCore Pointer to main engine interface. \see JTS::IEngineCore
		\return Returns true if class was created inside library and false if already created class is returned.
		*/
	extern bool GetEngine(JTS::IEngineCore *&pEngineCore);

	/** Deletes main engine class. Must be called just before exeting application.
		*/
	extern void FreeEngine();

#endif //_JTS_ENG_H
