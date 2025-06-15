/*
 * Windows 12 Ignite - Modern Setup Experience
 * Features modern UI, smooth animations, and guided installation
 */

#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include "../../themes/ignite/theme.h"

// Setup Constants
#define SETUP_WINDOW_WIDTH      800
#define SETUP_WINDOW_HEIGHT     600
#define SETUP_PROGRESS_HEIGHT   8
#define SETUP_BUTTON_WIDTH      120
#define SETUP_BUTTON_HEIGHT     36

// Setup Pages
typedef enum _SETUP_PAGE {
    PAGE_WELCOME = 0,
    PAGE_LICENSE,
    PAGE_PARTITION,
    PAGE_COPYING,
    PAGE_CONFIGURATION,
    PAGE_COMPLETE
} SETUP_PAGE;

// Control IDs
#define IDC_NEXT_BUTTON         2001
#define IDC_BACK_BUTTON         2002
#define IDC_CANCEL_BUTTON       2003
#define IDC_PROGRESS_BAR        2004
#define IDC_STATUS_TEXT         2005

class CModernSetup {
private:
    HWND m_hwnd;
    HWND m_hwndNext;
    HWND m_hwndBack;
    HWND m_hwndCancel;
    HWND m_hwndProgress;
    HWND m_hwndStatus;
    
    SETUP_PAGE m_currentPage;
    HFONT m_hTitleFont;
    HFONT m_hNormalFont;
    HFONT m_hSmallFont;
    
    BOOL m_bInstalling;
    DWORD m_dwInstallProgress;
    DWORD m_dwInstallStart;
    
public:
    CModernSetup() :
        m_hwnd(nullptr),
        m_hwndNext(nullptr),
        m_hwndBack(nullptr),
        m_hwndCancel(nullptr),
        m_hwndProgress(nullptr),
        m_hwndStatus(nullptr),
        m_currentPage(PAGE_WELCOME),
        m_hTitleFont(nullptr),
        m_hNormalFont(nullptr),
        m_hSmallFont(nullptr),
        m_bInstalling(FALSE),
        m_dwInstallProgress(0),
        m_dwInstallStart(0) {}
    
    ~CModernSetup() {
        Cleanup();
    }
    
    BOOL Initialize() {
        // Initialize common controls
        INITCOMMONCONTROLSEX icc = {};
        icc.dwSize = sizeof(icc);
        icc.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icc);
        
        // Register window class
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = SetupWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = L"ModernSetup";
        wc.cbWndExtra = sizeof(CModernSetup*);
        
        RegisterClassEx(&wc);
        
        // Get screen dimensions and center window
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int x = (screenWidth - SETUP_WINDOW_WIDTH) / 2;
        int y = (screenHeight - SETUP_WINDOW_HEIGHT) / 2;
        
        // Create main window
        m_hwnd = CreateWindowEx(
            WS_EX_DLGMODALFRAME,
            L"ModernSetup",
            L"Windows 12 Ignite Setup",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            x, y, SETUP_WINDOW_WIDTH, SETUP_WINDOW_HEIGHT,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            this
        );
        
        if (!m_hwnd) return FALSE;
        
        // Initialize Ignite theme
        IgniteTheme_CreateRenderer(m_hwnd);
        
        IGNITE_WINDOW_ATTRS attrs = {};
        attrs.effectType = IGNITE_EFFECT_ACRYLIC;
        attrs.dwCornerRadius = 12;
        attrs.dwBlurRadius = 30;
        attrs.backgroundColor = IGNITE_COLOR_SURFACE;
        attrs.fOpacity = 0.95f;
        attrs.bDropShadow = TRUE;
        
        IgniteTheme_SetWindowAttributes(m_hwnd, &attrs);
        
        // Create fonts
        CreateFonts();
        
        // Create controls
        CreateControls();
        
        // Update page
        UpdatePage();
        
        return TRUE;
    }
    
