#include "theme.h"
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <math.h>

// Windows 12 Ignite Renderer Implementation
// DirectX 11 and Direct2D powered rendering engine

static IGNITE_RENDER_CONTEXT g_RenderContext = {0};
static BOOL g_bInitialized = FALSE;

// Initialize the Ignite rendering system
BOOL WINAPI IgniteTheme_Initialize(PIGNITE_RENDER_CONTEXT pContext)
{
    HRESULT hr = S_OK;
    
    if (g_bInitialized) {
        if (pContext) {
            *pContext = g_RenderContext;
        }
        return TRUE;
    }

    ZeroMemory(&g_RenderContext, sizeof(g_RenderContext));

    // Create D3D11 Device
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    hr = D3D11CreateDevice(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &g_RenderContext.pDevice,
        NULL,
        &g_RenderContext.pContext
    );

    if (FAILED(hr)) {
        return FALSE;
    }

    // Create D2D Factory
    D2D1_FACTORY_OPTIONS options = {0};
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        __uuidof(ID2D1Factory),
        &options,
        (void**)&g_RenderContext.pD2DFactory
    );

    if (FAILED(hr)) {
        IgniteTheme_Shutdown(&g_RenderContext);
        return FALSE;
    }

    // Create DirectWrite Factory
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        (IUnknown**)&g_RenderContext.pWriteFactory
    );

    if (FAILED(hr)) {
        IgniteTheme_Shutdown(&g_RenderContext);
        return FALSE;
    }

    // Create WIC Factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory),
        (void**)&g_RenderContext.pWICFactory
    );

    if (FAILED(hr)) {
        IgniteTheme_Shutdown(&g_RenderContext);
        return FALSE;
    }

    // Load default configuration
    g_RenderContext.Config.bEnabled = TRUE;
    g_RenderContext.Config.bAcrylicEnabled = TRUE;
    g_RenderContext.Config.bAnimationsEnabled = TRUE;
    g_RenderContext.Config.bBlurEnabled = TRUE;
    g_RenderContext.Config.fCornerRadius = IGNITE_CORNER_RADIUS;
    g_RenderContext.Config.fBlurRadius = IGNITE_BLUR_RADIUS;
    g_RenderContext.Config.fAcrylicOpacity = IGNITE_ACRYLIC_OPACITY;
    g_RenderContext.Config.dwAccentColor = IGNITE_COLOR_ACCENT;
    g_RenderContext.Config.dwAnimationDuration = IGNITE_ANIMATION_DURATION;

    g_bInitialized = TRUE;

    if (pContext) {
        *pContext = g_RenderContext;
    }

    return TRUE;
}

// Shutdown the rendering system
VOID WINAPI IgniteTheme_Shutdown(PIGNITE_RENDER_CONTEXT pContext)
{
    if (g_RenderContext.pWICFactory) {
        g_RenderContext.pWICFactory->Release();
        g_RenderContext.pWICFactory = NULL;
    }

    if (g_RenderContext.pWriteFactory) {
        g_RenderContext.pWriteFactory->Release();
        g_RenderContext.pWriteFactory = NULL;
    }

    if (g_RenderContext.pRenderTarget) {
        g_RenderContext.pRenderTarget->Release();
        g_RenderContext.pRenderTarget = NULL;
    }

    if (g_RenderContext.pD2DFactory) {
        g_RenderContext.pD2DFactory->Release();
        g_RenderContext.pD2DFactory = NULL;
    }

    if (g_RenderContext.pContext) {
        g_RenderContext.pContext->Release();
        g_RenderContext.pContext = NULL;
    }

    if (g_RenderContext.pDevice) {
        g_RenderContext.pDevice->Release();
        g_RenderContext.pDevice = NULL;
    }

    ZeroMemory(&g_RenderContext, sizeof(g_RenderContext));
    g_bInitialized = FALSE;
}

// Create an acrylic brush with blur and transparency
HRESULT WINAPI IgniteRender_CreateAcrylicBrush(PIGNITE_RENDER_CONTEXT pContext, 
                                               DWORD dwColor, 
                                               FLOAT fOpacity,
                                               ID2D1Brush** ppBrush)
{
    if (!pContext || !ppBrush) {
        return E_INVALIDARG;
    }

    if (!pContext->pRenderTarget) {
        return IGNITE_E_NOT_INITIALIZED;
    }

    // Extract color components
    FLOAT r = ((dwColor >> 16) & 0xFF) / 255.0f;
    FLOAT g = ((dwColor >> 8) & 0xFF) / 255.0f;
    FLOAT b = (dwColor & 0xFF) / 255.0f;
    FLOAT a = fOpacity;

    D2D1_COLOR_F color = D2D1::ColorF(r, g, b, a);

    // Create solid color brush for now (acrylic effect would need more complex implementation)
    ID2D1SolidColorBrush* pSolidBrush = NULL;
    HRESULT hr = pContext->pRenderTarget->CreateSolidColorBrush(color, &pSolidBrush);
    
    if (SUCCEEDED(hr)) {
        *ppBrush = pSolidBrush;
    }

    return hr;
}

