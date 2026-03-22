#include "dashboardclass.h"

DashboardClass::DashboardClass()
{
	m_renderTarget = nullptr;
	m_wicFactory = nullptr;
	m_speedometerBitmap = nullptr;
	m_needleBitmap = nullptr;
	m_screenWidth = 0;
	m_screenHeight = 0;
	m_needlePivotAtBottom = true;
	m_lastNeedleAngle = 180.0f;  // 초기 각도는 180도 (6시 방향, 아래)
}

DashboardClass::DashboardClass(const DashboardClass&)
{
}

DashboardClass::~DashboardClass()
{
}

bool DashboardClass::Initialize(ID3D11Device* device , D3DClass* d3dClass , HWND hwnd , int screenWidth , int screenHeight)
{
	HRESULT result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// D2D 렌더타겟 받아오기
	m_renderTarget = d3dClass->GetD2DRenderTarget();
	if ( !m_renderTarget )
	{
		return false;
	}

	result = CoInitialize(nullptr);
	if ( FAILED(result) && result != RPC_E_CHANGED_MODE )
	{
		if ( result != S_FALSE )
		{
			return false;
		}
	}

	// WIC Factory 생성
	result = CoCreateInstance(
		CLSID_WICImagingFactory ,
		nullptr ,
		CLSCTX_INPROC_SERVER ,
		IID_PPV_ARGS(&m_wicFactory)
	);
	if ( FAILED(result) )
	{
		return false;
	}

	// 속도계 배경 이미지 로드
	if ( !LoadImageFromFile(L"./data/speedometer.png" , &m_speedometerBitmap) )
	{
		// PNG 실패시 BMP 시도
		if ( !LoadImageFromFile(L"./data/speedometer.bmp" , &m_speedometerBitmap) )
		{
			return false;
		}
	}

	// 속도계 침 이미지 로드
	if ( !LoadImageFromFile(L"./data/pin.png" , &m_needleBitmap) )
	{
		// PNG 실패시 BMP 시도
		if ( !LoadImageFromFile(L"./data/pin.bmp" , &m_needleBitmap) )
		{
			return false;
		}
	}

	return true;
}

void DashboardClass::Shutdown()
{
	if ( m_needleBitmap )
	{
		m_needleBitmap->Release();
		m_needleBitmap = nullptr;
	}

	if ( m_speedometerBitmap )
	{
		m_speedometerBitmap->Release();
		m_speedometerBitmap = nullptr;
	}

	if ( m_wicFactory )
	{
		m_wicFactory->Release();
		m_wicFactory = nullptr;
	}

	m_renderTarget = nullptr;
}

