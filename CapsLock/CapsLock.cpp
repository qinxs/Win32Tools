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

    // ���������� ֻ��������һ��ʵ��
    g_hMutex = CreateMutex(NULL, FALSE, szWindowClass);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, TEXT("�������������� . . ."), TEXT("��ʾ - CapsLock"), MB_ICONINFORMATION);
        return 1;
    }

    // Create the main window. This could be a hidden window if you don't need
    HWND hWnd = CreateWindow(szWindowClass,
        szTitle,
        WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,    // ���������
        GetSystemMetrics(SM_CXSCREEN) / 2 - 150, GetSystemMetrics(SM_CYSCREEN) / 2 - 100,   // λ�þ���
        300, 180,   // ���ڴ�С
        NULL,       // �����ھ��
        NULL,       // ���ڲ˵����
        hInstance, NULL
    );

    if (hWnd == NULL)
    {
        return 0;
    }

    AddTrayIcon(hWnd);

    // ���ü���ȫ�ּ���
    g_hHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,     // �������͡�������Ϣ��
        KeyboardProc,       // ������
        hInstance,          // ��ǰʵ�����
        0                   // �����߳�ID(NULLΪȫ�ּ���)
    );

    // �ж��Ƿ�ɹ�
    if (g_hHook == NULL)
    {
        MessageBox(hWnd, TEXT("��ӹ���ʧ��"), TEXT("���� - CapsLock"), MB_OK | MB_ICONERROR);
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

    // ������Դ
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
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);   // ����Ĭ�ϱ���
    // wcex.hbrBackground   = CreateSolidBrush(RGB(135, 206, 235));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    RegisterClassEx(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hBrush;   // ��ˢ
    static HFONT hFont;     // �߼�����

    switch (message)
    {
    case WM_CREATE:	    // ���ڴ���ʱ�����Ϣ.
        {
            g_bAutoStart = IsAutoStartEnabled();

            // ������ˢ
            hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
            // hBrush = CreateSolidBrush(RGB(0, 0, 0));

            static TCHAR greeting[32];
            LoadString(g_hInst, IDS_GREETING, greeting, ARRAYSIZE(greeting));
            // ������̬�ı���ؼ�
            HWND hStatic = CreateWindow(TEXT("static"),
                greeting,
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
                20, 30,         // ����
                240, 60,        // ���
                hWnd,           // �����ھ��
                (HMENU)101,     // �ؼ�ID
                g_hInst,        // ��ǰ����ʵ�����
                NULL
            );

            // ��������
            hFont = CreateFont(-14, -7,     // height, width
                0, 0,                       // escapement, orientation
                FW_BOLD,                    // font weight
                FALSE, FALSE, FALSE,        // б��-�»���-ɾ����
                DEFAULT_CHARSET,            // charset
                OUT_CHARACTER_PRECIS,       // output precision
                CLIP_CHARACTER_PRECIS,      // clip precision
                DEFAULT_QUALITY,            // quality
                FF_DONTCARE,
                TEXT("΢���ź�")            // font
            );

            // ���ÿؼ�������
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
    // ���þ�̬�ؼ���ɫ
    case WM_CTLCOLORSTATIC:
        {
            // OutputDebugString(TEXT("WM_CTLCOLORSTATIC\n"));
            HDC hdc = (HDC)wParam;
            // SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            SetTextColor(hdc, RGB(237, 113, 97));   // ����ɫ
            // SetBkMode(hdc, TRANSPARENT);          // ͸������
            // SetBkColor(hdc, RGB(0, 255, 255) );   // ���ֱ�����ɫ
            return (INT_PTR)hBrush;                 // ���뷵�ػ�ˢ���
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
            case WM_LBUTTONDBLCLK:  // ˫�����̵���Ϣ���˳�
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

        // ÿ�ζ������������µ� hMenu�����������ַ�ʽ����ѡ��״̬
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
    PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;  // ��ȡ������Ϣ

    // CapsLock����Ϣ
    if (nCode >= 0 && p->vkCode == VK_CAPITAL)
    {
        SetTrayIconAndTip();
    }

    // ����Ϣ�������´���
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

// �Ƿ񿪻�����
BOOL IsAutoStartEnabled()
{
    HKEY hKey = NULL;
    BOOL isEnabled = FALSE;

    // ϵͳ������
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_RUN_PATH, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return isEnabled;

    // ���exeȫ·��
    TCHAR szAppPath[MAX_PATH];
    GetModuleFileName(NULL, szAppPath, MAX_PATH);

    // �ж�ע������Ƿ��Ѿ�����
    TCHAR szAppPathInReg[MAX_PATH] = {};
    DWORD nLength = MAX_PATH;
    long result = RegGetValue(hKey, NULL, szWindowClass, RRF_RT_REG_SZ, NULL, szAppPathInReg, &nLength);

    if (result == ERROR_SUCCESS)
    {
        // ·��һ��
        if (lstrcmpi(szAppPath, szAppPathInReg) != 0)
        {
            RegSetValueEx(hKey, szWindowClass, 0, REG_SZ, (LPBYTE)szAppPath, (lstrlen(szAppPath) + 1) * sizeof(TCHAR));
        }
        isEnabled = TRUE;
    }

    // �ر�ע���
    RegCloseKey(hKey);
    return isEnabled;
}

// ����/ȡ�� ��������
void SetAutoStart(BOOL flag)
{
    HKEY hKey = NULL;
    long result;

    // ϵͳ������
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_RUN_PATH, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
        return;

    if (flag)
    {
        // ���ȫ·��
        TCHAR szAppPath[MAX_PATH];
        GetModuleFileName(NULL, szAppPath, MAX_PATH);

        result = RegSetValueEx(hKey, szWindowClass, 0, REG_SZ, (LPBYTE)szAppPath, (lstrlen(szAppPath) + 1) * sizeof(TCHAR));
    }
    else {
        // ɾ��
        result = RegDeleteValue(hKey, szWindowClass);
    }

    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS)
    {
        g_bAutoStart = flag;
    }
}