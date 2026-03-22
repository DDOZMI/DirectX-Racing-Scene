#ifndef _DASHBOARDCLASS_H_
#define _DASHBOARDCLASS_H_

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")

#include <d2d1_1.h>
#include <d3d11.h>
#include <directxmath.h>
#include <wincodec.h>

#include "d3dclass.h"
#include "driveclass.h"

using namespace DirectX;

class DashboardClass
{
public:
	DashboardClass();
	DashboardClass(const DashboardClass&);
	~DashboardClass();

	bool Initialize(ID3D11Device* device , D3DClass* d3dClass , HWND hwnd , int screenWidth , int screenHeight);
	void Shutdown();
	bool Render(DriveClass* drive);

	// 디버깅용 함수
	void SetNeedlePivotMode(bool bottomCenter) { m_needlePivotAtBottom = bottomCenter; }
	float GetLastNeedleAngle() const { return m_lastNeedleAngle; }

private:
	bool LoadImageFromFile(const WCHAR* filename , ID2D1Bitmap** bitmap);
	void DrawRotatedBitmap(ID2D1Bitmap* bitmap , float x , float y , float width , float height , float rotationAngle);
	float CalculateNeedleAngle(float currentSpeed , float maxSpeed) const;

private:
	ID2D1RenderTarget* m_renderTarget;
	IWICImagingFactory* m_wicFactory;
	ID2D1Bitmap* m_speedometerBitmap;
	ID2D1Bitmap* m_needleBitmap;

	int m_screenWidth;
	int m_screenHeight;

	// 속도계 설정
	static constexpr float SPEEDOMETER_SIZE = 300.0f;
	static constexpr float NEEDLE_WIDTH = 300.0f;
	static constexpr float NEEDLE_HEIGHT = 300.0f;
	static constexpr float MAX_RPM = 8000.0f;
	static constexpr float ROTATION_RANGE = 270.0f;
	static constexpr float MARGIN = 30.0f;

	// 디버깅용 변수
	bool m_needlePivotAtBottom;
	float m_lastNeedleAngle;
};

#endif