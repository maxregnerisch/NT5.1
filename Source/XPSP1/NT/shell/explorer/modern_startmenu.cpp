#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include "../themes/ignite/theme.h"

// Windows 12 Ignite Modern Start Menu Implementation
// Full-screen modern design with blur effects and search

#define STARTMENU_WIDTH 600
#define STARTMENU_HEIGHT 700
#define STARTMENU_PADDING 20
#define TILE_SIZE 80
#define TILE_SPACING 10

// Start menu window class
static const WCHAR* MODERN_STARTMENU_CLASS = L"ModernIgniteStartMenu";
static HWND g_hStartMenuWnd = NULL;
static HWND g_hSearchBox = NULL;
static HWND g_hAppList = NULL;
static HWND g_hPowerButton = NULL;
static BOOL g_bFullScreen = FALSE;
static BOOL g_bVisible = FALSE;

// App tile structure
typedef struct _APP_TILE {
    WCHAR szName[256];
    WCHAR szPath[MAX_PATH];
    HICON hIcon;
    RECT rcTile;
    BOOL bHover;
    FLOAT fAnimationProgress;
    DWORD dwAnimationStart;
} APP_TILE, *PAPP_TILE;

static APP_TILE g_AppTiles[64];
static int g_nTileCount = 0;

// Forward declarations
LRESULT CALLBACK ModernStartMenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SearchBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawModernStartMenu(HDC hdc, RECT* pRect);
void DrawAppTile(HDC hdc, PAPP_TILE pTile);
void LoadApplications();
void UpdateTileLayout();
void AnimateStartMenu(BOOL bShow);

