/*
 * Windows 12 Ignite - Modern Start Menu
 */

#include "../cabinet.h"
#include "../../themes/ignite/theme.h"

class CIgniteStartMenu {
private:
    HWND m_hwnd;
    BOOL m_bVisible;
    
public:
    CIgniteStartMenu() : m_hwnd(nullptr), m_bVisible(FALSE) {}
    
    BOOL Initialize() {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StartMenuWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = L"IgniteStartMenu";
        wc.cbWndExtra = sizeof(CIgniteStartMenu*);
        
        RegisterClassEx(&wc);
        
        m_hwnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            L"IgniteStartMenu", L"",
            WS_POPUP,
            100, 100, 400, 600,
            nullptr, nullptr, GetModuleHandle(nullptr), this
        );
        
        if (m_hwnd) {
            IgniteTheme_CreateRenderer(m_hwnd);
            IGNITE_WINDOW_ATTRS attrs = {};
            attrs.effectType = IGNITE_EFFECT_ACRYLIC;
            attrs.dwCornerRadius = 12;
            attrs.dwBlurRadius = 30;
            attrs.backgroundColor = IGNITE_COLOR_SURFACE;
            attrs.fOpacity = 0.9f;
            IgniteTheme_SetWindowAttributes(m_hwnd, &attrs);
        }
        
        return m_hwnd != nullptr;
    }
    
    void Show() {
        if (m_hwnd) {
            ShowWindow(m_hwnd, SW_SHOW);
            m_bVisible = TRUE;
        }
    }
    
    void Hide() {
        if (m_hwnd) {
            ShowWindow(m_hwnd, SW_HIDE);
            m_bVisible = FALSE;
        }
    }
    
    static LRESULT CALLBACK StartMenuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        CIgniteStartMenu* pThis = nullptr;
        if (msg == WM_NCCREATE) {
            pThis = (CIgniteStartMenu*)((CREATESTRUCT*)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)pThis);
        } else {
            pThis = (CIgniteStartMenu*)GetWindowLongPtr(hwnd, 0);
        }
        
        if (pThis) {
            switch (msg) {
                case WM_PAINT: {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hwnd, &ps);
                    IgniteTheme_RenderWindow(hwnd, hdc, &ps.rcPaint);
                    EndPaint(hwnd, &ps);
                    return 0;
                }
            }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
};

static CIgniteStartMenu g_StartMenu;

extern "C" {
    BOOL InitIgniteStartMenu() { return g_StartMenu.Initialize(); }
    void ShowIgniteStartMenu() { g_StartMenu.Show(); }
    void HideIgniteStartMenu() { g_StartMenu.Hide(); }
}

