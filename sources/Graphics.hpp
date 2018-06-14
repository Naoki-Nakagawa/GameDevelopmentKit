﻿class Graphics : public Window::Proceedable
{
public:
    static ID3D11Device& GetDevice3D()
    {
        return *GetInstance().device3D.Get();
    }
    static ID3D11DeviceContext& GetContext3D()
    {
        return *GetInstance().context3D.Get();
    }
    static ID2D1Device& GetDevice2D()
    {
        return *GetInstance().device2D.Get();
    }
    static ID2D1DeviceContext& GetContext2D()
    {
        return *GetInstance().context2D.Get();
    }
    static IDXGISwapChain& GetSwapChain()
    {
        return *GetInstance().swapChain.Get();
    }
    static IWICImagingFactory& GetTextureFactory()
    {
        return *GetInstance().textureFactory.Get();
    }
    static IDWriteFactory& GetTextFactory()
    {
        return *GetInstance().textFactory.Get();
    }
    static void Update()
    {
        if (Input::GetKey(VK_MENU) && Input::GetKeyDown(VK_RETURN))
        {
            GetInstance().isFullScreen = !GetInstance().isFullScreen;
            Window::SetFullScreen(GetInstance().isFullScreen);
        }

        GetInstance().swapChain->Present(1, 0);
    }

private:
    friend std::unique_ptr<Graphics>::deleter_type;

    ComPtr<ID3D11Device> device3D = nullptr;
    ComPtr<ID3D11DeviceContext> context3D = nullptr;
    ComPtr<ID2D1Device> device2D = nullptr;
    ComPtr<ID2D1DeviceContext> context2D = nullptr;
    ComPtr<IDXGISwapChain> swapChain = nullptr;
    ComPtr<IWICImagingFactory> textureFactory = nullptr;
    ComPtr<IDWriteFactory> textFactory = nullptr;
    bool isFullScreen = false;

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    Graphics()
    {
        InitializeApplication();

        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        std::vector<D3D_DRIVER_TYPE> driverTypes
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
            D3D_DRIVER_TYPE_SOFTWARE,
        };

        std::vector<D3D_FEATURE_LEVEL> featureLevels
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        for (size_t i = 0; i < driverTypes.size(); i++)
        {
            HRESULT r = D3D11CreateDevice(nullptr, driverTypes[i], nullptr, flags, featureLevels.data(), (UINT)featureLevels.size(), D3D11_SDK_VERSION, device3D.GetAddressOf(), nullptr, context3D.GetAddressOf());

            if (SUCCEEDED(r))
                break;
        }

        context3D->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ComPtr<ID3D11BlendState> blendState = nullptr;
        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        device3D->CreateBlendState(&blendDesc, &blendState);
        context3D->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

        D2D1_FACTORY_OPTIONS options = {};
#if defined(_DEBUG)
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

        ComPtr<ID2D1Factory1> factory = nullptr;
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, factory.GetAddressOf());

        ComPtr<IDXGIDevice> device = nullptr;
        device3D->QueryInterface(device.GetAddressOf());

        factory->CreateDevice(device.Get(), device2D.GetAddressOf());

        device2D->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, context2D.GetAddressOf());

        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(textureFactory.GetAddressOf()));

        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(textFactory.GetAddressOf()));

        Create();

        Window::AddProcedure(this);
    }
    virtual ~Graphics()
    {
        Window::RemoveProcedure(this);
    }
    static Graphics& GetInstance()
    {
        static std::unique_ptr<Graphics> instance;

        if (instance == nullptr)
        {
            instance.reset(Instantiate());
        }

        return *instance;
    }
    static Graphics* Instantiate()
    {
        return new Graphics();
    }
    void Create()
    {
        ComPtr<IDXGIDevice> dxgi = nullptr;
        device3D->QueryInterface(dxgi.GetAddressOf());

        ComPtr<IDXGIAdapter> adapter = nullptr;
        dxgi->GetAdapter(&adapter);

        ComPtr<IDXGIFactory> factory = nullptr;
        adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferDesc.Width = Window::GetSize().x;
        swapChainDesc.BufferDesc.Height = Window::GetSize().y;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 1;
        swapChainDesc.OutputWindow = Window::GetHandle();
        swapChainDesc.Windowed = true;

        swapChain.Reset();
        factory->CreateSwapChain(device3D.Get(), &swapChainDesc, swapChain.GetAddressOf());
        factory->MakeWindowAssociation(Window::GetHandle(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

        D3D11_VIEWPORT viewPort = {};
        viewPort.Width = static_cast<float>(Window::GetSize().x);
        viewPort.Height = static_cast<float>(Window::GetSize().y);
        viewPort.MaxDepth = 1.0f;
        context3D->RSSetViewports(1, &viewPort);
    }
    void OnProceed(HWND, UINT message, WPARAM, LPARAM) override
    {
        if (message != WM_SIZE)
            return;

        if (Window::GetSize().x <= 0.0f || Window::GetSize().y <= 0.0f)
            return;

        Create();
    }
};