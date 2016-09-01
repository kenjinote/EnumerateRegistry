#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

TCHAR szClassName[] = TEXT("Window");
TCHAR szText[1024];

void QueryKey(HWND hEdit, HKEY hKey)
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

								 // Enumerate the subkeys, until RegEnumKeyEx fails.

	if (cSubKeys)
	{
		wsprintf(szText, TEXT("Number of subkeys: %d\r\n"), cSubKeys);
		SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szText);

		for (i = 0; i<cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i,
				achKey,
				&cbName,
				NULL,
				NULL,
				NULL,
				&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS)
			{
				wsprintf(szText, TEXT("(%d) %s\r\n"), i + 1, achKey);
				SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szText);
			}
		}
	}

	// Enumerate the key values. 

	if (cValues)
	{
		wsprintf(szText, TEXT("Number of values: %d\r\n"), cValues);
		SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szText);

		for (i = 0, retCode = ERROR_SUCCESS; i<cValues; i++)
		{
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumValue(hKey, i,
				achValue,
				&cchValue,
				NULL,
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS)
			{
				wsprintf(szText, TEXT("(%d) %s\r\n"), i + 1, achValue);
				SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szText);
			}
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1;
	static HWND hEdit2;
	static HWND hButton;
	switch (msg)
	{
	case WM_CREATE:
		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("SOFTWARE"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("取得"), WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit1, 10, 10, LOWORD(lParam) - 94, 32, TRUE);
		MoveWindow(hButton, LOWORD(lParam) - 74, 10, 64, 32, TRUE);
		MoveWindow(hEdit2, 10, 50, LOWORD(lParam) - 20, HIWORD(lParam) - 60, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			SetWindowText(hEdit2, 0);
			const int nSize = GetWindowTextLength(hEdit1);
			LPTSTR lpszText = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR) * (nSize + 1));
			GetWindowText(hEdit1, lpszText, nSize + 1);
			HKEY hTestKey;
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				lpszText,
				0,
				KEY_READ,
				&hTestKey) == ERROR_SUCCESS
				)
			{
				QueryKey(hEdit2, hTestKey);
			}
			RegCloseKey(hTestKey);
			GlobalFree(lpszText);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		0,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		0,
		0,
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("指定されたレジストリのサブキーと値を列挙"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