// Draw a rounded rectangle
HRESULT WINAPI IgniteRender_DrawRoundedRect(PIGNITE_RENDER_CONTEXT pContext,
                                            const D2D1_RECT_F* pRect,
                                            FLOAT fRadius,
                                            ID2D1Brush* pBrush)
{
    if (!pContext || !pRect || !pBrush) {
        return E_INVALIDARG;
    }

    if (!pContext->pRenderTarget) {
        return IGNITE_E_NOT_INITIALIZED;
    }

    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(*pRect, fRadius, fRadius);
    pContext->pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);

    return S_OK;
}

// Apply blur effect to a bitmap
HRESULT WINAPI IgniteRender_ApplyBlurEffect(PIGNITE_RENDER_CONTEXT pContext,
                                            ID2D1Bitmap* pSource,
                                            FLOAT fRadius,
                                            ID2D1Bitmap** ppResult)
{
    if (!pContext || !pSource || !ppResult) {
        return E_INVALIDARG;
    }

    // For now, return the source bitmap (full blur implementation would require effects)
    pSource->AddRef();
    *ppResult = pSource;

    return S_OK;
}

// Window management functions
BOOL WINAPI IgniteWindow_EnableAcrylic(HWND hWnd, BOOL bEnable)
{
    if (!IsWindow(hWnd)) {
        return FALSE;
    }

    // Set window property to track acrylic state
    SetProp(hWnd, L"IgniteAcrylic", (HANDLE)(bEnable ? 1 : 0));

    // Force window redraw
    InvalidateRect(hWnd, NULL, TRUE);
    
    return TRUE;
}

BOOL WINAPI IgniteWindow_SetRoundedCorners(HWND hWnd, FLOAT fRadius)
{
    if (!IsWindow(hWnd)) {
        return FALSE;
    }

    // Store radius as window property
    DWORD dwRadius = (DWORD)(fRadius * 100); // Store as integer
    SetProp(hWnd, L"IgniteCornerRadius", (HANDLE)dwRadius);

    // Force window redraw
    InvalidateRect(hWnd, NULL, TRUE);
    
    return TRUE;
}

BOOL WINAPI IgniteWindow_EnableDropShadow(HWND hWnd, BOOL bEnable)
{
    if (!IsWindow(hWnd)) {
        return FALSE;
    }

    SetProp(hWnd, L"IgniteDropShadow", (HANDLE)(bEnable ? 1 : 0));
    InvalidateRect(hWnd, NULL, TRUE);
    
    return TRUE;
}

// Animation system
static DWORD g_dwAnimationTimer = 0;
static HWND g_hAnimationWindows[32] = {0};
static IGNITE_ANIMATION_TYPE g_AnimationTypes[32] = {0};
static DWORD g_AnimationDurations[32] = {0};
static LARGE_INTEGER g_AnimationStarts[32] = {0};
static int g_nAnimationCount = 0;

VOID CALLBACK AnimationTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    LARGE_INTEGER liNow;
    QueryPerformanceCounter(&liNow);

    for (int i = 0; i < g_nAnimationCount; i++) {
        if (g_hAnimationWindows[i] && IsWindow(g_hAnimationWindows[i])) {
            LARGE_INTEGER liFreq;
            QueryPerformanceFrequency(&liFreq);
            
            DWORD dwElapsed = (DWORD)(((liNow.QuadPart - g_AnimationStarts[i].QuadPart) * 1000) / liFreq.QuadPart);
            
            if (dwElapsed >= g_AnimationDurations[i]) {
                // Animation complete
                PostMessage(g_hAnimationWindows[i], WM_IGNITE_ANIMATION_COMPLETE, 0, 0);
                
                // Remove from list
                for (int j = i; j < g_nAnimationCount - 1; j++) {
                    g_hAnimationWindows[j] = g_hAnimationWindows[j + 1];
                    g_AnimationTypes[j] = g_AnimationTypes[j + 1];
                    g_AnimationDurations[j] = g_AnimationDurations[j + 1];
                    g_AnimationStarts[j] = g_AnimationStarts[j + 1];
                }
                g_nAnimationCount--;
                i--;
            } else {
                // Update animation
                InvalidateRect(g_hAnimationWindows[i], NULL, TRUE);
            }
        }
    }

    if (g_nAnimationCount == 0 && g_dwAnimationTimer) {
        KillTimer(NULL, g_dwAnimationTimer);
        g_dwAnimationTimer = 0;
    }
}

