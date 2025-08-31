#include "pickingclass.h"
#include <float.h>

PickingClass::PickingClass()
{
    m_screenWidth = 0;
    m_screenHeight = 0;
    m_isPickingActive = false;
    m_pickedModelName = "Disabled";
    m_pickedModelIndex = -1;
    m_previousLeftMouseState = false;
    m_previousRightMouseState = false;
}

PickingClass::PickingClass(const PickingClass& other)
{
}

PickingClass::~PickingClass()
{
}

bool PickingClass::Initialize(int screenWidth, int screenHeight)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_isPickingActive = false;
    m_pickedModelName = "Disabled";
    m_pickedModelIndex = -1;

    return true;
}

void PickingClass::Shutdown()
{
    
}

bool PickingClass::ProcessPicking(CameraClass* camera, vector<ModelClass*>& models,
    vector<XMMATRIX>& worldMatrices, vector<string>& modelNames,
    bool leftMousePressed, bool rightMousePressed)
{
    // ���콺 ��ư ���� ���� (���� �����Ӱ� ��)
    bool leftMouseClicked = leftMousePressed && !m_previousLeftMouseState;
    bool rightMouseClicked = rightMousePressed && !m_previousRightMouseState;

    // ���콺 ��Ŭ������ ��ŷ ����
    if (rightMouseClicked && m_isPickingActive)
    {
        m_isPickingActive = false;
        m_pickedModelName = "Disabled";
        m_pickedModelIndex = -1;
    }

    // �̹� ��ŷ�� Ȱ��ȭ�Ǿ� ������ ���ο� ��ŷ ó������ ����
    if (leftMouseClicked && !m_isPickingActive)
    {
        // ��ũ�� �߾ӿ��� ��ŷ ���� ����
        XMVECTOR rayPos, rayDir;
        CreatePickingRayFromScreenCenter(camera, rayPos, rayDir);

        float closestDistance = FLT_MAX;
        int hitModelIndex = -1;

        // car models (�ε��� 1~6)�� �˻�
        for (int i = 1; i <= 6 && i < models.size(); i++)
        {
            if (models[i] != nullptr)
            {
                vector<VertexType> vertices;
                vector<unsigned long> indices;

                // �� ������ ����
                if (ExtractModelData(models[i], vertices, indices))
                {
                    // ����-�� ���� �׽�Ʈ
                    float distance = TestRayTriangleIntersection(rayPos, rayDir, vertices, indices, worldMatrices[i]);

                    if (distance < closestDistance)
                    {
                        closestDistance = distance;
                        hitModelIndex = i;
                    }
                }
            }
        }

        // ��Ʈ�� ���� ������ ��ŷ Ȱ��ȭ
        if (hitModelIndex != -1 && closestDistance < FLT_MAX)
        {
            m_isPickingActive = true;
            m_pickedModelIndex = hitModelIndex;
            m_pickedModelName = modelNames[hitModelIndex];
        }
    }

    // ���� ���콺 ���� ������Ʈ
    m_previousLeftMouseState = leftMousePressed;
    m_previousRightMouseState = rightMousePressed;

    return true;
}

void PickingClass::CreatePickingRayFromScreenCenter(CameraClass* camera, XMVECTOR& rayPos, XMVECTOR& rayDir)
{
    // Center of screen
    float screenCenterX = 0.0f;
    float screenCenterY = 0.0f;

    // Get Camera position
    XMFLOAT3 cameraPos = camera->GetPosition();
    rayPos = XMLoadFloat3(&cameraPos);

    // Calculate ray vector in viewing space at center of screen
    XMVECTOR rayDirViewSpace = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    // Get view matrix
    XMMATRIX viewMatrix;
    camera->GetViewMatrix(viewMatrix);

    // Transform view matrix to world matrix
    XMMATRIX inverseViewMatrix = XMMatrixInverse(nullptr, viewMatrix);
    rayDir = XMVector3TransformNormal(rayDirViewSpace, inverseViewMatrix);
    rayDir = XMVector3Normalize(rayDir);
}

