// WideHot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

enum HKS {
	HK_L, HK_M, HK_R,
	HK_L2, HK_M2, HK_R2,
	HK_LS, HK_MS, HK_RS,
	HK_COUNT
};

struct Hotkey {
	HKS id;
	int key;
};

int _cdecl _tmain(int argc,	TCHAR *argv[])
{
	// define the hotkeys

	const std::array<Hotkey, HK_COUNT> hotkeys{
		Hotkey{ HK_L, VK_NUMPAD4 },
		Hotkey{ HK_M, VK_NUMPAD5 },
		Hotkey{ HK_R, VK_NUMPAD6 },
		Hotkey{ HK_L2, VK_NUMPAD7 },
		Hotkey{ HK_M2, VK_NUMPAD8 },
		Hotkey{ HK_R2, VK_NUMPAD9 },
		Hotkey{ HK_LS, VK_NUMPAD1 },
		Hotkey{ HK_MS, VK_NUMPAD2 },
		Hotkey{ HK_RS, VK_NUMPAD3 },
	};

	for(auto hk : hotkeys) {
		if(!RegisterHotKey(NULL, hk.id, MOD_WIN | MOD_NOREPEAT, hk.key)) {
			_tprintf(_T("Failed to register Hotkey %d\n"), hk.id);
			_tprintf(_T("Probably there is already an instance running.\n"));
			Sleep(10000);
			exit(-1);
		}
	}

	// get the work area

	RECT workarea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);
	auto workWidth = workarea.right - workarea.left;

	// if everything went well, free the console

	FreeConsole();

	MSG msg = { 0 };
	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		if(msg.message == WM_HOTKEY) {
			auto fgw = GetForegroundWindow();
			if(fgw == nullptr) continue;

			// get the maximized size

			ShowWindow(fgw, SW_MINIMIZE);
			ShowWindow(fgw, SW_MAXIMIZE);

			RECT maxRect;
			GetWindowRect(fgw, &maxRect);

			// calculate the new width/offset based on the numpad key

			int width = workWidth / 3;
			int offset = width * msg.wParam;
			if(msg.wParam >= HK_L2 && msg.wParam < HK_LS) {
				offset = width/2 * (msg.wParam - HK_L2);
				width *= 2;
			}
			else if(msg.wParam >= HK_LS) {
				width /= 2;
				offset = 0;
				if(msg.wParam == HK_MS) offset = (int)(2.5 * width);
				if(msg.wParam == HK_RS) offset = 5 * width;
			}

			SetWindowPos(fgw, HWND_TOP, offset + maxRect.left, maxRect.top, width - maxRect.left * 2, maxRect.bottom - maxRect.top, 0);

			// Workarounds
			bool workaround = false;

			// Adobe
			TCHAR str[255];
			GetWindowText(fgw, str, 255);
			std::wstring title(str);
			if(title.find(_T("Adobe")) != std::wstring::npos) {
				workaround = true;
			}

			// Electron 
			RECT newRect;
			GetWindowRect(fgw, &newRect);
			if(EqualRect(&maxRect, &newRect)) {
				workaround = true;
			}

			if(workaround) {
				ShowWindow(fgw, SW_SHOWNORMAL);
				SetWindowPos(fgw, HWND_TOP, offset + maxRect.left, maxRect.top, width - maxRect.left * 2, maxRect.bottom - maxRect.top, 0);
			}
			
			// Notepad focus workaround
			SetForegroundWindow(fgw);
		}
	}

	return 0;
}