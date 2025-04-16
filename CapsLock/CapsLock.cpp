// we need commctrl v6 for LoadIconMetric()
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name = 'Microsoft.Windows.Common-Controls' version = '6.0.0.0' \
processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include "resource.h"

#define REG_RUN_PATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")

// Global variables
HINSTANCE       g_hInst;
NOTIFYICONDATA  nid;
HANDLE          g_hMutex;
HHOOK           g_hHook;
BOOL            g_bAutoStart = FALSE;

const UINT WMAPP_NOTIFYCALLBACK = WM_APP + 1;

// The main window title and class name.
const  TCHAR szWindowClass[] = TEXT("CapsLock.7bxing");
static TCHAR szTitle[32];

// Forward declarations of functions included in this code module:
void                RegisterWindowClass();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL                AddTrayIcon(HWND hWnd);
BOOL                DeleteTrayIcon();
void                ShowContextMenu(HWND hWnd, POINT pt);
void                SetTrayIconAndTip();
LRESULT CALLBACK    KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL                IsAutoStartEnabled();
void                SetAutoStart(BOOL flag);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    g_hInst = hInstance;
    RegisterWindowClass();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));

    // 创建互斥体 只允许运行一个实例
    g_hMutex = CreateMutex(NULL, FALSE, szWindowClass);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, TEXT("程序已在运行中 . . ."), TEXT("提示 - CapsLock"), MB_ICONINFORMATION);
        return 1;
    }

    // Create the main window. This could be a hidden window if you don't need
    HWND hWnd = CreateWindow(szWindowClass,
        szTitle,
        WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,    // 不允许最大化
        GetSystemMetrics(SM_CXSCREEN) / 2 - 150, GetSystemMetrics(SM_CYSCREEN) / 2 - 100,   // 位置居中
        300, 180,   // 窗口大小
        NULL,       // 父窗口句柄
        NULL,       // 窗口菜单句柄
        hInstance, NULL
    );

    if (hWnd == NULL)
    {
        return 0;
    }

    AddTrayIcon(hWnd);

    // 设置键盘全局监听
    g_hHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,     // 监听类型【键盘消息】
        KeyboardProc,       // 处理函数
        hInstance,          // 当前实例句柄
        0                   // 监听线程ID(NULL为全局监听)
    );

    // 判断是否成功
    if (g_hHook == NULL)
    {
        MessageBox(hWnd, TEXT("添加钩子失败"), TEXT("错误 - CapsLock"), MB_OK | MB_ICONERROR);
        return 1;
    }

    // ShowWindow(hWnd, SW_HIDE);
    // UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 清理资源
    if (g_hMutex)
    {
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
    }
    DeleteTrayIcon();
    UnhookWindowsHookEx(g_hHook);

    return (int)msg.wParam;
}