float PickingClass::TestRayTriangleIntersection(XMVECTOR rayPos, XMVECTOR rayDir,
    vector<VertexType>& vertices, vector<unsigned long>& indices,
    XMMATRIX worldMatrix)
{
    float closestDistance = FLT_MAX;

    // ��� �ﰢ���� ���� ���� �׽�Ʈ
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        // �ﰢ���� �� ���� ��������
        XMVECTOR v1 = XMLoadFloat3(&vertices[indices[i]].position);
        XMVECTOR v2 = XMLoadFloat3(&vertices[indices[i + 1]].position);
        XMVECTOR v3 = XMLoadFloat3(&vertices[indices[i + 2]].position);

        // ���� �������� ��ȯ
        v1 = XMVector3TransformCoord(v1, worldMatrix);
        v2 = XMVector3TransformCoord(v2, worldMatrix);
        v3 = XMVector3TransformCoord(v3, worldMatrix);

        // �ﰢ���� ���� ���
        XMVECTOR edge1 = v2 - v1;
        XMVECTOR edge2 = v3 - v1;
        XMVECTOR normal = XMVector3Cross(edge1, edge2);
        normal = XMVector3Normalize(normal);

        // ��� ������ ��� (Ax + By + Cz + D = 0)
        float A = XMVectorGetX(normal);
        float B = XMVectorGetY(normal);
        float C = XMVectorGetZ(normal);
        float D = -(A * XMVectorGetX(v1) + B * XMVectorGetY(v1) + C * XMVectorGetZ(v1));

        // ���̿� ����� ������ ���
        float denominator = A * XMVectorGetX(rayDir) + B * XMVectorGetY(rayDir) + C * XMVectorGetZ(rayDir);

        if (abs(denominator) > 0.0001f) // �������� ���� ���
        {
            float numerator = -(A * XMVectorGetX(rayPos) + B * XMVectorGetY(rayPos) + C * XMVectorGetZ(rayPos) + D);
            float t = numerator / denominator;

            if (t > 0.0f) // ī�޶� ���ʿ� �ִ� ���
            {
                // ������ ���
                XMVECTOR intersectionPoint = rayPos + rayDir * t;

                // ���� �ﰢ�� ���ο� �ִ��� Ȯ��
                if (IsPointInTriangle(v1, v2, v3, intersectionPoint))
                {
                    if (t < closestDistance)
                    {
                        closestDistance = t;
                    }
                }
            }
        }
    }

    return closestDistance;
}

bool PickingClass::IsPointInTriangle(XMVECTOR& v1, XMVECTOR& v2, XMVECTOR& v3, XMVECTOR& point)
{
    // �ﰢ���� �� �𼭸��� ���� ���� ���� �� �ִ��� Ȯ��
    XMVECTOR cross1 = XMVector3Cross(v3 - v2, point - v2);
    XMVECTOR cross2 = XMVector3Cross(v3 - v2, v1 - v2);

    if (XMVectorGetX(XMVector3Dot(cross1, cross2)) >= 0)
    {
        cross1 = XMVector3Cross(v3 - v1, point - v1);
        cross2 = XMVector3Cross(v3 - v1, v2 - v1);

        if (XMVectorGetX(XMVector3Dot(cross1, cross2)) >= 0)
        {
            cross1 = XMVector3Cross(v2 - v1, point - v1);
            cross2 = XMVector3Cross(v2 - v1, v3 - v1);

            if (XMVectorGetX(XMVector3Dot(cross1, cross2)) >= 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool PickingClass::ExtractModelData(ModelClass* model, vector<VertexType>& vertices, vector<unsigned long>& indices)
{
    if (!model)
        return false;

    ModelClass::VertexType* modelVertices = model->GetVertices();
    unsigned long* modelIndices = model->GetIndices();
    unsigned int vertexCount = model->GetVertexCount();
    unsigned int indexCount = model->GetIndexCount();

    if (!modelVertices || !modelIndices || vertexCount == 0 || indexCount == 0)
        return false;

    // ���Ϳ� ����
    vertices.clear();
    indices.clear();

    vertices.reserve(vertexCount);
    indices.reserve(indexCount);

    for (unsigned int i = 0; i < vertexCount; i++)
    {
        VertexType vertex;
        vertex.position = modelVertices[i].position;
        vertex.texture = modelVertices[i].texture;
        vertex.normal = modelVertices[i].normal;
        vertex.materialIndex = modelVertices[i].materialIndex;
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < indexCount; i++)
    {
        indices.push_back(modelIndices[i]);
    }

    return true;
}

bool PickingClass::ExtractModelDataAlternative(ModelClass* model, vector<VertexType>& vertices, vector<unsigned long>& indices)
{
    if (!model)
        return false;

    // ModelClass�� PickingVertexType ���
    vector<ModelClass::PickingVertexType> modelVertices;
    vector<unsigned long> modelIndices;

    if (!model->GetVertexDataForPicking(modelVertices, modelIndices))
        return false;

    // PickingClass�� VertexType���� ��ȯ
    vertices.clear();
    indices.clear();

    vertices.reserve(modelVertices.size());
    indices.reserve(modelIndices.size());

    for (const auto& modelVertex : modelVertices)
    {
        VertexType vertex;
        vertex.position = modelVertex.position;
        vertex.texture = modelVertex.texture;
        vertex.normal = modelVertex.normal;
        vertex.materialIndex = modelVertex.materialIndex;
        vertices.push_back(vertex);
    }

    indices = modelIndices;

    return true;
}