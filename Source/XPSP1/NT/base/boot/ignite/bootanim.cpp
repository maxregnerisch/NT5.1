/*
 * Windows 12 Ignite - Modern Boot Animation
 * Features smooth animations, modern graphics, and GPU acceleration
 */

#include <ntddk.h>
#include <windef.h>
#include <wingdi.h>
#include "../../shell/themes/ignite/theme.h"

// Boot Animation Constants
#define BOOT_ANIM_WIDTH         1920
#define BOOT_ANIM_HEIGHT        1080
#define BOOT_ANIM_FPS           60
#define BOOT_ANIM_DURATION      5000    // 5 seconds
#define BOOT_LOGO_SIZE          128
#define BOOT_PROGRESS_WIDTH     400
#define BOOT_PROGRESS_HEIGHT    8

// Animation States
typedef enum _BOOT_ANIM_STATE {
    BOOT_STATE_INIT = 0,
    BOOT_STATE_LOGO_FADE_IN,
    BOOT_STATE_PROGRESS_SHOW,
    BOOT_STATE_LOADING,
    BOOT_STATE_FADE_OUT,
    BOOT_STATE_COMPLETE
} BOOT_ANIM_STATE;

// Boot Animation Context
typedef struct _BOOT_ANIMATION_CONTEXT {
    BOOT_ANIM_STATE state;
    DWORD dwStartTime;
    DWORD dwCurrentTime;
    FLOAT fProgress;
    FLOAT fLogoAlpha;
    FLOAT fProgressAlpha;
    BOOL bHardwareAccel;
    HDC hdcScreen;
    HDC hdcBuffer;
    HBITMAP hbmBuffer;
    RECT rcScreen;
    HFONT hFont;
} BOOT_ANIMATION_CONTEXT;

static BOOT_ANIMATION_CONTEXT g_BootContext = {0};

// Modern Windows 12 Ignite Colors
#define IGNITE_BOOT_BG_START    RGB(16, 24, 48)     // Deep blue
#define IGNITE_BOOT_BG_END      RGB(32, 48, 96)     // Lighter blue
#define IGNITE_BOOT_ACCENT      RGB(0, 120, 215)    // Windows blue
#define IGNITE_BOOT_TEXT        RGB(255, 255, 255)  // White
#define IGNITE_BOOT_PROGRESS    RGB(0, 204, 255)    // Cyan accent

BOOL InitBootAnimation() {
    ZeroMemory(&g_BootContext, sizeof(g_BootContext));
    
    // Get screen dimensions
    g_BootContext.rcScreen.left = 0;
    g_BootContext.rcScreen.top = 0;
    g_BootContext.rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
    g_BootContext.rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);
    
    // Create screen DC
    g_BootContext.hdcScreen = GetDC(NULL);
    if (!g_BootContext.hdcScreen) return FALSE;
    
    // Create double buffer
    g_BootContext.hdcBuffer = CreateCompatibleDC(g_BootContext.hdcScreen);
    if (!g_BootContext.hdcBuffer) return FALSE;
    
    g_BootContext.hbmBuffer = CreateCompatibleBitmap(
        g_BootContext.hdcScreen,
        g_BootContext.rcScreen.right,
        g_BootContext.rcScreen.bottom
    );
    
    if (!g_BootContext.hbmBuffer) return FALSE;
    
    SelectObject(g_BootContext.hdcBuffer, g_BootContext.hbmBuffer);
    
    // Create modern font
    g_BootContext.hFont = CreateFont(
        -24, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    
    // Initialize state
    g_BootContext.state = BOOT_STATE_INIT;
    g_BootContext.dwStartTime = GetTickCount();
    g_BootContext.fProgress = 0.0f;
    g_BootContext.fLogoAlpha = 0.0f;
    g_BootContext.fProgressAlpha = 0.0f;
    
    // Try to enable hardware acceleration
    g_BootContext.bHardwareAccel = IgniteTheme_CreateRenderer(NULL);
    
    return TRUE;
}

void CleanupBootAnimation() {
    if (g_BootContext.hFont) {
        DeleteObject(g_BootContext.hFont);
        g_BootContext.hFont = NULL;
    }
    
    if (g_BootContext.hbmBuffer) {
        DeleteObject(g_BootContext.hbmBuffer);
        g_BootContext.hbmBuffer = NULL;
    }
    
    if (g_BootContext.hdcBuffer) {
        DeleteDC(g_BootContext.hdcBuffer);
        g_BootContext.hdcBuffer = NULL;
    }
    
    if (g_BootContext.hdcScreen) {
        ReleaseDC(NULL, g_BootContext.hdcScreen);
        g_BootContext.hdcScreen = NULL;
    }
}

