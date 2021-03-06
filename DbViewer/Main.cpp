#define _WIN32_WINNT 0x0501
#define WINVER 0x0501
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <gdiplus.h>
#include <iostream>
#include <clocale>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <locale>
using namespace Gdiplus;
using namespace std;
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Ole32.lib")

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.Lib")
#pragma comment(lib, "gdi32.Lib")

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "sqlite3.h"
#include "DbHelper.h"
#include "StringHelper.h"

#define IDB_Load     3301  
#define IDB_SQL     3302 
#define IDB_ExportCsv     3303
#define IDB_ExportHtml     3304



HINSTANCE g_hInst;

HWND hWndListView;
HWND pathEdit;
HWND keyEdit;
HWND sqlEdit;
HWND sqlListView;

HWND hwndTab;
HWND loadButton;
HWND execButton;


DbHelper dbHelper;

LRESULT CALLBACK WindowFunc(HWND,UINT,WPARAM,LPARAM);

TCHAR szWinName[] = TEXT("Win32Main");

int WINAPI WinMain (HINSTANCE hThisInst,
					HINSTANCE hPrevInstance,
					LPSTR lpszArgument,
					int nWinMode)
{
	HRESULT hRes = ::CoInitialize(NULL);
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken; 
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	setlocale(LC_ALL, "en_US.utf8");

	int ret;
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wcl;
	INITCOMMONCONTROLSEX ic;

	g_hInst = hThisInst;
	ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ic.dwICC = 0x00004000|ICC_LISTVIEW_CLASSES;//ICC_STANDARD_CLASSES|ICC_BAR_CLASSES;
	ret = InitCommonControlsEx(&ic);
	wcl.cbSize = sizeof(WNDCLASSEX);
	wcl.hInstance = hThisInst;
	wcl.lpszClassName = szWinName;
	wcl.lpfnWndProc = WindowFunc;
	wcl.style = CS_DBLCLKS;//CS_HREDRAW|CS_VREDRAW
	wcl.hIcon = LoadIcon(hThisInst,MAKEINTRESOURCE(IDI_APP));
	wcl.hIconSm = NULL;
	wcl.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	if(!RegisterClassEx(&wcl)) return 0;
	HWND parent = NULL;
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		szWinName,
		TEXT("Database Viewer"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		640,
		parent,//HWND_DESKTOP
		NULL,
		hThisInst,
		NULL
		);

	::SetLayeredWindowAttributes( hwnd, RGB(0,0,255), 222, ULW_ALPHA|ULW_COLORKEY);
	ShowWindow(hwnd,SW_SHOW);
	UpdateWindow(hwnd);

	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);
	::CoUninitialize();
	return msg.wParam;
}

