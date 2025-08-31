#include "fontclass.h"

FontClass::FontClass()
{
    m_renderTarget = nullptr;
    m_writeFactory = nullptr;
    m_textFormat = nullptr;
	m_titleFormat = nullptr;
    m_brush = nullptr;
    m_hwnd = nullptr;
}

FontClass::FontClass(const FontClass&)
{
    m_renderTarget = nullptr;
	m_writeFactory = nullptr;
	m_textFormat = nullptr;
	m_titleFormat = nullptr;
	m_brush = nullptr;
	m_hwnd = nullptr;

    m_screenWidth = 0;
    m_screenHeight = 0;
}

FontClass::~FontClass()
{

}

bool FontClass::Initialize(ID3D11Device* device, D3DClass* d3dClass, HWND hwnd, int screenWidth, int screenHeight)
{
    HRESULT result;
    m_hwnd = hwnd;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // D2D 렌더타겟 받아오기
    m_renderTarget = d3dClass->GetD2DRenderTarget();
    if (!m_renderTarget)
    {
        return false;
    }

    result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_writeFactory));
    if (FAILED(result))
    {
        return false;
    }

    // 텍스트 파라미터 설정
    result = m_writeFactory->CreateTextFormat(
        L"Arial",
        nullptr,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.0f, // 폰트 크기 담당
        L"en-us",
        &m_textFormat);
    if (FAILED(result))
    {
        return false;
    }

    result = m_writeFactory->CreateTextFormat(
        L"Tahoma",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_ITALIC,
        DWRITE_FONT_STRETCH_NORMAL,
        40.0f, // 폰트 크기 담당
        L"en-us",
        &m_titleFormat);
    if (FAILED(result))
    {
        return false;
    }

    // Create brush
    result = m_renderTarget->CreateSolidColorBrush(
        ColorF(ColorF::White), // 텍스트 색상 설정
        &m_brush);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void FontClass::Shutdown()
{
    if (m_brush)
    {
        m_brush->Release();
        m_brush = nullptr;
    }

    if (m_textFormat)
    {
        m_textFormat->Release();
        m_textFormat = nullptr;
    }

    if (m_writeFactory)
    {
        m_writeFactory->Release();
        m_writeFactory = nullptr;
    }

    m_renderTarget = nullptr;
}

bool FontClass::Render(int polygonCount, int fps, int cpu, int objectCount)
{
    D2D1_SIZE_F size = m_renderTarget->GetSize();
    D2D1_RECT_F rect = RectF(
        20.0f,
        20.0f,
        size.width - 10.0f,
        40.0f);

    // 각종 실행정보 출력하기
    wstring text = 
        L"Screen Resolution: " + to_wstring(m_screenWidth) + L" x " + to_wstring(m_screenHeight) + "\n"
        L"Objects: " + to_wstring(objectCount) + L"\n"
        L"Polygons : " + to_wstring(polygonCount) + L"\n"
        L"CPU: " + to_wstring(cpu) + L"%\n"
        L"FPS: " + to_wstring(fps);

    m_renderTarget->DrawText(
        text.c_str(),
        text.length(),
        m_textFormat,
        rect,
        m_brush);

    return true;
}

bool FontClass::RenderTitle()
{
    if (!m_renderTarget)
    {
        return false;
    }

    D2D1_SIZE_F size = m_renderTarget->GetSize();

    D2D1_RECT_F rect = D2D1::RectF(
        size.width - 300.0f,
        20.0f,
        size.width - 20.0f,
        60.0f
    );

    // scene information texts
    wstring titleInfo = 
        L"C077021 LEE DONGHOON\n\n"
        L"W,A,S,D : Move\n"
        L"Mouse : Rotate camera\n\n"
        L"T : Hide / Show Scene Info\n"
        L"C : Culling option\n"
        L"I/O/P : Lighting option\n"
        L"1/2/3 : Filter option\n"
        L"4/5 : Render mode option\n"
        L"6/7/8 : Day / Sunset / Night";

    m_renderTarget->DrawText(
        titleInfo.c_str(),
        titleInfo.length(),
        m_textFormat,
        rect,
        m_brush
    );

    return true;
}

bool FontClass::RenderPickingStatus(const string& pickedModelName, bool isPickingActive)
{
    if (!m_renderTarget)
    {
        return false;
    }

    D2D1_SIZE_F size = m_renderTarget->GetSize();

    // picking status position setup
    D2D1_RECT_F rect = D2D1::RectF(
        size.width / 2.0f - 150.0f, 
        size.height - 80.0f,        
        size.width / 2.0f + 150.0f,  
        size.height - 40.0f          
    );

	// change text color based on picking status
    D2D1_COLOR_F textColor;
    if (isPickingActive)
    {
		textColor = D2D1::ColorF(D2D1::ColorF::DeepPink);  // 활성화시 색상 변경
    }
    else
    {
        textColor = D2D1::ColorF(D2D1::ColorF::White); // 비활성화시 원복
    }
    m_brush->SetColor(textColor);

	wstring pickingText = L"Picking Status: ";
    wstring modelNameW(pickedModelName.begin(), pickedModelName.end());
    pickingText += modelNameW;

    m_renderTarget->DrawText(
        pickingText.c_str(),
        pickingText.length(),
        m_textFormat,
        rect,
        m_brush
    );

    return true;
}

bool FontClass::RenderLoadingScreen(float progress)
{
    if (!m_renderTarget)
    {
        return false;
    }

    D2D1_SIZE_F size = m_renderTarget->GetSize();

	// title position setup
    D2D1_RECT_F titleRect = D2D1::RectF(
        size.width / 2.0f - 180.0f,
        size.height / 2.0f - 100.0f,
        size.width / 2.0f + 180.0f,
        size.height / 2.0f - 50.0f
    );

    wstring titleText = L"Racing Track";

    m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));

	m_titleFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_renderTarget->DrawText(
        titleText.c_str(),
        titleText.length(),
        m_titleFormat,
        titleRect,
        m_brush
    );

	// loading bar setup
    int barWidth = 400;
    int barHeight = 20;
    float centerX = size.width / 2.0f;
    float centerY = size.height / 2.0f;

    // loading bar rectangle setup
    D2D1_RECT_F backgroundRect = D2D1::RectF(
        centerX - barWidth / 2.0f,
        centerY,
        centerX + barWidth / 2.0f,
        centerY + barHeight
    );

	// draw background of loading bar rectangle
    m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::Gray));
    m_renderTarget->DrawRectangle(backgroundRect, m_brush, 2.0f);

	// fill the loading bar based on progress
    if (progress > 0.0f)
    {
        D2D1_RECT_F progressRect = D2D1::RectF(
            centerX - barWidth / 2.0f + 2.0f,
            centerY + 2.0f,
            centerX - barWidth / 2.0f + 2.0f + (barWidth - 4.0f) * progress,
            centerY + barHeight - 2.0f
        );

        m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::LimeGreen));
        m_renderTarget->FillRectangle(progressRect, m_brush);
    }

	// loading progress text setup
    D2D1_RECT_F progressTextRect = D2D1::RectF(
        centerX - 100.0f,
        centerY + 30.0f,
        centerX + 100.0f,
        centerY + 60.0f
    );

    wstring progressText = L"Loading... " + to_wstring(static_cast<int>(progress * 100)) + L"%";

    m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    m_renderTarget->DrawText(
        progressText.c_str(),
        progressText.length(),
        m_textFormat,
        progressTextRect,
        m_brush
    );

    return true;
}