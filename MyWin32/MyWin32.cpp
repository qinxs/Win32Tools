#include <Windows.h>
#include <stdio.h>
#include "resource.h"


// ȫ�ֱ���
HINSTANCE g_hInst;                              // ��ǰʵ��
TCHAR szTitle[32];                              // �������ı�
TCHAR szWindowClass[] = L"ClassName.7bxing";    // ����������

// �˴���ģ���а����ĺ�����ǰ������
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

    // ��ʵ������洢��ȫ�ֱ�����
    g_hInst = hInstance;

    // TODO: �ڴ˷��ô���

    // ��ʼ��ȫ���ַ���
    LoadString(hInstance, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));
    MyRegisterClass(hInstance);

    // ִ��Ӧ�ó����ʼ��
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // ��ݼ�
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYWIN32));

    MSG msg;

    // ����Ϣѭ��
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        //if (TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        //    continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}


// ע�ᴰ����
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGO));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);     // ����Ĭ�ϱ���
    //wcex.hbrBackground  = CreateSolidBrush(RGB(135, 206, 235));
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU);
    wcex.lpszClassName  = szWindowClass;
    //wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_LOGO_SMALL));

    return RegisterClassEx(&wcex);
}

//
// ��������ʾ�����򴰿�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindow(szWindowClass,
       szTitle,
       WS_OVERLAPPEDWINDOW,
       //WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,    // ���������
       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
       //GetSystemMetrics(SM_CXSCREEN) / 2 - 400, GetSystemMetrics(SM_CYSCREEN) / 2 - 300,   // λ�þ���
       //800, 600,   // ���ڴ�С
       NULL,       // �����ھ��
       NULL,       // ���ڲ˵����
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
//  ���ڹ���
//
//  WM_COMMAND  - ����Ӧ�ó���˵�
//  WM_PAINT    - ����������
//  WM_DESTROY  - �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // �����˵�ѡ��
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
            // TODO: �ڴ˴����ʹ�� hdc ���κλ�ͼ����...
            EndPaint(hWnd, &ps);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// �����ڡ���
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

// ��д�������
// ʾ��: OutputDebugStringEx(L"DEBUG_INFO | %d %s\r\n", 1234, L"this is test")
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
