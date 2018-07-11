class Input
{
public:
    static bool GetKey(int VK_CODE)
    {
        return Get()._keyState[VK_CODE] & 0x80;
    }
    static bool GetKeyUp(int VK_CODE)
    {
        return !(Get()._keyState[VK_CODE] & 0x80) && (Get()._preKeyState[VK_CODE] & 0x80);
    }
    static bool GetKeyDown(int VK_CODE)
    {
        return (Get()._keyState[VK_CODE] & 0x80) && !(Get()._preKeyState[VK_CODE] & 0x80);
    }
    static Float2 GetMousePosition()
    {
        return Get()._mousePosition;
    }
    static int GetMouseWheel()
    {
        return Get()._mouseWheel;
    }
    static void SetMousePosition(float x, float y)
    {
        if (GetActiveWindow() != Window::GetHandle())
            return;

        POINT point = {};
        point.x = (int)x + Window::GetSize().x / 2;
        point.y = (int)-y + Window::GetSize().y / 2;
        ClientToScreen(Window::GetHandle(), &point);
        SetCursorPos(point.x, point.y);

        Get()._mousePosition.x = x;
        Get()._mousePosition.y = y;
    }
    static void SetShowCursor(bool isShowCursor)
    {
        if (Get()._isShowCursor == isShowCursor)
            return;

        Get()._isShowCursor = isShowCursor;
        ShowCursor(isShowCursor);
    }
    static void Update()
    {
        Get()._mouseWheel = 0;

        POINT point = {};
        GetCursorPos(&point);
        ScreenToClient(Window::GetHandle(), &point);

        Get()._mousePosition.x = (float)point.x - Window::GetSize().x / 2;
        Get()._mousePosition.y = (float)-point.y + Window::GetSize().y / 2;

        for (int i = 0; i < 256; i++)
        {
            Get()._preKeyState[i] = Get()._keyState[i];
        }

        GetKeyboardState(Get()._keyState);
    }

private:
    struct Property : public Window::Proceedable
    {
        Float2 _mousePosition;
        int _mouseWheel = 0;
        BYTE _preKeyState[256];
        BYTE _keyState[256];
        bool _isShowCursor = true;

        void OnProceedMessage(HWND, UINT message, WPARAM wParam, LPARAM) override
        {
            if (message == WM_MOUSEWHEEL)
            {
                _mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
            }
        }
    };

    static Property& Get()
    {
        static std::unique_ptr<Property> prop;

        if (prop == nullptr)
        {
            prop.reset(new Property());

            InitializeApplication();
            Window::AddProcedure(&Get());

            Update();
        }

        return *prop;
    }
};