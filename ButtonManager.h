#pragma once
#include <Windows.h>

class ButtonManager {
public:
    ButtonManager(HWND parentHwnd,
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
                  HMENU buttonId);

    ~ButtonManager();

    ButtonManager(const ButtonManager&) = delete;
    ButtonManager& operator=(const ButtonManager&) = delete;
    ButtonManager(ButtonManager&&) = delete;
    ButtonManager& operator=(ButtonManager&&) = delete;

    void SetSizeAndPosition(int x, int y, int width, int height);
    void ComputeResize(int updWinWidth, int updWinHeight);
    void DestroyButton();

    [[nodiscard]] COLORREF GetBgColor() const;
    [[nodiscard]] COLORREF GetTextColor() const;
    [[nodiscard]] COLORREF GetBorderColor() const;
    [[nodiscard]] int GetWidth() const;
    [[nodiscard]] int GetHeight() const;

private:
    HWND  hButton = nullptr;
    HFONT hFont   = nullptr;

    int buttonX = 0, buttonY = 0;
    int widthDivisor = 1, heightDivisor = 1;
    int buttonWidth = 0, buttonHeight = 0;
    int fontSize = 0;

    COLORREF bgColor = 0, textColor = 0, borderColor = 0;
};