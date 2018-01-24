﻿#include "App.h"
#include <fstream>
#include <codecvt>
#include <string>
#include <regex>
#include <crtdbg.h>

using namespace std;
using namespace DirectX;
using namespace GameLibrary;

wstring GetSourceCode(const wchar_t* filePath) {
	wifstream sourceFile(filePath);
	if (sourceFile.fail()) {
		wstring empty = L"";
		return empty;
	}
	sourceFile.imbue(locale(locale(""), new codecvt_utf8_utf16<wchar_t, 0x10ffff, consume_header>()));
	istreambuf_iterator<wchar_t> iterator(sourceFile);
	istreambuf_iterator<wchar_t> last;
	wstring sourceCode(iterator, last);
	sourceFile.close();
	return sourceCode;
}
void MargeSourceCode(const wchar_t* fileName, wstring& sourceCode) {
	wstring from = L"#include \"" + wstring(fileName) + L"\"";
	wstring filePath = L"sources/" + wstring(fileName);

	wstring::size_type pos = sourceCode.find(from);
	sourceCode.replace(pos, from.size(), GetSourceCode(filePath.c_str()));
}
void LinkLibrary() {
	wstring library = GetSourceCode(L"sources/App.h");

	MargeSourceCode(L"Window.h", library);
	MargeSourceCode(L"Screen.h", library);
	MargeSourceCode(L"Input.h", library);
	MargeSourceCode(L"Time.h", library);
	MargeSourceCode(L"Camera.h", library);
	MargeSourceCode(L"Mesh.h", library);
	MargeSourceCode(L"Sprite.h", library);
	MargeSourceCode(L"Text.h", library);

	wofstream libraryFile(L"GameLibrary.h");
	libraryFile.imbue(locale(locale(""), new codecvt_utf8_utf16<wchar_t, 0x10ffff, generate_header>()));
	libraryFile << library;
	libraryFile.close();
}

int Main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	LinkLibrary();

	Camera camera;
	camera.position.z = -800.0f;

	Sprite sprite1(L"assets/clock.png");
	sprite1.position = XMFLOAT2(200.0f, 200.0f);
	Sprite sprite2(L"assets/4.JPG");
	sprite2.position = XMFLOAT2(0.0f, 0.0f);

	App::AddFont(L"assets/衡山毛筆フォント行書.ttf");
	App::SetTitle(L"くぁwせdrftgyふじこlp");

	Text text1(L"あ");
	text1.scale.x = text1.scale.y = 0.5f;
	Text text2(L"", L"ＭＳ 明朝");
	Text text3(L"玉", L"衡山毛筆フォント行書");

	Mesh mesh(L"assets/monkey.obj");

	bool isFullscreen = false;
	float t = 0.0f;
	while (App::Refresh()) {
		t += App::GetDeltaTime();

		camera.Update();

		sprite1.angle = t * (360.0f);
		sprite1.scale.y = 1.3f;
		sprite1.Draw();
		sprite1.angle = t * (360.0f / 60.0f);
		sprite1.scale.y = 1.0f;
		sprite1.Draw();
		sprite1.angle = t * (360.0f / 60.0f / 12.0f);
		sprite1.scale.y = 0.7f;
		sprite1.Draw();

		sprite2.angle = App::GetMousePosition().x;
		sprite2.Draw();

		if (App::GetKeyDown(VK_RBUTTON)) {
			isFullscreen = !isFullscreen;
			App::SetFullScreen(isFullscreen);
		}

		if (App::GetKey(VK_LBUTTON)) {
			text1.position = XMFLOAT2((float)App::GetMousePosition().x, (float)App::GetMousePosition().y);
		}
		text1.angle = App::GetTime() * -100.0f;
		text1.Draw();

		text2.position = XMFLOAT2(App::GetWindowSize().x / 2.0f, App::GetWindowSize().y / 2.0f - 200.0f);
		text2.Draw();

		text3.position = XMFLOAT2(App::GetWindowSize().x / 2.0f, App::GetWindowSize().y / 2.0f + 200.0f);
		text3.scale.x = text3.scale.y = 2.0f + sinf(t) * 1.0f;
		text3.Draw();
	}

	return 0;
}
