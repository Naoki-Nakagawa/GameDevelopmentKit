﻿class Text : public Sprite
{
public:
    Text(const std::wstring& text = L"", float fontSize = 16.0f, const wchar_t* const fontFace = L"ＭＳ ゴシック")
    {
        Sprite::Initialize();
        color = Float4(0.0f, 0.0f, 0.0f, 1.0f);
        Create(text, fontSize, fontFace);
    }
    void Create(const std::wstring& text = L"", float fontSize = 16.0f, const wchar_t* const fontFace = L"ＭＳ ゴシック")
    {
        if (text == L"")
            return;

        brush.Reset();
        Graphics::GetContext2D().CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), brush.GetAddressOf());
        Graphics::GetContext2D().SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

        Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat = nullptr;
        Graphics::GetTextFactory().CreateTextFormat(fontFace, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"ja-jp", textFormat.GetAddressOf());

        textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        textLayout.Reset();
        Graphics::GetTextFactory().CreateTextLayout(text.c_str(), (UINT32)text.length(), textFormat.Get(), FLT_MAX, FLT_MAX, textLayout.GetAddressOf());
        
        DWRITE_TEXT_METRICS textMetrics;
        textLayout->GetMetrics(&textMetrics);

        textLayout.Reset();
        Graphics::GetTextFactory().CreateTextLayout(text.c_str(), (UINT32)text.length(), textFormat.Get(), textMetrics.width, textMetrics.height, textLayout.GetAddressOf());

        std::unique_ptr<BYTE[]> buffer(new BYTE[(int)textMetrics.width * (int)textMetrics.height * 4]);
        texture.Create(buffer.get(), (int)textMetrics.width, (int)textMetrics.height);

        ATL::CComPtr<IDXGISurface> surface = nullptr;
        texture.GetInterface().QueryInterface(&surface);

        D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};
        bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;

        bitmap.Release();
        Graphics::GetContext2D().CreateBitmapFromDxgiSurface(surface, bitmapProperties, &bitmap);

        mesh.GetMaterial().SetTexture(0, &texture);

        SetPivot(0.0f);
    }
    void Draw()
    {
        Graphics::GetContext2D().SetTarget(bitmap);

        Graphics::GetContext2D().BeginDraw();

        Graphics::GetContext2D().Clear(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.0f));
        Graphics::GetContext2D().DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), textLayout.Get(), brush.Get());

        Graphics::GetContext2D().EndDraw();

        Sprite::Draw();
    }
    void Load(const wchar_t* const filePath) = delete;
    void Create(const BYTE* const buffer, int width, int height) = delete;

private:
    ATL::CComPtr<ID2D1Bitmap1> bitmap = nullptr;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush = nullptr;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout = nullptr;
};