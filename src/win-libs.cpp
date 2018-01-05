#include "clientversion.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "crypt32")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "Version")
#pragma comment(lib, "Netapi32")
#pragma comment(lib, "Userenv")


#pragma comment(lib, "legacy_stdio_definitions")
#ifndef IOB_FIX
#define IOB_FIX
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#endif

#pragma comment(lib, "libleveldb")
//#pragma comment(lib, "mpir")
//#pragma comment(lib, "libssl64MD")
//#pragma comment(lib, "libcrypto64MD")
#pragma comment(lib, "event")
#pragma comment(lib, "event_core")
#pragma comment(lib, "event_extra")
#pragma comment(lib, "libprotobuf")
#pragma comment(lib, "imm32")
#pragma comment(lib, "winmm")

#ifdef USE_QT

#include <QtPlugin>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

#pragma comment(lib, "Qt5Core")
#pragma comment(lib, "Qt5Network")
#pragma comment(lib, "Qt5Gui")
#pragma comment(lib, "qtlibpng")
#pragma comment(lib, "Qt5Widgets")
#pragma comment(lib, "Qt5ThemeSupport")
#pragma comment(lib, "qtharfbuzz")
#pragma comment(lib, "qtfreetype")
#pragma comment(lib, "qtmain")
#pragma comment(lib, "qtpcre2")
#pragma comment(lib, "qwindows")
#pragma comment(lib, "Qt5FontDatabaseSupport")
#pragma comment(lib, "Qt5EventDispatcherSupport")
#pragma comment(lib, "Qt5AccessibilitySupport")
#pragma comment(lib, "Qt5EglSupport")
#pragma comment(lib, "Qt5NetworkAuth")
#pragma comment(lib, "Qt5OpenGLExtensions")
#pragma comment(lib, "Qt5PlatformCompositorSupport")
#pragma comment(lib, "Qt5PrintSupport")
#pragma comment(lib, "Qt5Bootstrap")
#pragma comment(lib, "Qt5OpenGL")
#pragma comment(lib, "libEGL")
#pragma comment(lib, "libGLESv2")
#pragma comment(lib, "qdirect2d")
#pragma comment(lib, "qminimal")
#pragma comment(lib, "Qt5WinExtras")
#pragma comment(lib, "preprocessor")
#pragma comment(lib, "d3d9")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "translator")
#pragma comment(lib, "Qt5FbSupport")
#pragma comment(lib, "Dwmapi")

#include <windows.h>
#undef WinMain

extern "C" {

	int WINAPI WinMain(
		_In_ HINSTANCE hInstance,
		_In_ HINSTANCE hPrevInstance,
		_In_ LPSTR     lpCmdLine,
		_In_ int       nCmdShow
		);

	int WINAPI wWinMain(
		_In_ HINSTANCE hInstance,
		_In_ HINSTANCE hPrevInstance,
		_In_ LPWSTR     lpCmdLine,
		_In_ int       nCmdShow
		) {

		return WinMain(hInstance, hPrevInstance, 0, nCmdShow);


	}
} // "C"

#endif // USE_QT
