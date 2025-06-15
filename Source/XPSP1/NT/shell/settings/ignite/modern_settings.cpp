/*
 * Windows 12 Ignite - Modern Settings Panel
 */

#include <windows.h>
#include "../../themes/ignite/theme.h"

class CModernSettings {
private:
    HWND m_hwnd;
    
public:
    BOOL Initialize() {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = SettingsWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"ModernSettings";
        RegisterClassEx(&wc);
        
        m_hwnd = CreateWindowEx(
            WS_EX_LAYERED,
            L"ModernSettings", L"Settings",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            100, 100, 800, 600,
            nullptr, nullptr, GetModuleHandle(nullptr), this
        );
        
        if (m_hwnd) {
            IgniteTheme_CreateRenderer(m_hwnd);
            IGNITE_WINDOW_ATTRS attrs = {};
            attrs.effectType = IGNITE_EFFECT_ACRYLIC;
            attrs.dwCornerRadius = 12;
            attrs.backgroundColor = IGNITE_COLOR_SURFACE;
            IgniteTheme_SetWindowAttributes(m_hwnd, &attrs);
        }
        
        return m_hwnd != nullptr;
    }
    
    static LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
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

extern "C" {
    BOOL InitModernSettings() {
        static CModernSettings settings;
        return settings.Initialize();
    }
}

