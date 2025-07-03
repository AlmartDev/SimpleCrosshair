// ------------------------------------------------
// Crosshair Overlay 1.0 Minimal
// By Alonso Martínez (@almartdev)
// ------------------------------------------------

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <d2d1.h>
#include <shellapi.h>
#include <dwmapi.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwmapi")
#pragma comment(lib, "shell32")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

ID2D1Factory* pFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;
HINSTANCE gInstance = nullptr;
HWND gHwnd = nullptr;

void AddTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"Crosshair Overlay");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowTrayMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hwnd);  // Required before TrackPopupMenu
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            AddTrayIcon(hwnd);
            break;

        case WM_PAINT:
        case WM_DISPLAYCHANGE: {
            if (!pRenderTarget) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

                D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
                    D2D1_RENDER_TARGET_TYPE_DEFAULT,
                    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
                );

                pFactory->CreateHwndRenderTarget(
                    rtProps,
                    D2D1::HwndRenderTargetProperties(hwnd, size),
                    &pRenderTarget
                );

                pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pBrush);
            }

            pRenderTarget->BeginDraw();
            pRenderTarget->Clear(D2D1::ColorF(0, 0.0f));  // Fully transparent background

            D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
            float cx = rtSize.width / 2;
            float cy = rtSize.height / 2;
            float len = 10.0f;
            float thickness = 1.0f;

            pRenderTarget->DrawLine(D2D1::Point2F(cx - len, cy), D2D1::Point2F(cx + len, cy), pBrush, thickness);
            pRenderTarget->DrawLine(D2D1::Point2F(cx, cy - len), D2D1::Point2F(cx, cy + len), pBrush, thickness);

            pRenderTarget->EndDraw();
            ValidateRect(hwnd, nullptr);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                ShowTrayMenu(hwnd);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_TRAY_EXIT) {
                PostQuitMessage(0);
            }
            break;

        case WM_DESTROY:
            RemoveTrayIcon(hwnd);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    gInstance = hInstance;
    SetProcessDPIAware();
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

    const wchar_t CLASS_NAME[] = L"CrosshairOverlay";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_CROSS);
    RegisterClass(&wc);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"",
        WS_POPUP,
        0, 0, width, height,
        nullptr, nullptr, hInstance, nullptr
    );

    gHwnd = hwnd;

    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (pBrush) pBrush->Release();
    if (pRenderTarget) pRenderTarget->Release();
    if (pFactory) pFactory->Release();
    return 0;
}
