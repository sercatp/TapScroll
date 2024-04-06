#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <thread>
#include "wtypes.h"

// Global variables
HHOOK miHook;

extern int cursorShiftThreshold = 20; // Square of the pixel shift to activate click instead of scroll on button3 press
extern bool katMouse = true; // Scroll window under the mouse, not the active one
extern bool mWheelEventSent = true; // Locker to send events, ignoring processing logic
extern bool LockMiddleButton = false;

extern int elements[1000] = {};
extern HWND hwndList[1000] = {};
extern int counterx = 0;
extern bool mButtonDown = false; //flag to ignore or implement ButtonDown events.
extern int xPos = 0, yPos = 0; // Current coordinates
extern int prev_yPos = NULL, prev_yPos2 = NULL, prev_xPos = NULL, prev_xPos2 = NULL;
extern int dpi = 0;
extern HWND handle = NULL, hwnd = NULL, handle1 = NULL;
int vertical = 0; // Screen resolution
int horizontal = 0;
int vMinScroll = NULL, vMaxScroll = NULL; // Scroll range of the current window


// New Thread to simulate middle button click. Replaces regular button click with this simulation.
//It is a separate thread. Placing this in the same thread with LowLevelMouseProc is bad and deadlocks the mouse function.
DWORD WINAPI  SimulateMiddleButtonClick(LPVOID lpParam) {
    INPUT input;

    // Simulate middle button down
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

    Sleep(10);
    SendInput(1, &input, sizeof(INPUT));
    Sleep(20);
    mButtonDown = false; //to prevent recurtion on MButtonUp
    input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
    SendInput(1, &input, sizeof(INPUT));

    printf("    Thread finished\n");
    return 0;
}


// Function to scroll content based on mouse movement
void ScrollContent(HWND handle, int prev_yPos, int prev_yPos2, int prev_xPos, int prev_xPos2, int xPos, int yPos) {
    SendMessage(handle, WM_MOUSEWHEEL, MAKEWPARAM(0, (yPos - prev_yPos2) * 15), MAKELPARAM(prev_xPos, prev_yPos)); // Vertical scroll only
    std::cout << ".";
}


// Low-level mouse hook callback function
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    auto& ms = *(const MOUSEHOOKSTRUCT*)lParam;
    xPos = ms.pt.x;
    yPos = ms.pt.y;

    if (wParam == WM_MOUSEMOVE) {
        if ((mButtonDown) and ((prev_yPos2 != yPos) or (prev_xPos2 != xPos))) {
            // If mouse cursor was moved for more than sqrt(moveCursor) pixels then do not send the mouse button click on release
            if (pow((prev_xPos - xPos), 2) + pow((prev_yPos - yPos), 2) > cursorShiftThreshold) {
                LockMiddleButton = true;
            }
            ScrollContent(handle, prev_yPos, prev_yPos2, prev_xPos, prev_xPos2, xPos, yPos);
            prev_yPos2 = yPos;
            prev_xPos2 = xPos;
        }
    }

    if (wParam == WM_MBUTTONDOWN) {
        // Check if [-right] Ctrl key is pressed. This is to temporary disable scrolling functionality from external programs. 
        // and prevent issues with StrokesPlus gesture "Open link in New Tab" when middle key is pressed (and page is scrolled which is undesireble)
        if (GetKeyState(VK_CONTROL) > -1) {

            printf("Middle Button Pressed %i %i \n", xPos, yPos);
            POINT P;
            prev_yPos = yPos;
            prev_xPos = xPos;
            prev_yPos2 = yPos;
            prev_xPos2 = xPos;

            LockMiddleButton = false;

            GetCursorPos(&P);
            handle = WindowFromPoint(P);
            //x, y = GetLParam(handle, xPos, yPos);

            GetScrollRange(handle, SB_VERT, &vMinScroll, &vMaxScroll);
            std::cout << "handle is : " << handle << " range=" << vMaxScroll << " " << vMinScroll << "\n";

            // Ignore sending mButtonDown event until the button is released
            if (mButtonDown == false) {
                mButtonDown = true;
                return -1; //- (mbutton coord bug)
            }
        }
    }

    if (wParam == WM_MBUTTONUP) {
        // If mouse cursor was moved for less than sqrt(moveCursor) pixels then send the mouse button click
        if (!LockMiddleButton) {
            if (mButtonDown == true) {
                HANDLE hThread = CreateThread(NULL, 0, SimulateMiddleButtonClick, NULL, 0, NULL);
                CloseHandle(hThread);
                printf("Thread started\n");
                return -1;
            }
        }
        else {
            mButtonDown = false;
            printf("Middle Button Released\n");
            //return -1; - (mbutton coord bug) // Do not send MButtonUp event if the action was scroll (low distance mouse move)
        }

    }

    return CallNextHookEx(miHook, nCode, wParam, lParam);
}

// Thread procedure to set up the mouse hook
DWORD WINAPI ThreadProc(void*) {
    SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(miHook);
    return 0;
}

// Callback function to enumerate child windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    TCHAR buff[256];

    ::GetWindowText(hwnd, (LPWSTR)buff, 255);
    int nCtrlID = ::GetDlgCtrlID(hwnd);
    hwndList[counterx] = hwnd;

    elements[counterx] = nCtrlID;
    counterx++;

    return TRUE;
}

// Function to enumerate child windows
void OnEnumerateChildCtrls(HWND hwnd) {
    counterx = 0;
    elements[0] = 0;
    EnumChildWindows(hwnd, EnumWindowsProc, 0);
}

// Function to get the horizontal and vertical screen sizes in pixels
void GetDesktopResolution(int& horizontal, int& vertical) {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

// Main function
int main() {

    //Hide console window
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);

    // Start the thread to set up the message-only window
    CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);

    // Your existing main loop
    while (true)
    {
        Sleep(150000); // Sleep for some time, or your application's main processing logic
    }

    return 0;
}