void DrawModernGradientBackground(HDC hdc, RECT* pRect) {
    // Create modern gradient background
    TRIVERTEX vertices[4] = {
        // Top gradient
        { pRect->left, pRect->top, 
          GetRValue(IGNITE_BOOT_BG_START) << 8, 
          GetGValue(IGNITE_BOOT_BG_START) << 8, 
          GetBValue(IGNITE_BOOT_BG_START) << 8, 0xFF00 },
        { pRect->right, pRect->top + (pRect->bottom - pRect->top) / 2,
          GetRValue(IGNITE_BOOT_BG_END) << 8,
          GetGValue(IGNITE_BOOT_BG_END) << 8,
          GetBValue(IGNITE_BOOT_BG_END) << 8, 0xFF00 },
        // Bottom gradient  
        { pRect->left, pRect->top + (pRect->bottom - pRect->top) / 2,
          GetRValue(IGNITE_BOOT_BG_END) << 8,
          GetGValue(IGNITE_BOOT_BG_END) << 8,
          GetBValue(IGNITE_BOOT_BG_END) << 8, 0xFF00 },
        { pRect->right, pRect->bottom,
          GetRValue(IGNITE_BOOT_BG_START) << 8,
          GetGValue(IGNITE_BOOT_BG_START) << 8,
          GetBValue(IGNITE_BOOT_BG_START) << 8, 0xFF00 }
    };
    
    GRADIENT_RECT gradRects[2] = {
        { 0, 1 },  // Top gradient
        { 2, 3 }   // Bottom gradient
    };
    
    GradientFill(hdc, vertices, 4, gradRects, 2, GRADIENT_FILL_RECT_V);
}

void DrawWindowsLogo(HDC hdc, int x, int y, int size, FLOAT alpha) {
    if (alpha <= 0.0f) return;
    
    // Create logo brush with alpha
    BYTE alphaValue = (BYTE)(255 * alpha);
    HBRUSH hBrush = CreateSolidBrush(IGNITE_BOOT_ACCENT);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    // Draw Windows logo squares (simplified)
    int squareSize = size / 3;
    int gap = size / 12;
    
    // Top left square
    Rectangle(hdc, x, y, x + squareSize, y + squareSize);
    
    // Top right square
    Rectangle(hdc, x + squareSize + gap, y, 
              x + squareSize + gap + squareSize, y + squareSize);
    
    // Bottom left square
    Rectangle(hdc, x, y + squareSize + gap,
              x + squareSize, y + squareSize + gap + squareSize);
    
    // Bottom right square
    Rectangle(hdc, x + squareSize + gap, y + squareSize + gap,
              x + squareSize + gap + squareSize, 
              y + squareSize + gap + squareSize);
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

void DrawModernProgressBar(HDC hdc, int x, int y, int width, int height, FLOAT progress, FLOAT alpha) {
    if (alpha <= 0.0f) return;
    
    // Background
    HBRUSH hBgBrush = CreateSolidBrush(RGB(64, 64, 64));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBgBrush);
    
    HRGN hRgn = CreateRoundRectRgn(x, y, x + width, y + height, height, height);
    FillRgn(hdc, hRgn, hBgBrush);
    DeleteObject(hRgn);
    
    // Progress fill
    if (progress > 0.0f) {
        int fillWidth = (int)(width * progress);
        HBRUSH hFillBrush = CreateSolidBrush(IGNITE_BOOT_PROGRESS);
        
        hRgn = CreateRoundRectRgn(x, y, x + fillWidth, y + height, height, height);
        FillRgn(hdc, hRgn, hFillBrush);
        DeleteObject(hRgn);
        DeleteObject(hFillBrush);
    }
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBgBrush);
}

void DrawBootText(HDC hdc, LPCWSTR text, int x, int y, FLOAT alpha) {
    if (alpha <= 0.0f) return;
    
    SetTextColor(hdc, IGNITE_BOOT_TEXT);
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_BootContext.hFont);
    
    // Calculate text position for centering
    SIZE textSize;
    GetTextExtentPoint32(hdc, text, wcslen(text), &textSize);
    
    int textX = x - textSize.cx / 2;
    int textY = y - textSize.cy / 2;
    
    TextOut(hdc, textX, textY, text, wcslen(text));
    
    SelectObject(hdc, hOldFont);
}

