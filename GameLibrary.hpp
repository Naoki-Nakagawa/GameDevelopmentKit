﻿// © 2017 itukikikuti
#pragma once

#define OEMRESOURCE
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <strsafe.h>
#include <vector>
#include <wincodec.h>
#include <windows.h>
#include <wrl.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define GAMELIBRARY_BEGIN namespace GameLibrary {
#define GAMELIBRARY_END }
#define Main() WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
#define PUBLIC public:
#define PRIVATE private:
#define PROTECTED protected:

GAMELIBRARY_BEGIN

struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texcoord;
	DirectX::XMFLOAT3 normal;

	//Vertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texcoord, DirectX::XMFLOAT3 normal) {
	//	this->position = position;
	//	this->texcoord = texcoord;
	//	this->normal = normal;
	//}
};


class App {
public:
class Window {
	PRIVATE HWND handle;
	PRIVATE const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX;
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
		windowClass.lpszClassName = L"GameLibrary";
		windowClass.hIconSm = nullptr;
		RegisterClassExW(&windowClass);

		handle = CreateWindowW(L"GameLibrary", L"GameLibrary", style, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, instance, nullptr);

		SetSize(1280, 720);

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
	PUBLIC DirectX::XMINT2 GetSize() {
		RECT clientRect = {};
		GetClientRect(handle, &clientRect);

		return DirectX::XMINT2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	}
	PUBLIC void SetSize(int width, int height) {
		RECT windowRect = {};
		RECT clientRect = {};
		GetWindowRect(handle, &windowRect);
		GetClientRect(handle, &clientRect);

		int w = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left) + width;
		int h = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top) + height;

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
		static DirectX::XMINT2 size = GetSize();

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

class Graphics {
	PRIVATE const int SWAP_CHAIN_COUNT = 2;
	PRIVATE const DXGI_FORMAT SWAP_CHAIN_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
	PRIVATE const int MULTI_SAMPLE_COUNT = 1;
	PRIVATE const int MULTI_SAMPLE_QUALITY = 0;

	PRIVATE Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11DeviceContext> context = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;

	PUBLIC Graphics() {
		int createDeviceFlag = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_DRIVER_TYPE driverTypes[] = {
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		int driverTypeCount = sizeof(driverTypes) / sizeof(driverTypes[0]);

		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		int featureLevelCount = sizeof(featureLevels) / sizeof(featureLevels[0]);

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = SWAP_CHAIN_COUNT;
		swapChainDesc.BufferDesc.Width = App::GetWindowSize().x;
		swapChainDesc.BufferDesc.Height = App::GetWindowSize().y;
		swapChainDesc.BufferDesc.Format = SWAP_CHAIN_FORMAT;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		swapChainDesc.OutputWindow = App::GetWindowHandle();
		swapChainDesc.SampleDesc.Count = MULTI_SAMPLE_COUNT;
		swapChainDesc.SampleDesc.Quality = MULTI_SAMPLE_QUALITY;
		swapChainDesc.Windowed = true;

		for (int i = 0; i < driverTypeCount; i++) {
			HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, driverTypes[i], nullptr, createDeviceFlag, featureLevels, featureLevelCount, D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(), device.GetAddressOf(), nullptr, context.GetAddressOf());

			if (SUCCEEDED(result)) {
				break;
			}
		}

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
		CompileShader(nullptr, "VS", "vs_4_0", vertexShaderBlob.GetAddressOf());
		device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		context->VSSetShader(vertexShader.Get(), nullptr, 0);

		Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
		CompileShader(nullptr, "PS", "ps_4_0", pixelShaderBlob.GetAddressOf());
		device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		context->PSSetShader(pixelShader.Get(), nullptr, 0);

		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		int inputElementDescCount = sizeof(inputElementDesc) / sizeof(inputElementDesc[0]);

		device->CreateInputLayout(inputElementDesc, inputElementDescCount, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
		context->IASetInputLayout(inputLayout.Get());

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Microsoft::WRL::ComPtr<ID3D11BlendState> blendState = nullptr;
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		device->CreateBlendState(&blendDesc, &blendState);
		context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState = nullptr;
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
		context->RSSetState(rasterizerState.Get());
	}
	PUBLIC ~Graphics() {
	}
	PUBLIC ID3D11Device& GetDevice() {
		return *device.Get();
	}
	PUBLIC IDXGISwapChain& GetMemory() {
		return *swapChain.Get();
	}
	PUBLIC ID3D11DeviceContext& GetContext() {
		return *context.Get();
	}
	PRIVATE void CompileShader(const wchar_t* filePath, const char* entryPoint, const char* shaderModel, ID3DBlob** out) {
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

		if (filePath == nullptr) {
			char* shader =
				"cbuffer Object : register(b0) {"
				"    matrix _world;"
				"    float4 _color;"
				"};"
				"cbuffer Camera : register(b1) {"
				"    matrix _view;"
				"    matrix _projection;"
				"};"
				"Texture2D tex : register(t0);"
				"SamplerState samp : register(s0);"
				"struct VertexOutput {"
				"    float4 position : SV_POSITION;"
				"    float4 color : COLOR;"
				"    float2 uv : TEXCOORD;"
				"};"
				"VertexOutput VS(float4 vertex : POSITION, float2 uv : TEXCOORD) {"
				"    VertexOutput output = (VertexOutput)0;"
				"    output.position = mul(_world, vertex);"
				"    output.position = mul(_view, output.position);"
				"    output.position = mul(_projection, output.position);"
				"    output.color = _color;"
				"    output.uv = uv;"
				"    return output;"
				"}"
				"float4 PS(VertexOutput input) : SV_TARGET {"
				"    return tex.Sample(samp, input.uv) * input.color;"
				"}";

			D3DCompile(shader, strlen(shader), nullptr, nullptr, nullptr, entryPoint, shaderModel, shaderFlags, 0, out, &errorBlob);
		}
		else {
			D3DCompileFromFile(filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, shaderFlags, 0, out, &errorBlob);
		}

		if (errorBlob != nullptr) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			MessageBoxA(App::GetWindowHandle(), (char*)errorBlob->GetBufferPointer(), "Shader Error", MB_OK);
		}
	}
};

class Input {
	PRIVATE DirectX::XMFLOAT2 mousePosition;
	PRIVATE BYTE preKeyState[256];
	PRIVATE BYTE keyState[256];
	PRIVATE bool isShowCursor = true;

