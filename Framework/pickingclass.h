#ifndef _PICKINGCLASS_H_
#define _PICKINGCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include <string>
#include "modelclass.h"
#include "cameraclass.h"

using namespace DirectX;
using namespace std;

class PickingClass
{
public:
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
        XMFLOAT3 normal;
        int materialIndex;
    };

public:
    PickingClass();
    PickingClass(const PickingClass&);
    ~PickingClass();

    bool Initialize(int screenWidth, int screenHeight);
    void Shutdown();

    bool ProcessPicking(CameraClass* camera, vector<ModelClass*>& models,
        vector<XMMATRIX>& worldMatrices, vector<string>& modelNames,
        bool leftMousePressed, bool rightMousePressed);

    // ���� picking ���� ��ȯ
    string GetPickedModelName() const { return m_pickedModelName; }
    bool IsPickingActive() const { return m_isPickingActive; }
    int GetPickedModelIndex() const { return m_pickedModelIndex; }

private:
    void CreatePickingRayFromScreenCenter(CameraClass* camera, XMVECTOR& rayPos, XMVECTOR& rayDir);

    // Ray-Triangle ���� �׽�Ʈ
    float TestRayTriangleIntersection(XMVECTOR rayPos, XMVECTOR rayDir,
        vector<VertexType>& vertices, vector<unsigned long>& indices,
        XMMATRIX worldMatrix);

    // ���� �ﰢ�� ���ο� �ִ��� Ȯ��
    bool IsPointInTriangle(XMVECTOR& v1, XMVECTOR& v2, XMVECTOR& v3, XMVECTOR& point);

    bool ExtractModelData(ModelClass* model, vector<VertexType>& vertices, vector<unsigned long>& indices);
    bool ExtractModelDataAlternative(ModelClass* model, vector<VertexType>& vertices, vector<unsigned long>& indices);

private:
    int m_screenWidth, m_screenHeight;
    bool m_isPickingActive;
    string m_pickedModelName;
    int m_pickedModelIndex;

    // ���콺 ���� ����
    bool m_previousLeftMouseState;
    bool m_previousRightMouseState;
};

#endif