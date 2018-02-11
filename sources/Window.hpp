﻿class Window {
	PRIVATE HWND handle;
	PRIVATE const DWORD style = WS_OVERLAPPEDWINDOW;
	PRIVATE std::vector<UINT> messages;

	PUBLIC Window() {
		HINSTANCE instance = GetModuleHandleW(nullptr);

		WNDCLASSEXW windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = Procedure;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = instance;
		windowClass.hIcon = nullptr;
		windowClass.hCursor = (HCURSOR)LoadImageW(nullptr, MAKEINTRESOURCEW(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
		windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		windowClass.lpszMenuName = nullptr;
		windowClass.lpszClassName = L"XLibrary11";
		windowClass.hIconSm = nullptr;
		RegisterClassExW(&windowClass);

		handle = CreateWindowW(L"XLibrary11", L"XLibrary11", style, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, instance, nullptr);

		SetSize(1280.0f, 720.0f);

		ShowWindow(handle, SW_SHOWNORMAL);

		SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}
	PUBLIC ~Window() {
	}
	PUBLIC HWND GetHandle() {
		return handle;
	}
	PUBLIC std::vector<UINT>& GetMessages() {
		return messages;
	}
	PUBLIC Float2 GetSize() {
		RECT clientRect = {};
		GetClientRect(handle, &clientRect);

		return Float2(static_cast<float>(clientRect.right - clientRect.left), static_cast<float>(clientRect.bottom - clientRect.top));
	}
	PUBLIC void SetSize(float width, float height) {
		RECT windowRect = {};
		RECT clientRect = {};
		GetWindowRect(handle, &windowRect);
		GetClientRect(handle, &clientRect);

		int w = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left) + static_cast<int>(width);
		int h = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top) + static_cast<int>(height);

		int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
		int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

		SetWindowPos(handle, nullptr, x, y, w, h, SWP_FRAMECHANGED);
	}
	PUBLIC wchar_t* GetTitle() {
		wchar_t* title = nullptr;
		GetWindowTextW(handle, title, GetWindowTextLengthW(handle));
		return title;
	}
	PUBLIC void SetTitle(const wchar_t* title) {
		SetWindowTextW(handle, title);
	}
	PUBLIC void SetFullScreen(bool isFullscreen) {
		static Float2 size = GetSize();

		if (isFullscreen) {
			size = GetSize();
			int w = GetSystemMetrics(SM_CXSCREEN);
			int h = GetSystemMetrics(SM_CYSCREEN);
			SetWindowLongPtrW(handle, GWL_STYLE, WS_VISIBLE | WS_POPUP);
			SetWindowPos(handle, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
		}
		else {
			SetWindowLongPtrW(handle, GWL_STYLE, WS_VISIBLE | style);
			SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
			SetSize(size.x, size.y);
		}
	}
	PUBLIC bool Update() {
		static MSG message = {};

		messages.clear();

		while (message.message != WM_QUIT) {
			if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessageW(&message);
			}
			else {
				return true;
			}
		}

		return false;
	}
	PRIVATE LRESULT Proceed(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
		messages.push_back(message);

		switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProcW(handle, message, wParam, lParam);
		}
		return 0;
	}
	PRIVATE static LRESULT WINAPI Procedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
		Window* window = (Window*)(GetWindowLongPtrW(handle, GWLP_USERDATA));
		if (window) {
			return window->Proceed(handle, message, wParam, lParam);
		}
		return DefWindowProcW(handle, message, wParam, lParam);
	}
};