	PUBLIC Input() {
		Update();
	}
	PUBLIC ~Input() {
	}
	PUBLIC bool GetKey(int keyCode) {
		return keyState[keyCode] & 0x80;
	}
	PUBLIC bool GetKeyUp(int keyCode) {
		return !(keyState[keyCode] & 0x80) && (preKeyState[keyCode] & 0x80);
	}
	PUBLIC bool GetKeyDown(int keyCode) {
		return (keyState[keyCode] & 0x80) && !(preKeyState[keyCode] & 0x80);
	}
	PUBLIC DirectX::XMFLOAT2 GetMousePosition() {
		return mousePosition;
	}
	PUBLIC void SetMousePosition(float x, float y) {
		if (GetActiveWindow() != App::GetWindowHandle()) {
			return;
		}

		mousePosition.x = x;
		mousePosition.y = y;
		POINT point = { static_cast<int>(x), static_cast<int>(y) };
		ClientToScreen(App::GetWindowHandle(), &point);
		SetCursorPos(point.x, point.y);
	}
	PUBLIC void SetShowCursor(bool isShowCursor) {
		if (this->isShowCursor == isShowCursor) {
			return;
		}

		this->isShowCursor = isShowCursor;
		ShowCursor(isShowCursor);
	}
	PUBLIC void Update() {
		POINT point = {};
		GetCursorPos(&point);

		ScreenToClient(App::GetWindowHandle(), &point);
		mousePosition = DirectX::XMFLOAT2(static_cast<float>(point.x), static_cast<float>(point.y));

		for (int i = 0; i < 256; i++) {
			preKeyState[i] = keyState[i];
		}

		GetKeyboardState(keyState);
	}
};

class Timer {
	PRIVATE float time = 0.0f;
	PRIVATE float deltaTime = 0.0f;
	PRIVATE int frameRate = 0;
	PRIVATE float second = 0.0f;
	PRIVATE int frameCount = 0;
	LARGE_INTEGER preCount;
	LARGE_INTEGER frequency;

