#include <dwmapi.h>
#include <memory>
#include <Windows.h>

#include <Resource.h>
#include "app/AppState.h"
#include "app/ButtonManager.h"
#include "app/WindowProcHandler.h"

namespace {
    constexpr wchar_t kClassName[] = L"MyWindowClass";
    constexpr wchar_t kWindowTitle[] = L"Basic C++ Win32 Application";

    constexpr int kInitialWidth = 1280;
    constexpr int kInitialHeight = 720;

    constexpr INT_PTR kBtnClickId = 1;
    constexpr INT_PTR kBtnRandomId = 2;

    constexpr DWORD kUseImmersiveDarkMode = 20;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // 1) Registering the window class as per Win32 documentation
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProcHandler::WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kClassName;
    wc.hIcon = static_cast<HICON>(LoadImageW(
        hInstance,
        MAKEINTRESOURCEW(IDI_ICON1),
        IMAGE_ICON,
        0, 0,
        LR_DEFAULTSIZE | LR_SHARED
    ));

    if (!RegisterClassW(&wc)) {
        return 0;
    }

    // 2) Creating app state and passing its pointer to the window via lpCreateParams
    auto state = std::make_unique<AppState>();
    AppState *stateRaw = state.get();

    // 3) Creates the parent window and stores the handle into local var
    HWND hwnd = CreateWindowExW(
        0,
        kClassName,
        L"",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        kInitialWidth, kInitialHeight,
        nullptr, nullptr,
        hInstance,
        stateRaw
    );

    // If window creation fails, terminates the program
    if (!hwnd) {
        return 0;
    }

    // The window now owns 'stateRaw' and will delete it in WM_NCDESTROY.
    [[maybe_unused]] AppState *ownedByWindow = state.release();

    // 4) Creating the buttons
    ButtonManager button1(
        hwnd, hInstance,
        0, 0,
        5, 7,
        L"Click Here",
        RGB(35, 35, 35), RGB(255, 255, 255), RGB(255, 255, 255),
        32, L"Helvetica",
        reinterpret_cast<HMENU>(kBtnClickId)
    );

    ButtonManager button2(
        hwnd, hInstance,
        0, 0,
        5, 7,
        L"Random Color",
        RGB(35, 35, 35), RGB(255, 255, 255), RGB(255, 255, 255),
        32, L"Helvetica",
        reinterpret_cast<HMENU>(kBtnRandomId)
    );

    // Stores button pointers into the app state
    stateRaw->btn1 = &button1;
    stateRaw->btn2 = &button2;

    // Attempts to set the Window to dark mode using custom constant
    const DWORD enable = TRUE;
    if (FAILED(DwmSetWindowAttribute(hwnd, kUseImmersiveDarkMode, &enable, sizeof(enable)))) {
        OutputDebugStringW(L"Dark Mode failed \n");
    }

    // 5) Shows the window and runs the message loop
    ShowWindow(hwnd, SW_MAXIMIZE);
    SetWindowTextW(hwnd, kWindowTitle);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterClassW(kClassName, hInstance);
    return 0;
}
