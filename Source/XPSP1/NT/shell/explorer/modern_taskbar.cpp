#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include "../themes/ignite/theme.h"

// Windows 12 Ignite Modern Taskbar Implementation
// Centered icons, translucent background, modern animations

#define TASKBAR_HEIGHT 48
#define TASKBAR_ICON_SIZE 32
#define TASKBAR_PADDING 8
#define TASKBAR_CENTER_OFFSET 200

// Taskbar window class
static const WCHAR* MODERN_TASKBAR_CLASS = L"ModernIgniteTaskbar";
static HWND g_hTaskbarWnd = NULL;
static HWND g_hStartButton = NULL;
static HWND g_hTaskList = NULL;
static HWND g_hSystemTray = NULL;
static BOOL g_bCenterIcons = TRUE;
static BOOL g_bTransparent = TRUE;

// Taskbar button structure
typedef struct _TASKBAR_BUTTON {
    HWND hWnd;
    HICON hIcon;
    WCHAR szTitle[256];
    RECT rcButton;
    BOOL bActive;
    BOOL bHover;
    FLOAT fAnimationProgress;
    DWORD dwAnimationStart;
} TASKBAR_BUTTON, *PTASKBAR_BUTTON;

static TASKBAR_BUTTON g_TaskbarButtons[32];
static int g_nButtonCount = 0;

// Forward declarations
LRESULT CALLBACK ModernTaskbarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ModernStartButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawModernTaskbar(HDC hdc, RECT* pRect);
void DrawTaskbarButton(HDC hdc, PTASKBAR_BUTTON pButton);
void UpdateTaskbarLayout();
void AnimateTaskbarButton(PTASKBAR_BUTTON pButton, BOOL bHover);

// Initialize modern taskbar
BOOL WINAPI IgniteTaskbar_EnableModernStyle(BOOL bEnable)
{
    if (bEnable && !g_hTaskbarWnd) {
        // Register taskbar window class
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = ModernTaskbarWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL; // We'll handle painting
        wc.lpszClassName = MODERN_TASKBAR_CLASS;
        
        if (!RegisterClassEx(&wc)) {
            return FALSE;
        }

        // Get screen dimensions
        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

        // Create taskbar window
        g_hTaskbarWnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            MODERN_TASKBAR_CLASS,
            L"Modern Taskbar",
            WS_POPUP | WS_VISIBLE,
            0, nScreenHeight - TASKBAR_HEIGHT,
            nScreenWidth, TASKBAR_HEIGHT,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        if (!g_hTaskbarWnd) {
            return FALSE;
        }

        // Enable acrylic effect
        IgniteWindow_EnableAcrylic(g_hTaskbarWnd, TRUE);
        IgniteWindow_SetRoundedCorners(g_hTaskbarWnd, 0); // No rounded corners for taskbar

        // Create start button
        g_hStartButton = CreateWindowEx(
            0, L"BUTTON", L"⊞",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            TASKBAR_PADDING, TASKBAR_PADDING,
            TASKBAR_ICON_SIZE + 16, TASKBAR_HEIGHT - (TASKBAR_PADDING * 2),
            g_hTaskbarWnd, (HMENU)1001, GetModuleHandle(NULL), NULL
        );

        // Subclass start button for modern styling
        SetWindowSubclass(g_hStartButton, ModernStartButtonProc, 0, 0);

        // Create task list area (invisible container)
        g_hTaskList = CreateWindowEx(
            0, L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE,
            TASKBAR_ICON_SIZE + 32, 0,
            nScreenWidth - (TASKBAR_ICON_SIZE + 32) - 200, TASKBAR_HEIGHT,
            g_hTaskbarWnd, (HMENU)1002, GetModuleHandle(NULL), NULL
        );

        // Create system tray area
        g_hSystemTray = CreateWindowEx(
            0, L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE,
            nScreenWidth - 200, 0,
            200, TASKBAR_HEIGHT,
            g_hTaskbarWnd, (HMENU)1003, GetModuleHandle(NULL), NULL
        );

        // Start animation
        IgniteAnim_Start(g_hTaskbarWnd, IGNITE_ANIM_SLIDE_IN_UP, 300);

        return TRUE;
    }
    else if (!bEnable && g_hTaskbarWnd) {
        // Destroy taskbar
        DestroyWindow(g_hTaskbarWnd);
        g_hTaskbarWnd = NULL;
        g_hStartButton = NULL;
        g_hTaskList = NULL;
        g_hSystemTray = NULL;
        
        UnregisterClass(MODERN_TASKBAR_CLASS, GetModuleHandle(NULL));
        return TRUE;
    }

    return FALSE;
}