	PUBLIC Timer() {
		preCount = GetCounter();
		frequency = GetCountFrequency();
	}
	PUBLIC ~Timer() {
	}
	PUBLIC float GetTime() {
		return time;
	}
	PUBLIC float GetDeltaTime() {
		return deltaTime;
	}
	PUBLIC int GetFrameRate() {
		return frameRate;
	}
	PUBLIC void Update() {

		LARGE_INTEGER count = GetCounter();
		deltaTime = (float)(count.QuadPart - preCount.QuadPart) / frequency.QuadPart;
		preCount = GetCounter();

		time += deltaTime;


		frameCount++;
		second += deltaTime;
		if (second >= 1.0f) {
			frameRate = frameCount;
			frameCount = 0;
			second -= 1.0f;
		}
	}
	PRIVATE LARGE_INTEGER GetCounter() {
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter;
	}
	PRIVATE LARGE_INTEGER GetCountFrequency() {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return frequency;
	}
};


	PUBLIC App() = delete;
	PUBLIC static HWND GetWindowHandle() {
		return GetWindow().GetHandle();
	}
	PUBLIC static std::vector<UINT>& GetWindowMessages() {
		return GetWindow().GetMessages();
	}
	PUBLIC static DirectX::XMINT2 GetWindowSize() {
		return GetWindow().GetSize();
	}
	PUBLIC static void SetWindowSize(int width, int height) {
		GetWindow().SetSize(width, height);
	}
	PUBLIC static wchar_t* GetTitle() {
		return GetWindow().GetTitle();
	}
	PUBLIC static void SetTitle(const wchar_t* title) {
		GetWindow().SetTitle(title);
	}
	PUBLIC static void SetFullScreen(bool isFullscreen) {
		GetWindow().SetFullScreen(isFullscreen);
	}
	PUBLIC static ID3D11Device& GetGraphicsDevice() {
		return GetGraphics().GetDevice();
	}
	PUBLIC static ID3D11DeviceContext& GetGraphicsContext() {
		return GetGraphics().GetContext();
	}
	PUBLIC static IDXGISwapChain& GetGraphicsMemory() {
		return GetGraphics().GetMemory();
	}
	PUBLIC static bool GetKey(int VK_CODE) {
		return GetInput().GetKey(VK_CODE);
	}
	PUBLIC static bool GetKeyUp(int VK_CODE) {
		return GetInput().GetKeyUp(VK_CODE);
	}
	PUBLIC static bool GetKeyDown(int VK_CODE) {
		return GetInput().GetKeyDown(VK_CODE);
	}
	PUBLIC static DirectX::XMFLOAT2 GetMousePosition() {
		return GetInput().GetMousePosition();
	}
	PUBLIC static void SetMousePosition(DirectX::XMFLOAT2 position) {
		GetInput().SetMousePosition(position.x, position.y);
	}
	PUBLIC static void SetMousePosition(float x, float y) {
		GetInput().SetMousePosition(x, y);
	}
	PUBLIC static void SetShowCursor(bool isShowCursor) {
		GetInput().SetShowCursor(isShowCursor);
	}
	PUBLIC static float GetTime() {
		return GetTimer().GetTime();
	}
	PUBLIC static float GetDeltaTime() {
		return GetTimer().GetDeltaTime();
	}
	PUBLIC static int GetFrameRate() {
		return GetTimer().GetFrameRate();
	}
	PUBLIC static void AddFont(const wchar_t* filePath) {
		AddFontResourceExW(filePath, FR_PRIVATE, nullptr);
	}
	PUBLIC static bool Refresh() {
		GetGraphicsMemory().Present(1, 0);

		if (!GetWindow().Update()) return false;
		GetInput().Update();
		GetTimer().Update();

		return true;
	}
	PRIVATE static Window& GetWindow() {
		static std::unique_ptr<Window> window(new Window());
		return *window.get();
	}
	PRIVATE static Graphics& GetGraphics() {
		static std::unique_ptr<Graphics> graphics(new Graphics());
		return *graphics.get();
	}
	PRIVATE static Input& GetInput() {
		static std::unique_ptr<Input> input(new Input());
		return *input.get();
	}
	PRIVATE static Timer& GetTimer() {
		static std::unique_ptr<Timer> timer(new Timer());
		return *timer.get();
	}
};

class Texture {
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	PUBLIC Texture() {
	}
	PUBLIC ~Texture() {
	}
};