void RegisterWindowClass()
{
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = g_hInst;
    wcex.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);   // 窗口默认背景
    // wcex.hbrBackground   = CreateSolidBrush(RGB(135, 206, 235));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    RegisterClassEx(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hBrush;   // 画刷
    static HFONT hFont;     // 逻辑字体

    switch (message)
    {
    case WM_CREATE:	    // 窗口创建时候的消息.
        {
            g_bAutoStart = IsAutoStartEnabled();

            // 创建画刷
            hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
            // hBrush = CreateSolidBrush(RGB(0, 0, 0));

            static TCHAR greeting[32];
            LoadString(g_hInst, IDS_GREETING, greeting, ARRAYSIZE(greeting));
            // 创建静态文本框控件
            HWND hStatic = CreateWindow(TEXT("static"),
                greeting,
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
                20, 30,         // 坐标
                240, 60,        // 宽高
                hWnd,           // 父窗口句柄
                (HMENU)101,     // 控件ID
                g_hInst,        // 当前程序实例句柄
                NULL
            );

            // 创建字体
            hFont = CreateFont(-14, -7,     // height, width
                0, 0,                       // escapement, orientation
                FW_BOLD,                    // font weight
                FALSE, FALSE, FALSE,        // 斜体-下划线-删除线
                DEFAULT_CHARSET,            // charset
                OUT_CHARACTER_PRECIS,       // output precision
                CLIP_CHARACTER_PRECIS,      // clip precision
                DEFAULT_QUALITY,            // quality
                FF_DONTCARE,
                TEXT("微软雅黑")            // font
            );

            // 设置控件的字体
            SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, NULL);
            // OutputDebugString(TEXT("WM_CREATE\n"));
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.
            // only the background and the child window need painting

            EndPaint(hWnd, &ps);
            // OutputDebugString(TEXT("WM_PAINT\n"));
        }
        break;
    // 设置静态控件颜色
    case WM_CTLCOLORSTATIC:
        {
            // OutputDebugString(TEXT("WM_CTLCOLORSTATIC\n"));
            HDC hdc = (HDC)wParam;
            // SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            SetTextColor(hdc, RGB(237, 113, 97));   // 淡红色
            // SetBkMode(hdc, TRANSPARENT);          // 透明背景
            // SetBkColor(hdc, RGB(0, 255, 255) );   // 文字背景颜色
            return (INT_PTR)hBrush;                 // 必须返回画刷句柄
        }
        break;
    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        break;
    case WM_DESTROY:
        DeleteObject(hBrush);
        DeleteObject(hFont);        
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        {
            int const wmId = LOWORD(wParam);
            // OutputDebugString(TEXT("WM_COMMAND\n"));
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_TRAY_AUTOSTART:
                    g_bAutoStart = !g_bAutoStart;
                    SetAutoStart(g_bAutoStart);
                    break;
                case IDM_TRAY_SHOWMAIN:
                    ShowWindow(hWnd, SW_RESTORE);
                    SetActiveWindow(hWnd);
                    break;
                case IDM_TRAY_EXIT:
                    DestroyWindow(hWnd);
                    break;
            }
        }
        break;
    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam))
        {
            case WM_LBUTTONDBLCLK:  // 双击托盘的消息，退出
                ShowWindow(hWnd, SW_RESTORE);
                SetForegroundWindow(hWnd);
                break;
            case WM_CONTEXTMENU:
                POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
                ShowContextMenu(hWnd, pt);
                break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL AddTrayIcon(HWND hWnd) {
    nid = { sizeof(nid) };
    nid.hWnd = hWnd;
    nid.uID = IDI_ICON2;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    // nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
    SetTrayIconAndTip();
    Shell_NotifyIcon(NIM_ADD, &nid);

    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL DeleteTrayIcon()
{
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hWnd, POINT pt)
{
    HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_TRAYMENU));
    if (hMenu)
    {
        HMENU hTrayMenu = GetSubMenu(hMenu, 0);

        // 每次都是重新生成新的 hMenu，所以用这种方式处理选中状态
        CheckMenuItem(hTrayMenu, IDM_TRAY_AUTOSTART, g_bAutoStart ? MF_CHECKED : MF_UNCHECKED);
        if (hTrayMenu)
        {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hWnd);

            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
            {
                uFlags |= TPM_RIGHTALIGN;
            }
            else
            {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hTrayMenu, uFlags, pt.x, pt.y, hWnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}

void SetTrayIconAndTip()
{
    if (GetKeyState(VK_CAPITAL) & 1)
    {
        LoadString(g_hInst, IDS_TRAYTIP_ON, nid.szTip, ARRAYSIZE(nid.szTip));
        LoadIconMetric(g_hInst, MAKEINTRESOURCE(IDI_ICON2), LIM_SMALL, &nid.hIcon);
    }
    else
    {
        LoadString(g_hInst, IDS_TRAYTIP_OFF, nid.szTip, ARRAYSIZE(nid.szTip));
        LoadIconMetric(g_hInst, MAKEINTRESOURCE(IDI_ICON3), LIM_SMALL, &nid.hIcon);
    }
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;  // 获取按键消息

    // CapsLock键消息
    if (nCode >= 0 && p->vkCode == VK_CAPITAL)
    {
        SetTrayIconAndTip();
    }

    // 将消息继续往下传递
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

// 是否开机启动
BOOL IsAutoStartEnabled()
{
    HKEY hKey = NULL;
    BOOL isEnabled = FALSE;

    // 系统启动项
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_RUN_PATH, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return isEnabled;

    // 获得exe全路径
    TCHAR szAppPath[MAX_PATH];
    GetModuleFileName(NULL, szAppPath, MAX_PATH);

    // 判断注册表项是否已经存在
    TCHAR szAppPathInReg[MAX_PATH] = {};
    DWORD nLength = MAX_PATH;
    long result = RegGetValue(hKey, NULL, szWindowClass, RRF_RT_REG_SZ, NULL, szAppPathInReg, &nLength);

    if (result == ERROR_SUCCESS)
    {
        // 路径一致
        if (lstrcmpi(szAppPath, szAppPathInReg) != 0)
        {
            RegSetValueEx(hKey, szWindowClass, 0, REG_SZ, (LPBYTE)szAppPath, (lstrlen(szAppPath) + 1) * sizeof(TCHAR));
        }
        isEnabled = TRUE;
    }

    // 关闭注册表
    RegCloseKey(hKey);
    return isEnabled;
}

// 设置/取消 开机启动
void SetAutoStart(BOOL flag)
{
    HKEY hKey = NULL;
    long result;

    // 系统启动项
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_RUN_PATH, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return;

    if (flag)
    {
        // 获得全路径
        TCHAR szAppPath[MAX_PATH];
        GetModuleFileName(NULL, szAppPath, MAX_PATH);

        result = RegSetValueEx(hKey, szWindowClass, 0, REG_SZ, (LPBYTE)szAppPath, (lstrlen(szAppPath) + 1) * sizeof(TCHAR));
    }
    else {
        // 删除
        result = RegDeleteValue(hKey, szWindowClass);
    }

    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS)
    {
        g_bAutoStart = flag;
    }
}