BOOL WINAPI IgniteAnim_Start(HWND hWnd, IGNITE_ANIMATION_TYPE type, DWORD dwDuration)
{
    if (!IsWindow(hWnd) || g_nAnimationCount >= 32) {
        return FALSE;
    }

    // Add to animation list
    g_hAnimationWindows[g_nAnimationCount] = hWnd;
    g_AnimationTypes[g_nAnimationCount] = type;
    g_AnimationDurations[g_nAnimationCount] = dwDuration;
    QueryPerformanceCounter(&g_AnimationStarts[g_nAnimationCount]);
    g_nAnimationCount++;

    // Start timer if not already running
    if (!g_dwAnimationTimer) {
        g_dwAnimationTimer = SetTimer(NULL, 0, 16, AnimationTimerProc); // 60 FPS
    }

    return TRUE;
}

BOOL WINAPI IgniteAnim_Stop(HWND hWnd)
{
    for (int i = 0; i < g_nAnimationCount; i++) {
        if (g_hAnimationWindows[i] == hWnd) {
            // Remove from list
            for (int j = i; j < g_nAnimationCount - 1; j++) {
                g_hAnimationWindows[j] = g_hAnimationWindows[j + 1];
                g_AnimationTypes[j] = g_AnimationTypes[j + 1];
                g_AnimationDurations[j] = g_AnimationDurations[j + 1];
                g_AnimationStarts[j] = g_AnimationStarts[j + 1];
            }
            g_nAnimationCount--;
            return TRUE;
        }
    }
    return FALSE;
}

FLOAT WINAPI IgniteAnim_GetProgress(HWND hWnd)
{
    for (int i = 0; i < g_nAnimationCount; i++) {
        if (g_hAnimationWindows[i] == hWnd) {
            LARGE_INTEGER liNow, liFreq;
            QueryPerformanceCounter(&liNow);
            QueryPerformanceFrequency(&liFreq);
            
            DWORD dwElapsed = (DWORD)(((liNow.QuadPart - g_AnimationStarts[i].QuadPart) * 1000) / liFreq.QuadPart);
            return min(1.0f, (FLOAT)dwElapsed / (FLOAT)g_AnimationDurations[i]);
        }
    }
    return 0.0f;
}

BOOL WINAPI IgniteAnim_IsRunning(HWND hWnd)
{
    for (int i = 0; i < g_nAnimationCount; i++) {
        if (g_hAnimationWindows[i] == hWnd) {
            return TRUE;
        }
    }
    return FALSE;
}

// Utility functions
FLOAT WINAPI IgniteUtil_EaseInOut(FLOAT t)
{
    return t * t * (3.0f - 2.0f * t);
}

FLOAT WINAPI IgniteUtil_EaseIn(FLOAT t)
{
    return t * t;
}

FLOAT WINAPI IgniteUtil_EaseOut(FLOAT t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

DWORD WINAPI IgniteUtil_BlendColors(DWORD dwColor1, DWORD dwColor2, FLOAT fBlend)
{
    BYTE r1 = (dwColor1 >> 16) & 0xFF;
    BYTE g1 = (dwColor1 >> 8) & 0xFF;
    BYTE b1 = dwColor1 & 0xFF;
    BYTE a1 = (dwColor1 >> 24) & 0xFF;

    BYTE r2 = (dwColor2 >> 16) & 0xFF;
    BYTE g2 = (dwColor2 >> 8) & 0xFF;
    BYTE b2 = dwColor2 & 0xFF;
    BYTE a2 = (dwColor2 >> 24) & 0xFF;

    BYTE r = (BYTE)(r1 + (r2 - r1) * fBlend);
    BYTE g = (BYTE)(g1 + (g2 - g1) * fBlend);
    BYTE b = (BYTE)(b1 + (b2 - b1) * fBlend);
    BYTE a = (BYTE)(a1 + (a2 - a1) * fBlend);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

BOOL WINAPI IgniteUtil_IsCompositionEnabled(VOID)
{
    // For XP, we'll always return TRUE since we're implementing our own composition
    return TRUE;
}
