#include <windows.h>
#include <stdio.h>

/*
������Win10��ʵ�ֶ�̬��ֽ�ļ�demo��ּ���ü��Ĵ�����ʾ��ԭ��
��������һ���ż���������ֱ�����У�������Щ����Ҫ����ʵ������޸�
һ��Ҫ��ȷ��װ��Դ���ffplay

����벻������Ȩ��ʹ��ʱ�뱣��������Ϣ��
���ߣ�ż���е�С�Ժ� (https://space.bilibili.com/39665558)
��˿��23��

������Ƶ��https://www.bilibili.com/video/BV1HZ4y1978a
*/

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM Lparam)
{
    HWND hDefView = FindWindowEx(hwnd, 0, L"SHELLDLL_DefView", 0);
    if (hDefView != 0) {
        // ��������һ�����ڣ�����ΪWorkerW��������
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
    // ��Ƶ·����ffplay����������ʵ�������
    // -fs; -x 800 -y 600; fsΪȫ��
    LPCWSTR lpParameter = L" F:\\��Ƶ.mp4  -noborder -x 800 -y 600 -loop 0 -hide_banner";
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    // ���������
    //si.dwFlags = STARTF_USESHOWWINDOW;
    //si.wShowWindow = SW_HIDE;

    // �������� ��Ӵ˻�������
    // set SDL_AUDIODRIVER=directsound
    //if (!SetEnvironmentVariable(L"SDL_AUDIODRIVER", L"directsound")) 
    //{
    //   printf("SetEnvironmentVariable failed (%d)\n", GetLastError()); 
    //   return FALSE;
    //}

    // �������ҵ�����ffplay��·����Ҫ����ʵ�������
    if (CreateProcess(
        szAppName,
        (LPWSTR)lpParameter,
        NULL, NULL, FALSE, 0, NULL, NULL,
        &si, &pi))
    {
        Sleep(200); // �ȴ���Ƶ������������ɡ�����ѭ����ȡ���ڳߴ�������Sleep()

        // Wait until child process exits.
        //WaitForSingleObject( pi.hProcess, INFINITE );

        // Close process and thread handles. 
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );

        HWND hProgman = FindWindow(L"Progman", 0);				// �ҵ�PM����
        SendMessageTimeout(hProgman, 0x52C, 0, 0, 0, 100, 0);	// ������������Ϣ
        HWND hFfplay = FindWindow(L"SDL_app", 0);				// �ҵ���Ƶ����
        SetParent(hFfplay, hProgman);							// ����Ƶ��������ΪPM���Ӵ���
        EnumWindows(EnumWindowsProc, 0);						// �ҵ��ڶ���WorkerW���ڲ�������
    }

    return 0;
}
