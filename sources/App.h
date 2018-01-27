﻿// © 2017 Naoki Nakagawa
#ifndef _GAME_LIBRARY_
#define _GAME_LIBRARY_

#define OEMRESOURCE
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <strsafe.h>
#include <vector>
#include <wincodec.h>
#include <windows.h>
#include <wrl.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define GAME_LIBRARY_BEGIN namespace GameLibrary {
#define GAME_LIBRARY_END }
#define Main() WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
#define PUBLIC public:
#define PRIVATE private:
#define PROTECTED protected:

GAME_LIBRARY_BEGIN

class App {
public:
#include "Window.h"
#include "Graphic.h"
#include "Input.h"
#include "Timer.h"

	PUBLIC App() = delete;
	PUBLIC static HWND GetWindowHandle() {
		return GetWindow().GetHandle();
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
	PUBLIC static ID3D11Device& GetGraphicDevice() {
		return GetGraphic().GetDevice();
	}
	PUBLIC static ID3D11DeviceContext& GetGraphicContext() {
		return GetGraphic().GetContext();
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
		static float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		GetGraphic().GetSwapChain().Present(1, 0);

		static MSG message = {};

		while (message.message != WM_QUIT) {
			if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessageW(&message);
			}
			else {
				GetGraphicContext().ClearRenderTargetView(&GetGraphic().GetRenderTargetView(), color);

				GetInput().Update();
				GetTimer().Update();

				return true;
			}
		}

		return false;
	}
	PRIVATE static Window& GetWindow() {
		static std::unique_ptr<Window> window(new Window(ProcessWindow));
		return *window.get();
	}
	PRIVATE static Graphic& GetGraphic() {
		static std::unique_ptr<Graphic> graphic(new Graphic());
		return *graphic.get();
	}
	PRIVATE static Input& GetInput() {
		static std::unique_ptr<Input> input(new Input());
		return *input.get();
	}
	PRIVATE static Timer& GetTimer() {
		static std::unique_ptr<Timer> timer(new Timer());
		return *timer.get();
	}
	PRIVATE static LRESULT WINAPI ProcessWindow(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProcW(window, message, wParam, lParam);
		}
		return 0;
	}
};

#include "Camera.h"
#include "Mesh.h"
#include "Sprite.h"
#include "Text.h"

GAME_LIBRARY_END

#endif