void UpdateBootAnimation() {
    g_BootContext.dwCurrentTime = GetTickCount();
    DWORD elapsed = g_BootContext.dwCurrentTime - g_BootContext.dwStartTime;
    
    // State machine for boot animation
    switch (g_BootContext.state) {
        case BOOT_STATE_INIT:
            if (elapsed > 500) {  // Wait 500ms
                g_BootContext.state = BOOT_STATE_LOGO_FADE_IN;
                g_BootContext.dwStartTime = g_BootContext.dwCurrentTime;
            }
            break;
            
        case BOOT_STATE_LOGO_FADE_IN:
            g_BootContext.fLogoAlpha = min(1.0f, elapsed / 1000.0f);  // 1 second fade
            if (elapsed > 1000) {
                g_BootContext.state = BOOT_STATE_PROGRESS_SHOW;
                g_BootContext.dwStartTime = g_BootContext.dwCurrentTime;
            }
            break;
            
        case BOOT_STATE_PROGRESS_SHOW:
            g_BootContext.fProgressAlpha = min(1.0f, elapsed / 500.0f);  // 0.5 second fade
            if (elapsed > 500) {
                g_BootContext.state = BOOT_STATE_LOADING;
                g_BootContext.dwStartTime = g_BootContext.dwCurrentTime;
            }
            break;
            
        case BOOT_STATE_LOADING:
            // Smooth progress animation
            g_BootContext.fProgress = min(1.0f, elapsed / 3000.0f);  // 3 second loading
            if (elapsed > 3000) {
                g_BootContext.state = BOOT_STATE_FADE_OUT;
                g_BootContext.dwStartTime = g_BootContext.dwCurrentTime;
            }
            break;
            
        case BOOT_STATE_FADE_OUT:
            {
                FLOAT fadeProgress = elapsed / 500.0f;  // 0.5 second fade out
                g_BootContext.fLogoAlpha = max(0.0f, 1.0f - fadeProgress);
                g_BootContext.fProgressAlpha = max(0.0f, 1.0f - fadeProgress);
                
                if (elapsed > 500) {
                    g_BootContext.state = BOOT_STATE_COMPLETE;
                }
            }
            break;
            
        case BOOT_STATE_COMPLETE:
            // Animation complete
            break;
    }
}

void RenderBootAnimation() {
    // Clear buffer with gradient background
    DrawModernGradientBackground(g_BootContext.hdcBuffer, &g_BootContext.rcScreen);
    
    // Calculate center positions
    int centerX = g_BootContext.rcScreen.right / 2;
    int centerY = g_BootContext.rcScreen.bottom / 2;
    
    // Draw Windows logo
    DrawWindowsLogo(g_BootContext.hdcBuffer, 
                   centerX - BOOT_LOGO_SIZE / 2, 
                   centerY - BOOT_LOGO_SIZE / 2 - 50,
                   BOOT_LOGO_SIZE, 
                   g_BootContext.fLogoAlpha);
    
    // Draw "Windows 12 Ignite" text
    DrawBootText(g_BootContext.hdcBuffer, L"Windows 12 Ignite", 
                centerX, centerY + 50, g_BootContext.fLogoAlpha);
    
    // Draw progress bar
    DrawModernProgressBar(g_BootContext.hdcBuffer,
                         centerX - BOOT_PROGRESS_WIDTH / 2,
                         centerY + 100,
                         BOOT_PROGRESS_WIDTH,
                         BOOT_PROGRESS_HEIGHT,
                         g_BootContext.fProgress,
                         g_BootContext.fProgressAlpha);
    
    // Draw loading text
    if (g_BootContext.state == BOOT_STATE_LOADING) {
        WCHAR loadingText[64];
        swprintf(loadingText, L"Loading... %d%%", (int)(g_BootContext.fProgress * 100));
        DrawBootText(g_BootContext.hdcBuffer, loadingText,
                    centerX, centerY + 140, g_BootContext.fProgressAlpha);
    }
    
    // Copy buffer to screen
    BitBlt(g_BootContext.hdcScreen, 0, 0,
           g_BootContext.rcScreen.right, g_BootContext.rcScreen.bottom,
           g_BootContext.hdcBuffer, 0, 0, SRCCOPY);
}

BOOL IsBootAnimationComplete() {
    return g_BootContext.state == BOOT_STATE_COMPLETE;
}

// Main boot animation loop
void RunBootAnimation() {
    if (!InitBootAnimation()) {
        return;
    }
    
    // Animation loop
    while (!IsBootAnimationComplete()) {
        UpdateBootAnimation();
        RenderBootAnimation();
        
        // Maintain 60 FPS
        Sleep(1000 / BOOT_ANIM_FPS);
    }
    
    CleanupBootAnimation();
}

// Export functions
extern "C" {
    NTSTATUS BootAnimationEntry() {
        RunBootAnimation();
        return STATUS_SUCCESS;
    }
    
    void StartIgniteBootAnimation() {
        RunBootAnimation();
    }
}

