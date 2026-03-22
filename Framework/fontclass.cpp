#include "fontclass.h"

FontClass::FontClass()
{
    m_renderTarget = nullptr;
    m_writeFactory = nullptr;
    m_textFormat = nullptr;
	m_titleFormat = nullptr;
	m_lapFormat = nullptr;
    m_brush = nullptr;
    m_hwnd = nullptr;

	m_screenWidth = 0;
	m_screenHeight = 0;
}

FontClass::FontClass(const FontClass&)
{

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

	result = m_writeFactory->CreateTextFormat(
		L"Tahoma" ,
		nullptr ,
		DWRITE_FONT_WEIGHT_BOLD ,
		DWRITE_FONT_STYLE_NORMAL ,
		DWRITE_FONT_STRETCH_NORMAL ,
		80.0f , // 폰트 크기 담당
		L"en-us" ,
		&m_lapFormat);
	if ( FAILED(result) )
	{
		return false;
	}

	result = m_writeFactory->CreateTextFormat(
		L"Tahoma" ,
		nullptr ,
		DWRITE_FONT_WEIGHT_NORMAL ,
		DWRITE_FONT_STYLE_NORMAL ,
		DWRITE_FONT_STRETCH_NORMAL ,
		30.0f , // 폰트 크기 담당
		L"en-us" ,
		&m_bestLapFormat);

	if ( FAILED(result) )
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

void FontClass::SetRectBackground(D2D1_RECT_F rect) const
{
	ID2D1SolidColorBrush* backgroundBrush = nullptr;
	HRESULT hr = m_renderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black , 0.5f) ,  // 검은색, 투명도 70%
		&backgroundBrush
	);

	if ( SUCCEEDED(hr) && backgroundBrush )
	{
		// 둥근 모서리 사각형 설정 (radiusX, radiusY로 둥글기 조절)
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			rect ,
			10.0f ,  // X축 반지름 (모서리 둥글기)
			10.0f   // Y축 반지름 (모서리 둥글기)
		);

		// 둥근 배경 사각형 그리기
		m_renderTarget->FillRoundedRectangle(roundedRect , backgroundBrush);

		// 브러시 해제
		backgroundBrush->Release();
	}
}

bool FontClass::Render(int polygonCount, int fps, int cpu, int objectCount)
{
    D2D1_SIZE_F size = m_renderTarget->GetSize();
    D2D1_RECT_F rect = RectF(
        20.0f,
        20.0f,
        306.0f,
        140.0f);

    // 각종 실행정보 출력하기
    wstring text = 
        L" Screen Resolution: " + to_wstring(m_screenWidth) + L" x " + to_wstring(m_screenHeight) + "\n"
        L" Objects: " + to_wstring(objectCount) + L"\n"
        L" Polygons : " + to_wstring(polygonCount) + L"\n"
        L" CPU: " + to_wstring(cpu) + L"%\n"
        L" FPS: " + to_wstring(fps);

	m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::Lime));
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
        size.width - 280.0f,
        20.0f,
        size.width - 20.0f,
        350.0f
    );

	SetRectBackground(rect);

    // scene information texts
    wstring titleInfo = 
        L" C077021 LEE DONGHOON\n\n"
        L" W,A,S,D : Move\n"
        L" Mouse : Rotate camera\n\n"
        L" T : Scene Info UI\n"
		L" B : Collision Info UI\n"
		L" V : Collision ON / OFF\n"
		L" C : Culling ON / OFF\n"
        L" I/O/P : Lighting option\n"
        L" 1/2/3 : Filter option\n"
        L" 4/5 : Render mode option\n"
        L" 6/7/8 : Day / Sunset / Night\n"
		L" F9 : Sound On / OFF";

	m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
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

	wstring pickingText = L"Picking Status: ";
    wstring modelNameW(pickedModelName.begin(), pickedModelName.end());
    pickingText += modelNameW;

	m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::Lime));
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

