﻿/*! © 2017 itukikikuti | MIT License | gitlab.com/itukikikuti/XLibrary11 */
#pragma once

#define OEMRESOURCE
#include <cstdio>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <Windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dsound.h>
#include <dwrite.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Shlwapi.h>
#include <wincodec.h>
#include <wrl.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "windowscodecs.lib")

#define MAIN() APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)

XLIBRARY_NAMESPACE_BEGIN

using namespace DirectX;
using Microsoft::WRL::ComPtr;

static void InitializeApplication()
{
    static bool isInitialized = false;

    if (!isInitialized)
    {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        isInitialized = true;
    }
}

#include "Utility.hpp"

#include "Window.hpp"
#include "Input.hpp"
#include "Graphics.hpp"
#include "Audio.hpp"
#include "Timer.hpp"
#include "Random.hpp"

#include "Texture.hpp"
#include "Material.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Sprite.hpp"
#include "Text.hpp"
#include "Sound.hpp"

static bool Refresh()
{
    Graphics::Update();
    Input::Update();
    Timer::Update();
    return Window::Update();
}

XLIBRARY_NAMESPACE_END
