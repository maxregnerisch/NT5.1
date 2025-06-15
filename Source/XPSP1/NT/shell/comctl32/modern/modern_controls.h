#pragma once
#include "../../../themes/ignite/theme.h"

// Modern Control Types
typedef enum {
    MODERN_BUTTON = 1,
    MODERN_EDIT,
    MODERN_LISTBOX,
    MODERN_COMBOBOX,
    MODERN_SCROLLBAR,
    MODERN_PROGRESSBAR
} MODERN_CONTROL_TYPE;

// Modern Control Functions
BOOL InitModernControls();
LRESULT ModernControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, MODERN_CONTROL_TYPE type);
void DrawModernButton(HDC hdc, RECT* pRect, DWORD state);
void DrawModernEdit(HDC hdc, RECT* pRect, DWORD state);
void DrawModernScrollbar(HDC hdc, RECT* pRect, DWORD state);

