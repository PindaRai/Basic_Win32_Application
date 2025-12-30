#pragma once
#include <Windows.h>

class ButtonManager;

/**
 * AppState is owned by the parent window, passed through
 * lpCreateParams (heap-allocated) and destroyed in
 * the parents' WM_NCDESTROY function
 */
struct AppState {
    // UI utilities
    COLORREF bgColor = RGB(20, 20, 20);
    HFONT childLabelFont = nullptr;

    // Tracking the child window lifecycle
    HWND childHwnd = nullptr;
    bool childOpen = false;

    // Non-owning pointers to ButtonManager instances created in main
    ButtonManager *btn1 = nullptr;
    ButtonManager *btn2 = nullptr;
};
