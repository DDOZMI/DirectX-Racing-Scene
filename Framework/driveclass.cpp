#include "driveclass.h"
#include <algorithm>

DriveClass::DriveClass()
{
    m_isDriving = false;
    m_drivingCarIndex = -1;

    m_carPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_carRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_initialRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);

    m_currentSpeed = 0.0f;
    m_currentTurnAngle = 0.0f;

    // �⺻ ���� ���� ����
    m_maxSpeed = 100.0f;
    m_acceleration = 15.0f;
    m_deceleration = 30.0f;
    m_naturalDeceleration = 5.0f;
    m_turnSpeed = 2.0f;

    // ī�޶� ����
    m_cameraDistance = 15.0f;
    m_cameraHeight = 8.0f;
    m_cameraOffset = XMFLOAT3(0.0f, m_cameraHeight, -m_cameraDistance);

    m_minTurnRadius = 5.0f;   
    m_maxTurnAngle = 70.0f;     
    m_steeringResponse = 100.0f; 
    m_targetTurnAngle = 0.0f;    
    m_gripLimit = 25.0f;     
}

DriveClass::DriveClass(const DriveClass& other)
{
}

DriveClass::~DriveClass()
{
}

bool DriveClass::Initialize()
{
    return true;
}

void DriveClass::Shutdown()
{
    m_isDriving = false;
    m_drivingCarIndex = -1;
}

void DriveClass::StartDriving(int carModelIndex, XMFLOAT3 initialPosition, XMFLOAT3 initialRotation)
{
    m_isDriving = true;
    m_drivingCarIndex = carModelIndex;
    m_carPosition = initialPosition;
    m_carRotation = initialRotation;
    m_initialRotation = initialRotation; // �ʱ� ȸ�� ����

    // ���� ���� �� �ʱ�ȭ
    m_currentSpeed = 0.0f;
    m_currentTurnAngle = 0.0f;
}

void DriveClass::StopDriving()
{
    m_isDriving = false;
    m_drivingCarIndex = -1;
    m_currentSpeed = 0.0f;
    m_currentTurnAngle = 0.0f;
}

void DriveClass::UpdateDriving(InputClass* input, float deltaTime)
{
    if (!m_isDriving)
        return;

    ProcessMovement(input, deltaTime);
    ProcessSteering(input, deltaTime);
    ApplyPhysics(deltaTime);
    LimitSpeed();
}

void DriveClass::ProcessMovement(InputClass* input, float deltaTime)
{
    bool accelerating = input->IsKeyDown('W') || input->IsKeyDown(VK_UP);
    bool reversing = input->IsKeyDown('S') || input->IsKeyDown(VK_DOWN);
    bool braking = input->IsKeyDown(VK_SPACE);

    if (accelerating)
    {
        if (m_currentSpeed < 0.0f)
        {
            // ���� �߿� ����Ű�� ������ �극��ũ ȿ��
            m_currentSpeed += m_deceleration * deltaTime;
        }
        else
        {
            // ���� ����
            m_currentSpeed += m_acceleration * deltaTime;
        }
    }
    else if (reversing)
    {
        if (m_currentSpeed > 0.0f)
        {
            // ���� �߿� ����Ű�� ������ �극��ũ ȿ��
            m_currentSpeed -= m_deceleration * deltaTime;
        }
        else
        {
            // ���� ���� (�ִ� �ӵ��� ����)
            m_currentSpeed -= m_acceleration * 0.5f * deltaTime;
        }
    }
    else if (braking)
    {
        // �극��ũ ����
        if (m_currentSpeed > 0.0f)
        {
            m_currentSpeed -= m_deceleration * deltaTime;
            if (m_currentSpeed < 0.0f) m_currentSpeed = 0.0f;
        }
        else if (m_currentSpeed < 0.0f)
        {
            m_currentSpeed += m_deceleration * deltaTime;
            if (m_currentSpeed > 0.0f) m_currentSpeed = 0.0f;
        }
    }
    else
    {
        // �ڿ� ����
        if (m_currentSpeed > 0.0f)
        {
            m_currentSpeed -= m_naturalDeceleration * deltaTime;
            if (m_currentSpeed < 0.0f) m_currentSpeed = 0.0f;
        }
        else if (m_currentSpeed < 0.0f)
        {
            m_currentSpeed += m_naturalDeceleration * deltaTime;
            if (m_currentSpeed > 0.0f) m_currentSpeed = 0.0f;
        }
    }
}

