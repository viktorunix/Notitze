#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#define WINVER 0x0A00
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdbool.h>

extern void* GetWindowHandle(void);

static WNDPROC originalWndProc = NULL;
static float currentTabletPressure = 1.0f;
static bool isTabletActive =  false;

LRESULT CALLBACK TabletWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    if(msg == 0x0245 || msg == 0x0246){
        //tablet
        UINT32 pointerId = LOWORD(wParam);

        typedef BOOL (WINAPI *GetPointerPenInfoFunc)(UINT32, POINTER_PEN_INFO*);
        static GetPointerPenInfoFunc getPenInfo = NULL;
        static bool init = false;

        if(!init){
            HMODULE user32 = GetModuleHandleA("user32.dll");
            if(user32) getPenInfo = (GetPointerPenInfoFunc)GetProcAddress(user32, "GetPointerPenInfo");
            init = true;
        }
        POINTER_PEN_INFO penInfo;
        if (getPenInfo && getPenInfo(pointerId, &penInfo)) {
            currentTabletPressure = (float)penInfo.pressure / 1024.0f;
            if (currentTabletPressure > 1.0f) currentTabletPressure = 1.0f;
            isTabletActive = true;
        }
    }
    else if (msg == 0x0200) {
        //mouse
        if ((GetMessageExtraInfo() & 0xFFFFFF00) != 0xFF515700) {
            isTabletActive = false;
        }
    }
    if (originalWndProc) return CallWindowProc(originalWndProc, hwnd, msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
void InitTabletSupport(void) {
    HWND hwnd = (HWND)GetWindowHandle();
    if (hwnd) {
        originalWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)TabletWindowProc);
    }
}

bool IsUsingTablet(void) {
    return isTabletActive;
}

float GetTabletPressure(void) {
    return currentTabletPressure;
}
#else

#include <stdbool.h>
void InitTabletSupport(void) {}
bool IsUsingTablet(void) { return false; }
float GetTabletPressure(void) { return 1.0f; }
#endif
