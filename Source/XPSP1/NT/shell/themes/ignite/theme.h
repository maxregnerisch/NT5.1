#pragma once

#ifndef _IGNITE_THEME_H_
#define _IGNITE_THEME_H_

#include <windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

// Windows 12 Ignite Theme Engine
// Modern UI framework for Windows XP with DirectX 11 acceleration

#ifdef __cplusplus
extern "C" {
#endif

// Theme Constants
#define IGNITE_VERSION_MAJOR    1
#define IGNITE_VERSION_MINOR    0
#define IGNITE_VERSION_BUILD    0

// Visual Constants
#define IGNITE_CORNER_RADIUS    12.0f
#define IGNITE_BLUR_RADIUS      20.0f
#define IGNITE_ANIMATION_DURATION 250
#define IGNITE_ACRYLIC_OPACITY  0.85f

// Color Definitions (Windows 12 Ignite Palette)
#define IGNITE_COLOR_ACCENT     0xFF0078D4  // Windows Blue
#define IGNITE_COLOR_ACCENT_LIGHT 0xFF40E0FF  // Light Blue
#define IGNITE_COLOR_SURFACE    0xF0F8F8F8  // Light Surface
#define IGNITE_COLOR_SURFACE_DARK 0xF0202020  // Dark Surface
#define IGNITE_COLOR_TEXT       0xFF000000  // Text
#define IGNITE_COLOR_TEXT_SECONDARY 0xFF666666  // Secondary Text

// Theme Structures
typedef struct _IGNITE_THEME_CONFIG {
    BOOL bEnabled;
    BOOL bAcrylicEnabled;
    BOOL bAnimationsEnabled;
    BOOL bBlurEnabled;
    FLOAT fCornerRadius;
    FLOAT fBlurRadius;
    FLOAT fAcrylicOpacity;
    DWORD dwAccentColor;
    DWORD dwAnimationDuration;
} IGNITE_THEME_CONFIG, *PIGNITE_THEME_CONFIG;

typedef struct _IGNITE_RENDER_CONTEXT {
    ID3D11Device* pDevice;
    ID3D11DeviceContext* pContext;
    ID2D1Factory* pD2DFactory;
    ID2D1RenderTarget* pRenderTarget;
    IDWriteFactory* pWriteFactory;
    IWICImagingFactory* pWICFactory;
    IGNITE_THEME_CONFIG Config;
} IGNITE_RENDER_CONTEXT, *PIGNITE_RENDER_CONTEXT;

typedef struct _IGNITE_WINDOW_DATA {
    HWND hWnd;
    BOOL bHasAcrylic;
    BOOL bHasRoundedCorners;
    BOOL bHasDropShadow;
    FLOAT fBlurAmount;
    DWORD dwAnimationState;
    LARGE_INTEGER llAnimationStart;
} IGNITE_WINDOW_DATA, *PIGNITE_WINDOW_DATA;

// Core Theme Functions
BOOL WINAPI IgniteTheme_Initialize(PIGNITE_RENDER_CONTEXT pContext);
VOID WINAPI IgniteTheme_Shutdown(PIGNITE_RENDER_CONTEXT pContext);
BOOL WINAPI IgniteTheme_LoadConfig(PIGNITE_THEME_CONFIG pConfig);
BOOL WINAPI IgniteTheme_SaveConfig(PIGNITE_THEME_CONFIG pConfig);

// Rendering Functions
HRESULT WINAPI IgniteRender_CreateAcrylicBrush(PIGNITE_RENDER_CONTEXT pContext, 
                                               DWORD dwColor, 
                                               FLOAT fOpacity,
                                               ID2D1Brush** ppBrush);
HRESULT WINAPI IgniteRender_DrawRoundedRect(PIGNITE_RENDER_CONTEXT pContext,
                                            const D2D1_RECT_F* pRect,
                                            FLOAT fRadius,
                                            ID2D1Brush* pBrush);
HRESULT WINAPI IgniteRender_ApplyBlurEffect(PIGNITE_RENDER_CONTEXT pContext,
                                            ID2D1Bitmap* pSource,
                                            FLOAT fRadius,
                                            ID2D1Bitmap** ppResult);

// Window Management
BOOL WINAPI IgniteWindow_EnableAcrylic(HWND hWnd, BOOL bEnable);
BOOL WINAPI IgniteWindow_SetRoundedCorners(HWND hWnd, FLOAT fRadius);
BOOL WINAPI IgniteWindow_EnableDropShadow(HWND hWnd, BOOL bEnable);
BOOL WINAPI IgniteWindow_StartAnimation(HWND hWnd, DWORD dwType, DWORD dwDuration);

// Animation System
typedef enum _IGNITE_ANIMATION_TYPE {
    IGNITE_ANIM_FADE_IN = 1,
    IGNITE_ANIM_FADE_OUT,
    IGNITE_ANIM_SLIDE_IN_LEFT,
    IGNITE_ANIM_SLIDE_IN_RIGHT,
    IGNITE_ANIM_SLIDE_IN_UP,
    IGNITE_ANIM_SLIDE_IN_DOWN,
    IGNITE_ANIM_SCALE_IN,
    IGNITE_ANIM_SCALE_OUT,
    IGNITE_ANIM_BOUNCE
} IGNITE_ANIMATION_TYPE;

BOOL WINAPI IgniteAnim_Start(HWND hWnd, IGNITE_ANIMATION_TYPE type, DWORD dwDuration);
BOOL WINAPI IgniteAnim_Stop(HWND hWnd);
FLOAT WINAPI IgniteAnim_GetProgress(HWND hWnd);
BOOL WINAPI IgniteAnim_IsRunning(HWND hWnd);

// Modern Controls
BOOL WINAPI IgniteControls_EnableModernStyle(HWND hWnd);
BOOL WINAPI IgniteControls_SetAccentColor(HWND hWnd, DWORD dwColor);
BOOL WINAPI IgniteControls_EnableHoverEffects(HWND hWnd, BOOL bEnable);

// Taskbar Integration
BOOL WINAPI IgniteTaskbar_EnableModernStyle(BOOL bEnable);
BOOL WINAPI IgniteTaskbar_SetCenterAlignment(BOOL bCenter);
BOOL WINAPI IgniteTaskbar_EnableTransparency(BOOL bEnable);
BOOL WINAPI IgniteTaskbar_SetBlurAmount(FLOAT fAmount);

// Start Menu Integration
BOOL WINAPI IgniteStartMenu_EnableModernStyle(BOOL bEnable);
BOOL WINAPI IgniteStartMenu_SetFullScreen(BOOL bFullScreen);
BOOL WINAPI IgniteStartMenu_EnableSearch(BOOL bEnable);

// Desktop Integration
BOOL WINAPI IgniteDesktop_EnableModernWallpaper(BOOL bEnable);
BOOL WINAPI IgniteDesktop_SetGradientBackground(DWORD dwColor1, DWORD dwColor2);
BOOL WINAPI IgniteDesktop_EnableIconEffects(BOOL bEnable);

// Snap Layouts
typedef enum _IGNITE_SNAP_LAYOUT {
    IGNITE_SNAP_NONE = 0,
    IGNITE_SNAP_LEFT_HALF,
    IGNITE_SNAP_RIGHT_HALF,
    IGNITE_SNAP_TOP_HALF,
    IGNITE_SNAP_BOTTOM_HALF,
    IGNITE_SNAP_TOP_LEFT_QUARTER,
    IGNITE_SNAP_TOP_RIGHT_QUARTER,
    IGNITE_SNAP_BOTTOM_LEFT_QUARTER,
    IGNITE_SNAP_BOTTOM_RIGHT_QUARTER,
    IGNITE_SNAP_CENTER_THIRD,
    IGNITE_SNAP_LEFT_THIRD,
    IGNITE_SNAP_RIGHT_THIRD
} IGNITE_SNAP_LAYOUT;

BOOL WINAPI IgniteSnap_EnableSnapLayouts(BOOL bEnable);
BOOL WINAPI IgniteSnap_SnapWindow(HWND hWnd, IGNITE_SNAP_LAYOUT layout);
BOOL WINAPI IgniteSnap_ShowSnapPreview(HWND hWnd, POINT pt);
BOOL WINAPI IgniteSnap_HideSnapPreview(VOID);

// Virtual Desktops
BOOL WINAPI IgniteVirtualDesktop_Create(LPCWSTR lpszName, DWORD* pdwDesktopId);
BOOL WINAPI IgniteVirtualDesktop_Switch(DWORD dwDesktopId);
BOOL WINAPI IgniteVirtualDesktop_MoveWindow(HWND hWnd, DWORD dwDesktopId);
DWORD WINAPI IgniteVirtualDesktop_GetCurrent(VOID);
BOOL WINAPI IgniteVirtualDesktop_GetList(DWORD* pdwDesktops, DWORD* pdwCount);

// Notification System
typedef struct _IGNITE_NOTIFICATION {
    WCHAR szTitle[256];
    WCHAR szMessage[1024];
    WCHAR szIcon[MAX_PATH];
    DWORD dwDuration;
    DWORD dwType;
    HWND hWndCallback;
    UINT uCallbackMsg;
} IGNITE_NOTIFICATION, *PIGNITE_NOTIFICATION;

BOOL WINAPI IgniteNotify_Show(PIGNITE_NOTIFICATION pNotification);
BOOL WINAPI IgniteNotify_Hide(DWORD dwNotificationId);
BOOL WINAPI IgniteNotify_Clear(VOID);

// Utility Functions
FLOAT WINAPI IgniteUtil_EaseInOut(FLOAT t);
FLOAT WINAPI IgniteUtil_EaseIn(FLOAT t);
FLOAT WINAPI IgniteUtil_EaseOut(FLOAT t);
DWORD WINAPI IgniteUtil_BlendColors(DWORD dwColor1, DWORD dwColor2, FLOAT fBlend);
BOOL WINAPI IgniteUtil_IsCompositionEnabled(VOID);

// Message Definitions
#define WM_IGNITE_THEME_CHANGED    (WM_USER + 0x1000)
#define WM_IGNITE_ANIMATION_COMPLETE (WM_USER + 0x1001)
#define WM_IGNITE_ACRYLIC_UPDATE   (WM_USER + 0x1002)

// Error Codes
#define IGNITE_E_NOT_INITIALIZED   0x80070001
#define IGNITE_E_DEVICE_LOST       0x80070002
#define IGNITE_E_INVALID_PARAMETER 0x80070003
#define IGNITE_E_OUT_OF_MEMORY     0x80070004

#ifdef __cplusplus
}
#endif

#endif // _IGNITE_THEME_H_
