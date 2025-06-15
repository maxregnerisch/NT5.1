/*
 * Modern Window Management - Snap Layouts
 */

#include "../../../shell/themes/ignite/theme.h"

typedef struct {
    RECT rect;
    HWND hwnd;
    BOOL bSnapped;
} SNAP_WINDOW;

static SNAP_WINDOW g_SnapWindows[10];
static int g_SnapCount = 0;

BOOL SnapWindowToLayout(HWND hwnd, int layout) {
    if (!IsWindow(hwnd) || g_SnapCount >= 10) return FALSE;
    
    RECT rcScreen;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
    
    RECT rcSnap = rcScreen;
    
    switch (layout) {
        case 1: // Left half
            rcSnap.right = rcSnap.left + (rcSnap.right - rcSnap.left) / 2;
            break;
        case 2: // Right half
            rcSnap.left = rcSnap.left + (rcSnap.right - rcSnap.left) / 2;
            break;
        case 3: // Top left quarter
            rcSnap.right = rcSnap.left + (rcSnap.right - rcSnap.left) / 2;
            rcSnap.bottom = rcSnap.top + (rcSnap.bottom - rcSnap.top) / 2;
            break;
        case 4: // Top right quarter
            rcSnap.left = rcSnap.left + (rcSnap.right - rcSnap.left) / 2;
            rcSnap.bottom = rcSnap.top + (rcSnap.bottom - rcSnap.top) / 2;
            break;
    }
    
    SetWindowPos(hwnd, nullptr, rcSnap.left, rcSnap.top,
                rcSnap.right - rcSnap.left, rcSnap.bottom - rcSnap.top,
                SWP_NOZORDER | SWP_NOACTIVATE);
    
    g_SnapWindows[g_SnapCount].hwnd = hwnd;
    g_SnapWindows[g_SnapCount].rect = rcSnap;
    g_SnapWindows[g_SnapCount].bSnapped = TRUE;
    g_SnapCount++;
    
    return TRUE;
}

extern "C" {
    BOOL InitSnapLayouts() { g_SnapCount = 0; return TRUE; }
    BOOL SnapWindow(HWND hwnd, int layout) { return SnapWindowToLayout(hwnd, layout); }
}