class Material {
	PUBLIC Material() {
	}
	PUBLIC ~Material() {
	}
};

class Camera {
	PRIVATE struct ConstantBuffer {
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	PUBLIC DirectX::XMFLOAT3 position;
	PUBLIC DirectX::XMFLOAT3 angles;
	PRIVATE float fieldOfView;
	PRIVATE float nearClip;
	PRIVATE float farClip;
	PRIVATE ConstantBuffer cbuffer;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;

	PUBLIC Camera() {
		CreateRenderTarget();

		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		App::GetGraphicsDevice().CreateBuffer(&constantBufferDesc, nullptr, constantBuffer.GetAddressOf());

		SetPerspective(60.0f, 0.1f, 2000.0f);

		position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		angles = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	PUBLIC ~Camera() {
	}
	PUBLIC void SetPerspective(float fieldOfView, float nearClip, float farClip) {
		this->fieldOfView = fieldOfView;
		this->nearClip = nearClip;
		this->farClip = farClip;
		cbuffer.projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fieldOfView), App::GetWindowSize().x / (float)App::GetWindowSize().y, nearClip, farClip);
	}
	PUBLIC void Refresh() {
		for (UINT message : App::GetWindowMessages()) {
			if (message == WM_SIZE) {
				Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullView = nullptr;
				App::GetGraphicsContext().OMSetRenderTargets(1, nullView.GetAddressOf(), nullptr);
				renderTarget.Reset();
				texture.Reset();
				App::GetGraphicsContext().Flush();
				App::GetGraphicsMemory().ResizeBuffers(2, App::GetWindowSize().x, App::GetWindowSize().y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

				CreateRenderTarget();

				SetPerspective(fieldOfView, nearClip, farClip);
				break;
			}
		}

		static float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		App::GetGraphicsContext().ClearRenderTargetView(renderTarget.Get(), color);

		cbuffer.view =
			DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(angles.x), DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z)) *
			DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		cbuffer.view = DirectX::XMMatrixInverse(nullptr, cbuffer.view);
		App::GetGraphicsContext().UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		App::GetGraphicsContext().VSSetConstantBuffers(1, 1, constantBuffer.GetAddressOf());
		App::GetGraphicsContext().PSSetConstantBuffers(1, 1, constantBuffer.GetAddressOf());
	}
	PRIVATE void CreateRenderTarget() {
		App::GetGraphicsMemory().GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture.GetAddressOf()));
		App::GetGraphicsDevice().CreateRenderTargetView(texture.Get(), nullptr, renderTarget.GetAddressOf());
		App::GetGraphicsContext().OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);

		D3D11_VIEWPORT viewPort = {};
		viewPort.Width = (float)App::GetWindowSize().x;
		viewPort.Height = (float)App::GetWindowSize().y;
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		App::GetGraphicsContext().RSSetViewports(1, &viewPort);
	}
};

class Mesh {
	PRIVATE struct ConstantBuffer {
		DirectX::XMMATRIX world;
		DirectX::XMFLOAT4 color;
	};

	PUBLIC DirectX::XMFLOAT3 position;
	PUBLIC DirectX::XMFLOAT3 angles;
	PUBLIC DirectX::XMFLOAT3 scale;
	PUBLIC DirectX::XMFLOAT4 color;
	PUBLIC std::vector<Vertex> vertexes;
	PUBLIC std::vector<int> indexes;
	PRIVATE ConstantBuffer cbuffer;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer = nullptr;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer = nullptr;

	PUBLIC Mesh() {
		position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		angles = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		App::GetGraphicsDevice().CreateBuffer(&constantBufferDesc, nullptr, constantBuffer.GetAddressOf());
	}
	PUBLIC ~Mesh() {
	}
	PUBLIC void Apply() {
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexes.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
		vertexSubresourceData.pSysMem = &vertexes[0];
		App::GetGraphicsDevice().CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, vertexBuffer.GetAddressOf());

		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(int) * indexes.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
		indexSubresourceData.pSysMem = &indexes[0];
		App::GetGraphicsDevice().CreateBuffer(&indexBufferDesc, &indexSubresourceData, indexBuffer.GetAddressOf());
	}
	PUBLIC void Draw() {
		cbuffer.world =
			DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
			DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(angles.x), DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z)) *
			DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		cbuffer.color = color;

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		App::GetGraphicsContext().IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

		App::GetGraphicsContext().IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		App::GetGraphicsContext().UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		App::GetGraphicsContext().VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		App::GetGraphicsContext().PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		App::GetGraphicsContext().DrawIndexed(static_cast<UINT>(indexes.size()), 0, 0);
	}
	PUBLIC static Mesh CreateQuad() {
		Mesh mesh;
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });

		mesh.indexes.push_back(0);
		mesh.indexes.push_back(1);
		mesh.indexes.push_back(2);
		mesh.indexes.push_back(3);
		mesh.indexes.push_back(2);
		mesh.indexes.push_back(1);

		mesh.Apply();
		return mesh;
	}
};