bool DashboardClass::LoadImageFromFile(const WCHAR* filename , ID2D1Bitmap** bitmap)
{
	HRESULT result;
	IWICBitmapDecoder* decoder = nullptr;
	IWICBitmapFrameDecode* frame = nullptr;
	IWICFormatConverter* converter = nullptr;

	// 디코더 생성
	result = m_wicFactory->CreateDecoderFromFilename(
		filename ,
		nullptr ,
		GENERIC_READ ,
		WICDecodeMetadataCacheOnLoad ,
		&decoder
	);
	if ( FAILED(result) )
	{
		wchar_t errorMsg[ 512 ];
		swprintf_s(errorMsg , L"Failed to load image file: %s (HRESULT: 0x%08X)" , filename , result);
		OutputDebugString(errorMsg);
		return false;
	}

	// 첫 번째 프레임 가져오기
	result = decoder->GetFrame(0 , &frame);
	if ( FAILED(result) )
	{
		OutputDebugString(L"Failed to get frame from image");
		decoder->Release();
		return false;
	}

	// 포맷 변환기 생성
	result = m_wicFactory->CreateFormatConverter(&converter);
	if ( FAILED(result) )
	{
		OutputDebugString(L"Failed to create format converter");
		frame->Release();
		decoder->Release();
		return false;
	}

	// 32bppPBGRA 포맷으로 변환 (알파 채널 지원)
	result = converter->Initialize(
		frame ,
		GUID_WICPixelFormat32bppPBGRA ,
		WICBitmapDitherTypeNone ,
		nullptr ,
		0.0 ,
		WICBitmapPaletteTypeMedianCut
	);
	if ( FAILED(result) )
	{
		OutputDebugString(L"Failed to initialize format converter");
		converter->Release();
		frame->Release();
		decoder->Release();
		return false;
	}

	// D2D 비트맵 생성 (알파 모드 설정)
	D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties(
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM , D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	result = m_renderTarget->CreateBitmapFromWicBitmap(
		converter ,
		&bitmapProps ,
		bitmap
	);

	if ( FAILED(result) )
	{
		wchar_t errorMsg[ 256 ];
		swprintf_s(errorMsg , L"Failed to create D2D bitmap from WIC (HRESULT: 0x%08X)" , result);
		OutputDebugString(errorMsg);
	}

	// 리소스 정리
	converter->Release();
	frame->Release();
	decoder->Release();

	return SUCCEEDED(result);
}

void DashboardClass::DrawRotatedBitmap(ID2D1Bitmap* bitmap , float x , float y , float width , float height , float rotationAngle)
{
	if ( !bitmap || !m_renderTarget )
		return;

	// 회전 중심점은 이미지의 중앙
	D2D1_POINT_2F pivotPoint = D2D1::Point2F(x + width / 2.0f , y + height / 2.0f);

	// 현재 변환 저장
	D2D1::Matrix3x2F oldTransform;
	m_renderTarget->GetTransform(&oldTransform);

	// 회전 변환 적용
	D2D1::Matrix3x2F rotation = D2D1::Matrix3x2F::Rotation(rotationAngle , pivotPoint);
	m_renderTarget->SetTransform(rotation * oldTransform);

	// 비트맵 그리기
	D2D1_RECT_F destRect = D2D1::RectF(x , y , x + width , y + height);
	m_renderTarget->DrawBitmap(
		bitmap ,
		destRect ,
		1.0f ,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR ,
		nullptr
	);

	// 변환 복원
	m_renderTarget->SetTransform(oldTransform);
}

float DashboardClass::CalculateNeedleAngle(float currentSpeed , float maxSpeed) const
{
	float absSpeed = abs(currentSpeed); // 속도 음수 방지

	// 속도를 0~1 범위로 정규화
	float normalizedSpeed = min(absSpeed / maxSpeed , 1.0f);

	// RPM을 0~8000 범위로 변환 (속도에 비례)
	float rpm = normalizedSpeed * MAX_RPM;

	float angle = 0.0f + ( rpm / MAX_RPM ) * ROTATION_RANGE;

	return angle;
}

bool DashboardClass::Render(DriveClass* drive)
{
	if ( !m_renderTarget || !drive )
		return false;

	// 운전 중일 때만 표시
	if ( !drive->IsDriving() )
		return true;

	// 속도계 위치 계산 (화면 우측 하단)
	float speedometerX = MARGIN;
	float speedometerY = m_screenHeight - SPEEDOMETER_SIZE - MARGIN;

	// 속도계 배경 그리기
	if ( m_speedometerBitmap )
	{
		D2D1_RECT_F speedometerRect = D2D1::RectF(
			speedometerX ,
			speedometerY ,
			speedometerX + SPEEDOMETER_SIZE ,
			speedometerY + SPEEDOMETER_SIZE
		);

		m_renderTarget->DrawBitmap(
			m_speedometerBitmap ,
			speedometerRect ,
			1.0f ,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR ,
			nullptr
		);
	}

	// 현재 속도 가져오기
	float currentSpeed = drive->GetCurrentSpeed();
	float maxSpeed = drive->GetMaxSpeed();

	// 침 각도 계산
	float needleAngle = CalculateNeedleAngle(currentSpeed , maxSpeed);
	m_lastNeedleAngle = needleAngle;  // 디버깅용 저장

	// 침 위치 계산 (속도계 중심에서 침의 중앙이 위치하도록)
	float centerX = speedometerX + SPEEDOMETER_SIZE / 2.0f;
	float centerY = speedometerY + SPEEDOMETER_SIZE / 2.0f;

	// 침의 중앙이 속도계 중심에 오도록 위치 조정
	float needleX = centerX - NEEDLE_WIDTH / 2.0f;
	float needleY = centerY - NEEDLE_HEIGHT / 2.0f;

	// 침 그리기 (회전 적용)
	if ( m_needleBitmap )
	{
		DrawRotatedBitmap(
			m_needleBitmap ,
			needleX ,
			needleY ,
			NEEDLE_WIDTH ,
			NEEDLE_HEIGHT ,
			needleAngle
		);
	}

	return true;
}