﻿#include "App.h"
#include "Library.h"
#include <crtdbg.h>

using namespace std;
using namespace DirectX;
using namespace GameLibrary;

int Main() {
	Library::Generate(L"GameLibrary.hpp");
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Camera camera;
	Mesh mesh;
	Texture tex(L"assets/box.jpg");

	while (App::Refresh()) {
		camera.Update();

		mesh.angles.y += App::GetDeltaTime() * 50.0f;
		mesh.Draw();
	}

	return 0;
}
