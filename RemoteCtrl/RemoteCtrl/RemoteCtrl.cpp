// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#pragma comment(linker,"/subsystem:windows /entry:WinMainCRTStartup")
//#pragma comment(linker,"/subsystem:windows /entry:mainCRTStartup")
//#pragma comment(linker,"/subsystem:console /entry:mainCRTStartup")
//#pragma comment(linker,"/subsystem:console /entry:WinMainCRTStartup")

// 唯一的应用程序对象
//分支001

CWinApp theApp;

using namespace std;
//开机启动的时候 程序的权限丝跟随启动用户的
// 如果权限不同将导致启动失败
//开机启动对环境变量有影响 如果依赖DLL则可能启动失败
//复制这些dll到system35或者sysWOW64下边

void WriteRegisterTable(const CString& strPath)
{
	CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	char sPath[MAX_PATH] = "";
	char sSys[MAX_PATH] = "";
	std::string strExe = "\\RemoteCtrl.exe ";
	GetCurrentDirectoryA(MAX_PATH, sPath);
	GetSystemDirectoryA(sSys, sizeof(sSys));
	std::string strCmd = "mklink " + std::string(sSys) + strExe + std::string(sPath) + strExe;
	int ret = system(strCmd.c_str());
	HKEY hKey = NULL;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	if (ret != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		::exit(0);
	}
	ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
	if (ret != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		::exit(0);
	}
	RegCloseKey(hKey);
}

void WriteStartupDir(const CString& strPath)
{
	CString strCmd = GetCommandLine();
	strCmd.Replace(_T("\""), _T(""));
	BOOL ret = CopyFile(strCmd, strPath, FALSE);
	if (ret == FALSE)
	{
		MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		::exit(0);
	}
}

void ChooseAutoInvoke()
{
	//CString strPath =(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
	CString strPath = _T("C:\\Users\\21041\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe");
	if (PathFileExists(strPath))
	{
		return;
	}
	CString strInfo = _T("该程序只允许用于合法的用途\n");
	strInfo += _T("继续运行该程序将使该机器处于被监控状态\n");
	strInfo += _T("如果你不希望这样请按取消按钮退出程序\n");
	strInfo += _T("按下是按钮该程序将复制到你的机器，并随机器启动而自动运行\n");
	strInfo += _T("按下否按钮该程序将只运行一次，不会在系统中留下任何东西\n");
	int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
	if (ret == IDYES)
	{
		//WriteRegisterTable();
		WriteStartupDir(strPath);
	}
	else if (ret == IDCANCEL)
	{
		::exit(0);
	}
	return;
}

void ShowError()
{
	//LPWSTR lpMessageBuf = NULL;
	LPSTR lpMessageBuf = NULL;
	//strerror(errno); 标准c语言库
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
		NULL, GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMessageBuf, 0, NULL);
	OutputDebugString(lpMessageBuf);
	MessageBox(NULL, lpMessageBuf, _T("发生错误"), 0);
	LocalFree(lpMessageBuf);
}

bool IsAdmin()
{
	HANDLE hToken = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		ShowError();
		return false;
	}
	TOKEN_ELEVATION eve;
	DWORD len = 0;
	if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE)
	{
		ShowError();
		return false;
	}
	CloseHandle(hToken);
	if (len == sizeof(eve))
	{
		return eve.TokenIsElevated;
	}
	printf("length of tokeninformation is %d\r\n", len);
	return false;
}

void RunAsAdmin()
{
	//TOOD 获取管理员权限、使用管理员权限创建进程
	HANDLE hToken = NULL;
	BOOL ret = LogonUser("Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
	if (!ret)
	{
		ShowError();
		MessageBox(NULL, _T("登录错误"), _T("程序错误"), 0);
		::exit(0);
	}
	OutputDebugString("Logon administrator success!\r\n");
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	TCHAR sPath[MAX_PATH] = _T("");
	GetCurrentDirectory(MAX_PATH, sPath);
	CString strCmd = sPath;
	strCmd += _T("\\RemoteCtrl.exe");
	/*ret = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, NULL, (LPWSTR)(LPCTSTR)strCmd,
		CREATE_UNICODE_ENVIRONMENT, NULL, NULL,(LPSTARTUPINFOW)& si, &pi);*/
	ret = CreateProcessWithLogonW((LPCWSTR)_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL,
		(LPWSTR)(LPCTSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, (LPSTARTUPINFOW)&si, &pi);
	CloseHandle(hToken);
	if (!ret)
	{
		ShowError();
		MessageBox(NULL, strCmd, _T("创建进程失败"), 0);
		::exit(0);
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}



int main()
{
	int nRetCode = 0;
	
	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		
		// 初始化 MFC 并在失败时显示错误   
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC初始化失败\n");
			nRetCode = 1;
		}
		else
		{
			if (IsAdmin())
			{
				OutputDebugString("current is run as administrator!\r\n");
				MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
			}
			else
			{
				OutputDebugString("current is run as normal user!\r\n");
				MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
				RunAsAdmin();
				return nRetCode;
			}
			CCommand cmd;
			ChooseAutoInvoke();
			int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
			switch (ret)
			{
			case -1:
				MessageBox(NULL, _T("网络初始化异常，未能正常初始化，请检查网络状态！"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
				::exit(0);
				break;
			case -2:
				MessageBox(NULL, _T("多次无法正常接入用户，结束程序"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
				::exit(0);
				break;
			}
		}
	}
	else
	{
		// TODO: 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}

	return nRetCode;
}
