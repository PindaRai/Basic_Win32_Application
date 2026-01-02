#include <Windows.h>
#include <cwchar>
#include <dwmapi.h>
#include <random>

#include "app/AppState.h"
#include "app/ButtonManager.h"
#include "Resource.h"
#include "app/WindowProcHandler.h"

namespace {
    constexpr int kChildLabelId = 1000;
    constexpr int kChildOkId = 1001;

    constexpr int kBtnClickId = 1;
    constexpr int kBtnRandomId = 2;

    constexpr COLORREF kChildBg = RGB(30, 30, 30);

    constexpr DWORD kUseImmersiveDarkMode = 20;

    // Function generates random RGB value for the parent window background
    COLORREF RandomColour() {
        static std::mt19937 gen{std::random_device{}()};
        static std::uniform_int_distribution<int> dis(0, 255);
        return RGB(dis(gen), dis(gen), dis(gen));
    }
}

/**
 * Child window proc. function, it uses the parent's AppState via GWLP_USERDATA, creates a label and ok button
 * using styles inherited from parent window for UI attributes
 * @param hwnd Window handle
 * @param uMsg Window message queue feed
 * @param wParam Word parameter
 * @param lParam Long parameter
 * @return
 */
LRESULT CALLBACK WindowProcHandler::ChildWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_NCCREATE) {
        const auto *cs = reinterpret_cast<const CREATESTRUCTW *>(lParam);
        auto *state = static_cast<AppState *>(cs->lpCreateParams);

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

        if (state) state->childHwnd = hwnd;
        return TRUE;
    }

    auto *state = reinterpret_cast<AppState *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    // Main switch statement incorporating child windows' messages
    switch (uMsg) {
        // Creates the label and button windows inside child window and sets state inside AppState
        case WM_CREATE: {
            HWND hLabel = CreateWindowW(
                L"STATIC", L"Button clicked",
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0,
                hwnd,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChildLabelId)),
                nullptr, nullptr
            );

            HWND hButton = CreateWindowW(
                L"BUTTON", L"OK",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
                0, 0, 100, 45,
                hwnd,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChildOkId)),
                nullptr, nullptr
            );

            if (hButton) {
                SendMessageW(hButton, WM_SETFONT,
                             reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)),
                             TRUE);
            }

            if (state && hLabel) {
                if (!state->childLabelFont) {
                    state->childLabelFont = CreateFontW(
                        32, 0, 0, 0, FW_BLACK,
                        FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                        DEFAULT_PITCH | FF_DONTCARE,
                        L"Helvetica"
                    );
                }

                if (state->childLabelFont) {
                    SendMessageW(hLabel, WM_SETFONT,
                                 reinterpret_cast<WPARAM>(state->childLabelFont),
                                 TRUE);
                }
            }

            return 0;
        }

        // Inherits parent buttons' UI attributes and paints them on
        case WM_DRAWITEM: {
            const auto *dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            if (!dis || dis->CtlID != static_cast<UINT>(kChildOkId) || !state || !state->btn1) break;

            HDC hdc = dis->hDC;
            RECT rc = dis->rcItem;

            const COLORREF bg = state->btn1->GetBgColor();
            const COLORREF border = state->btn1->GetBorderColor();
            const COLORREF text = state->btn1->GetTextColor();

            HBRUSH bgBrush = CreateSolidBrush(bg);
            FillRect(hdc, &rc, bgBrush);
            DeleteObject(bgBrush);

            HBRUSH borderBrush = CreateSolidBrush(border);
            FrameRect(hdc, &rc, borderBrush);
            DeleteObject(borderBrush);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, text);
            DrawTextW(hdc, L"OK", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            return TRUE;
        }

        // Paints dark bg pre-emptively to avoid default bright colour flicker
        case WM_ERASEBKGND: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            RECT rc;
            GetClientRect(hwnd, &rc);

            HBRUSH b = CreateSolidBrush(kChildBg);
            FillRect(hdc, &rc, b);
            DeleteObject(b);

            return 1;
        }

        // Ensuring readable light font on dark background
        case WM_CTLCOLORSTATIC: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
        }

        // Sets dimensions of child window UI objects
        case WM_SIZE: {
            const int w = LOWORD(lParam);
            const int h = HIWORD(lParam);

            HWND hLabel = GetDlgItem(hwnd, kChildLabelId);
            HWND hButton = GetDlgItem(hwnd, kChildOkId);
            if (!hLabel || !hButton) return 0;

            constexpr wchar_t labelText[] = L"Button clicked";
            SIZE textSize{};

            HDC hdc = GetDC(hLabel);
            if (!hdc) return 0;

            auto hFont = reinterpret_cast<HFONT>(SendMessageW(hLabel, WM_GETFONT, 0, 0));
            HFONT old = hFont ? static_cast<HFONT>(SelectObject(hdc, hFont)) : nullptr;

            GetTextExtentPoint32W(hdc, labelText, static_cast<int>(wcslen(labelText)), &textSize);

            if (old) SelectObject(hdc, old);
            ReleaseDC(hLabel, hdc);

            const int centerX = (w - textSize.cx) / 2;
            const int centerY = (h - textSize.cy) / 2;

            const int bx = (w - 100) / 2;
            const int by = (h + textSize.cy) / 2;

            SetWindowPos(hLabel, nullptr, centerX, centerY - 37, textSize.cx, textSize.cy, SWP_NOZORDER);
            SetWindowPos(hButton, nullptr, bx, by, 100, 45, SWP_NOZORDER);

            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        // When user confirms exiting child window on dialog, it destroys it
        case WM_COMMAND: {
            if (LOWORD(wParam) == kChildOkId) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        //  Clearing GWLP_USERDATA upon child window destruction
        case WM_DESTROY:
            if (state) {
                state->childOpen = false;
                if (state->childHwnd == hwnd) state->childHwnd = nullptr;
            }
            return 0;

        // Detaching AppState pointer from this HWND
        case WM_NCDESTROY:
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);

        default:
            break;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}