private:
    void CreateFonts() {
        // Title font
        m_hTitleFont = CreateFont(
            -28, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
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
        
        // Small font
        m_hSmallFont = CreateFont(
            -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
    }
    
    void CreateControls() {
        RECT rcClient;
        GetClientRect(m_hwnd, &rcClient);
        
        int buttonY = rcClient.bottom - 60;
        int buttonSpacing = 10;
        
        // Next button
        m_hwndNext = CreateWindowEx(
            0,
            L"BUTTON",
            L"Next",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            rcClient.right - SETUP_BUTTON_WIDTH - 20,
            buttonY,
            SETUP_BUTTON_WIDTH,
            SETUP_BUTTON_HEIGHT,
            m_hwnd,
            (HMENU)IDC_NEXT_BUTTON,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Back button
        m_hwndBack = CreateWindowEx(
            0,
            L"BUTTON",
            L"Back",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            rcClient.right - (SETUP_BUTTON_WIDTH * 2) - 30 - buttonSpacing,
            buttonY,
            SETUP_BUTTON_WIDTH,
            SETUP_BUTTON_HEIGHT,
            m_hwnd,
            (HMENU)IDC_BACK_BUTTON,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Cancel button
        m_hwndCancel = CreateWindowEx(
            0,
            L"BUTTON",
            L"Cancel",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            20,
            buttonY,
            SETUP_BUTTON_WIDTH,
            SETUP_BUTTON_HEIGHT,
            m_hwnd,
            (HMENU)IDC_CANCEL_BUTTON,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Progress bar
        m_hwndProgress = CreateWindowEx(
            0,
            PROGRESS_CLASS,
            nullptr,
            WS_CHILD | PBS_SMOOTH,
            50,
            rcClient.bottom - 120,
            rcClient.right - 100,
            SETUP_PROGRESS_HEIGHT,
            m_hwnd,
            (HMENU)IDC_PROGRESS_BAR,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Status text
        m_hwndStatus = CreateWindowEx(
            0,
            L"STATIC",
            L"",
            WS_CHILD | SS_LEFT,
            50,
            rcClient.bottom - 140,
            rcClient.right - 100,
            20,
            m_hwnd,
            (HMENU)IDC_STATUS_TEXT,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Set fonts
        SendMessage(m_hwndNext, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
        SendMessage(m_hwndBack, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
        SendMessage(m_hwndCancel, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
        SendMessage(m_hwndStatus, WM_SETFONT, (WPARAM)m_hSmallFont, TRUE);
        
        // Set progress bar range
        SendMessage(m_hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    }
    
    static LRESULT CALLBACK SetupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        CModernSetup* pThis = nullptr;
        
        if (msg == WM_NCCREATE) {
            CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
            pThis = (CModernSetup*)pCS->lpCreateParams;
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)pThis);
        } else {
            pThis = (CModernSetup*)GetWindowLongPtr(hwnd, 0);
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
                
            case WM_COMMAND:
                OnCommand(LOWORD(wParam), HIWORD(wParam));
                return 0;
                
            case WM_TIMER:
                if (wParam == 1) {
                    OnInstallTimer();
                }
                return 0;
                
            case WM_CLOSE:
                if (MessageBox(hwnd, L"Are you sure you want to cancel setup?", L"Cancel Setup", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    PostQuitMessage(0);
                }
                return 0;
                
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
        
        // Draw page content
        DrawPageContent(hdc);
        
        EndPaint(m_hwnd, &ps);
    }
    
    void DrawPageContent(HDC hdc) {
        RECT rcClient;
        GetClientRect(m_hwnd, &rcClient);
        
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, IGNITE_COLOR_TEXT_PRIMARY);
        
        switch (m_currentPage) {
            case PAGE_WELCOME:
                DrawWelcomePage(hdc, &rcClient);
                break;
                
            case PAGE_LICENSE:
                DrawLicensePage(hdc, &rcClient);
                break;
                
            case PAGE_PARTITION:
                DrawPartitionPage(hdc, &rcClient);
                break;
                
            case PAGE_COPYING:
                DrawCopyingPage(hdc, &rcClient);
                break;
                
            case PAGE_CONFIGURATION:
                DrawConfigurationPage(hdc, &rcClient);
                break;
                
            case PAGE_COMPLETE:
                DrawCompletePage(hdc, &rcClient);
                break;
        }
    }
    
    void DrawWelcomePage(HDC hdc, RECT* pRect) {
        // Draw Windows logo
        DrawWindowsLogo(hdc, pRect->right / 2 - 64, 80, 128);
        
        // Title
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        RECT rcTitle = { 50, 220, pRect->right - 50, 260 };
        DrawText(hdc, L"Welcome to Windows 12 Ignite", -1, &rcTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Description
        SelectObject(hdc, m_hNormalFont);
        RECT rcDesc = { 50, 280, pRect->right - 50, 380 };
        DrawText(hdc, 
            L"This setup program will install Windows 12 Ignite on your computer.\n\n"
            L"Windows 12 Ignite features a modern interface with beautiful visual effects, "
            L"improved performance, and enhanced security while maintaining compatibility "
            L"with your existing applications.",
            -1, &rcDesc, DT_LEFT | DT_WORDBREAK);
        
        SelectObject(hdc, hOldFont);
    }
    
    void DrawLicensePage(HDC hdc, RECT* pRect) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        RECT rcTitle = { 50, 50, pRect->right - 50, 90 };
        DrawText(hdc, L"License Agreement", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, m_hSmallFont);
        RECT rcLicense = { 50, 100, pRect->right - 50, pRect->bottom - 180 };
        DrawText(hdc,
            L"MICROSOFT SOFTWARE LICENSE TERMS\n"
            L"WINDOWS 12 IGNITE\n\n"
            L"These license terms are an agreement between Microsoft Corporation and you. "
            L"Please read them. They apply to the software named above, which includes the "
            L"media on which you received it, if any.\n\n"
            L"BY USING THE SOFTWARE, YOU ACCEPT THESE TERMS. IF YOU DO NOT ACCEPT THEM, "
            L"DO NOT USE THE SOFTWARE.\n\n"
            L"If you comply with these license terms, you have the rights below.\n\n"
            L"1. INSTALLATION AND USE RIGHTS.\n"
            L"   You may install and use one copy of the software on your device.\n\n"
            L"2. SCOPE OF LICENSE.\n"
            L"   The software is licensed, not sold. This agreement only gives you some "
            L"rights to use the software...",
            -1, &rcLicense, DT_LEFT | DT_WORDBREAK);
        
        SelectObject(hdc, hOldFont);
    }
    
    void DrawPartitionPage(HDC hdc, RECT* pRect) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        RECT rcTitle = { 50, 50, pRect->right - 50, 90 };
        DrawText(hdc, L"Choose Installation Location", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, m_hNormalFont);
        RECT rcDesc = { 50, 100, pRect->right - 50, 140 };
        DrawText(hdc, L"Select the drive where you want to install Windows 12 Ignite:", -1, &rcDesc, DT_LEFT | DT_WORDBREAK);
        
        // Draw partition list (simplified)
        RECT rcPartition = { 50, 160, pRect->right - 50, 200 };
        Rectangle(hdc, rcPartition.left, rcPartition.top, rcPartition.right, rcPartition.bottom);
        
        RECT rcPartText = { rcPartition.left + 10, rcPartition.top + 5, rcPartition.right - 10, rcPartition.bottom - 5 };
        DrawText(hdc, L"Drive C: (Primary) - 100 GB Available", -1, &rcPartText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, hOldFont);
    }
    
    void DrawCopyingPage(HDC hdc, RECT* pRect) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        RECT rcTitle = { 50, 50, pRect->right - 50, 90 };
        DrawText(hdc, L"Installing Windows 12 Ignite", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, m_hNormalFont);
        RECT rcDesc = { 50, 100, pRect->right - 50, 140 };
        DrawText(hdc, L"Please wait while Windows 12 Ignite is being installed on your computer.", -1, &rcDesc, DT_LEFT | DT_WORDBREAK);
        
        // Draw installation steps
        RECT rcSteps = { 50, 180, pRect->right - 50, 350 };
        DrawText(hdc,
            L"Installation steps:\n\n"
            L"• Copying Windows files\n"
            L"• Installing modern UI components\n"
            L"• Configuring Ignite theme engine\n"
            L"• Setting up hardware acceleration\n"
            L"• Installing device drivers\n"
            L"• Configuring system settings",
            -1, &rcSteps, DT_LEFT | DT_WORDBREAK);
        
        SelectObject(hdc, hOldFont);
    }
    
    void DrawConfigurationPage(HDC hdc, RECT* pRect) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        RECT rcTitle = { 50, 50, pRect->right - 50, 90 };
        DrawText(hdc, L"Finalizing Installation", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, m_hNormalFont);
        RECT rcDesc = { 50, 100, pRect->right - 50, 140 };
        DrawText(hdc, L"Windows is preparing your system for first use.", -1, &rcDesc, DT_LEFT | DT_WORDBREAK);
        
        SelectObject(hdc, hOldFont);
    }
    
    void DrawCompletePage(HDC hdc, RECT* pRect) {
        // Draw Windows logo
        DrawWindowsLogo(hdc, pRect->right / 2 - 64, 80, 128);
        
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
        RECT rcTitle = { 50, 220, pRect->right - 50, 260 };
        DrawText(hdc, L"Installation Complete", -1, &rcTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, m_hNormalFont);
        RECT rcDesc = { 50, 280, pRect->right - 50, 380 };
        DrawText(hdc,
            L"Windows 12 Ignite has been successfully installed on your computer.\n\n"
            L"Your computer will restart automatically to complete the installation. "
            L"After restart, you'll experience the modern Windows 12 Ignite interface "
            L"with beautiful visual effects and improved performance.",
            -1, &rcDesc, DT_LEFT | DT_WORDBREAK);
        
        SelectObject(hdc, hOldFont);
    }
    
    void DrawWindowsLogo(HDC hdc, int x, int y, int size) {
        HBRUSH hBrush = CreateSolidBrush(IGNITE_COLOR_ACCENT);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        
        int squareSize = size / 3;
        int gap = size / 12;
        
        // Draw Windows logo squares
        Rectangle(hdc, x, y, x + squareSize, y + squareSize);
        Rectangle(hdc, x + squareSize + gap, y, x + squareSize + gap + squareSize, y + squareSize);
        Rectangle(hdc, x, y + squareSize + gap, x + squareSize, y + squareSize + gap + squareSize);
        Rectangle(hdc, x + squareSize + gap, y + squareSize + gap, x + squareSize + gap + squareSize, y + squareSize + gap + squareSize);
        
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }
    
    void OnCommand(WORD id, WORD code) {
        switch (id) {
            case IDC_NEXT_BUTTON:
                OnNext();
                break;
                
            case IDC_BACK_BUTTON:
                OnBack();
                break;
                
            case IDC_CANCEL_BUTTON:
                PostMessage(m_hwnd, WM_CLOSE, 0, 0);
                break;
        }
    }
    
    void OnNext() {
        switch (m_currentPage) {
            case PAGE_WELCOME:
                m_currentPage = PAGE_LICENSE;
                break;
                
            case PAGE_LICENSE:
                m_currentPage = PAGE_PARTITION;
                break;
                
            case PAGE_PARTITION:
                m_currentPage = PAGE_COPYING;
                StartInstallation();
                break;
                
            case PAGE_COPYING:
                if (!m_bInstalling) {
                    m_currentPage = PAGE_CONFIGURATION;
                }
                break;
                
            case PAGE_CONFIGURATION:
                m_currentPage = PAGE_COMPLETE;
                break;
                
            case PAGE_COMPLETE:
                // Restart computer
                ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_MAJOR_OTHER);
                break;
        }
        
        UpdatePage();
    }
    
    void OnBack() {
        if (m_currentPage > PAGE_WELCOME && !m_bInstalling) {
            m_currentPage = (SETUP_PAGE)(m_currentPage - 1);
            UpdatePage();
        }
    }
    
    void UpdatePage() {
        // Update button states
        EnableWindow(m_hwndBack, m_currentPage > PAGE_WELCOME && !m_bInstalling);
        
        if (m_currentPage == PAGE_COMPLETE) {
            SetWindowText(m_hwndNext, L"Restart");
        } else {
            SetWindowText(m_hwndNext, L"Next");
        }
        
        // Show/hide progress bar
        ShowWindow(m_hwndProgress, (m_currentPage == PAGE_COPYING || m_currentPage == PAGE_CONFIGURATION) ? SW_SHOW : SW_HIDE);
        ShowWindow(m_hwndStatus, (m_currentPage == PAGE_COPYING || m_currentPage == PAGE_CONFIGURATION) ? SW_SHOW : SW_HIDE);
        
        InvalidateRect(m_hwnd, nullptr, TRUE);
    }
    
    void StartInstallation() {
        m_bInstalling = TRUE;
        m_dwInstallStart = GetTickCount();
        m_dwInstallProgress = 0;
        
        EnableWindow(m_hwndNext, FALSE);
        EnableWindow(m_hwndBack, FALSE);
        
        SetWindowText(m_hwndStatus, L"Copying files...");
        
        // Start installation timer
        SetTimer(m_hwnd, 1, 100, nullptr);
    }
    
    void OnInstallTimer() {
        if (!m_bInstalling) return;
        
        DWORD elapsed = GetTickCount() - m_dwInstallStart;
        
        // Simulate installation progress (10 seconds total)
        m_dwInstallProgress = min(100, elapsed / 100);
        
        SendMessage(m_hwndProgress, PBM_SETPOS, m_dwInstallProgress, 0);
        
        // Update status text based on progress
        if (m_dwInstallProgress < 20) {
            SetWindowText(m_hwndStatus, L"Copying Windows files...");
        } else if (m_dwInstallProgress < 40) {
            SetWindowText(m_hwndStatus, L"Installing modern UI components...");
        } else if (m_dwInstallProgress < 60) {
            SetWindowText(m_hwndStatus, L"Configuring Ignite theme engine...");
        } else if (m_dwInstallProgress < 80) {
            SetWindowText(m_hwndStatus, L"Installing device drivers...");
        } else if (m_dwInstallProgress < 100) {
            SetWindowText(m_hwndStatus, L"Finalizing installation...");
        } else {
            SetWindowText(m_hwndStatus, L"Installation complete!");
            m_bInstalling = FALSE;
            KillTimer(m_hwnd, 1);
            EnableWindow(m_hwndNext, TRUE);
        }
    }
    
    void Cleanup() {
        if (m_hTitleFont) { DeleteObject(m_hTitleFont); m_hTitleFont = nullptr; }
        if (m_hNormalFont) { DeleteObject(m_hNormalFont); m_hNormalFont = nullptr; }
        if (m_hSmallFont) { DeleteObject(m_hSmallFont); m_hSmallFont = nullptr; }
    }
};

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CoInitialize(nullptr);
    
    CModernSetup setup;
    if (!setup.Initialize()) {
        MessageBox(nullptr, L"Failed to initialize Modern Setup", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    CoUninitialize();
    return (int)msg.wParam;
}

extern "C" {
    BOOL StartModernSetup() {
        CModernSetup setup;
        return setup.Initialize();
    }
}

