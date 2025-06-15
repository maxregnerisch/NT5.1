/*
 * Windows 12 Ignite - Modern Login Screen
 * Features blur effects, modern design, and smooth animations
 */

#include <windows.h>
#include <commctrl.h>
#include "../../themes/ignite/theme.h"

// Login Screen Constants
#define LOGIN_WINDOW_WIDTH      400
#define LOGIN_WINDOW_HEIGHT     500
#define LOGIN_AVATAR_SIZE       120
#define LOGIN_FIELD_WIDTH       300
#define LOGIN_FIELD_HEIGHT      40
#define LOGIN_BUTTON_WIDTH      120
#define LOGIN_BUTTON_HEIGHT     36

// Control IDs
#define IDC_USERNAME_EDIT       1001
#define IDC_PASSWORD_EDIT       1002
#define IDC_LOGIN_BUTTON        1003
#define IDC_SHUTDOWN_BUTTON     1004
#define IDC_RESTART_BUTTON      1005
#define IDC_SWITCH_USER_BUTTON  1006

// Login Screen Class
class CModernLogonUI {
private:
    HWND m_hwnd;
    HWND m_hwndUsername;
    HWND m_hwndPassword;
    HWND m_hwndLoginBtn;
    HWND m_hwndShutdownBtn;
    HWND m_hwndRestartBtn;
    HWND m_hwndSwitchUserBtn;
    
    HFONT m_hTitleFont;
    HFONT m_hNormalFont;
    HBITMAP m_hAvatarBitmap;
    
    BOOL m_bAnimating;
    DWORD m_dwAnimStart;
    FLOAT m_fAnimProgress;
    
public:
    CModernLogonUI() : 
        m_hwnd(nullptr),
        m_hwndUsername(nullptr),
        m_hwndPassword(nullptr),
        m_hwndLoginBtn(nullptr),
        m_hwndShutdownBtn(nullptr),
        m_hwndRestartBtn(nullptr),
        m_hwndSwitchUserBtn(nullptr),
        m_hTitleFont(nullptr),
        m_hNormalFont(nullptr),
        m_hAvatarBitmap(nullptr),
        m_bAnimating(TRUE),
        m_dwAnimStart(GetTickCount()),
        m_fAnimProgress(0.0f) {}
    
    ~CModernLogonUI() {
        Cleanup();
    }
    