BOOL WINAPI IgniteTaskbar_SetCenterAlignment(BOOL bCenter)
{
    g_bCenterIcons = bCenter;
    if (g_hTaskbarWnd) {
        UpdateTaskbarLayout();
        InvalidateRect(g_hTaskbarWnd, NULL, TRUE);
    }
    return TRUE;
}

BOOL WINAPI IgniteTaskbar_EnableTransparency(BOOL bEnable)
{
    g_bTransparent = bEnable;
    if (g_hTaskbarWnd) {
        IgniteWindow_EnableAcrylic(g_hTaskbarWnd, bEnable);
        InvalidateRect(g_hTaskbarWnd, NULL, TRUE);
    }
    return TRUE;
}

BOOL WINAPI IgniteTaskbar_SetBlurAmount(FLOAT fAmount)
{
    // Store blur amount as window property
    if (g_hTaskbarWnd) {
        DWORD dwBlur = (DWORD)(fAmount * 100);
        SetProp(g_hTaskbarWnd, L"IgniteBlurAmount", (HANDLE)dwBlur);
        InvalidateRect(g_hTaskbarWnd, NULL, TRUE);
    }
    return TRUE;
}

// Taskbar window procedure
LRESULT CALLBACK ModernTaskbarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            
            DrawModernTaskbar(hdc, &rcClient);
            
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // We handle background painting

        case WM_LBUTTONDOWN:
        {
            // Handle taskbar button clicks
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            
            for (int i = 0; i < g_nButtonCount; i++) {
                if (PtInRect(&g_TaskbarButtons[i].rcButton, pt)) {
                    // Activate window
                    if (IsIconic(g_TaskbarButtons[i].hWnd)) {
                        ShowWindow(g_TaskbarButtons[i].hWnd, SW_RESTORE);
                    }
                    SetForegroundWindow(g_TaskbarButtons[i].hWnd);
                    break;
                }
            }
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            
            // Update hover states
            for (int i = 0; i < g_nButtonCount; i++) {
                BOOL bWasHover = g_TaskbarButtons[i].bHover;
                g_TaskbarButtons[i].bHover = PtInRect(&g_TaskbarButtons[i].rcButton, pt);
                
                if (bWasHover != g_TaskbarButtons[i].bHover) {
                    AnimateTaskbarButton(&g_TaskbarButtons[i], g_TaskbarButtons[i].bHover);
                    InvalidateRect(hWnd, &g_TaskbarButtons[i].rcButton, FALSE);
                }
            }
            return 0;
        }

        case WM_TIMER:
        {
            // Animation timer
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Start button procedure
LRESULT CALLBACK ModernStartButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT rcButton;
            GetClientRect(hWnd, &rcButton);
            
            // Create modern start button appearance
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 212)); // Windows blue
            FillRect(hdc, &rcButton, hBrush);
            DeleteObject(hBrush);
            
            // Draw Windows logo (simplified)
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            
            HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            DrawText(hdc, L"⊞", -1, &rcButton, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            // Show start menu
            IgniteStartMenu_EnableModernStyle(TRUE);
            return 0;
        }
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Draw the modern taskbar
void DrawModernTaskbar(HDC hdc, RECT* pRect)
{
    // Create memory DC for double buffering
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, pRect->right, pRect->bottom);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // Draw translucent background
    if (g_bTransparent) {
        // Create semi-transparent brush
        HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdcMem, pRect, hBrush);
        DeleteObject(hBrush);
        
        // Add subtle gradient
        for (int y = 0; y < pRect->bottom; y++) {
            int alpha = 200 + (y * 55 / pRect->bottom);
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(alpha, alpha, alpha));
            HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);
            
            MoveToEx(hdcMem, 0, y, NULL);
            LineTo(hdcMem, pRect->right, y);
            
            SelectObject(hdcMem, hOldPen);
            DeleteObject(hPen);
        }
    } else {
        // Solid background
        HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdcMem, pRect, hBrush);
        DeleteObject(hBrush);
    }

    // Draw taskbar buttons
    for (int i = 0; i < g_nButtonCount; i++) {
        DrawTaskbarButton(hdcMem, &g_TaskbarButtons[i]);
    }

    // Copy to main DC
    BitBlt(hdc, 0, 0, pRect->right, pRect->bottom, hdcMem, 0, 0, SRCCOPY);

    // Cleanup
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
}

