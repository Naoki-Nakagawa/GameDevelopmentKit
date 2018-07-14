class Input
{
public:
    static bool GetKey(int VK_CODE)
    {
        return Get().keyState[VK_CODE] & 0x80;
    }
    static bool GetKeyUp(int VK_CODE)
    {
        return !(Get().keyState[VK_CODE] & 0x80) && (Get().prevKeyState[VK_CODE] & 0x80);
    }
    static bool GetKeyDown(int VK_CODE)
    {
        return (Get().keyState[VK_CODE] & 0x80) && !(Get().prevKeyState[VK_CODE] & 0x80);
    }
    static Float2 GetMousePosition()
    {
        return Get().mousePosition;
    }
    static void SetMousePosition(float x, float y)
    {
        if (GetActiveWindow() != Window::GetHandle())
            return;

        POINT point = {};
        point.x = int(x + Window::GetSize().x / 2);
        point.y = int(-y + Window::GetSize().y / 2);
        ClientToScreen(Window::GetHandle(), &point);
        SetCursorPos(point.x, point.y);

        Get().mousePosition.x = x;
        Get().mousePosition.y = y;
    }
    static int GetMouseWheel()
    {
        return Get().mouseWheel;
    }
    static void SetShowCursor(bool isShowCursor)
    {
        if (Get().isShowCursor == isShowCursor)
            return;

        Get().isShowCursor = isShowCursor;
        ShowCursor(isShowCursor);
    }
    static bool GetPadButton(int id, int XINPUT_GAMEPAD_CODE)
    {
        return Get().padState[id].Gamepad.wButtons & XINPUT_GAMEPAD_CODE;
    }
    static bool GetPadButtonUp(int id, int XINPUT_GAMEPAD_CODE)
    {
        return !(Get().padState[id].Gamepad.wButtons & XINPUT_GAMEPAD_CODE) && (Get().prevPadState[id].Gamepad.wButtons & XINPUT_GAMEPAD_CODE);
    }
    static bool GetPadButtonDown(int id, int XINPUT_GAMEPAD_CODE)
    {
        return (Get().padState[id].Gamepad.wButtons & XINPUT_GAMEPAD_CODE) && !(Get().prevPadState[id].Gamepad.wButtons & XINPUT_GAMEPAD_CODE);
    }
    static float GetPadLeftTrigger(int id)
    {
        return float(Get().padState[id].Gamepad.bLeftTrigger) / std::numeric_limits<BYTE>::max();
    }
    static float GetPadRightTrigger(int id)
    {
        return float(Get().padState[id].Gamepad.bRightTrigger) / std::numeric_limits<BYTE>::max();
    }
    static Float2 GetPadLeftThumb(int id)
    {
        static const float deadZone = float(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / std::numeric_limits<SHORT>::max();

        Float2 value;
        value.x = float(Get().padState[id].Gamepad.sThumbLX) / std::numeric_limits<SHORT>::max();
        value.y = float(Get().padState[id].Gamepad.sThumbLY) / std::numeric_limits<SHORT>::max();

        if (-deadZone < value.x && value.x < deadZone)
            value.x = 0.0f;
        if (-deadZone < value.y && value.y < deadZone)
            value.y = 0.0f;

        return value;
    }
    static Float2 GetPadRightThumb(int id)
    {
        static const float deadZone = float(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / std::numeric_limits<SHORT>::max();

        Float2 value;
        value.x = float(Get().padState[id].Gamepad.sThumbRX) / std::numeric_limits<SHORT>::max();
        value.y = float(Get().padState[id].Gamepad.sThumbRY) / std::numeric_limits<SHORT>::max();

        if (-deadZone < value.x && value.x < deadZone)
            value.x = 0.0f;
        if (-deadZone < value.y && value.y < deadZone)
            value.y = 0.0f;

        return value;
    }
    static void Update()
    {
        Get().mouseWheel = 0;

        POINT point = {};
        GetCursorPos(&point);
        ScreenToClient(Window::GetHandle(), &point);

        Get().mousePosition.x = float(point.x - Window::GetSize().x / 2);
        Get().mousePosition.y = float(-point.y + Window::GetSize().y / 2);

        for (int i = 0; i < 256; i++)
        {
            Get().prevKeyState[i] = Get().keyState[i];
        }

        GetKeyboardState(Get().keyState);

        for (int i = 0; i < 4; i++)
        {
            Get().prevPadState[i] = Get().padState[i];
            XInputGetState(i, &Get().padState[i]);
        }
    }

private:
    struct Property : public Window::Proceedable
    {
        Float2 mousePosition;
        int mouseWheel = 0;
        BYTE prevKeyState[256];
        BYTE keyState[256];
        bool isShowCursor = true;
        XINPUT_STATE prevPadState[4];
        XINPUT_STATE padState[4];

        void OnProceedMessage(HWND, UINT message, WPARAM wParam, LPARAM) override
        {
            if (message == WM_MOUSEWHEEL)
            {
                mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
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