// Initialize modern start menu
BOOL WINAPI IgniteStartMenu_EnableModernStyle(BOOL bEnable)
{
    if (bEnable && !g_hStartMenuWnd) {
        // Register start menu window class
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
        wc.lpfnWndProc = ModernStartMenuWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL; // We'll handle painting
        wc.lpszClassName = MODERN_STARTMENU_CLASS;
        
        if (!RegisterClassEx(&wc)) {
            return FALSE;
        }

        // Get screen dimensions
        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

        int menuWidth = g_bFullScreen ? nScreenWidth : STARTMENU_WIDTH;
        int menuHeight = g_bFullScreen ? nScreenHeight : STARTMENU_HEIGHT;
        int menuX = g_bFullScreen ? 0 : (nScreenWidth - menuWidth) / 2;
        int menuY = g_bFullScreen ? 0 : (nScreenHeight - menuHeight) / 2;

        // Create start menu window
        g_hStartMenuWnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            MODERN_STARTMENU_CLASS,
            L"Modern Start Menu",
            WS_POPUP,
            menuX, menuY,
            menuWidth, menuHeight,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        if (!g_hStartMenuWnd) {
            return FALSE;
        }

        // Enable acrylic effect and rounded corners
        IgniteWindow_EnableAcrylic(g_hStartMenuWnd, TRUE);
        if (!g_bFullScreen) {
            IgniteWindow_SetRoundedCorners(g_hStartMenuWnd, 12.0f);
        }
        IgniteWindow_EnableDropShadow(g_hStartMenuWnd, TRUE);

        // Create search box
        g_hSearchBox = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", L"Search apps and files...",
            WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
            STARTMENU_PADDING, STARTMENU_PADDING,
            menuWidth - (STARTMENU_PADDING * 2), 40,
            g_hStartMenuWnd, (HMENU)2001, GetModuleHandle(NULL), NULL
        );

        // Subclass search box for modern styling
        SetWindowSubclass(g_hSearchBox, SearchBoxProc, 0, 0);

        // Create app list area
        g_hAppList = CreateWindowEx(
            0, L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE,
            STARTMENU_PADDING, 80,
            menuWidth - (STARTMENU_PADDING * 2), menuHeight - 140,
            g_hStartMenuWnd, (HMENU)2002, GetModuleHandle(NULL), NULL
        );

        // Create power button
        g_hPowerButton = CreateWindowEx(
            0, L"BUTTON", L"⏻",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            menuWidth - 60, menuHeight - 60,
            40, 40,
            g_hStartMenuWnd, (HMENU)2003, GetModuleHandle(NULL), NULL
        );

        // Load applications
        LoadApplications();
        UpdateTileLayout();

        // Show with animation
        AnimateStartMenu(TRUE);
        g_bVisible = TRUE;

        return TRUE;
    }
    else if (!bEnable && g_hStartMenuWnd) {
        // Hide with animation
        AnimateStartMenu(FALSE);
        
        // Destroy after animation
        SetTimer(g_hStartMenuWnd, 999, 300, NULL);
        g_bVisible = FALSE;
        
        return TRUE;
    }
    else if (bEnable && g_hStartMenuWnd) {
        // Toggle visibility
        if (g_bVisible) {
            IgniteStartMenu_EnableModernStyle(FALSE);
        } else {
            ShowWindow(g_hStartMenuWnd, SW_SHOW);
            AnimateStartMenu(TRUE);
            g_bVisible = TRUE;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL WINAPI IgniteStartMenu_SetFullScreen(BOOL bFullScreen)
{
    g_bFullScreen = bFullScreen;
    
    if (g_hStartMenuWnd) {
        // Recreate with new size
        IgniteStartMenu_EnableModernStyle(FALSE);
        Sleep(350); // Wait for animation
        IgniteStartMenu_EnableModernStyle(TRUE);
    }
    
    return TRUE;
}

BOOL WINAPI IgniteStartMenu_EnableSearch(BOOL bEnable)
{
    if (g_hSearchBox) {
        ShowWindow(g_hSearchBox, bEnable ? SW_SHOW : SW_HIDE);
        
        // Adjust layout
        if (g_hAppList) {
            RECT rcMenu;
            GetClientRect(g_hStartMenuWnd, &rcMenu);
            
            int listY = bEnable ? 80 : STARTMENU_PADDING;
            int listHeight = rcMenu.bottom - listY - 60;
            
            SetWindowPos(g_hAppList, NULL, 
                        STARTMENU_PADDING, listY,
                        rcMenu.right - (STARTMENU_PADDING * 2), listHeight,
                        SWP_NOZORDER);
        }
        
        UpdateTileLayout();
        InvalidateRect(g_hStartMenuWnd, NULL, TRUE);
    }
    
    return TRUE;
}

// Start menu window procedure
LRESULT CALLBACK ModernStartMenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            
            DrawModernStartMenu(hdc, &rcClient);
            
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // We handle background painting

        case WM_LBUTTONDOWN:
        {
            // Handle tile clicks
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            
            for (int i = 0; i < g_nTileCount; i++) {
                if (PtInRect(&g_AppTiles[i].rcTile, pt)) {
                    // Launch application
                    ShellExecute(NULL, L"open", g_AppTiles[i].szPath, NULL, NULL, SW_SHOWNORMAL);
                    
                    // Hide start menu
                    IgniteStartMenu_EnableModernStyle(FALSE);
                    break;
                }
            }
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            
            // Update hover states
            for (int i = 0; i < g_nTileCount; i++) {
                BOOL bWasHover = g_AppTiles[i].bHover;
                g_AppTiles[i].bHover = PtInRect(&g_AppTiles[i].rcTile, pt);
                
                if (bWasHover != g_AppTiles[i].bHover) {
                    InvalidateRect(hWnd, &g_AppTiles[i].rcTile, FALSE);
                }
            }
            return 0;
        }

        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE) {
                // Close start menu
                IgniteStartMenu_EnableModernStyle(FALSE);
            }
            return 0;
        }

        case WM_ACTIVATE:
        {
            if (LOWORD(wParam) == WA_INACTIVE) {
                // Close when losing focus
                IgniteStartMenu_EnableModernStyle(FALSE);
            }
            return 0;
        }

        case WM_TIMER:
        {
            if (wParam == 999) {
                // Destroy after animation
                KillTimer(hWnd, 999);
                DestroyWindow(hWnd);
                g_hStartMenuWnd = NULL;
                g_hSearchBox = NULL;
                g_hAppList = NULL;
                g_hPowerButton = NULL;
                UnregisterClass(MODERN_STARTMENU_CLASS, GetModuleHandle(NULL));
            }
            return 0;
        }

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == 2003) { // Power button
                // Show power options
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 3001, L"Sleep");
                AppendMenu(hMenu, MF_STRING, 3002, L"Restart");
                AppendMenu(hMenu, MF_STRING, 3003, L"Shut down");
                
                POINT pt;
                GetCursorPos(&pt);
                
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                       pt.x, pt.y, 0, hWnd, NULL);
                
                switch (cmd) {
                    case 3001: // Sleep
                        SetSystemPowerState(TRUE, FALSE);
                        break;
                    case 3002: // Restart
                        ExitWindowsEx(EWX_REBOOT, 0);
                        break;
                    case 3003: // Shutdown
                        ExitWindowsEx(EWX_SHUTDOWN, 0);
                        break;
                }
                
                DestroyMenu(hMenu);
                IgniteStartMenu_EnableModernStyle(FALSE);
            }
            return 0;
        }

        case WM_DESTROY:
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Search box procedure
LRESULT CALLBACK SearchBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_SETFOCUS:
        {
            // Clear placeholder text
            WCHAR szText[256];
            GetWindowText(hWnd, szText, 256);
            if (wcscmp(szText, L"Search apps and files...") == 0) {
                SetWindowText(hWnd, L"");
            }
            break;
        }

        case WM_KILLFOCUS:
        {
            // Restore placeholder if empty
            WCHAR szText[256];
            GetWindowText(hWnd, szText, 256);
            if (wcslen(szText) == 0) {
                SetWindowText(hWnd, L"Search apps and files...");
            }
            break;
        }

        case WM_CHAR:
        {
            // Handle search as user types
            if (wParam != VK_RETURN) {
                // Filter tiles based on search text
                // Implementation would go here
            }
            break;
        }
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Draw the modern start menu
void DrawModernStartMenu(HDC hdc, RECT* pRect)
{
    // Create memory DC for double buffering
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, pRect->right, pRect->bottom);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // Draw acrylic background
    HBRUSH hBrush = CreateSolidBrush(RGB(248, 248, 248));
    FillRect(hdcMem, pRect, hBrush);
    DeleteObject(hBrush);

    // Add subtle gradient overlay
    for (int y = 0; y < pRect->bottom; y++) {
        int alpha = 248 - (y * 20 / pRect->bottom);
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(alpha, alpha, alpha));
        HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);
        
        MoveToEx(hdcMem, 0, y, NULL);
        LineTo(hdcMem, pRect->right, y);
        
        SelectObject(hdcMem, hOldPen);
        DeleteObject(hPen);
    }

    // Draw app tiles
    for (int i = 0; i < g_nTileCount; i++) {
        DrawAppTile(hdcMem, &g_AppTiles[i]);
    }

    // Draw title
    SetTextColor(hdcMem, RGB(32, 32, 32));
    SetBkMode(hdcMem, TRANSPARENT);
    
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
    
    RECT rcTitle = {STARTMENU_PADDING, pRect->bottom - 100, pRect->right - 80, pRect->bottom - 60};
    DrawText(hdcMem, L"All apps", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdcMem, hOldFont);
    DeleteObject(hFont);

    // Copy to main DC
    BitBlt(hdc, 0, 0, pRect->right, pRect->bottom, hdcMem, 0, 0, SRCCOPY);

    // Cleanup
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
}

