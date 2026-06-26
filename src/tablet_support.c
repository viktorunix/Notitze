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
#elif defined(__linux__)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdatomic.h>
#include <poll.h>

#define MAX_TABLETS 8

// Fallback just in case your compiler headers are older
#ifndef ABS_MT_PRESSURE
#define ABS_MT_PRESSURE 0x3a
#endif

// START AT 0.0f! If it gets no events, it will stay 0.0f instead of faking 1.0f.
static _Atomic float currentTabletPressure = 0.0f;
static _Atomic bool isTabletActive = false;
static int tabletFds[MAX_TABLETS];
static int tabletCount = 0;
static float maxHardwarePressure = 1024.0f;

static void* TabletPollerThread(void *arg){
    struct pollfd pfds[MAX_TABLETS];
    for(int i = 0; i < tabletCount; i++){
        pfds[i].fd = tabletFds[i];
        pfds[i].events = POLLIN;
    }

    while(poll(pfds, tabletCount, -1) > 0){
        for(int i = 0; i < tabletCount; i++){
            if(pfds[i].revents & POLLIN){
                struct input_event ev;

                while(read(pfds[i].fd, &ev, sizeof(ev)) > 0){

                    // Check BOTH standard pressure and multitouch pressure
                    if(ev.type == EV_ABS && (ev.code == ABS_PRESSURE || ev.code == ABS_MT_PRESSURE)){

                        if((float)ev.value > maxHardwarePressure){
                            maxHardwarePressure = (float)ev.value;
                        }

                        float p = (float)ev.value / maxHardwarePressure;
                        if(p > 1.0f) p = 1.0f;
                        if(p < 0.0f) p = 0.0f;

                        atomic_store(&currentTabletPressure, p);

                        if(ev.value > 0) {
                            atomic_store(&isTabletActive, true);
                        }


                        printf(">> KERNEL RAW: %d | NORMALIZED: %.2f\n", ev.value, p);
                    }
                    else if (ev.type == EV_KEY) {
                        if (ev.code == BTN_TOUCH || ev.code == BTN_TOOL_PEN){
                            if (ev.value == 0) {
                                atomic_store(&isTabletActive, false);
                                atomic_store(&currentTabletPressure, 0.0f);
                                printf(">> PEN LIFTED\n");
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

void InitTabletSupport(void) {
    DIR *dir = opendir("/dev/input");
    if(!dir) return;

    struct dirent *entry;
    char path[256];
    char deviceName[256] = "Unknown Device";

    while((entry = readdir(dir)) != NULL){
        if(strncmp(entry->d_name, "event", 5) == 0 && tabletCount < MAX_TABLETS){
            snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);

            int fd = open(path, O_RDONLY | O_NONBLOCK);
            if(fd != -1){
                uint8_t absBitmask[(ABS_MAX / 8) + 1] = {0};
                uint8_t keyBitmask[(KEY_MAX / 8) + 1] = {0};

                if(ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBitmask)), absBitmask) >= 0 &&
                   ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask) >= 0){

                    bool hasPressure = absBitmask[ABS_PRESSURE / 8] & (1 << (ABS_PRESSURE % 8));
                    bool hasPenTool = keyBitmask[BTN_TOOL_PEN / 8] & (1 << (BTN_TOOL_PEN % 8));

                    if(hasPressure && hasPenTool) {
                        ioctl(fd, EVIOCGNAME(sizeof(deviceName)), deviceName);
                        printf("[NOTITZE] Hooked Tablet Node: %s\n", deviceName);

                        struct input_absinfo absInfo;
                        if(ioctl(fd, EVIOCGABS(ABS_PRESSURE), &absInfo) >= 0){
                            if ((float)absInfo.maximum > maxHardwarePressure){
                                maxHardwarePressure = (float)absInfo.maximum;
                            }
                        }
                        tabletFds[tabletCount++] = fd;
                    } else{
                        close(fd);
                    }
                } else{
                    close(fd);
                }
            }
        }
    }
    closedir(dir);

    if(tabletCount > 0){
        pthread_t threadId;
        pthread_create(&threadId, NULL, TabletPollerThread, NULL);
        pthread_detach(threadId);
        printf("[NOTITZE] Tablet successfully hooked! Threads active.\n");
    } else{
        printf("[NOTITZE] No pressure-sensitive tablet found. Falling back to mouse velocity.\n");
    }
}

bool IsUsingTablet(void) {
    return atomic_load(&isTabletActive);
}

float GetTabletPressure(void) {
    return atomic_load(&currentTabletPressure);
}
#endif
