#include "resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int           ReadStatus(HWND hwnd);
int           WriteBytes(HWND hwnd, int online, int fps);
int           GetProcess(void);
void          GetDSVersion(HWND hwnd);

DWORD  pid;
DWORD  access = PROCESS_VM_READ | 
                PROCESS_QUERY_INFORMATION | 
                PROCESS_VM_WRITE | 
                PROCESS_VM_OPERATION;
HANDLE proc;
DWORD  baseAddress = 0x13784A0;
DWORD  offsetOnline = 0xB4D;
DWORD  offsetFPS = 0xB4C;
DWORD  versionAddress = 0x400080;
int    versionOffset = 0;

enum {
    CANT_ENUM_PROCESSES = -2,
    INVALID_HANDLE = -1,
    DS_NOT_FOUND = 0,
    DS_FOUND = 1,
    ONLINE = 1,
    OFFLINE = 2,
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    int try_write, status;

    switch (Message) {
    case WM_INITDIALOG:
	SetDlgItemText(hwnd, IDT_INFO, "Refresh to check status!");
	HICON hIcon, hIconSm;
	hIcon   = (HICON) LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) hIcon);
	hIconSm = (HICON) LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) hIconSm);
        break;
    case WM_COMMAND:
	switch (LOWORD(wParam)) {
      	case IDC_REFRESH:
	    status = ReadStatus(hwnd);
	    if (status == OFFLINE)
		SetDlgItemText(hwnd, IDT_INFO, "Status: OFFLINE");
	    else if (status == ONLINE)
		SetDlgItemText(hwnd, IDT_INFO, "Status: ONLINE");
	    else if (status == DS_NOT_FOUND) {
		SetDlgItemText(hwnd, IDT_INFO, "Dark Souls not found!");
		SetDlgItemText(hwnd, IDT_VERSION, "Online Toggle");
	    } else if (status < DS_NOT_FOUND)
		SetDlgItemText(hwnd, IDT_INFO, "Error finding process!");
	    break;
	case IDC_TOGGLEON:
	    try_write = WriteBytes(hwnd, 1, 0);
	    if (try_write == DS_FOUND)
		SetDlgItemText(hwnd, IDT_INFO, "Status: ONLINE");
	    else if (try_write == DS_NOT_FOUND) {
		SetDlgItemText(hwnd, IDT_INFO, "Dark Souls not found!");
		SetDlgItemText(hwnd, IDT_VERSION, "Online Toggle");
	    } else if (try_write < DS_NOT_FOUND)
		SetDlgItemText(hwnd, IDT_INFO, "Error finding process!");
	    break;
	case IDC_TOGGLEOFF:
    	    try_write = WriteBytes(hwnd, 0, 1);
	    if (try_write == DS_FOUND)
		SetDlgItemText(hwnd, IDT_INFO, "Status: OFFLINE");
	    else if (try_write == DS_NOT_FOUND) {
		SetDlgItemText(hwnd, IDT_INFO, "Dark Souls not found!");
		SetDlgItemText(hwnd, IDT_VERSION, "Online Toggle");
	    } else if (try_write < DS_NOT_FOUND)
		SetDlgItemText(hwnd, IDT_INFO, "Error finding process!");
	    break;
	}
        break;
    case WM_CLOSE:
	EndDialog(hwnd, 0);
    default:
	return FALSE;
    }
    return TRUE;
}

int GetProcess(void)
{

    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        CloseHandle(hProcessSnap);
        return INVALID_HANDLE;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return CANT_ENUM_PROCESSES;
    }

    do {
        if (_stricmp(pe32.szExeFile, "DARKSOULS.exe") == 0) {
            pid = pe32.th32ProcessID;
            CloseHandle(hProcessSnap);
            return DS_FOUND;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return DS_NOT_FOUND;
}

int ReadStatus(HWND hwnd)
{
    int isopen = GetProcess();
    if (isopen == DS_FOUND) {
        GetDSVersion(hwnd);
        int online = 0;
        void *tmpPtr;
        proc = OpenProcess(access, FALSE, pid);
        ReadProcessMemory(proc, (void*) (baseAddress + versionOffset), &tmpPtr, 4, NULL);
        ReadProcessMemory(proc, tmpPtr+offsetOnline, &online, 1, NULL);
        CloseHandle(proc);
        return (online) ? ONLINE : OFFLINE;
    } else
        return isopen;
}

int WriteBytes(HWND hwnd, int online, int fps)
{

    int isopen = GetProcess();
    if (isopen == DS_FOUND) {
        GetDSVersion(hwnd);
        void *tmpPtr;
        proc = OpenProcess(access, FALSE, pid);
        ReadProcessMemory(proc, (void*) (baseAddress + versionOffset), &tmpPtr, 4, NULL);
        WriteProcessMemory(proc, tmpPtr+offsetOnline, &online, 1, NULL);
        WriteProcessMemory(proc, tmpPtr+offsetFPS, &fps, 1, NULL);
        CloseHandle(proc);
        return DS_FOUND;
    } else
        return isopen;
}

void GetDSVersion(HWND hwnd)
{
    versionOffset = 0;
    proc = OpenProcess(access, FALSE, pid);
    unsigned int versionNumber;
    ReadProcessMemory(proc, (void*) versionAddress, &versionNumber, 4, NULL);
    CloseHandle(proc);
    switch (versionNumber) {
    case 0xCE9634B4:
	versionOffset = 0x41C0;
	SetDlgItemText(hwnd, IDT_VERSION, "Dark Souls (Debug Version)");
	break;
    case 0xE91B11E2:
	versionOffset = -0x3000;
	SetDlgItemText(hwnd, IDT_VERSION, "Dark Souls (Steamworks Beta)");
	break;
    case 0xFC293654:
	versionOffset = 0;
	SetDlgItemText(hwnd, IDT_VERSION, "Dark Souls (Latest Release Ver.)");
	break;
    }
}
