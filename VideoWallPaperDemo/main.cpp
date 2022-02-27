#include <windows.h>
#include <stdio.h>

/*
这是在Win10中实现动态壁纸的简单demo，旨在用简洁的代码演示其原理
本例子有一定门槛，不可以直接运行，其中有些代码要根据实际情况修改
一定要正确安装开源软件ffplay

★代码不保留版权，使用时请保留如下信息：
作者：偶尔有点小迷糊 (https://space.bilibili.com/39665558)
粉丝：23万

讲解视频：https://www.bilibili.com/video/BV1HZ4y1978a
*/

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM Lparam)
{
    HWND hDefView = FindWindowEx(hwnd, 0, L"SHELLDLL_DefView", 0);
    if (hDefView != 0) {
        // 找它的下一个窗口，类名为WorkerW，隐藏它
        HWND hWorkerw = FindWindowEx(0, hwnd, L"WorkerW", 0);
        ShowWindow(hWorkerw, SW_HIDE);

        return FALSE;
    }
    return TRUE;
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
)
{
    TCHAR szAppName[] = L"D:\\PortableApps\\ffmpeg\\bin\\ffplay.exe";
    // 视频路径及ffplay参数，根据实际情况改
    // -fs; -x 800 -y 600; fs为全屏
    LPCWSTR lpParameter = L" F:\\视频.mp4  -noborder -x 800 -y 600 -loop 0 -hide_banner";
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    // 隐藏命令窗口
    //si.dwFlags = STARTF_USESHOWWINDOW;
    //si.wShowWindow = SW_HIDE;

    // 播放声音 添加此环境变量
    // set SDL_AUDIODRIVER=directsound
    //if (!SetEnvironmentVariable(L"SDL_AUDIODRIVER", L"directsound")) 
    //{
    //   printf("SetEnvironmentVariable failed (%d)\n", GetLastError()); 
    //   return FALSE;
    //}

    // 下面是我电脑上ffplay的路径，要根据实际情况改
    if (CreateProcess(
        szAppName,
        (LPWSTR)lpParameter,
        NULL, NULL, FALSE, 0, NULL, NULL,
        &si, &pi))
    {
        Sleep(200); // 等待视频播放器启动完成。可用循环获取窗口尺寸来代替Sleep()

        // Wait until child process exits.
        //WaitForSingleObject( pi.hProcess, INFINITE );

        // Close process and thread handles. 
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );

        HWND hProgman = FindWindow(L"Progman", 0);				// 找到PM窗口
        SendMessageTimeout(hProgman, 0x52C, 0, 0, 0, 100, 0);	// 给它发特殊消息
        HWND hFfplay = FindWindow(L"SDL_app", 0);				// 找到视频窗口
        SetParent(hFfplay, hProgman);							// 将视频窗口设置为PM的子窗口
        EnumWindows(EnumWindowsProc, 0);						// 找到第二个WorkerW窗口并隐藏它
    }

    return 0;
}
