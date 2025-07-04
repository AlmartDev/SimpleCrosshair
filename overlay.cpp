// ------------------------------------------------
// Crosshair Overlay 1.0 Gui-included
// By Alonso Martínez (@almartdev)
// ------------------------------------------------

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <d2d1.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <commdlg.h>
#include <wchar.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwmapi")
#pragma comment(lib, "shell32")
#pragma comment(lib, "comdlg32")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SETTINGS 1002
#define ID_EDIT_LENGTH 2001
#define ID_EDIT_THICKNESS 2002
#define ID_BTN_COLOR 2003

ID2D1Factory* pFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;
HINSTANCE gInstance = nullptr;
HWND gHwnd = nullptr;

// Crosshair settings
float crosshairLength = 10.0f;
float crosshairThickness = 1.0f;
COLORREF crosshairColor = RGB(255, 0, 0);

void UpdateBrushColor() {
    if (pBrush) {
        pBrush->Release();
        pBrush = nullptr;
    }

    FLOAT r = GetRValue(crosshairColor) / 255.0f;
    FLOAT g = GetGValue(crosshairColor) / 255.0f;
    FLOAT b = GetBValue(crosshairColor) / 255.0f;

    pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b), &pBrush);
}

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
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_SETTINGS, L"Settings");
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(hMenu);
}

// Settings window procedure
LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hLengthEdit, hThicknessEdit;

    switch (msg) {
        case WM_CREATE: {
            CreateWindowW(L"STATIC", L"Length:", WS_VISIBLE | WS_CHILD,
                          10, 10, 60, 20, hwnd, nullptr, gInstance, nullptr);
            hLengthEdit = CreateWindowW(L"EDIT", nullptr,
                          WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                          80, 10, 100, 20, hwnd, (HMENU)ID_EDIT_LENGTH, gInstance, nullptr);

            CreateWindowW(L"STATIC", L"Thickness:", WS_VISIBLE | WS_CHILD,
                          10, 40, 60, 20, hwnd, nullptr, gInstance, nullptr);
            hThicknessEdit = CreateWindowW(L"EDIT", nullptr,
                          WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                          80, 40, 100, 20, hwnd, (HMENU)ID_EDIT_THICKNESS, gInstance, nullptr);

            CreateWindowW(L"BUTTON", L"Color", WS_VISIBLE | WS_CHILD,
                          10, 70, 60, 25, hwnd, (HMENU)ID_BTN_COLOR, gInstance, nullptr);

            CreateWindowW(L"BUTTON", L"OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                          80, 70, 50, 25, hwnd, (HMENU)IDOK, gInstance, nullptr);

            CreateWindowW(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
                          140, 70, 60, 25, hwnd, (HMENU)IDCANCEL, gInstance, nullptr);

            wchar_t buf[32];
            swprintf_s(buf, L"%.0f", crosshairLength);
            SetWindowTextW(hLengthEdit, buf);
            swprintf_s(buf, L"%.0f", crosshairThickness);
            SetWindowTextW(hThicknessEdit, buf);
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BTN_COLOR: {
                    CHOOSECOLOR cc = { sizeof(cc) };
                    COLORREF custom[16] = {};
                    cc.hwndOwner = hwnd;
                    cc.rgbResult = crosshairColor;
                    cc.lpCustColors = custom;
                    cc.Flags = CC_RGBINIT | CC_FULLOPEN;
                    if (ChooseColor(&cc)) {
                        crosshairColor = cc.rgbResult;
                    }
                    break;
                }
                case IDOK: {
                    wchar_t buf[32];
                    GetWindowTextW(hLengthEdit, buf, 32);
                    crosshairLength = (float)_wtoi(buf);
                    GetWindowTextW(hThicknessEdit, buf, 32);
                    crosshairThickness = (float)_wtoi(buf);
                    UpdateBrushColor();
                    InvalidateRect(gHwnd, nullptr, TRUE);
                    DestroyWindow(hwnd);
                    break;
                }
                case IDCANCEL:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void ShowSettingsWindow(HWND parent) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = SettingsProc;
    wc.hInstance = gInstance;
    wc.lpszClassName = L"SettingsWindow";
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0, L"SettingsWindow", L"Crosshair Settings",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 230, 150,
        parent, nullptr, gInstance, nullptr
    );
    ShowWindow(hWnd, SW_SHOW);
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

                UpdateBrushColor();
            }

            pRenderTarget->BeginDraw();
            pRenderTarget->Clear(D2D1::ColorF(0, 0.0f));

            D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
            float cx = rtSize.width / 2;
            float cy = rtSize.height / 2;

            pRenderTarget->DrawLine(D2D1::Point2F(cx - crosshairLength, cy), D2D1::Point2F(cx + crosshairLength, cy), pBrush, crosshairThickness);
            pRenderTarget->DrawLine(D2D1::Point2F(cx, cy - crosshairLength), D2D1::Point2F(cx, cy + crosshairLength), pBrush, crosshairThickness);

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
            switch (LOWORD(wParam)) {
                case ID_TRAY_EXIT:
                    PostQuitMessage(0);
                    break;
                case ID_TRAY_SETTINGS:
                    ShowSettingsWindow(hwnd);
                    break;
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
