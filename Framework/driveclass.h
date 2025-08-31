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

    // ���� ���� ���� �Լ���
    void StartDriving(int carModelIndex, XMFLOAT3 initialPosition, XMFLOAT3 initialRotation);
    void StopDriving();
    void UpdateDriving(InputClass* input, float deltaTime);

    // ������ ī�޶� ���� ��ȯ
    bool IsDriving() const { return m_isDriving; }
    int GetDrivingCarIndex() const { return m_drivingCarIndex; }
    XMMATRIX GetCarWorldMatrix(XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f)) const;  // ������ �Ķ���� �߰�
    void UpdateCamera(CameraClass* camera);

    // ���� ���� �Ӽ� ����
    void SetCarPhysics(float maxSpeed, float acceleration, float deceleration, float turnSpeed);

private:
    void ProcessMovement(InputClass* input, float deltaTime);
    void ProcessSteering(InputClass* input, float deltaTime);
    void ApplyPhysics(float deltaTime);
    void LimitSpeed();

    float CalculateSteeringEffectiveness(float speed) const;

private:
    // ���� ����
    bool m_isDriving;
    int m_drivingCarIndex;

    // ���� ��ġ �� ����
    XMFLOAT3 m_carPosition;
    XMFLOAT3 m_carRotation;    // x: pitch, y: yaw, z: roll
    XMFLOAT3 m_initialRotation; // JSON���� ������ �ʱ� ȸ�� (�߰�)

    // ���� ���� �Ӽ�
    float m_currentSpeed;      // ���� �ӵ�
    float m_currentTurnAngle;  // ���� ȸ����

    // ���� ������
    float m_maxSpeed;          // �ִ� �ӵ�
    float m_acceleration;      // ���ӵ�
    float m_deceleration;      // ���ӵ� (�극��ũ)
    float m_naturalDeceleration; // �ڿ� ���ӵ�
    float m_turnSpeed;         // ȸ�� �ӵ�

    // ī�޶� ������ (���� �Ĺ� ���)
    XMFLOAT3 m_cameraOffset;   // ���� ��� ī�޶� ��ġ
    float m_cameraDistance;    // �������� �Ÿ�
    float m_cameraHeight;      // ���� �� ����

    // ���� ���� ����
    float m_minTurnRadius;      // �ּ� ȸ�� �ݰ�
    float m_maxTurnAngle;       // �ִ� ���Ⱒ
    float m_steeringResponse;   // ���� ���� �ӵ�
    float m_targetTurnAngle;    // ��ǥ ���Ⱒ
    float m_gripLimit;          // �׸� �Ѱ���
};

#endif