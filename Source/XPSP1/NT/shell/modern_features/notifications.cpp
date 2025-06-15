/*
 * Modern Notification System
 */

#include "../../themes/ignite/theme.h"

class CModernNotifications {
private:
    HWND m_hwnd;
    
public:
    BOOL Initialize() {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = NotificationWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"ModernNotifications";
        RegisterClassEx(&wc);
        
        RECT rcScreen;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
        
        m_hwnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            L"ModernNotifications", L"",
            WS_POPUP,
            rcScreen.right - 350, rcScreen.top + 50,
            300, 100,
            nullptr, nullptr, GetModuleHandle(nullptr), this
        );
        
        return m_hwnd != nullptr;
    }
    
    void ShowNotification(LPCWSTR title, LPCWSTR message) {
        if (m_hwnd) {
            ShowWindow(m_hwnd, SW_SHOW);
            SetTimer(m_hwnd, 1, 5000, nullptr); // Auto-hide after 5 seconds
        }
    }
    
    static LRESULT CALLBACK NotificationWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_TIMER:
                if (wParam == 1) {
                    ShowWindow(hwnd, SW_HIDE);
                    KillTimer(hwnd, 1);
                }
                return 0;
                
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                IgniteTheme_RenderWindow(hwnd, hdc, &ps.rcPaint);
                EndPaint(hwnd, &ps);
                return 0;
            }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
};

static CModernNotifications g_Notifications;

extern "C" {
    BOOL InitModernNotifications() { return g_Notifications.Initialize(); }
    void ShowModernNotification(LPCWSTR title, LPCWSTR message) { g_Notifications.ShowNotification(title, message); }
}

