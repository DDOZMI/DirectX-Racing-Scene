#ifndef _IRENDERABLE_H_
#define _IRENDERABLE_H_

#include <d3d11.h>
#include <directxmath.h>
#include <string>

using namespace DirectX;

// 렌더링에 필요한 공통 데이터를 담는 구조체
struct RenderContext
{
    ID3D11DeviceContext* deviceContext;
    XMMATRIX* worldMatrix;
    XMMATRIX viewMatrix;
    XMMATRIX projectionMatrix;
    XMFLOAT3 cameraPosition;
    float deltaTime;

    // 특정 렌더링 타입에 필요한 추가 데이터들
    void* additionalData;

    RenderContext(ID3D11DeviceContext* dc = nullptr)
        : deviceContext(dc), worldMatrix(nullptr), additionalData(nullptr), deltaTime(0.0f)
    {
        viewMatrix = XMMatrixIdentity();
        projectionMatrix = XMMatrixIdentity();
        cameraPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }
};

// UI 렌더링에 특화된 데이터 구조체
struct UIRenderData
{
    int polygonCount;
    int fps;
    int cpu;
    int objectCount;
    std::string pickedModelName;
    bool isPickingActive;
    float loadingProgress;

    UIRenderData() : polygonCount(0), fps(0), cpu(0), objectCount(0),
        isPickingActive(false), loadingProgress(0.0f) {
    }
};

// 렌더링 가능한 객체들의 공통 인터페이스
class IRenderable
{
public:
    virtual ~IRenderable() = default;

    // 기본 초기화/정리 인터페이스
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    // 기본 렌더링 인터페이스
    virtual bool Render(const RenderContext& context) = 0;

    // 렌더링 가능 상태 확인
    virtual bool IsReadyToRender() const = 0;

    // 렌더링 타입 식별
    enum class RenderableType
    {
        MODEL,
        UI_TEXT,
        UI_ELEMENT,
        PARTICLE,
        TERRAIN
    };

    virtual RenderableType GetRenderableType() const = 0;

    // 선택적: 렌더링 우선순위 (낮은 숫자가 높은 우선순위)
    virtual int GetRenderPriority() const { return 100; }

    // 선택적: 투명도 관련
    virtual bool IsTransparent() const { return false; }
    virtual bool RequiresAlphaBlending() const { return false; }
};

// 3D 모델 렌더링을 위한 확장 인터페이스
class I3DRenderable : public IRenderable
{
public:
    virtual bool Render3D(const RenderContext& context,
        unsigned int instanceCount = 1,
        float rotation = 0.0f) = 0;

    virtual unsigned int GetIndexCount() const = 0;
    virtual unsigned int GetVertexCount() const = 0;
    virtual unsigned int GetPolygonCount() const = 0;
};

// UI 렌더링을 위한 확장 인터페이스  
class IUIRenderable : public IRenderable
{
public:
    virtual bool RenderUI(const RenderContext& context, const UIRenderData& uiData) = 0;

    virtual bool SetVisible(bool visible) = 0;
    virtual bool IsVisible() const = 0;
};

#endif