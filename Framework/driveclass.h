#ifndef _DRIVECLASS_H_
#define _DRIVECLASS_H_

#include <directxmath.h>
#include "cameraclass.h"
#include "inputclass.h"

using namespace DirectX;

class DriveClass
{
public:
    DriveClass();
    DriveClass(const DriveClass&);
    ~DriveClass();

    bool Initialize();
    void Shutdown();

    // 차량 운전 관련 함수들
    void StartDriving(int carModelIndex, XMFLOAT3 initialPosition, XMFLOAT3 initialRotation);
    void StopDriving();
    void UpdateDriving(InputClass* input, float deltaTime);

    // 차량과 카메라 상태 반환
    bool IsDriving() const { return m_isDriving; }
    int GetDrivingCarIndex() const { return m_drivingCarIndex; }
    XMMATRIX GetCarWorldMatrix(XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f)) const;  // 스케일 파라미터 추가
    void UpdateCamera(CameraClass* camera);

    // 차량 물리 속성 설정
    void SetCarPhysics(float maxSpeed, float acceleration, float deceleration, float turnSpeed);

private:
    void ProcessMovement(InputClass* input, float deltaTime);
    void ProcessSteering(InputClass* input, float deltaTime);
    void ApplyPhysics(float deltaTime);
    void LimitSpeed();

    float CalculateSteeringEffectiveness(float speed) const;

private:
    // 운전 상태
    bool m_isDriving;
    int m_drivingCarIndex;

    // 차량 위치 및 방향
    XMFLOAT3 m_carPosition;
    XMFLOAT3 m_carRotation;    // x: pitch, y: yaw, z: roll
    XMFLOAT3 m_initialRotation; // JSON에서 설정된 초기 회전 (추가)

    // 차량 물리 속성
    float m_currentSpeed;      // 현재 속도
    float m_currentTurnAngle;  // 현재 회전각

    // 차량 설정값
    float m_maxSpeed;          // 최대 속도
    float m_acceleration;      // 가속도
    float m_deceleration;      // 감속도 (브레이크)
    float m_naturalDeceleration; // 자연 감속도
    float m_turnSpeed;         // 회전 속도

    // 카메라 오프셋 (차량 후방 상단)
    XMFLOAT3 m_cameraOffset;   // 차량 대비 카메라 위치
    float m_cameraDistance;    // 차량과의 거리
    float m_cameraHeight;      // 차량 위 높이

    // 조향 관련 변수
    float m_minTurnRadius;      // 최소 회전 반경
    float m_maxTurnAngle;       // 최대 조향각
    float m_steeringResponse;   // 조향 반응 속도
    float m_targetTurnAngle;    // 목표 조향각
    float m_gripLimit;          // 그립 한계점
};

#endif