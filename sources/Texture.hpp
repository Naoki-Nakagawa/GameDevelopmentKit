﻿class Texture
{
public:
    Texture()
    {
        InitializeApplication();
    }
    Texture(const wchar_t* const filePath)
    {
        InitializeApplication();
        Load(filePath);
    }
    Texture(const BYTE* const buffer, int width, int height)
    {
        InitializeApplication();
        Create(buffer, width, height);
    }
    ~Texture()
    {
    }
    void Load(const wchar_t* const filePath)
    {
        ATL::CComPtr<IWICBitmapDecoder> decoder = nullptr;

        Graphics::GetTextureFactory().CreateDecoderFromFilename(filePath, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
        ATL::CComPtr<IWICBitmapFrameDecode> frame = nullptr;
        decoder->GetFrame(0, &frame);
        UINT width, height;
        frame->GetSize(&width, &height);

        WICPixelFormatGUID pixelFormat;
        frame->GetPixelFormat(&pixelFormat);
        std::unique_ptr<BYTE[]> buffer(new BYTE[width * height * 4]);

        if (pixelFormat != GUID_WICPixelFormat32bppBGRA)
        {
            ATL::CComPtr<IWICFormatConverter> formatConverter = nullptr;
            Graphics::GetTextureFactory().CreateFormatConverter(&formatConverter);

            formatConverter->Initialize(frame, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);

            formatConverter->CopyPixels(0, width * 4, width * height * 4, buffer.get());
        }
        else
        {
            frame->CopyPixels(0, width * 4, width * height * 4, buffer.get());
        }

        Create(buffer.get(), width, height);
    }
    void Create(const BYTE* const buffer, int width, int height)
    {
        size = DirectX::XMINT2(width, height);

        texture.Release();
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = buffer;
        textureSubresourceData.SysMemPitch = width * 4;
        textureSubresourceData.SysMemSlicePitch = width * height * 4;
        Graphics::GetDevice3D().CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

        shaderResourceView.Release();
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
        shaderResourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;
        Graphics::GetDevice3D().CreateShaderResourceView(texture, &shaderResourceViewDesc, &shaderResourceView);

        samplerState.Release();
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 0.0f;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        Graphics::GetDevice3D().CreateSamplerState(&samplerDesc, &samplerState);
    }
    DirectX::XMINT2 GetSize() const
    {
        return size;
    }
    void Attach(int slot)
    {
        if (texture == nullptr)
            return;

        Graphics::GetContext3D().PSSetShaderResources(slot, 1, &shaderResourceView.p);
        Graphics::GetContext3D().PSSetSamplers(slot, 1, &samplerState.p);
    }
    ID3D11Texture2D& GetInterface()
    {
        return *texture;
    }

private:
    DirectX::XMINT2 size;
    ATL::CComPtr<ID3D11Texture2D> texture = nullptr;
    ATL::CComPtr<ID3D11ShaderResourceView> shaderResourceView = nullptr;
    ATL::CComPtr<ID3D11SamplerState> samplerState = nullptr;
};