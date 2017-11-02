// (c) 2017 Naoki Nakagawa
#include <d3d11.h>
#include <wincodec.h>
#include <DirectXMath.h>
#include "Graphics.h"
#include "Sprite.h"

using namespace std;
using namespace DirectX;

Sprite::Sprite(wchar_t* path) {
	HRESULT result = {};

	Vertex quad[] = {
		{ XMFLOAT3(-0.5f, 0.5f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	};
	vertexCount = sizeof(quad) / sizeof(quad[0]);

	int index[] = {
		0, 1, 2,
		3, 2, 1,
	};
	indexCount = sizeof(index) / sizeof(index[0]);

	constant.view = XMMatrixLookAtLH(XMVectorSet(Graphics::CLIENT_WIDTH / 2.0f, -Graphics::CLIENT_HEIGHT / 2.0f, 0.0f, 0.0f), XMVectorSet(Graphics::CLIENT_WIDTH / 2.0f, -Graphics::CLIENT_HEIGHT / 2.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	constant.projection = XMMatrixOrthographicLH(Graphics::CLIENT_WIDTH * 1.0f, Graphics::CLIENT_HEIGHT * 1.0f, -1.0f, 1.0f);

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
	vertexSubresourceData.pSysMem = quad;
	result = Graphics::GetInstance().GetDevice().CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);

	if (FAILED(result)) {
		throw bad_alloc();
	}
	else {
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		Graphics::GetInstance().GetDeviceContext().IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		Graphics::GetInstance().GetDeviceContext().IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(int) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
	indexSubresourceData.pSysMem = index;
	result = Graphics::GetInstance().GetDevice().CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);

	if (FAILED(result)) {
		throw bad_alloc();
	}
	else {
		Graphics::GetInstance().GetDeviceContext().IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(Constant);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	result = Graphics::GetInstance().GetDevice().CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

	if (FAILED(result)) {
		throw bad_alloc();
	}
	else {
		Graphics::GetInstance().GetDeviceContext().VSSetConstantBuffers(0, 1, &constantBuffer);
	}

	IWICImagingFactory* factory = nullptr;
	result = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));

	if (FAILED(result)) {
		throw bad_alloc();
	}

	IWICBitmapDecoder* decoder = nullptr;
	result = factory->CreateDecoderFromFilename(path, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);

	if (FAILED(result)) {
		throw bad_alloc();
	}

	IWICBitmapFrameDecode* frame = nullptr;
	result = decoder->GetFrame(0, &frame);

	if (FAILED(result)) {
		throw bad_alloc();
	}

	UINT textureWidth, textureHeight;
	result = frame->GetSize(&textureWidth, &textureHeight);

	if (FAILED(result)) {
		throw bad_alloc();
	}

	WICPixelFormatGUID pixelFormat;
	result = frame->GetPixelFormat(&pixelFormat);

	if (FAILED(result)) {
		throw bad_alloc();
	}

	BYTE* textureBuffer = new BYTE[textureWidth * textureHeight * 4];

	if (pixelFormat != GUID_WICPixelFormat32bppRGBA) {
		IWICFormatConverter* formatConverter = nullptr;
		result = factory->CreateFormatConverter(&formatConverter);
		
		if (FAILED(result)) {
			throw bad_alloc();
		}

		result = formatConverter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		
		if (FAILED(result)) {
			throw bad_alloc();
		}

		result = formatConverter->CopyPixels(0, textureWidth * 4, textureWidth * textureHeight * 4, textureBuffer);
		
		if (FAILED(result)) {
			throw bad_alloc();
		}
	}
	else {
		result = frame->CopyPixels(0, textureWidth * 4, textureWidth * textureHeight * 4, textureBuffer);

		if (FAILED(result)) {
			throw bad_alloc();
		}
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA textureSubresourceData;
	textureSubresourceData.pSysMem = textureBuffer;
	textureSubresourceData.SysMemPitch = textureWidth * 4;
	textureSubresourceData.SysMemSlicePitch = textureWidth * textureHeight * 4;
	result = Graphics::GetInstance().GetDevice().CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);
	
	if (FAILED(result)) {
		throw bad_alloc();
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	result = Graphics::GetInstance().GetDevice().CreateShaderResourceView(texture, &shaderResourceViewDesc, &shaderResourceView);
	
	if (FAILED(result)) {
		throw bad_alloc();
	}

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = Graphics::GetInstance().GetDevice().CreateSamplerState(&samplerDesc, &samplerState);
	
	if (FAILED(result)) {
		throw bad_alloc();
	}

	Graphics::GetInstance().GetDeviceContext().PSSetShaderResources(0, 1, &shaderResourceView);
	Graphics::GetInstance().GetDeviceContext().PSSetSamplers(0, 1, &samplerState);

	delete[] textureBuffer;
}

Sprite::~Sprite() {
	if (texture)
		texture->Release();

	if (shaderResourceView)
		shaderResourceView->Release();

	if (samplerState)
		samplerState->Release();

	if (vertexBuffer)
		vertexBuffer->Release();

	if (indexBuffer)
		indexBuffer->Release();

	if (constantBuffer)
		constantBuffer->Release();
}

void Sprite::Draw(float x, float y, float angle, float scale) {
	constant.world = XMMatrixIdentity();
	constant.world *= XMMatrixScaling(256.0f * scale, 256.0f * scale, 1.0f);
	constant.world *= XMMatrixRotationZ(XMConvertToRadians(-angle));
	constant.world *= XMMatrixTranslation(x, -y, 0.0f);

	Graphics::GetInstance().GetDeviceContext().UpdateSubresource(constantBuffer, 0, nullptr, &constant, 0, 0);
	Graphics::GetInstance().GetDeviceContext().DrawIndexed(indexCount, 0, 0);
}
