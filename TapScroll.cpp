#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <thread>
#include "wtypes.h"

HHOOK miHook;

extern int elements[1000] ={};
extern HWND hwndList[1000] = {};
extern int counterx=0;
extern BOOL mButtonDown=false;
extern int xPos=0, yPos=0; //current coordinates
extern int prev_yPos=NULL, prev_yPos2 = NULL, prev_xPos = NULL;
extern HWND handle = NULL, hwnd=NULL;
//extern POINT P; //point where mousebutton was clicked
int vertical = 0; //screen resolution
int horizontal = 0;
int vMinScroll = NULL, vMaxScroll = NULL; //Scroll Randge of the current window
//extern RECT r; //active window rectangle

void ScrollContent(HWND handle, int prev_yPos, int prev_yPos2, int prev_xPos, int xPos, int yPos) {

	SendMessage(handle, WM_MOUSEWHEEL, MAKEWPARAM(0, (yPos - prev_yPos2) * 15), MAKELPARAM(prev_xPos, prev_yPos));
	std::cout << "Range=" << (vMaxScroll-vMinScroll) << " handle : " << handle << " x : " << xPos << " y : " << yPos << "\n";
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	auto& ms = *(const MOUSEHOOKSTRUCT*)lParam; //The lParam parameter of MouseProc is not identical to the lParam parameter for WM_MOUSEMOVE.It is a MOUSEHOOKSTRUCT* .
	xPos = ms.pt.x; //GET_X_LPARAM(lParam);
	yPos = ms.pt.y;
	if (wParam == WM_MOUSEMOVE) {
		if ((mButtonDown) and (prev_yPos2!= yPos)) {
			ScrollContent(handle, prev_yPos, prev_yPos2, prev_xPos, xPos, yPos);
			prev_yPos2 = yPos;
		}
	}

	if (wParam == WM_MBUTTONDOWN) {
		printf("Middle Button Pressed %i %i \n", yPos, xPos);
		POINT P;
		prev_yPos = yPos;
		prev_yPos2 = yPos;
		prev_xPos = xPos;
		mButtonDown = true;

		hwnd = GetForegroundWindow();
		GetCursorPos(&P);
		handle = WindowFromPoint(P);
		GetScrollRange(handle, SB_VERT, &vMinScroll, &vMaxScroll);
		std::cout << "handle is : " << handle << "\n";
		//return -1;
	}
	if (wParam == WM_MBUTTONUP) {
		printf("Middle Button Released\n");
		mButtonDown = false;
	}

	return CallNextHookEx(miHook, nCode, wParam, lParam);
}

DWORD WINAPI ThreadProc(void*)
{
	SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		//std::cout << "task1 says: " << &msg;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(miHook);
	return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	TCHAR buff[256];
	HWND pphwnd;


	::GetWindowText(hwnd, (LPWSTR)buff, 255);
	int nCtrlID = ::GetDlgCtrlID(hwnd);
	hwndList[counterx] = hwnd;
	//std::cout << "HWND " << hwnd << std::endl;

	//test1 = GetScrollInfo(hwnd, SB_VERT, &si1);
	//if (test1) printf("is a scroll ");
	//if (test1) printf("is a scroll\n");

	//if ((nCtrlID != elements[counterx]) and (nCtrlID != 0)) 
	{
		elements[counterx] = nCtrlID;
		//printf("elemntID %i %i hwnd:%i \n", elements[counterx], counterx, hwndList[counterx]);
		counterx++;
	}
	
	return TRUE;
}

void OnEnumerateChildCtrls(HWND hwnd)
{
	counterx = 0;
	elements[0] = 0;
	EnumChildWindows(hwnd, EnumWindowsProc, 0);
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop; // Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow(); // Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);// The top left corner will have coordinates (0,0) and the bottom right corner will have coordinates (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}


int  main() {
	//HWND hwnd = FindWindow(NULL, L"Windows Desktop Wizard.cpp - Notepad2 (Administrator)");
	HWND handle1; 
	POINT P;
	RECT r;
	//HWND hWndScrollBar;
	BOOL test1=true;
	int i;
	int test2=0, x, y;
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	
	CreateThread(NULL, 0, ThreadProc, NULL, 0, 0);
	while (true)
	{
		Sleep(150000);
	}

	

	GetDesktopResolution(horizontal, vertical);
	std::cout << horizontal << '\n' << vertical << '\n';

}