void DriveClass::ProcessSteering(InputClass* input, float deltaTime)
{
    bool turningLeft = input->IsKeyDown('A') || input->IsKeyDown(VK_LEFT);
    bool turningRight = input->IsKeyDown(VK_RIGHT) || input->IsKeyDown('D');

    float currentSpeedAbs = abs(m_currentSpeed);

    // 1. �ӵ��� ���� �ִ� ���Ⱒ ���� (�������� Ŀ��)
    float speedRatio = currentSpeedAbs / m_maxSpeed;
    float maxAllowedTurnAngle = m_maxTurnAngle;

    if (currentSpeedAbs > 5.0f) // �ּ� �ӵ� �̻󿡼��� ���� ����
    {
        // �ӵ��� �������� ���Ⱒ�� �ް��� ���� (���� �Լ� ���)
        float speedFactor = 1.0f - (speedRatio * speedRatio * 0.5f);
        speedFactor = max(0.5f, speedFactor); // �ּ� 10%�� ����
        maxAllowedTurnAngle = m_maxTurnAngle * speedFactor;
    }

    // 2. ��ǥ ���Ⱒ ����
    if (turningLeft && currentSpeedAbs > 0.1f)
    {
        m_targetTurnAngle = -maxAllowedTurnAngle;
    }
    else if (turningRight && currentSpeedAbs > 0.1f)
    {
        m_targetTurnAngle = maxAllowedTurnAngle;
    }
    else
    {
        m_targetTurnAngle = 0.0f; // �߸����� ����
    }

    // 3. ���� ���� �ӵ� ���� (��ӿ��� �� ������)
    float steeringSpeed = m_steeringResponse;
    if (currentSpeedAbs > m_gripLimit * 0.5f)
    {
        // ��ӿ����� ���� ������ �� ������
        float slowdownFactor = 1.0f - (speedRatio * 0.1f);
        steeringSpeed *= slowdownFactor;
    }

    // 4. �ε巯�� ���Ⱒ ����
    float angleDifference = m_targetTurnAngle - m_currentTurnAngle;
    float maxChange = steeringSpeed * deltaTime;

    if (abs(angleDifference) > maxChange)
    {
        m_currentTurnAngle += (angleDifference > 0 ? maxChange : -maxChange);
    }
    else
    {
        m_currentTurnAngle = m_targetTurnAngle;
    }

    // 5. ���� ���� ȸ�� ���� (�����Ƽ�� �ùķ��̼�)
    float effectiveSteeringRatio = CalculateSteeringEffectiveness(currentSpeedAbs);
    float actualRotationSpeed = (m_currentTurnAngle * XM_PI / 180.0f) * effectiveSteeringRatio;

    // ����/������ ���� ���� ���� ����
    if (m_currentSpeed > 0.0f)
    {
        m_carRotation.y += actualRotationSpeed * deltaTime;
    }
    else if (m_currentSpeed < 0.0f)
    {
        m_carRotation.y -= actualRotationSpeed * deltaTime;
    }
}

void DriveClass::ApplyPhysics(float deltaTime)
{
    // ������ ���� ���� ���
    XMMATRIX rotationMatrix = XMMatrixRotationY(m_carRotation.y);
    XMVECTOR forwardVector = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotationMatrix);

    // ��ġ ������Ʈ
    XMVECTOR currentPos = XMLoadFloat3(&m_carPosition);
    XMVECTOR velocity = -forwardVector * m_currentSpeed * deltaTime;
    currentPos = XMVectorAdd(currentPos, velocity);

    XMStoreFloat3(&m_carPosition, currentPos);
}

