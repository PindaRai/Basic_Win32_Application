#include "app/ButtonManager.h"

/**
 * ButtonManager is implemented as a RAII wrapper encapsulates the Win32 button
 * creation logic (CreateWindowEx, font setup, resizing math, destruction)
 * @param parentHwnd Parent window handle
 * @param hInstance Handle instance
 * @param x Button x-axis pos.
 * @param y Button y-axis pos.
 * @param widthDivisor Value scaling button width
 * @param heightDivisor Value scaling button height
 * @param buttonName
 * @param bgColor
 * @param textColor
 * @param borderColor
 * @param fontSize
 * @param fontFamily
 * @param buttonId Button instance identifier
 */
ButtonManager::ButtonManager(HWND parentHwnd,
                             HINSTANCE hInstance,
                             int x, int y,
                             int widthDivisor,
                             int heightDivisor,
                             LPCWSTR buttonName,
                             COLORREF bgColor,
                             COLORREF textColor,
                             COLORREF borderColor,
                             int fontSize,
                             LPCWSTR fontFamily,
                             HMENU buttonId)
    : buttonX(x),
      buttonY(y),
      widthDivisor(widthDivisor),
      heightDivisor(heightDivisor),
      fontSize(fontSize),
      bgColor(bgColor),
      textColor(textColor),
      borderColor(borderColor) {

    // Creating the underlying Win32 button control
    hButton = CreateWindowExW(
        0,
        L"BUTTON",
        buttonName,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        buttonX, buttonY,
        buttonWidth, buttonHeight,
        parentHwnd,
        buttonId,
        hInstance,
        nullptr
    );

    if (!hButton) {
        return;
    }

    // Creating a custom GDI font, this is owned by object and later deleted
    hFont = CreateFontW(
        this->fontSize, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        fontFamily
    );

    // Attaching font to the control
    if (hFont) {
        SendMessageW(hButton, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }
}

// Object destructor restores default font prior to deleting custom one
ButtonManager::~ButtonManager() {
    if (hButton && IsWindow(hButton)) {
        SendMessageW(hButton,
                     WM_SETFONT,
                     reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)),
                     TRUE);
    }

    DestroyButton();

    if (hFont) {
        DeleteObject(hFont);
        hFont = nullptr;
    }
}

// Destroys instance of ButtonManager object
void ButtonManager::DestroyButton() {
    if (hButton && IsWindow(hButton)) {
        DestroyWindow(hButton);
    }
    hButton = nullptr;
}

// Function sets the size and pos. of button in window
void ButtonManager::SetSizeAndPosition(int x, int y, int width, int height) {
    if (!hButton) return;

    buttonX = x;
    buttonY = y;
    buttonWidth = width;
    buttonHeight = height;

    SetWindowPos(hButton, nullptr, buttonX, buttonY, buttonWidth, buttonHeight, SWP_NOZORDER);
}

// Function computes updated dimensions, responsive to window dimensions
void ButtonManager::ComputeResize(int updWinWidth, int updWinHeight) {
    const int newWidth = updWinWidth / widthDivisor;
    const int newHeight = updWinHeight / heightDivisor;

    constexpr int kMinWidth = 200;
    constexpr int kMinHeight = 50;

    buttonWidth = (newWidth < kMinWidth) ? kMinWidth : newWidth;
    buttonHeight = (newHeight < kMinHeight) ? kMinHeight : newHeight;
}

COLORREF ButtonManager::GetBgColor() const { return bgColor; }
COLORREF ButtonManager::GetTextColor() const { return textColor; }
COLORREF ButtonManager::GetBorderColor() const { return borderColor; }
int ButtonManager::GetWidth() const { return buttonWidth; }
int ButtonManager::GetHeight() const { return buttonHeight; }
