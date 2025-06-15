/*
 * Windows 12 Ignite - Modern Desktop
 */

#include "../cabinet.h"
#include "../../themes/ignite/theme.h"

class CModernDesktop {
private:
    HWND m_hwnd;
    
public:
    BOOL Initialize() {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = DesktopWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"ModernDesktop";
        
        RegisterClassEx(&wc);
        
        RECT rcScreen;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
        
        m_hwnd = CreateWindowEx(
            WS_EX_TOOLWINDOW,
            L"ModernDesktop", L"",
            WS_POPUP | WS_VISIBLE,
            rcScreen.left, rcScreen.top,
            rcScreen.right - rcScreen.left,
            rcScreen.bottom - rcScreen.top,
            nullptr, nullptr, GetModuleHandle(nullptr), this
        );
        
        return m_hwnd != nullptr;
    }
    
    static LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Modern gradient background
                RECT rc;
                GetClientRect(hwnd, &rc);
                
                TRIVERTEX vertices[2] = {
                    { rc.left, rc.top, 0x2000, 0x4000, 0x8000, 0xFF00 },
                    { rc.right, rc.bottom, 0x1000, 0x2000, 0x6000, 0xFF00 }
                };
                
                GRADIENT_RECT gradRect = { 0, 1 };
                GradientFill(hdc, vertices, 2, &gradRect, 1, GRADIENT_FILL_RECT_V);
                
                EndPaint(hwnd, &ps);
                return 0;
            }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
};

static CModernDesktop g_Desktop;

extern "C" {
    BOOL InitModernDesktop() { return g_Desktop.Initialize(); }
}