bool FontClass::RenderCollisionDebug(CollisionClass* collision , DriveClass* drive ,
	const vector<string>& modelNames)
{
	if ( !m_renderTarget || !collision || !drive )
		return false;

	D2D1_SIZE_F size = m_renderTarget->GetSize();
	D2D1_RECT_F rect = D2D1::RectF(
		size.width - 280.0f ,
		size.height - 400.0f ,
		size.width - 20.0f ,
		size.height - 20.0f
	);

	wstring debugText = L"=== COLLISION INFO ===\n";

	if ( drive->IsDriving() )
	{
		int drivingIndex = drive->GetDrivingCarIndex();
		debugText += L"Driving Car Index: " + to_wstring(drivingIndex) + L"\n";

		if ( drivingIndex >= 0 && drivingIndex < modelNames.size() )
		{
			string modelName = modelNames[ drivingIndex ];
			debugText += L"Model: " + wstring(modelName.begin() , modelName.end()) + L"\n";
		}

		if ( drivingIndex >= 0 )
		{
			CollisionClass::AABB carAABB = collision->GetColliderAABB(drivingIndex);
			debugText += L"Car AABB Min: (" + to_wstring(( int ) carAABB.min.x) + L", "
				+ to_wstring(( int ) carAABB.min.y) + L", " + to_wstring(( int ) carAABB.min.z) + L")\n";
			debugText += L"Car AABB Max: (" + to_wstring(( int ) carAABB.max.x) + L", "
				+ to_wstring(( int ) carAABB.max.y) + L", " + to_wstring(( int ) carAABB.max.z) + L")\n";

			float width = carAABB.max.x - carAABB.min.x;
			float height = carAABB.max.y - carAABB.min.y;
			float depth = carAABB.max.z - carAABB.min.z;
			debugText += L"Size: " + to_wstring(( int ) width) + L" x " + to_wstring(( int ) height) + L" x " + to_wstring(( int ) depth) + L"\n";

			debugText += L"Collision Enabled: " + wstring(collision->IsCollisionSystemEnabled() ? L"YES" : L"NO") + L"\n";

			// 인스턴스 포함 충돌 검사 결과
			vector<pair<int , int>> collisions = collision->CheckCollisionsWithInstances(drivingIndex);
			debugText += L"Current Collisions: " + to_wstring(collisions.size()) + L"\n";

			for ( const auto& col : collisions )
			{
				int colIndex = col.first;
				int instanceIndex = col.second;

				if ( colIndex >= 0 && colIndex < modelNames.size() )
				{
					string colModelName = modelNames[ colIndex ];
					wstring colModelNameW(colModelName.begin() , colModelName.end());

					if ( instanceIndex >= 0 )
					{
						debugText += L"  - " + colModelNameW + L" (Instance " + to_wstring(instanceIndex) + L")\n";
					}
					else
					{
						debugText += L"  - " + colModelNameW + L"\n";
					}
				}
			}
		}
	}
	else
	{
		debugText += L"Not Driving\n";
	}

	// 전체 Collider 상태
	const vector<CollisionClass::ColliderInfo>& allColliders = collision->GetAllColliders();
	int enabledCount = 0;
	int instanceCount = 0;

	for ( const auto& collider : allColliders )
	{
		if ( collider.enableCollision )
		{
			enabledCount++;
			if ( collider.hasInstances )
			{
				instanceCount += static_cast< int >( collider.instanceColliders.size() );
			}
		}
	}

	debugText += L"Enabled Colliders: " + to_wstring(enabledCount) + L"/" + to_wstring(allColliders.size()) + L"\n";
	debugText += L"Instance Colliders: " + to_wstring(instanceCount) + L"\n";

	m_renderTarget->DrawText(
		debugText.c_str() ,
		debugText.length() ,
		m_textFormat ,
		rect ,
		m_brush
	);

	return true;
}

bool FontClass::RenderLapTime(DriveClass* drive)
{
	if ( !m_renderTarget || !drive )
		return false;

	// 운전 중일 때만 표시
	if ( !drive->IsDriving() )
		return true;

	D2D1_SIZE_F size = m_renderTarget->GetSize();

	D2D1_RECT_F rect = D2D1::RectF(
		size.width / 2.0f - 200.0f,
		size.height -900.0f,
		size.width /2.0f + 200.0f ,
		size.height - 760.0f
	);
	wstring lapText = L"";
	wstring bestLapText = L"";

	// 현재 랩 타임 표시
	float currentLap = drive->GetCurrentLapTime();
	wchar_t timeStr[ 64 ];
	swprintf_s(timeStr , L"%.3f\"\n" , currentLap);
	lapText += timeStr;

	m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	m_renderTarget->DrawText(
		lapText.c_str() ,
		lapText.length() ,
		m_lapFormat ,
		rect ,
		m_brush
	);

	m_lapFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	return true;
}

bool FontClass::RenderBestLapTime(DriveClass* drive)
{
	if ( !m_renderTarget || !drive )
		return false;

	// 운전 중일 때만 표시
	if ( !drive->IsDriving() )
		return true;

	D2D1_SIZE_F size = m_renderTarget->GetSize();

	D2D1_RECT_F rect = D2D1::RectF(
		size.width / 2.0f - 100.0f ,
		size.height - 800.0f ,
		size.width / 2.0f + 100.0f ,
		size.height - 740.0f
	);


	wchar_t timeStr[ 64 ];
	wstring bestLapText = L"";

	// 최고 랩 타임 표시
	float bestLap = drive->GetBestLapTime();
	if ( bestLap > 0.0f )
	{
		swprintf_s(timeStr , L"Best: %.3f\"" , bestLap);
		bestLapText += timeStr;
	}
	else
	{
		bestLapText += L"Best: ";
	}

	m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	m_renderTarget->DrawText(
		bestLapText.c_str() ,
		bestLapText.length() ,
		m_bestLapFormat ,
		rect ,
		m_brush
	);

	m_bestLapFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	return true;
}

bool FontClass::RenderDashboardSpeed(DriveClass* drive)
{
	if ( !m_renderTarget || !drive )
		return false;

	// 운전 중일 때만 표시
	if ( !drive->IsDriving() )
		return true;

	D2D1_SIZE_F size = m_renderTarget->GetSize();

	// 텍스트 영역 설정
	D2D1_RECT_F rect = D2D1::RectF(
		100.0f ,
		size.height - 150.0f ,
		400.0f ,
		size.height - 80.0f
	);

	// 속도 값 가져오기 및 포맷팅
	float speed = abs(drive->GetDisplaySpeed());
	int speedInt = static_cast< int >( speed );

	wchar_t speedStr[ 64 ];
	swprintf_s(speedStr , L"%d" , speedInt);

	wstring speedText = speedStr;

	m_brush->SetColor(D2D1::ColorF(D2D1::ColorF::OrangeRed));

	// 텍스트 정렬 설정
	m_bestLapFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_bestLapFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// 속도 표시
	m_renderTarget->DrawText(
		speedText.c_str() ,
		speedText.length() ,
		m_bestLapFormat ,
		rect ,
		m_brush
	);

	return true;
}