class Sprite {
	PROTECTED struct ConstantBuffer {
		DirectX::XMMATRIX world;
		DirectX::XMFLOAT4 color;
	};

	PUBLIC DirectX::XMFLOAT3 position;
	PUBLIC DirectX::XMFLOAT3 angles;
	PUBLIC DirectX::XMFLOAT3 scale;
	PUBLIC DirectX::XMFLOAT4 color;
	PROTECTED UINT width;
	PROTECTED UINT height;
	PROTECTED Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	PROTECTED ConstantBuffer cbuffer;
	PRIVATE int indexCount;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	PRIVATE Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	PUBLIC Sprite() {
		Initialize();
	}
	PUBLIC Sprite(const wchar_t* filePath) {
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

		Microsoft::WRL::ComPtr<IWICImagingFactory> factory = nullptr;
		CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));

		Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder = nullptr;
		BYTE* textureBuffer = nullptr;

		if (SUCCEEDED(factory->CreateDecoderFromFilename(filePath, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf()))) {
			Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame = nullptr;
			decoder->GetFrame(0, frame.GetAddressOf());
			frame->GetSize(&width, &height);

			WICPixelFormatGUID pixelFormat;
			frame->GetPixelFormat(&pixelFormat);
			textureBuffer = new BYTE[width * height * 4];

			if (pixelFormat != GUID_WICPixelFormat32bppRGBA) {
				Microsoft::WRL::ComPtr<IWICFormatConverter> formatConverter = nullptr;
				factory->CreateFormatConverter(formatConverter.GetAddressOf());

				formatConverter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);

				formatConverter->CopyPixels(0, width * 4, width * height * 4, textureBuffer);
			}
			else {
				frame->CopyPixels(0, width * 4, width * height * 4, textureBuffer);
			}
		}
		else {
			width = height = 100;
			textureBuffer = new BYTE[width * height * 4];
			for (UINT y = 0; y < height; y++) {
				for (UINT x = 0; x < width; x++) {
					BYTE color[4] = { 0xff, 0x00, 0xff, 0xff };
					memcpy(&textureBuffer[x * 4 + y * (width * 4)], color, sizeof(DWORD));
				}
			}
		}

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

		D3D11_SUBRESOURCE_DATA textureSubresourceData;
		textureSubresourceData.pSysMem = textureBuffer;
		textureSubresourceData.SysMemPitch = width * 4;
		textureSubresourceData.SysMemSlicePitch = width * height * 4;
		App::GetGraphicsDevice().CreateTexture2D(&textureDesc, &textureSubresourceData, texture.GetAddressOf());

		delete[] textureBuffer;

		Initialize();
	}
	PUBLIC virtual ~Sprite() {
	}
	PUBLIC DirectX::XMINT2 GetSize() {
		return DirectX::XMINT2(width, height);
	}
	PUBLIC void Draw() {
		cbuffer.world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) * DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(angles.x), DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z))* DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		cbuffer.color = color;
		App::GetGraphicsContext().UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
		App::GetGraphicsContext().VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		App::GetGraphicsContext().PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		App::GetGraphicsContext().PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());
		App::GetGraphicsContext().PSSetSamplers(0, 1, samplerState.GetAddressOf());

		App::GetGraphicsContext().DrawIndexed(indexCount, 0, 0);
	}
	PROTECTED void Initialize() {
		position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		angles = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		Vertex quad[] = {
			{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		};
		int vertexCount = sizeof(quad) / sizeof(quad[0]);

		int index[] = {
			0, 1, 2,
			3, 2, 1,
		};
		indexCount = sizeof(index) / sizeof(index[0]);

		int x = App::GetWindowSize().x;
		int y = App::GetWindowSize().y;

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
		vertexSubresourceData.pSysMem = quad;
		App::GetGraphicsDevice().CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, vertexBuffer.GetAddressOf());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		App::GetGraphicsContext().IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		App::GetGraphicsContext().IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(int) * indexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
		indexSubresourceData.pSysMem = index;
		App::GetGraphicsDevice().CreateBuffer(&indexBufferDesc, &indexSubresourceData, indexBuffer.GetAddressOf());

		App::GetGraphicsContext().IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		App::GetGraphicsDevice().CreateBuffer(&constantBufferDesc, nullptr, constantBuffer.GetAddressOf());

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		App::GetGraphicsDevice().CreateShaderResourceView(texture.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());

		D3D11_SAMPLER_DESC samplerDesc;
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
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		App::GetGraphicsDevice().CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
	}
};

