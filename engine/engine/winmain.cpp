// engine.cpp : Defines the entry point for the application.
//

#include "mcv_platform.h"
#include "app.h"
#include <AntTweakBar.h>
#include <Windowsx.h>
#include "resource.h"

#include "utils/input.h"
using namespace utils;

#ifdef _TEST
#include "handles/testHandles.h"
#include "components/components.h"
#include "behavior/fsm.h"
#include "behavior/bt.h"
#endif

// Global Variables:
HINSTANCE hInst;								// current instance

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

void treatMsgs(MSG& msg)
{
    int n = 10;
    while(--n>=0 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
        switch (msg.message) {
            case WM_QUIT:
            case WM_PAINT:
            case WM_TIMER:
            case WM_MOUSEMOVE:
                n=-1;
                break;
        }
	}
}


int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	App &app = App::get();
	app.initFSM();

	// Initialize global strings
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

#ifndef _TEST
	// If the app can't start, exit
	if (!app.create()) {
        app.destroy();
		return FALSE;
    }
#endif

#ifdef _TEST
	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	treatMsgs(msg);
	utils::dbg("RUNNING TESTS\n");
	component::init();
	test::testHandles();
	test::testFSM();
	test::testBT();
	component::cleanup();
#else
	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	app.getPad().addToResetList(USER_VK_WHEELDOWN);
	app.getPad().addToResetList(USER_VK_WHEELUP);
	// First frame, we don't render or do anything, just initialize time
	//treatMsgs(msg);
	app.countTime();
	// Main loop:
	while (msg.message != WM_QUIT) {
		Mouse::refresh();
		app.getPad().reset();
		treatMsgs(msg);
		if (!app.getChangeLvl()){
			app.update();
		} else {
			app.changeLevel();
		}
		if (app.getGameState() == AppFSMExecutor::states::STATE_quit) break;
	}
	app.destroy();
#endif
	return (int)msg.wParam;
}


HICON hMyIcon;
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	hMyIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON), IMAGE_ICON, 32, 32, LR_SHARED);	
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = hMyIcon;
	wcex.hCursor = 0; //; LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0; // MAKEINTRESOURCE(IDC_ENGINE);
	wcex.lpszClassName = "Vinedetta";
	wcex.hIconSm = hMyIcon;

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	App& app = App::get();

	// Define the client area
	RECT rc = { 0, 0, app.config.xres, app.config.yres };

	// We need to tell windows the size of the full windows, including border
	// so the rect is bigger
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the actual window
	hWnd = CreateWindow("Vinedetta", "Vinedetta"
		, WS_OVERLAPPEDWINDOW
		, 0, 0//, CW_USEDEFAULT, CW_USEDEFAULT		// Position
		, rc.right - rc.left					// Width
		, rc.bottom - rc.top					// Height
		, NULL, NULL
		, hInstance
		, NULL);

	if (!hWnd)
		return FALSE;

	app.hWnd = hWnd;
    Mouse::init(hWnd, true);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
    App& app(App::get());

	// AntTweakBar
	TwEventWin(hWnd, message, wParam, lParam); // send event message to AntTweakBar

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
        app.setExit();
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
        app.setExit();
		PostQuitMessage(0);
		break;
	case WM_QUIT:
        app.setExit();
		PostQuitMessage(0);
		break;
        
    case WM_SETFOCUS:
        app.setFocus(true);
        break;
    case WM_KILLFOCUS:
        app.setFocus(false);
        Mouse::clearDelta();
        break;
    case WM_KEYUP:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
			app.getPad().onKey(wParam, false);
        }
        break;
    case WM_KEYDOWN:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            app.getPad().onKey(wParam, true);
        }
        break;
    case WM_MOUSEHOVER:
        dbg("Mouse entered screen\n");
        break;
    case WM_MOUSELEAVE:
        dbg("Mouse exited screen\n");
        break;
    case WM_MOUSEMOVE:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            Mouse::setSysMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
    case WM_MOUSEWHEEL:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            signed short wheel = static_cast<signed short>(HIWORD(wParam));
            Mouse::setWheel(wheel);
            app.getPad().onKey((wheel > 0)? USER_VK_WHEELUP : USER_VK_WHEELDOWN, true);
        }
        break;
    case WM_LBUTTONDOWN:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            Mouse::onLMB(true);
            app.getPad().onKey(VK_LBUTTON, true);
        }
        break;
    case WM_LBUTTONUP:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            Mouse::onLMB(false);
            app.getPad().onKey(VK_LBUTTON, false);
        }
        break;
    case WM_RBUTTONDOWN:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            Mouse::onRMB(true);
            app.getPad().onKey(VK_RBUTTON, true);
        }
        break;
    case WM_RBUTTONUP:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            Mouse::onRMB(false);
            app.getPad().onKey(VK_RBUTTON, false);
        }
        break;
    case WM_MBUTTONDOWN:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            Mouse::onMMB(true);
            app.getPad().onKey(VK_MBUTTON, true);
        }
        break;
    case WM_MBUTTONUP:
		if (app.hasFocus() && !app.isXboxControllerConnected()) {
            Mouse::onMMB(false);
            app.getPad().onKey(VK_MBUTTON, false);
        }
        break;
	default:
	    return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// mouse controller