// Draw a taskbar button
void DrawTaskbarButton(HDC hdc, PTASKBAR_BUTTON pButton)
{
    RECT rc = pButton->rcButton;
    
    // Button background
    COLORREF bgColor = RGB(220, 220, 220);
    if (pButton->bActive) {
        bgColor = RGB(0, 120, 212); // Active blue
    } else if (pButton->bHover) {
        bgColor = RGB(200, 200, 200); // Hover gray
    }
    
    // Apply animation
    if (pButton->fAnimationProgress > 0.0f) {
        COLORREF animColor = RGB(180, 180, 180);
        int r = GetRValue(bgColor) + (int)((GetRValue(animColor) - GetRValue(bgColor)) * pButton->fAnimationProgress);
        int g = GetGValue(bgColor) + (int)((GetGValue(animColor) - GetGValue(bgColor)) * pButton->fAnimationProgress);
        int b = GetBValue(bgColor) + (int)((GetBValue(animColor) - GetBValue(bgColor)) * pButton->fAnimationProgress);
        bgColor = RGB(r, g, b);
    }
    
    HBRUSH hBrush = CreateSolidBrush(bgColor);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);
    
    // Draw icon
    if (pButton->hIcon) {
        int iconX = rc.left + (rc.right - rc.left - 16) / 2;
        int iconY = rc.top + (rc.bottom - rc.top - 16) / 2;
        DrawIconEx(hdc, iconX, iconY, pButton->hIcon, 16, 16, 0, NULL, DI_NORMAL);
    }
    
    // Draw active indicator
    if (pButton->bActive) {
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 120, 212));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        
        MoveToEx(hdc, rc.left, rc.bottom - 2, NULL);
        LineTo(hdc, rc.right, rc.bottom - 2);
        
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

// Update taskbar layout
void UpdateTaskbarLayout()
{
    if (!g_hTaskbarWnd) return;
    
    RECT rcTaskbar;
    GetClientRect(g_hTaskbarWnd, &rcTaskbar);
    
    int buttonWidth = 48;
    int buttonHeight = TASKBAR_HEIGHT - (TASKBAR_PADDING * 2);
    int totalWidth = g_nButtonCount * buttonWidth;
    
    int startX;
    if (g_bCenterIcons) {
        startX = (rcTaskbar.right - totalWidth) / 2;
    } else {
        startX = TASKBAR_ICON_SIZE + 32 + TASKBAR_PADDING;
    }
    
    // Update button positions
    for (int i = 0; i < g_nButtonCount; i++) {
        g_TaskbarButtons[i].rcButton.left = startX + (i * buttonWidth);
        g_TaskbarButtons[i].rcButton.top = TASKBAR_PADDING;
        g_TaskbarButtons[i].rcButton.right = g_TaskbarButtons[i].rcButton.left + buttonWidth;
        g_TaskbarButtons[i].rcButton.bottom = g_TaskbarButtons[i].rcButton.top + buttonHeight;
    }
}

// Animate taskbar button
void AnimateTaskbarButton(PTASKBAR_BUTTON pButton, BOOL bHover)
{
    pButton->dwAnimationStart = GetTickCount();
    pButton->fAnimationProgress = bHover ? 1.0f : 0.0f;
    
    // Start animation timer
    SetTimer(g_hTaskbarWnd, 1, 16, NULL); // 60 FPS
}

// Add a window to the taskbar
BOOL AddTaskbarButton(HWND hWnd)
{
    if (g_nButtonCount >= 32) return FALSE;
    
    TASKBAR_BUTTON* pButton = &g_TaskbarButtons[g_nButtonCount];
    ZeroMemory(pButton, sizeof(TASKBAR_BUTTON));
    
    pButton->hWnd = hWnd;
    pButton->hIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
    if (!pButton->hIcon) {
        pButton->hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICONSM);
    }
    
    GetWindowText(hWnd, pButton->szTitle, 256);
    pButton->bActive = (GetForegroundWindow() == hWnd);
    
    g_nButtonCount++;
    UpdateTaskbarLayout();
    
    if (g_hTaskbarWnd) {
        InvalidateRect(g_hTaskbarWnd, NULL, TRUE);
    }
    
    return TRUE;
}

// Remove a window from the taskbar
BOOL RemoveTaskbarButton(HWND hWnd)
{
    for (int i = 0; i < g_nButtonCount; i++) {
        if (g_TaskbarButtons[i].hWnd == hWnd) {
            // Shift remaining buttons
            for (int j = i; j < g_nButtonCount - 1; j++) {
                g_TaskbarButtons[j] = g_TaskbarButtons[j + 1];
            }
            g_nButtonCount--;
            
            UpdateTaskbarLayout();
            if (g_hTaskbarWnd) {
                InvalidateRect(g_hTaskbarWnd, NULL, TRUE);
            }
            return TRUE;
        }
    }
    return FALSE;
}
