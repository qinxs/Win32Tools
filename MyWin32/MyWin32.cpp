#include <Windows.h>
#include <stdio.h>
#include "resource.h"


// 全局变量
HINSTANCE g_hInst;                              // 当前实例
TCHAR szTitle[32];                              // 标题栏文本
TCHAR szWindowClass[] = L"ClassName.7bxing";    // 主窗口类名

// 此代码模块中包含的函数的前向声明
void                OutputDebugStringEx(const wchar_t *strOutputString, ...);
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 将实例句柄存储在全局变量中
    g_hInst = hInstance;

    // TODO: 在此放置代码

    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));
    MyRegisterClass(hInstance);

    // 执行应用程序初始化
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // 快捷键
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYWIN32));

    MSG msg;

    // 主消息循环
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        //if (TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        //    continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}


// 注册窗口类
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGO));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);     // 窗口默认背景
    //wcex.hbrBackground  = CreateSolidBrush(RGB(135, 206, 235));
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU);
    wcex.lpszClassName  = szWindowClass;
    //wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_LOGO_SMALL));

    return RegisterClassEx(&wcex);
}

//
// 创建和显示主程序窗口
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindow(szWindowClass,
       szTitle,
       WS_OVERLAPPEDWINDOW,
       //WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,    // 不允许最大化
       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
       //GetSystemMetrics(SM_CXSCREEN) / 2 - 400, GetSystemMetrics(SM_CYSCREEN) / 2 - 300,   // 位置居中
       //800, 600,   // 窗口大小
       NULL,       // 父窗口句柄
       NULL,       // 窗口菜单句柄
       hInstance, NULL
   );

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  窗口过程
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择
            switch (wmId)
            {
            case ID_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                return 0;
            case ID_EXIT:
                DestroyWindow(hWnd);
                return 0;
            }
        }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// “关于”框
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// 重写输出函数
// 示例: OutputDebugStringEx(L"DEBUG_INFO | %d %s\r\n", 1234, L"this is test")
void OutputDebugStringEx(const wchar_t *strOutputString, ...)
{
#ifdef _DEBUG
    va_list vlArgs = NULL;
    va_start(vlArgs, strOutputString);
    size_t nLen = _vscwprintf(strOutputString, vlArgs) + 1;
    wchar_t *strBuffer = new wchar_t[nLen];
    _vsnwprintf_s(strBuffer, nLen, nLen, strOutputString, vlArgs);
    va_end(vlArgs);
    OutputDebugStringW(strBuffer);
    delete[] strBuffer;
#endif
}
