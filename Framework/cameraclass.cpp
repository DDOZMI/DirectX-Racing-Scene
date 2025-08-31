////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_position.z = 0.0f;

	m_rotation.x = 0.0f;
	m_rotation.y = 0.0f;
	m_rotation.z = 0.0f;

	moveSpeed = 0.5f;
	rotationSpeed = 0.001f;

	m_forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m_right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}


XMFLOAT3 CameraClass::GetPosition()
{
	return m_position;
}


XMFLOAT3 CameraClass::GetRotation()
{
	return m_rotation;
}

void CameraClass::Render()
{
	XMVECTOR position = XMLoadFloat3(&m_position);

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
		m_rotation.x, m_rotation.y, 0.0f);

	XMVECTOR lookAt = XMVector3TransformCoord(m_forward, rotationMatrix);
	XMVECTOR up = XMVector3TransformCoord(m_up, rotationMatrix);

	XMVECTOR target = position + lookAt;

	m_viewMatrix = XMMatrixLookAtLH(position, target, up);
}


void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void CameraClass::MoveForward(float speed)
{
	// 3인칭 프리 카메라로 하려면 아래 주석만 활성화
	XMVECTOR position = XMLoadFloat3(&m_position);
	XMVECTOR forward = XMVector3Transform(m_forward,
		XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, 0.0f));

	position += forward * speed * moveSpeed;
	XMStoreFloat3(&m_position, position);

	/*XMVECTOR position = XMLoadFloat3(&m_position);
	XMVECTOR forward = XMVector3Transform(m_forward,
		XMMatrixRotationRollPitchYaw(0.0f, m_rotation.y, 0.0f));

	forward = XMVectorSetY(forward, 0.0f);
	forward = XMVector3Normalize(forward);

	position += forward * speed * moveSpeed;

	position = XMVectorSetY(position, m_position.y);

	XMStoreFloat3(&m_position, position);*/
}

void CameraClass::MoveRight(float speed)
{
	// 3인칭 프리 카메라로 하려면 아래 주석만 활성화
	XMVECTOR position = XMLoadFloat3(&m_position);
	XMVECTOR right = XMVector3Transform(m_right,
		XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, 0.0f));

	position += right * speed * moveSpeed;
	XMStoreFloat3(&m_position, position);

	/*XMVECTOR position = XMLoadFloat3(&m_position);
	XMVECTOR right = XMVector3Transform(m_right,
		XMMatrixRotationRollPitchYaw(0.0f, m_rotation.y, 0.0f));

	right = XMVectorSetY(right, 0.0f);
	right = XMVector3Normalize(right);

	position += right * speed * moveSpeed;

	position = XMVectorSetY(position, m_position.y);

	XMStoreFloat3(&m_position, position);*/
}

void CameraClass::RotateByMouse(float mouseX, float mouseY)
{
	// 마우스 이동에 따른 회전
	m_rotation.y += mouseX * rotationSpeed;
	m_rotation.x += mouseY * rotationSpeed;

	// 위/아래 회전 제한
	m_rotation.x = std::max(-1.5533f, std::min(m_rotation.x, 1.5533f));
}