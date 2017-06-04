#include <cstdio>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>
#include <crtdbg.h>
#include "Graphics.h"
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

Graphics::Graphics() : frame(0) {
	instanceHandle = GetModuleHandle(0);
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = instanceHandle;
	windowClass.hIcon = LoadIcon(instanceHandle, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = TITLE;
	windowClass.hIconSm = LoadIcon(instanceHandle, IDI_APPLICATION);
	if (!RegisterClassEx(&windowClass)) return;

	windowHandle = CreateWindow(TITLE, TITLE, WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, instanceHandle, nullptr);
	if (!windowHandle) return;

	RECT windowRect = {};
	RECT clientRect = {};
	GetWindowRect(windowHandle, &windowRect);
	GetClientRect(windowHandle, &clientRect);
	int windowWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left) + CLIENT_WIDTH;
	int windowHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top) + CLIENT_HEIGHT;
	SetWindowPos(windowHandle, nullptr, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);

	ShowWindow(windowHandle, SW_SHOWNORMAL);

	direct3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!direct3D) return;

	D3DPRESENT_PARAMETERS presentParameter = {};
	presentParameter.BackBufferWidth = CLIENT_WIDTH;
	presentParameter.BackBufferHeight = CLIENT_HEIGHT;
	presentParameter.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameter.BackBufferCount = 0;
	presentParameter.MultiSampleType = D3DMULTISAMPLE_NONE;
	presentParameter.MultiSampleQuality = 0;
	presentParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameter.hDeviceWindow = nullptr;
	presentParameter.Windowed = true;
	presentParameter.EnableAutoDepthStencil = true;
	presentParameter.AutoDepthStencilFormat = D3DFMT_D24S8;
	presentParameter.Flags = 0;
	presentParameter.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	presentParameter.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	if (!CreateDevice(direct3D, &device, windowHandle, &presentParameter, D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING)) {
		if (!CreateDevice(direct3D, &device, windowHandle, &presentParameter, D3DDEVTYPE_HAL, D3DCREATE_SOFTWARE_VERTEXPROCESSING)) {
			if (!CreateDevice(direct3D, &device, windowHandle, &presentParameter, D3DDEVTYPE_REF, D3DCREATE_HARDWARE_VERTEXPROCESSING)) {
				if (!CreateDevice(direct3D, &device, windowHandle, &presentParameter, D3DDEVTYPE_REF, D3DCREATE_SOFTWARE_VERTEXPROCESSING)) {
					return;
				}
			}
		}
	}

	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	device->SetRenderState(D3DRS_ZWRITEENABLE, true);
	device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	D3DXVec3Normalize(&lightDirection, &D3DXVECTOR3(0.25f, -1.0f, 0.5f));

	Vertex vertex[] = {
		{ D3DXVECTOR3(-1.0f, 1.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), D3DXVECTOR2(0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, 1.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), D3DXVECTOR2(1.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, -1.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), D3DXVECTOR2(0.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, -1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), D3DXVECTOR2(1.0f, 1.0f) },

		{ D3DXVECTOR3(1.0f, 1.0f, -1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, -1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, 1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) },

		{ D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR2(0.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR2(1.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR2(0.0f, 1.0f) },
		{ D3DXVECTOR3(-1.0f, -1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR2(1.0f, 1.0f) },

		{ D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, 1.0f, -1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, -1.0f, 1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) },
		{ D3DXVECTOR3(-1.0f, -1.0f, -1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) },

		{ D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, 1.0f, -1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, 1.0f, -1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) },

		{ D3DXVECTOR3(-1.0f, -1.0f, -1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, -1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) },
		{ D3DXVECTOR3(-1.0f, -1.0f, 1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, 1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) },
	};
	vertexCount = sizeof(vertex) / sizeof(vertex[0]);
	vertexBuffer = new Vertex[vertexCount];
	memcpy(vertexBuffer, vertex, sizeof(vertex));
	WORD index[] = {
		0, 1, 2,
		3, 2, 1,

		4, 5, 6,
		7, 6, 5,

		8, 9, 10,
		11, 10, 9,

		12, 13, 14,
		15, 14, 13,

		16, 17, 18,
		19, 18, 17,

		20, 21, 22,
		23, 22, 21,
	};
	indexCount = sizeof(index) / sizeof(index[0]);
	indexBuffer = new WORD[indexCount];
	memcpy(indexBuffer, index, sizeof(index));

	HRESULT hresult = {};

	hresult = D3DXCreateTextureFromFile(device, "box.jpg", &texture);
	if (FAILED(hresult)) return;

	hresult = D3DXCreateEffectFromFile(device, "shader.fx", nullptr, nullptr, D3DXSHADER_DEBUG, nullptr, &shader, nullptr);
	if (FAILED(hresult)) return;
}

Graphics::~Graphics() {
	delete[] vertexBuffer;
	delete[] indexBuffer;
	texture->Release();
	shader->Release();
	device->Release();
	direct3D->Release();
}

void Graphics::Render() {
	frame++;
	device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128, 192, 255), 1.0f, 0);
	device->BeginScene();

	D3DXMatrixRotationY(&worldMatrix, frame * 0.01f);
	D3DXMatrixLookAtLH(&viewMatrix, &D3DXVECTOR3(0.0f, 2.0f, -10.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, D3DXToRadian(60), CLIENT_WIDTH / (float)CLIENT_HEIGHT, 0.01f, 100.0f);

	UINT passCount;
	shader->SetFloat("time", frame * 0.01f);
	shader->SetMatrix("worldMatrix", &worldMatrix);
	shader->SetMatrix("viewMatrix", &viewMatrix);
	shader->SetMatrix("projectionMatrix", &projectionMatrix);
	shader->SetVector("lightDirection", &(D3DXVECTOR4)lightDirection);
	shader->SetTexture("meshTexture", texture);
	shader->SetTechnique("main");
	shader->Begin(&passCount, 0);
	shader->BeginPass(0);

	device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, vertexCount, indexCount / 3, indexBuffer, D3DFMT_INDEX16, vertexBuffer, sizeof(Vertex));

	shader->EndPass();
	shader->End();

	device->EndScene();
	device->Present(nullptr, nullptr, nullptr, nullptr);
	Sleep(1);
}

LRESULT __stdcall Graphics::WindowProcedure(HWND windowHandle, UINT windowMessage, WPARAM wParam, LPARAM lParam) {
	if (windowMessage == WM_DESTROY) {
		PostQuitMessage(0);
	}
	return DefWindowProc(windowHandle, windowMessage, wParam, lParam);
}

bool Graphics::CreateDevice(LPDIRECT3D9 direct3D, LPDIRECT3DDEVICE9 *device, HWND windowHandle, D3DPRESENT_PARAMETERS *presentParameter, D3DDEVTYPE deviceType, DWORD behaviourFlag) {
	if (SUCCEEDED(direct3D->CreateDevice(D3DADAPTER_DEFAULT, deviceType, windowHandle, behaviourFlag, presentParameter, device))) {
		return true;
	}
	return false;
}