LRESULT CALLBACK tabSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	RECT rcClient; 
	GetClientRect(hWnd, &rcClient); 
	int w = rcClient.right-rcClient.left;
	int h = rcClient.bottom - rcClient.top;

	if (uMsg == WM_SIZE)
	{
		SetWindowPos(sqlEdit,NULL,35, 30, w-55-40, 50, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		SetWindowPos(execButton,NULL,w-60,         // x position 
			30,         // y position 
			50,        // Button width
			40, SWP_NOACTIVATE|SWP_NOZORDER);
		SetWindowPos(sqlListView,NULL,5, 90, w-15, h-100, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);


	}
	if (uMsg== WM_CONTEXTMENU) 
	{
		if ((HWND)wParam == sqlListView) {
			POINT cursor;
			GetCursorPos(&cursor);
			HMENU hPopupMenu = CreatePopupMenu();
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_ExportHtml, L"View Html");
			//InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_ExportCsv, L"Export Csv");
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);
		}
	}
	if ((uMsg == WM_COMMAND))
	{
		// LOWORD(wParam) is the ID, and lParam is the HWND,
		switch(LOWORD(wParam))  
		{  

		case IDB_SQL:
			{


				TCHAR text[255];
				char mbstr[255];
				GetWindowText(sqlEdit,text,255);
				std::wcstombs(mbstr, text, 255);
				vector<vector<string>> tables = dbHelper.QueryTable(mbstr);
				if (tables.size() > 0)
				{
					HWND head = ListView_GetHeader(sqlListView);
					while (Header_GetItemCount(head) > 0)
					{
						ListView_DeleteColumn(sqlListView,0);
					}
					ListView_DeleteAllItems(sqlListView);
					vector<string> cols = tables[0];
					for (int i = 0; i < cols.size(); i++)
					{
						LVCOLUMN vcl;  
						vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;  
						wchar_t name[255];
						//mbstowcs(name, cols[i].c_str(),255);
						StringHelper::Char2Wchar(cols[i].c_str(),name);
						vcl.pszText = name;  
						vcl.cx = w/cols.size();  
						vcl.iSubItem = i;
						ListView_InsertColumn(sqlListView, i, &vcl); 
					}
				}
				wchar_t name[65535];
				for (int i = 1; i < tables.size(); i++)
				{
					vector<string> data = tables[i];
					for (int j = 0; j < data.size(); j++)
					{
						LVITEM item;
						item.mask = LVIF_TEXT;
						item.cchTextMax = 6;
						//mbstowcs(name, data[j].c_str(),65535);
						StringHelper::Char2Wchar(data[j].c_str(),name);
						item.pszText = name;
						item.iItem = i - 1;
						item.iSubItem = j;
						if (j == 0) 
						{
							ListView_InsertItem(sqlListView, &item);
						}
						else 
						{
							ListView_SetItem(sqlListView, &item);
						}
					}
				}

			}

			break;
		case IDB_ExportHtml:
			{
				TCHAR text[255];
				char mbstr[255];
				GetWindowText(sqlEdit,text,255);
				std::wcstombs(mbstr, text, 255);
				vector<vector<string>> tables = dbHelper.QueryTable(mbstr);
				if (tables.size() > 0)
				{
					wstring data = StringHelper::ExportHtml(tables);

					TCHAR szTempFileName[MAX_PATH];  
					TCHAR lpTempPathBuffer[MAX_PATH];
					GetTempPath(MAX_PATH,          // length of the buffer
						lpTempPathBuffer);
					GetTempFileName(lpTempPathBuffer, // directory for tmp files
						TEXT("export"),     // temp file name prefix 
						0,                // create unique name 
						szTempFileName);  // buffer for name 
					wcscat(szTempFileName,L".html");
					std::wofstream f(szTempFileName);
					//typedef std::codecvt_facet<wchar_t, char, mbstate_t> Cvt;
					locale utf8locale(locale(), new codecvt_byname<wchar_t, char, mbstate_t> (""));
					f.imbue(utf8locale);
					f << data;
					f.close();
					ShellExecute(NULL, L"open", szTempFileName,
						NULL, NULL, SW_SHOWNORMAL);
				}
			}
			break;
		default:
			break;
		}

	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowFunc(HWND hwnd,UINT message,WPARAM
							wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	RECT rcClient; 
	GetClientRect(hwnd, &rcClient); 
	int w = rcClient.right-rcClient.left;
	int h = rcClient.bottom - rcClient.top;

	switch(message){
	case WM_CREATE:
		{


			HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			{
				HWND hwndStatic = CreateWindow(WC_STATIC, L"db path", 
					WS_CHILD | WS_VISIBLE , 
					0, 5, 50, 20,        // Position and dimensions; example only.
					hwnd, NULL, g_hInst,    // g_hInst is the global instance handle
					NULL); 
				SendMessage(hwndStatic, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
				hwndStatic = CreateWindow(WC_STATIC, L"  key", 
					WS_CHILD | WS_VISIBLE , 
					0, 30, 50, 20,        // Position and dimensions; example only.
					hwnd, NULL, g_hInst,    // g_hInst is the global instance handle
					NULL); 
				SendMessage(hwndStatic, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			}
			{
				pathEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
					WS_CHILD | WS_VISIBLE, 
					55, 5, w-55-5-50, 20, hwnd, NULL, g_hInst, NULL);
				SendMessage(pathEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			}
			{
				keyEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
					WS_CHILD | WS_VISIBLE, 
					55, 30, w-55-5-50, 20, hwnd, NULL, g_hInst, NULL);
				SendMessage(keyEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			}
			{
				loadButton = CreateWindow( 
					L"BUTTON",  // Predefined class; Unicode assumed 
					L"Load",      // Button text 
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
					w-50,         // x position 
					10,         // y position 
					50,        // Button width
					40,        // Button height
					hwnd,     // Parent window
					(HMENU)IDB_Load,       // No menu.
					g_hInst, 
					NULL);      // Pointer not needed.
				SendMessage(loadButton, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			}
			{
				hWndListView = CreateWindow(WC_LISTVIEW, 
					L"",
					WS_CHILD | WS_VISIBLE| LVS_REPORT | LVS_EDITLABELS| WS_BORDER,
					0, 60,
					145,
					h-60,
					hwnd,
					NULL,
					g_hInst,
					NULL);
				LVCOLUMN vcl;  
				vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;  
				// 第一列  
				vcl.pszText = L"tables";//列标题  
				vcl.cx = 145;//列宽  
				vcl.iSubItem = 0;//子项索引，第一列无子项  
				ListView_InsertColumn(hWndListView, 0, &vcl);  
				//char output[100];
				//log(itoa((int)hWndListView,output,10));
				SendMessage(hWndListView, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			}
			{

				TCITEM tie; 
				hwndTab = CreateWindow(WC_TABCONTROL, L"", 
					WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
					150, 60, w-150-5, h-60, 
					hwnd, NULL, g_hInst, NULL); 

				// Add tabs for each day of the week. 
				tie.mask = TCIF_TEXT | TCIF_IMAGE; 
				tie.iImage = -1; 
				//tie.pszText = L"Top 100"; 
				//if (TabCtrl_InsertItem(hwndTab, 0, &tie) == -1) 
				//{ 
				//	DestroyWindow(hwndTab); 
				//} 
				tie.pszText = L"SQL"; 
				if (TabCtrl_InsertItem(hwndTab, 1, &tie) == -1) 
				{ 
					DestroyWindow(hwndTab); 
				} 
				/*tie.pszText = L"Struct"; 
				if (TabCtrl_InsertItem(hwndTab, 2, &tie) == -1) 
				{ 
				DestroyWindow(hwndTab); 
				} */
				SendMessage(hwndTab, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
				SetWindowSubclass(hwndTab, &tabSubClassProc, 0, 0);
				{
					HWND hwndStatic = CreateWindow(WC_STATIC, L"sql", 
						WS_CHILD | WS_VISIBLE, 
						5, 30, 30, 20,        // Position and dimensions; example only.
						hwndTab, NULL, g_hInst,    // g_hInst is the global instance handle
						NULL); 
					SendMessage(hwndStatic, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

					sqlEdit = CreateWindowEx(
						0, L"EDIT",   // predefined class 
						NULL,         // no window title 
						WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER|
						ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 
						35, 30, w-55-40-150, 50,   // set size in WM_SIZE message 
						hwndTab,         // parent window 
						NULL,   // edit control ID 
						g_hInst, 
						NULL);        // pointer not needed 
					SendMessage(sqlEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

					execButton = CreateWindow( 
						L"BUTTON",  // Predefined class; Unicode assumed 
						L"Execute",      // Button text 
						WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
						w-60-150,         // x position 
						30,         // y position 
						50,        // Button width
						40,        // Button height
						hwndTab,     // Parent window
						(HMENU)IDB_SQL,       // No menu.
						g_hInst, 
						NULL);      // Pointer not needed.
					SendMessage(execButton, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

					sqlListView = CreateWindow(WC_LISTVIEW, 
						L"",
						WS_CHILD | WS_VISIBLE| LVS_REPORT | LVS_EDITLABELS | WS_BORDER,
						5, 90, w-150-15, h-150,
						hwndTab,
						NULL,
						g_hInst,
						NULL);
					SendMessage(sqlListView, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
					ListView_SetExtendedListViewStyleEx(sqlListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
				}
			}
		}
		return 0;
	case WM_SIZE:
		{

			RECT rcClient; 
			GetClientRect(hwnd, &rcClient); 
			int w = rcClient.right-rcClient.left;
			int h = rcClient.bottom - rcClient.top;

			{
				SetWindowPos(pathEdit,NULL,55, 5, w-55-5-50, 20, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);

			}
			{
				SetWindowPos(keyEdit,NULL,55, 30, w-55-5-50, 20, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);

			}
			{
				SetWindowPos(loadButton,NULL,w-50,         // x position 
					10,         // y position 
					50,        // Button width
					40, SWP_NOACTIVATE|SWP_NOZORDER);
			}
			{


				SetWindowPos(hWndListView,NULL,0, 60,
					145,
					h-60, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
			}
			{
				SetWindowPos(hwndTab,NULL,150, 60, w-150-5, h-60, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);


			}
		}
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		{
			// Create an off-screen DC for double-buffering
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbmMem = CreateCompatibleBitmap(hdc, w, h);

			HANDLE hOld   = SelectObject(hdcMem, hbmMem);
			{
				Graphics g(hdcMem);
				SolidBrush blueBrush( Color::White ) ;
				g.FillRectangle( &blueBrush,0,0,w,h ) ;
			}
			// Draw into hdcMem here

			// Transfer the off-screen DC to the screen
			BitBlt(hdc, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

			// Free-up the off-screen DC
			SelectObject(hdcMem, hOld);
			DeleteObject(hbmMem);
			DeleteDC    (hdcMem);
		}

		EndPaint(hwnd, &ps);
		return 0;
	case WM_MOUSELEAVE:
		return FALSE;
	case WM_MOUSEMOVE:
		return FALSE;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))  
			{  
			case IDB_Load:  
				{
					ListView_DeleteAllItems(hWndListView);
					//MessageBox(hwnd, L"Load", L"提示", MB_OK | MB_ICONINFORMATION);
					TCHAR text[255];
					char mbstr[255];
					GetWindowText(pathEdit,text,255);
					std::wcstombs(mbstr, text, 255);
					string path = mbstr;
					GetWindowText(keyEdit,text,255);
					std::wcstombs(mbstr, text, 255);
					string key = mbstr;

					dbHelper.SetDatabase(path,key);
					vector<string> tables = dbHelper.GetTables();
					for (int i = 0; i < tables.size(); i++)
					{
						LVITEM item;
						item.mask = LVIF_TEXT;
						item.cchTextMax = 6;
						item.iSubItem = 0;

						wchar_t name[255];
						mbstowcs(name, tables[i].c_str(),255);
						item.pszText = name;
						item.iItem = i;
						ListView_InsertItem(hWndListView, &item);
					}
				}
				break; 

			default:  
				break;  
			}  

		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}
