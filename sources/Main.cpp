#include <crtdbg.h>
#include "Game.h"

using namespace GameLibrary;

//int main() {
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Game::AddFont("�t�R�ѕM�t�H���g�s��.ttf");
	Game::SetTitle("����w��drftgy�ӂ���lp");

	Sprite sprite1(L"clock.png");
	Sprite sprite2(L"4.JPG");
	Text text1("��");
	Text text2("��", "�t�R�ѕM�t�H���g�s��");

	float t = 0.0f;
	while (Game::Update()) {
		t += Game::GetDeltaTime();

		sprite1.Draw(200.0f, 200.0f, t * (360.0f), 1.3f);
		sprite1.Draw(200.0f, 200.0f, t * (360.0f / 60.0f), 1.0f);
		sprite1.Draw(200.0f, 200.0f, t * (360.0f / 60.0f / 12.0f), 0.7f);

		sprite2.Draw(1000.0f, 250.0f, 0.0f, 1.0f);

		text1.Draw(Game::GetMousePosition().x, Game::GetMousePosition().y, t * -2000.0f, 0.5f);
		text1.Draw(Game::GetWidth() / 2.0f, Game::GetHeight() / 2.0f - 200.0f, 0.0f, 1.0f);
		text2.Draw(Game::GetWidth() / 2.0f, Game::GetHeight() / 2.0f + 200.0f, 0.0f, 2.0f + sinf(t) * 1.0f);
	}

	return 0;
}