// Draw an app tile
void DrawAppTile(HDC hdc, PAPP_TILE pTile)
{
    RECT rc = pTile->rcTile;
    
    // Tile background
    COLORREF bgColor = RGB(255, 255, 255);
    if (pTile->bHover) {
        bgColor = RGB(240, 240, 240);
    }
    
    HBRUSH hBrush = CreateSolidBrush(bgColor);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);
    
    // Draw subtle border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // Draw icon
    if (pTile->hIcon) {
        int iconSize = 32;
        int iconX = rc.left + (rc.right - rc.left - iconSize) / 2;
        int iconY = rc.top + 10;
        DrawIconEx(hdc, iconX, iconY, pTile->hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
    }
    
    // Draw app name
    SetTextColor(hdc, RGB(32, 32, 32));
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    RECT rcText = {rc.left + 4, rc.top + 50, rc.right - 4, rc.bottom - 4};
    DrawText(hdc, pTile->szName, -1, &rcText, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// Load applications from Start Menu folders
void LoadApplications()
{
    g_nTileCount = 0;
    
    // Get common programs folder
    WCHAR szPath[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, szPath) == S_OK) {
        // Scan for .lnk files
        WCHAR szSearch[MAX_PATH];
        wcscpy_s(szSearch, MAX_PATH, szPath);
        wcscat_s(szSearch, MAX_PATH, L"\\*.lnk");
        
        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile(szSearch, &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (g_nTileCount >= 64) break;
                
                APP_TILE* pTile = &g_AppTiles[g_nTileCount];
                ZeroMemory(pTile, sizeof(APP_TILE));
                
                // Get display name (remove .lnk extension)
                wcscpy_s(pTile->szName, 256, findData.cFileName);
                WCHAR* pExt = wcsrchr(pTile->szName, L'.');
                if (pExt) *pExt = L'\0';
                
                // Full path
                wcscpy_s(pTile->szPath, MAX_PATH, szPath);
                wcscat_s(pTile->szPath, MAX_PATH, L"\\");
                wcscat_s(pTile->szPath, MAX_PATH, findData.cFileName);
                
                // Extract icon
                pTile->hIcon = ExtractIcon(GetModuleHandle(NULL), pTile->szPath, 0);
                if (!pTile->hIcon) {
                    pTile->hIcon = LoadIcon(NULL, IDI_APPLICATION);
                }
                
                g_nTileCount++;
                
            } while (FindNextFile(hFind, &findData));
            
            FindClose(hFind);
        }
    }
    
    // Add some default system apps if none found
    if (g_nTileCount == 0) {
        // Calculator
        APP_TILE* pTile = &g_AppTiles[g_nTileCount++];
        wcscpy_s(pTile->szName, 256, L"Calculator");
        wcscpy_s(pTile->szPath, MAX_PATH, L"calc.exe");
        pTile->hIcon = LoadIcon(NULL, IDI_APPLICATION);
        
        // Notepad
        pTile = &g_AppTiles[g_nTileCount++];
        wcscpy_s(pTile->szName, 256, L"Notepad");
        wcscpy_s(pTile->szPath, MAX_PATH, L"notepad.exe");
        pTile->hIcon = LoadIcon(NULL, IDI_APPLICATION);
        
        // Paint
        pTile = &g_AppTiles[g_nTileCount++];
        wcscpy_s(pTile->szName, 256, L"Paint");
        wcscpy_s(pTile->szPath, MAX_PATH, L"mspaint.exe");
        pTile->hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
}

// Update tile layout
void UpdateTileLayout()
{
    if (!g_hStartMenuWnd) return;
    
    RECT rcMenu;
    GetClientRect(g_hStartMenuWnd, &rcMenu);
    
    int tilesPerRow = (rcMenu.right - (STARTMENU_PADDING * 2)) / (TILE_SIZE + TILE_SPACING);
    int startX = STARTMENU_PADDING;
    int startY = 100; // Below search box
    
    for (int i = 0; i < g_nTileCount; i++) {
        int row = i / tilesPerRow;
        int col = i % tilesPerRow;
        
        g_AppTiles[i].rcTile.left = startX + (col * (TILE_SIZE + TILE_SPACING));
        g_AppTiles[i].rcTile.top = startY + (row * (TILE_SIZE + TILE_SPACING));
        g_AppTiles[i].rcTile.right = g_AppTiles[i].rcTile.left + TILE_SIZE;
        g_AppTiles[i].rcTile.bottom = g_AppTiles[i].rcTile.top + TILE_SIZE;
    }
}

// Animate start menu show/hide
void AnimateStartMenu(BOOL bShow)
{
    if (!g_hStartMenuWnd) return;
    
    if (bShow) {
        ShowWindow(g_hStartMenuWnd, SW_SHOW);
        SetForegroundWindow(g_hStartMenuWnd);
        IgniteAnim_Start(g_hStartMenuWnd, IGNITE_ANIM_SCALE_IN, 250);
    } else {
        IgniteAnim_Start(g_hStartMenuWnd, IGNITE_ANIM_SCALE_OUT, 250);
    }
}