    BOOL Initialize() {
        // Register window class
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = LogonWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr; // Custom background
        wc.lpszClassName = L"ModernLogonUI";
        wc.cbWndExtra = sizeof(CModernLogonUI*);
        
        RegisterClassEx(&wc);
        
        // Get screen dimensions
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Create main window (fullscreen)
        m_hwnd = CreateWindowEx(
            WS_EX_TOPMOST,
            L"ModernLogonUI",
            L"Windows 12 Ignite",
            WS_POPUP | WS_VISIBLE,
            0, 0, screenWidth, screenHeight,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            this
        );
        
        if (!m_hwnd) return FALSE;
        
        // Initialize Ignite theme
        IgniteTheme_CreateRenderer(m_hwnd);
        
        IGNITE_WINDOW_ATTRS attrs = {};
        attrs.effectType = IGNITE_EFFECT_ACRYLIC;
        attrs.dwCornerRadius = 0; // Fullscreen
        attrs.dwBlurRadius = 50;
        attrs.backgroundColor = IGNITE_COLOR_SURFACE;
        attrs.fOpacity = 0.95f;
        attrs.bDropShadow = FALSE;
        
        IgniteTheme_SetWindowAttributes(m_hwnd, &attrs);
        
        // Create fonts
        CreateFonts();
        
        // Create controls
        CreateControls();
        
        // Start animation timer
        SetTimer(m_hwnd, 1, 16, nullptr); // 60 FPS
        
        return TRUE;
    }
    
private:
    void CreateFonts() {
        // Title font (large)
        m_hTitleFont = CreateFont(
            -32, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
        
        // Normal font
        m_hNormalFont = CreateFont(
            -16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
    }
    
    void CreateControls() {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Calculate center position
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        
        // Username field
        m_hwndUsername = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            centerX - LOGIN_FIELD_WIDTH / 2,
            centerY - 20,
            LOGIN_FIELD_WIDTH,
            LOGIN_FIELD_HEIGHT,
            m_hwnd,
            (HMENU)IDC_USERNAME_EDIT,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Password field
        m_hwndPassword = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL,
            centerX - LOGIN_FIELD_WIDTH / 2,
            centerY + 30,
            LOGIN_FIELD_WIDTH,
            LOGIN_FIELD_HEIGHT,
            m_hwnd,
            (HMENU)IDC_PASSWORD_EDIT,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Login button
        m_hwndLoginBtn = CreateWindowEx(
            0,
            L"BUTTON",
            L"Sign in",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            centerX - LOGIN_BUTTON_WIDTH / 2,
            centerY + 90,
            LOGIN_BUTTON_WIDTH,
            LOGIN_BUTTON_HEIGHT,
            m_hwnd,
            (HMENU)IDC_LOGIN_BUTTON,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Power options (bottom right)
        int buttonY = screenHeight - 60;
        int buttonSpacing = 80;
        
        m_hwndShutdownBtn = CreateWindowEx(
            0,
            L"BUTTON",
            L"⏻",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            screenWidth - 200,
            buttonY,
            50, 50,
            m_hwnd,
            (HMENU)IDC_SHUTDOWN_BUTTON,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        m_hwndRestartBtn = CreateWindowEx(
            0,
            L"BUTTON",
            L"↻",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            screenWidth - 140,
            buttonY,
            50, 50,
            m_hwnd,
            (HMENU)IDC_RESTART_BUTTON,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Set fonts
        SendMessage(m_hwndUsername, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
        SendMessage(m_hwndPassword, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
        SendMessage(m_hwndLoginBtn, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
        
        // Set placeholder text
        SendMessage(m_hwndUsername, EM_SETCUEBANNER, TRUE, (LPARAM)L"Username");
        SendMessage(m_hwndPassword, EM_SETCUEBANNER, TRUE, (LPARAM)L"Password");
        
        // Focus on username field
        SetFocus(m_hwndUsername);
    }
    
    static LRESULT CALLBACK LogonWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        CModernLogonUI* pThis = nullptr;
        
        if (msg == WM_NCCREATE) {
            CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
            pThis = (CModernLogonUI*)pCS->lpCreateParams;
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)pThis);
        } else {
            pThis = (CModernLogonUI*)GetWindowLongPtr(hwnd, 0);
        }
        
        if (pThis) {
            return pThis->HandleMessage(hwnd, msg, wParam, lParam);
        }
        
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_PAINT:
                OnPaint();
                return 0;
                
            case WM_TIMER:
                if (wParam == 1) {
                    OnTimer();
                }
                return 0;
                
            case WM_COMMAND:
                OnCommand(LOWORD(wParam), HIWORD(wParam));
                return 0;
                
            case WM_KEYDOWN:
                if (wParam == VK_RETURN) {
                    OnLogin();
                    return 0;
                }
                break;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    void OnPaint() {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hwnd, &ps);
        
        // Draw background with Ignite theme
        IgniteTheme_RenderWindow(m_hwnd, hdc, &ps.rcPaint);
        
        // Draw custom content
        DrawLoginContent(hdc);
        
        EndPaint(m_hwnd, &ps);
    }
    
    void DrawLoginContent(HDC hdc) {
        RECT rcClient;
        GetClientRect(m_hwnd, &rcClient);
        
        int centerX = rcClient.right / 2;
        int centerY = rcClient.bottom / 2;
        
        // Draw background gradient
        DrawModernBackground(hdc, &rcClient);
        
        // Draw user avatar with animation
        DrawUserAvatar(hdc, centerX, centerY - 120);
        
        // Draw welcome text
        DrawWelcomeText(hdc, centerX, centerY - 180);
        
        // Draw time and date
        DrawTimeAndDate(hdc, 50, 50);
    }
    
    void DrawModernBackground(HDC hdc, RECT* pRect) {
        // Create modern gradient background
        TRIVERTEX vertices[4] = {
            { pRect->left, pRect->top, 0x1000, 0x2000, 0x4000, 0xFF00 },
            { pRect->right, pRect->top, 0x2000, 0x3000, 0x6000, 0xFF00 },
            { pRect->left, pRect->bottom, 0x0800, 0x1000, 0x3000, 0xFF00 },
            { pRect->right, pRect->bottom, 0x1800, 0x2800, 0x5000, 0xFF00 }
        };
        
        GRADIENT_RECT gradRect = { 0, 1 };
        GradientFill(hdc, vertices, 4, &gradRect, 1, GRADIENT_FILL_RECT_H);
    }
    
    void DrawUserAvatar(HDC hdc, int x, int y) {
        // Draw modern circular avatar with animation
        FLOAT alpha = min(1.0f, m_fAnimProgress * 2.0f);
        
        if (alpha > 0.0f) {
            // Create circular region
            HRGN hRgn = CreateEllipticRgn(
                x - LOGIN_AVATAR_SIZE / 2,
                y - LOGIN_AVATAR_SIZE / 2,
                x + LOGIN_AVATAR_SIZE / 2,
                y + LOGIN_AVATAR_SIZE / 2
            );
            
            // Fill with modern color
            HBRUSH hBrush = CreateSolidBrush(IGNITE_COLOR_ACCENT);
            FillRgn(hdc, hRgn, hBrush);
            
            // Draw user icon (simplified)
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
            
            RECT rcIcon = {
                x - 20, y - 10,
                x + 20, y + 10
            };
            
            DrawText(hdc, L"👤", -1, &rcIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            SelectObject(hdc, hOldFont);
            DeleteObject(hBrush);
            DeleteObject(hRgn);
        }
    }
    
    void DrawWelcomeText(HDC hdc, int x, int y) {
        FLOAT alpha = min(1.0f, (m_fAnimProgress - 0.3f) * 2.0f);
        
        if (alpha > 0.0f) {
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
            
            RECT rcText = { x - 200, y - 20, x + 200, y + 20 };
            DrawText(hdc, L"Welcome", -1, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            SelectObject(hdc, hOldFont);
        }
    }
    
    void DrawTimeAndDate(HDC hdc, int x, int y) {
        // Get current time
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        WCHAR timeStr[64];
        WCHAR dateStr[64];
        
        GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, nullptr, timeStr, ARRAYSIZE(timeStr));
        GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, nullptr, dateStr, ARRAYSIZE(dateStr));
        
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        
        // Draw time (large)
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        TextOut(hdc, x, y, timeStr, wcslen(timeStr));
        
        // Draw date (smaller)
        SelectObject(hdc, m_hNormalFont);
        TextOut(hdc, x, y + 40, dateStr, wcslen(dateStr));
        
        SelectObject(hdc, hOldFont);
    }
    
    void OnTimer() {
        if (m_bAnimating) {
            DWORD elapsed = GetTickCount() - m_dwAnimStart;
            m_fAnimProgress = min(1.0f, elapsed / 1500.0f); // 1.5 second animation
            
            if (m_fAnimProgress >= 1.0f) {
                m_bAnimating = FALSE;
                KillTimer(m_hwnd, 1);
            }
            
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
    }
    
    void OnCommand(WORD id, WORD code) {
        switch (id) {
            case IDC_LOGIN_BUTTON:
                OnLogin();
                break;
                
            case IDC_SHUTDOWN_BUTTON:
                OnShutdown();
                break;
                
            case IDC_RESTART_BUTTON:
                OnRestart();
                break;
        }
    }
    
    void OnLogin() {
        WCHAR username[256];
        WCHAR password[256];
        
        GetWindowText(m_hwndUsername, username, ARRAYSIZE(username));
        GetWindowText(m_hwndPassword, password, ARRAYSIZE(password));
        
        // Validate credentials (simplified)
        if (wcslen(username) > 0 && wcslen(password) > 0) {
            // Show loading animation
            MessageBox(m_hwnd, L"Signing in...", L"Windows 12 Ignite", MB_OK | MB_ICONINFORMATION);
            
            // In real implementation, validate against SAM database
            // For now, just exit to desktop
            PostQuitMessage(0);
        } else {
            MessageBox(m_hwnd, L"Please enter your username and password.", L"Sign in", MB_OK | MB_ICONWARNING);
        }
    }
    
    void OnShutdown() {
        if (MessageBox(m_hwnd, L"Are you sure you want to shut down?", L"Shut down Windows", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            ExitWindowsEx(EWX_SHUTDOWN | EWX_POWEROFF, SHTDN_REASON_MAJOR_OTHER);
        }
    }
    
    void OnRestart() {
        if (MessageBox(m_hwnd, L"Are you sure you want to restart?", L"Restart Windows", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_MAJOR_OTHER);
        }
    }
    
    void Cleanup() {
        if (m_hTitleFont) { DeleteObject(m_hTitleFont); m_hTitleFont = nullptr; }
        if (m_hNormalFont) { DeleteObject(m_hNormalFont); m_hNormalFont = nullptr; }
        if (m_hAvatarBitmap) { DeleteObject(m_hAvatarBitmap); m_hAvatarBitmap = nullptr; }
    }
};

// Global instance
static std::unique_ptr<CModernLogonUI> g_pLogonUI;

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize COM
    CoInitialize(nullptr);
    
    // Create and initialize logon UI
    g_pLogonUI = std::make_unique<CModernLogonUI>();
    
    if (!g_pLogonUI->Initialize()) {
        MessageBox(nullptr, L"Failed to initialize Modern Logon UI", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    g_pLogonUI.reset();
    CoUninitialize();
    
    return (int)msg.wParam;
}

extern "C" {
    BOOL InitModernLogonUI() {
        g_pLogonUI = std::make_unique<CModernLogonUI>();
        return g_pLogonUI->Initialize();
    }
    
    void CleanupModernLogonUI() {
        g_pLogonUI.reset();
    }
}