/**
 * Parent window proc. function, it owns the AppState information, handles the Win32 aspect logic of buttons,
 * destroys both AppState and parent window upon program termination
 * @param hwnd Handle to window
 * @param uMsg Message queue
 * @param wParam Word parameter
 * @param lParam Long parameter
 * @return
 */
LRESULT CALLBACK WindowProcHandler::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_NCCREATE) {
        const auto *cs = reinterpret_cast<const CREATESTRUCTW *>(lParam);
        auto *state = static_cast<AppState *>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return TRUE;
    }

    auto *state = reinterpret_cast<AppState *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (uMsg) {
        // Sets the dimensions and position of elements in parent window
        case WM_SIZE: {
            if (!state || !state->btn1 || !state->btn2) return 0;

            const int w = LOWORD(lParam);
            const int h = HIWORD(lParam);

            state->btn1->ComputeResize(w, h);
            state->btn2->ComputeResize(w, h);

            const int bw = state->btn1->GetWidth();
            const int bh = state->btn1->GetHeight();

            state->btn1->SetSizeAndPosition((w - bw) / 2 - bw, (h - bh) / 2, bw, bh);
            state->btn2->SetSizeAndPosition((w - bw) / 2 + bw, (h - bh) / 2, bw, bh);

            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        // Handling of parent buttons' Win32 logic, by checking click ID and performing appropriate action
        case WM_COMMAND: {
            if (!state) return 0;
            const int id = LOWORD(wParam);

            // When clicking the "Click Here" button, if child wnd is already open (refocuses) otherwise creates one
            if (id == kBtnClickId) {
                // Child exists then just restores or refocuses
                if (state->childHwnd && IsWindow(state->childHwnd)) {
                    if (IsIconic(state->childHwnd)) {
                        ShowWindow(state->childHwnd, SW_RESTORE);
                    }
                    if (GetForegroundWindow() != state->childHwnd) {
                        SetForegroundWindow(state->childHwnd);
                    }
                    return 0;
                }

                // Child doesn't exist so creates one
                constexpr wchar_t CHILD_CLASS_NAME[] = L"ChildWindowClass";
                static bool childRegistered = false;

                HINSTANCE hInst = GetModuleHandleW(nullptr);

                if (!childRegistered) {
                    WNDCLASSW childWndClass{};
                    childWndClass.lpfnWndProc = ChildWindowProc;
                    childWndClass.hInstance = hInst;
                    childWndClass.lpszClassName = CHILD_CLASS_NAME;
                    childWndClass.hIcon = static_cast<HICON>(LoadImageW(
                        hInst, MAKEINTRESOURCEW(IDI_ICON1),
                        IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
                    ));

                    RegisterClassW(&childWndClass);
                    childRegistered = true;
                }

                const int screenW = GetSystemMetrics(SM_CXSCREEN);
                const int screenH = GetSystemMetrics(SM_CYSCREEN);
                const int childW = screenW / 5;
                const int childH = screenH / 6;
                const int xPos = (screenW - childW) / 2;
                const int yPos = (screenH - childH) / 2;

                HWND hChildWnd = CreateWindowExW(
                    0, CHILD_CLASS_NAME, L"Button Clicked",
                    WS_OVERLAPPEDWINDOW,
                    xPos, yPos, childW, childH,
                    nullptr, nullptr, hInst,
                    state
                );

                if (hChildWnd) {
                    state->childHwnd = hChildWnd;
                    state->childOpen = true;

                    DWORD attributeValue = 1;
                    if (FAILED(DwmSetWindowAttribute(hChildWnd, kUseImmersiveDarkMode,
                        &attributeValue, sizeof(attributeValue)))) {
                        OutputDebugStringW(L"DwmSetWindowAttribute: dark mode failed\n");
                    }

                    ShowWindow(hChildWnd, SW_SHOW);
                } else {
                    state->childOpen = false;
                    state->childHwnd = nullptr;
                }

                return 0;
            }

            // Otherwise the Random button randomizes the parent windows' background
            if (id == kBtnRandomId) {
                state->bgColor = RandomColour();
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
            }

            return 0;
        }

        // Sets the parent window UI object attributes, uses state data
        case WM_DRAWITEM: {
            if (!state || !state->btn1 || !state->btn2) break;

            const auto *dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            if (!dis) break;

            HDC hdc = dis->hDC;
            RECT rc = dis->rcItem;

            if (dis->CtlID == kBtnClickId) {
                const auto text = L"Click Here";

                HBRUSH bg = CreateSolidBrush(state->btn1->GetBgColor());
                FillRect(hdc, &rc, bg);
                DeleteObject(bg);

                HBRUSH border = CreateSolidBrush(state->btn1->GetBorderColor());
                FrameRect(hdc, &rc, border);
                DeleteObject(border);

                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, state->btn1->GetTextColor());
                DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }

            if (dis->CtlID == kBtnRandomId) {
                const auto text = L"Random Colour";

                HBRUSH bg = CreateSolidBrush(state->btn2->GetBgColor());
                FillRect(hdc, &rc, bg);
                DeleteObject(bg);

                HBRUSH border = CreateSolidBrush(state->btn2->GetBorderColor());
                FrameRect(hdc, &rc, border);
                DeleteObject(border);

                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, state->btn2->GetTextColor());
                DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }

            break;
        }

        // Painting background colour on parent window
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            COLORREF c = state ? state->bgColor : RGB(20, 20, 20);
            HBRUSH hBrush = CreateSolidBrush(c);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CLOSE: {
            const int result = MessageBoxW(hwnd, L"Do you want to close the window?", L"Confirmation",
                                           MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES) {
                if (state && state->childHwnd && IsWindow(state->childHwnd)) {
                    DestroyWindow(state->childHwnd);
                    state->childHwnd = nullptr;
                    state->childOpen = false;
                }
                DestroyWindow(hwnd);
            }
            return 0;
        }

        // Upon destruction it lets Windows OS know
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        // Cleans up all parent window objects including cached child windows' from memory
        case WM_NCDESTROY: {
            auto *dyingState = reinterpret_cast<AppState *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);

            if (dyingState) {
                if (dyingState->childHwnd && IsWindow(dyingState->childHwnd)) {
                    DestroyWindow(dyingState->childHwnd);
                    dyingState->childHwnd = nullptr;
                    dyingState->childOpen = false;
                }

                if (dyingState->childLabelFont) {
                    DeleteObject(dyingState->childLabelFont);
                    dyingState->childLabelFont = nullptr;
                }

                dyingState->btn1 = nullptr;
                dyingState->btn2 = nullptr;

                delete dyingState;
            }

            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }

        default:
            break;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
