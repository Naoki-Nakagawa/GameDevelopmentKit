﻿class Texture
{
public:
	Texture()
	{
		App::Initialize();
		std::unique_ptr<BYTE[]> buffer(new BYTE[4]{ 0xff, 0x00, 0xff, 0xff });
		Create(1, 1, buffer.get());
	}
	Texture(const wchar_t* const filePath)
	{
		App::Initialize();
		Load(filePath);
	}
	Texture(int width, int height, BYTE* buffer)
	{
		App::Initialize();
		Create(width, height, buffer);
	}
	virtual ~Texture()
	{
	}
	void Load(const wchar_t* const filePath)
	{
		App::GetWindowHandle();

		ATL::CComPtr<IWICImagingFactory> factory = nullptr;
		CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));

		ATL::CComPtr<IWICBitmapDecoder> decoder = nullptr;

		factory->CreateDecoderFromFilename(filePath, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
		ATL::CComPtr<IWICBitmapFrameDecode> frame = nullptr;
		decoder->GetFrame(0, &frame);
		UINT width, height;
		frame->GetSize(&width, &height);

		WICPixelFormatGUID pixelFormat;
		frame->GetPixelFormat(&pixelFormat);
		std::unique_ptr<BYTE[]> buffer(new BYTE[width * height * 4]);

		if (pixelFormat != GUID_WICPixelFormat32bppRGBA)
		{
			ATL::CComPtr<IWICFormatConverter> formatConverter = nullptr;
			factory->CreateFormatConverter(&formatConverter);

			formatConverter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);

			formatConverter->CopyPixels(0, width * 4, width * height * 4, buffer.get());
		}
		else
		{
			frame->CopyPixels(0, width * 4, width * height * 4, buffer.get());
		}

		Create(width, height, buffer.get());
	}
	Float2 GetSize() const
	{
		return Float2(static_cast<float>(width), static_cast<float>(height));
	}
	void SetSize(float width, float height)
	{

	}
	virtual void Attach(int slot)
	{
		App::GetGraphicsContext().PSSetShaderResources(slot, 1, &shaderResourceView.p);
		App::GetGraphicsContext().PSSetSamplers(slot, 1, &samplerState.p);
	}

private:
	int width;
	int height;
	ATL::CComPtr<ID3D11Texture2D> texture;
	ATL::CComPtr<ID3D11ShaderResourceView> shaderResourceView;
	ATL::CComPtr<ID3D11SamplerState> samplerState;

	virtual void Create(int width, int height, const BYTE* const buffer)
	{
		this->width = width;
		this->height = height;

		texture.Release();
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
		textureSubresourceData.pSysMem = buffer;
		textureSubresourceData.SysMemPitch = width * 4;
		textureSubresourceData.SysMemSlicePitch = width * height * 4;
		App::GetGraphicsDevice().CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

		shaderResourceView.Release();
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		App::GetGraphicsDevice().CreateShaderResourceView(texture, &shaderResourceViewDesc, &shaderResourceView);

		samplerState.Release();
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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
		App::GetGraphicsDevice().CreateSamplerState(&samplerDesc, &samplerState);
	}
};