void DriveClass::LimitSpeed()
{
    // �ִ� �ӵ� ����
    if (m_currentSpeed > m_maxSpeed)
        m_currentSpeed = m_maxSpeed;
    else if (m_currentSpeed < -m_maxSpeed * 0.5f) // ������ ���� �ӵ�
        m_currentSpeed = -m_maxSpeed * 0.5f;
}

XMMATRIX DriveClass::GetCarWorldMatrix(XMFLOAT3 scale) const
{
    if (!m_isDriving)
        return XMMatrixIdentity();

    // ������ ���� ��ȯ ��� ���� (������ -> �ʱ�ȸ�� -> �ȸ�� -> �̵� ����)
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // �ʱ� ȸ�� ���� (JSON���� ������ ȸ��)
    XMMATRIX initialRotationMatrix = XMMatrixRotationRollPitchYaw(
        m_initialRotation.x, m_initialRotation.y, m_initialRotation.z);

    // � ȸ�� ���� (���� �� ȸ��)
    XMMATRIX dynamicRotationMatrix = XMMatrixRotationRollPitchYaw(
        m_carRotation.x - m_initialRotation.x,
        m_carRotation.y - m_initialRotation.y,
        m_carRotation.z - m_initialRotation.z);

    XMMATRIX translationMatrix = XMMatrixTranslation(
        m_carPosition.x, m_carPosition.y, m_carPosition.z);

    return scaleMatrix * initialRotationMatrix * dynamicRotationMatrix * translationMatrix;
}

void DriveClass::UpdateCamera(CameraClass* camera)
{
    if (!m_isDriving || !camera)
        return;

    // ������ ��ü ȸ�� (�ʱ� ȸ�� + � ȸ��) ���
    float totalYaw = m_initialRotation.y + (m_carRotation.y - m_initialRotation.y);

    // ���� �������� ī�޶� ��ġ�ϱ� ���� 90�� �߰� ����
    // Settings.json�� �������� 270�� ȸ���Ǿ� �����Ƿ� �̸� ���
    float cameraYaw = totalYaw; // 90�� �߰��Ͽ� ��������

    // ī�޶� ��ġ ��� (���� ������ ���)
    float offsetX = sin(cameraYaw) * m_cameraDistance;
    float offsetZ = cos(cameraYaw) * m_cameraDistance;

    // ī�޶� ��ġ ����
    XMFLOAT3 cameraPos;
    cameraPos.x = m_carPosition.x + offsetX;
    cameraPos.y = m_carPosition.y + m_cameraHeight;
    cameraPos.z = m_carPosition.z + offsetZ;

    // ��ǥ ��ġ (���� �߽ɺ��� �ణ ����)
    XMFLOAT3 targetPos;
    targetPos.x = m_carPosition.x;
    targetPos.y = m_carPosition.y + 2.0f;
    targetPos.z = m_carPosition.z;

    camera->SetPosition(cameraPos.x, cameraPos.y, cameraPos.z);

    // ī�޶� ������ �ٶ󺸵��� ȸ�� ����
    XMVECTOR lookDirection = XMVectorSet(
        targetPos.x - cameraPos.x,
        targetPos.y - cameraPos.y,
        targetPos.z - cameraPos.z,
        0.0f
    );
    lookDirection = XMVector3Normalize(lookDirection);

    float yaw = atan2f(XMVectorGetX(lookDirection), XMVectorGetZ(lookDirection));
    float pitch = asinf(-XMVectorGetY(lookDirection));

    camera->SetRotation(pitch, yaw, 0.0f);
}

void DriveClass::SetCarPhysics(float maxSpeed, float acceleration, float deceleration, float turnSpeed)
{
    m_maxSpeed = maxSpeed;
    m_acceleration = acceleration;
    m_deceleration = deceleration;
    m_turnSpeed = turnSpeed;
}

float DriveClass::CalculateSteeringEffectiveness(float speed) const
{
    if (speed < m_gripLimit)
    {
        return 1.0f; // ���� ����
    }
    else
    {

        float overLimitRatio = (speed - m_gripLimit) / (m_maxSpeed - m_gripLimit);
        float effectiveness = 1.0f - (overLimitRatio * overLimitRatio * 0.7f);
        return max(0.8f, effectiveness);
    }
}