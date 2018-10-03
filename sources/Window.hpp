﻿/// ウィンドウに関する操作ができます。
class Window
{
public:
    /// 継承するとウィンドウのメッセージを受け取れます。
    class Proceedable
    {
    public:
        virtual void OnProceedMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    };
    /// ウィンドウの種類
    enum class Mode
    {
        Windowed,   ///< ウィンドウモード
        FullScreen, ///< フルスクリーンモード
    };

    /// ウィンドウのハンドルを取得します。
    static HWND GetHandle()
    {
        return Get().handle;
    }
    /// ウィンドウのサイズをピクセル単位で取得します。
    static DirectX::XMINT2 GetSize()
    {
        RECT clientRect = {};
        GetClientRect(Get().handle, &clientRect);

        return DirectX::XMINT2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
    }
    /// ウィンドウのサイズをピクセル単位で設定します。
    static void SetSize(int width, int height)
    {
        RECT windowRect = {};
        RECT clientRect = {};
        GetWindowRect(Get().handle, &windowRect);
        GetClientRect(Get().handle, &clientRect);

        int w = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left) + width;
        int h = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top) + height;
        int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

        SetWindowPos(Get().handle, nullptr, x, y, w, h, SWP_FRAMECHANGED);
    }
    /// ウィンドウのタイトルを取得します。
    static wchar_t* const GetTitle()
    {
        wchar_t* title = nullptr;
        GetWindowTextW(Get().handle, title, GetWindowTextLengthW(Get().handle));
        return title;
    }
    /// ウィンドウのタイトルを設定します。
    static void SetTitle(const wchar_t* const title)
    {
        SetWindowTextW(Get().handle, title);
    }
    /// ウィンドウモードを取得します。
    static Mode GetMode()
    {
        return Get().mode;
    }
    /// ウィンドウモードを設定します。
    static void SetMode(Mode mode)
    {
        if (Get().mode == mode)
            return;

        Get().mode = mode;

        static DirectX::XMINT2 size = GetSize();
        if (mode == Mode::FullScreen)
        {
            size = GetSize();
            int w = GetSystemMetrics(SM_CXSCREEN);
            int h = GetSystemMetrics(SM_CYSCREEN);
            SetWindowLongPtrW(Get().handle, GWL_STYLE, WS_VISIBLE | WS_POPUP);
            SetWindowPos(Get().handle, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
        }
        else
        {
            SetWindowLongPtrW(Get().handle, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);
            SetWindowPos(Get().handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
            SetSize(size.x, size.y);
        }
    }
    /// ウィンドウモードを切り替えます。
    static void ToggleMode()
    {
        if (Get().mode == Mode::Windowed)
            SetMode(Mode::FullScreen);
        else
            SetMode(Mode::Windowed);
    }
    static bool Update()
    {
        MSG message = {};

        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
                return false;

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        PostMessageW(Get().handle, WM_APP, 0, 0);

        if (GetSize().x <= 0.0f || GetSize().y <= 0.0f)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return true;
    }
    /// ウィンドウプロシージャを追加します。
    static void AddProcedure(Proceedable* const procedure)
    {
        Get().procedures.push_back(procedure);
    }
    /// ウィンドウプロシージャを削除します。
    static void RemoveProcedure(Proceedable* const procedure)
    {
        Get().procedures.remove(procedure);
    }

private:
    struct Property
    {
        const wchar_t* className = L"XLibrary11";
        HWND handle;
        Mode mode = Mode::Windowed;
        std::list<Proceedable*> procedures;
    };

    static Property& Get()
    {
        static std::unique_ptr<Property> prop;

        if (prop == nullptr)
        {
            prop.reset(new Property());

            InitializeApplication();

            HINSTANCE instance = GetModuleHandleW(nullptr);

            WNDCLASSW windowClass = {};
            windowClass.lpfnWndProc = ProceedMessage;
            windowClass.hInstance = instance;
            windowClass.hCursor = (HCURSOR)LoadImageW(nullptr, MAKEINTRESOURCEW(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED);
            windowClass.lpszClassName = Get().className;
            RegisterClassW(&windowClass);

            Get().handle = CreateWindowW(Get().className, Get().className, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);

            SetSize(640, 480);

            ShowWindow(Get().handle, SW_SHOWNORMAL);
        }

        return *prop;
    }
    static LRESULT CALLBACK ProceedMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        for (Proceedable* procedure : Get().procedures)
        {
            procedure->OnProceedMessage(window, message, wParam, lParam);
        }

        if (message == WM_DESTROY)
            PostQuitMessage(0);

        return DefWindowProcW(window, message, wParam, lParam);
    }
};