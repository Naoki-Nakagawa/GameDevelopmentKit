﻿#define XLIBRARY_NAMESPACE_BEGIN
#define XLIBRARY_NAMESPACE_END
#include "App.hpp"
#include "Library.cpp"
#include <sstream>
using namespace std;
using namespace DirectX;

int MAIN()
{
    Library::Generate(L"sources/App.hpp", L"XLibrary11.hpp");

    Camera camera;
    camera.position = Float3(0.0f, 1.0f, -5.0f);
    camera.SetPerspective(60.0f, 0.1f, 100.0f);

    Texture texture(L"assets/box.jpg");

    Mesh mesh;
    mesh.GetMaterial().SetTexture(0, &texture);

    Text text(L"菊池いつきです。\n今日は天気も良くおならが出そうです。\n何を言ってるのかわからないですよね、僕もわからないです！\nabcdefghijklmnopqrstuvwxyz", 32.0f);
    //text.SetPivot(Float2(-1.0f, 1.0f));
    text.position.y = 2.0f;
    text.scale = 1.0f / 100.0f;

    Text number;
    number.position.y = 3.0f;
    number.scale = 1.0f / 100.0f;

    Sound music(L"assets/music.mp3");
    music.SetLoop(true);
    music.SetVolume(0.5f);
    music.Play();

    Sound sound(L"assets/sound.wav");

    Sprite sprite(L"assets/clock.png");
    sprite.position.z = 100.0f;
    sprite.scale = 0.1f;

    while (App::Refresh())
    {
        camera.Update();

        if (App::GetKeyDown(VK_SPACE))
            sound.Play();

        text.angles.z += 0.1f;
        text.Draw();

        wstringstream ss;
        ss << App::GetTime();
        number.Create(ss.str(), 100.0f);
        number.Draw();

        mesh.angles.y += 1.0f;
        mesh.Draw();

        if (App::GetKey('1'))
            App::SetMousePosition(0.0f, 0.0f);

        sprite.position.x = App::GetMousePosition().x / 5.0f;
        sprite.position.y = App::GetMousePosition().y / 5.0f;
        sprite.Draw();
    }

    return 0;
}
