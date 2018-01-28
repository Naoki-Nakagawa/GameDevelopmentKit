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
	Sprite sprite(L"assets/4.JPG");
	Mesh mesh = Mesh::CreateQuad();

	sprite.Draw();
	camera.position.z = -1.0f;

	while (App::Refresh()) {
		camera.Refresh();

		if (!App::GetKey(VK_RETURN)) {
			mesh.angles.y += App::GetDeltaTime() * 50.0f;
		}
		mesh.Draw();
	}

	return 0;
}