class Text : public Sprite {
	PUBLIC Text(const wchar_t* text = L"", const wchar_t* fontFamily = L"") {
		if (text == L"") {
			text = L"\uFFFD";
		}

		LOGFONTW logFont = {};
		logFont.lfHeight = 256;
		logFont.lfWidth = 0;
		logFont.lfEscapement = 0;
		logFont.lfOrientation = 0;
		logFont.lfWeight = 0;
		logFont.lfItalic = false;
		logFont.lfUnderline = false;
		logFont.lfStrikeOut = false;
		logFont.lfCharSet = SHIFTJIS_CHARSET;
		logFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
		logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		logFont.lfQuality = PROOF_QUALITY;
		logFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		StringCchCopyW(logFont.lfFaceName, 32, fontFamily);
		HFONT font = CreateFontIndirectW(&logFont);

		HDC dc = GetDC(nullptr);
		HFONT oldFont = (HFONT)SelectObject(dc, font);
		UINT code = text[0];

		TEXTMETRICW textMetrics = {};
		GetTextMetricsW(dc, &textMetrics);
		GLYPHMETRICS glyphMetrics = {};
		const MAT2 matrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
		DWORD size = GetGlyphOutlineW(dc, code, GGO_GRAY4_BITMAP, &glyphMetrics, 0, nullptr, &matrix);
		BYTE* textureBuffer = new BYTE[size];
		GetGlyphOutlineW(dc, code, GGO_GRAY4_BITMAP, &glyphMetrics, size, textureBuffer, &matrix);

		SelectObject(dc, oldFont);
		DeleteObject(font);
		ReleaseDC(nullptr, dc);

		width = glyphMetrics.gmCellIncX;
		height = textMetrics.tmHeight;

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DYNAMIC;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		textureDesc.MiscFlags = 0;

		App::GetGraphicsDevice().CreateTexture2D(&textureDesc, nullptr, texture.GetAddressOf());

		D3D11_MAPPED_SUBRESOURCE mapped;
		App::GetGraphicsContext().Map(texture.Get(), D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &mapped);

		BYTE* bits = (BYTE*)mapped.pData;
		DirectX::XMINT2 origin;
		origin.x = glyphMetrics.gmptGlyphOrigin.x;
		origin.y = textMetrics.tmAscent - glyphMetrics.gmptGlyphOrigin.y;
		DirectX::XMINT2 bitmapSize;
		bitmapSize.x = glyphMetrics.gmBlackBoxX + (4 - (glyphMetrics.gmBlackBoxX % 4)) % 4;
		bitmapSize.y = glyphMetrics.gmBlackBoxY;
		const int LEVEL = 17;
		memset(bits, 0, mapped.RowPitch * textMetrics.tmHeight);

		for (int y = origin.y; y < origin.y + bitmapSize.y; y++) {
			for (int x = origin.x; x < origin.x + bitmapSize.x; x++) {
				DWORD alpha = (255 * textureBuffer[x - origin.x + bitmapSize.x * (y - origin.y)]) / (LEVEL - 1);
				DWORD color = 0x00ffffff | (alpha << 24);
				memcpy((BYTE*)bits + mapped.RowPitch * y + 4 * x, &color, sizeof(DWORD));
			}
		}

		App::GetGraphicsContext().Unmap(texture.Get(), D3D11CalcSubresource(0, 0, 1));
		delete[] textureBuffer;
	}
};


GAMELIBRARY_END
