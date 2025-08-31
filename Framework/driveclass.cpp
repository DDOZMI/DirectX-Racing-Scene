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

    // 기본 차량 물리 설정
    m_maxSpeed = 100.0f;
    m_acceleration = 15.0f;
    m_deceleration = 30.0f;
    m_naturalDeceleration = 5.0f;
    m_turnSpeed = 2.0f;

    // 카메라 설정
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
    m_initialRotation = initialRotation; // 초기 회전 저장

    // 운전 시작 시 초기화
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
            // 후진 중에 전진키를 누르면 브레이크 효과
            m_currentSpeed += m_deceleration * deltaTime;
        }
        else
        {
            // 전진 가속
            m_currentSpeed += m_acceleration * deltaTime;
        }
    }
    else if (reversing)
    {
        if (m_currentSpeed > 0.0f)
        {
            // 전진 중에 후진키를 누르면 브레이크 효과
            m_currentSpeed -= m_deceleration * deltaTime;
        }
        else
        {
            // 후진 가속 (최대 속도의 절반)
            m_currentSpeed -= m_acceleration * 0.5f * deltaTime;
        }
    }
    else if (braking)
    {
        // 브레이크 적용
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
        // 자연 감속
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

    // 1. 속도에 따른 최대 조향각 제한 (현실적인 커브)
    float speedRatio = currentSpeedAbs / m_maxSpeed;
    float maxAllowedTurnAngle = m_maxTurnAngle;

    if (currentSpeedAbs > 5.0f) // 최소 속도 이상에서만 제한 적용
    {
        // 속도가 높을수록 조향각이 급격히 감소 (제곱 함수 사용)
        float speedFactor = 1.0f - (speedRatio * speedRatio * 0.5f);
        speedFactor = max(0.5f, speedFactor); // 최소 10%는 보장
        maxAllowedTurnAngle = m_maxTurnAngle * speedFactor;
    }

    // 2. 목표 조향각 설정
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
        m_targetTurnAngle = 0.0f; // 중립으로 복귀
    }

    // 3. 조향 반응 속도 조정 (고속에서 더 느리게)
    float steeringSpeed = m_steeringResponse;
    if (currentSpeedAbs > m_gripLimit * 0.5f)
    {
        // 고속에서는 조향 반응이 더 느려짐
        float slowdownFactor = 1.0f - (speedRatio * 0.1f);
        steeringSpeed *= slowdownFactor;
    }

    // 4. 부드러운 조향각 보간
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

    // 5. 실제 차량 회전 적용 (언더스티어 시뮬레이션)
    float effectiveSteeringRatio = CalculateSteeringEffectiveness(currentSpeedAbs);
    float actualRotationSpeed = (m_currentTurnAngle * XM_PI / 180.0f) * effectiveSteeringRatio;

    // 전진/후진에 따른 조향 방향 결정
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
    // 차량의 전방 벡터 계산
    XMMATRIX rotationMatrix = XMMatrixRotationY(m_carRotation.y);
    XMVECTOR forwardVector = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotationMatrix);

    // 위치 업데이트
    XMVECTOR currentPos = XMLoadFloat3(&m_carPosition);
    XMVECTOR velocity = -forwardVector * m_currentSpeed * deltaTime;
    currentPos = XMVectorAdd(currentPos, velocity);

    XMStoreFloat3(&m_carPosition, currentPos);
}

void DriveClass::LimitSpeed()
{
    // 최대 속도 제한
    if (m_currentSpeed > m_maxSpeed)
        m_currentSpeed = m_maxSpeed;
    else if (m_currentSpeed < -m_maxSpeed * 0.5f) // 후진은 절반 속도
        m_currentSpeed = -m_maxSpeed * 0.5f;
}

XMMATRIX DriveClass::GetCarWorldMatrix(XMFLOAT3 scale) const
{
    if (!m_isDriving)
        return XMMatrixIdentity();

    // 차량의 월드 변환 행렬 생성 (스케일 -> 초기회전 -> 운동회전 -> 이동 순서)
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // 초기 회전 적용 (JSON에서 설정된 회전)
    XMMATRIX initialRotationMatrix = XMMatrixRotationRollPitchYaw(
        m_initialRotation.x, m_initialRotation.y, m_initialRotation.z);

    // 운동 회전 적용 (운전 중 회전)
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

    // 차량의 전체 회전 (초기 회전 + 운동 회전) 계산
    float totalYaw = m_initialRotation.y + (m_carRotation.y - m_initialRotation.y);

    // 차량 뒤쪽으로 카메라를 배치하기 위해 90도 추가 조정
    // Settings.json의 차량들이 270도 회전되어 있으므로 이를 고려
    float cameraYaw = totalYaw; // 90도 추가하여 뒤쪽으로

    // 카메라 위치 계산 (원형 오프셋 방식)
    float offsetX = sin(cameraYaw) * m_cameraDistance;
    float offsetZ = cos(cameraYaw) * m_cameraDistance;

    // 카메라 위치 설정
    XMFLOAT3 cameraPos;
    cameraPos.x = m_carPosition.x + offsetX;
    cameraPos.y = m_carPosition.y + m_cameraHeight;
    cameraPos.z = m_carPosition.z + offsetZ;

    // 목표 위치 (차량 중심보다 약간 위쪽)
    XMFLOAT3 targetPos;
    targetPos.x = m_carPosition.x;
    targetPos.y = m_carPosition.y + 2.0f;
    targetPos.z = m_carPosition.z;

    camera->SetPosition(cameraPos.x, cameraPos.y, cameraPos.z);

    // 카메라가 차량을 바라보도록 회전 설정
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
        return 1.0f; // 정상 조향
    }
    else
    {

        float overLimitRatio = (speed - m_gripLimit) / (m_maxSpeed - m_gripLimit);
        float effectiveness = 1.0f - (overLimitRatio * overLimitRatio * 0.7f);
        return max(0.8f, effectiveness);
    }
}