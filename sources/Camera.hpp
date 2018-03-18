class Camera : public App::Window::Proceedable
{
public:
    Float3 position;
    Float3 angles;
    Float4 color;
    
    Camera()
    {
        App::Initialize();

        position = Float3(0.0f, 0.0f, -5.0f);
        angles = Float3(0.0f, 0.0f, 0.0f);
        color = Float4(1.0f, 1.0f, 1.0f, 1.0f);

        SetPerspective(60.0f, 0.1f, 1000.0f);

        App::Window::AddProcedure(this);

        Create();
    }
    ~Camera()
    {
        App::Window::RemoveProcedure(this);
    }
    void SetPerspective(float fieldOfView, float nearClip, float farClip)
    {
        isPerspective = true;
        this->fieldOfView = fieldOfView;
        this->nearClip = nearClip;
        this->farClip = farClip;
        float aspectRatio = App::GetWindowSize().x / (float)App::GetWindowSize().y;
        constant.projection = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fieldOfView), aspectRatio, nearClip, farClip)
        );
    }
    void Camera::SetOrthographic(float size, float nearClip, float farClip)
    {
        isPerspective = false;
        this->size = size;
        this->nearClip = nearClip;
        this->farClip = farClip;
        float aspectRatio = App::GetWindowSize().x / (float)App::GetWindowSize().y;
        constant.projection = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixOrthographicLH(size * aspectRatio, size, nearClip, farClip)
        );
    }
    void Start()
    {
        constant.view = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixInverse(
                nullptr,
                DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angles.z)) *
                DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angles.y)) *
                DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(angles.x)) *
                DirectX::XMMatrixTranslation(position.x, position.y, position.z)
            )
        );

        App::GetGraphicsContext().UpdateSubresource(constantBuffer, 0, nullptr, &constant, 0, 0);
        App::GetGraphicsContext().VSSetConstantBuffers(0, 1, &constantBuffer.p);
        App::GetGraphicsContext().HSSetConstantBuffers(0, 1, &constantBuffer.p);
        App::GetGraphicsContext().DSSetConstantBuffers(0, 1, &constantBuffer.p);
        App::GetGraphicsContext().GSSetConstantBuffers(0, 1, &constantBuffer.p);
        App::GetGraphicsContext().PSSetConstantBuffers(0, 1, &constantBuffer.p);

        App::GetGraphicsContext().OMSetRenderTargets(1, &renderTargetView.p, depthStencilView);

        float clearColor[4] = { color.x, color.y, color.z, color.w };
        App::GetGraphicsContext().ClearRenderTargetView(renderTargetView, clearColor);
        App::GetGraphicsContext().ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
    void Stop()
    {
    }

private:
    struct Constant
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    bool isPerspective;
    float fieldOfView;
    float size;
    float nearClip;
    float farClip;
    Constant constant;
    ATL::CComPtr<ID3D11RenderTargetView> renderTargetView = nullptr;
    ATL::CComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;
    ATL::CComPtr<ID3D11Texture2D> renderTexture = nullptr;
    ATL::CComPtr<ID3D11Texture2D> depthTexture = nullptr;
    ATL::CComPtr<ID3D11Buffer> constantBuffer = nullptr;

    void Create()
    {
        renderTexture.Release();
        App::GetGraphicsSwapChain().GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&renderTexture));
        renderTargetView.Release();
        App::GetGraphicsDevice().CreateRenderTargetView(renderTexture, nullptr, &renderTargetView);

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        App::GetGraphicsSwapChain().GetDesc(&swapChainDesc);

        depthTexture.Release();
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = static_cast<UINT>(App::GetWindowSize().x);
        textureDesc.Height = static_cast<UINT>(App::GetWindowSize().y);
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        textureDesc.SampleDesc.Count = swapChainDesc.SampleDesc.Count;
        textureDesc.SampleDesc.Quality = swapChainDesc.SampleDesc.Quality;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
        App::GetGraphicsDevice().CreateTexture2D(&textureDesc, nullptr, &depthTexture);

        depthStencilView.Release();
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        if (swapChainDesc.SampleDesc.Count == 0)
        {
            depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;
        }
        else
        {
            depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
        App::GetGraphicsDevice().CreateDepthStencilView(depthTexture, &depthStencilViewDesc, &depthStencilView);

        constantBuffer.Release();
        D3D11_BUFFER_DESC constantBufferDesc = {};
        constantBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Constant));
        constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = 0;
        App::GetGraphicsDevice().CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
    }
    void OnProceed(HWND, UINT message, WPARAM, LPARAM) override
    {
        if (message != WM_SIZE)
            return;

        if (App::GetWindowSize().x <= 0.0f || App::GetWindowSize().y <= 0.0f)
            return;

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        App::GetGraphicsSwapChain().GetDesc(&swapChainDesc);

        ATL::CComPtr<ID3D11RenderTargetView> nullRenderTarget = nullptr;
        ATL::CComPtr<ID3D11DepthStencilView> nullDepthStencil = nullptr;
        App::GetGraphicsContext().OMSetRenderTargets(1, &nullRenderTarget, nullDepthStencil);
        renderTargetView.Release();
        depthStencilView.Release();
        renderTexture.Release();
        depthTexture.Release();
        App::GetGraphicsContext().Flush();
        App::GetGraphicsSwapChain().ResizeBuffers(swapChainDesc.BufferCount, static_cast<UINT>(App::GetWindowSize().x), static_cast<UINT>(App::GetWindowSize().y), swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

        if (isPerspective)
            SetPerspective(fieldOfView, nearClip, farClip);
        else
            SetOrthographic(size, nearClip, farClip);

        Create();
    }
};