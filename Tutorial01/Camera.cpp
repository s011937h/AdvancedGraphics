#define _XM_NO_INTRINSICS_
#include "Camera.h"


Camera::Camera(UINT windowWidth, UINT windowHeight, XMVECTOR eye, XMVECTOR at, XMVECTOR up, bool isLookAt)
{
	// Initialize the projection matrix
	XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	m_Roll = 0;
	m_Pitch = 0;
	m_Yaw = 0;

	XMStoreFloat4(&m_Position, eye);

	MoveCamera(0, 0, 0, 0);
}


Camera::~Camera()
{

}


void Camera::MoveCamera(float verticalMovement, float horizontalMovement, float mouseDeltaX, float mouseDeltaY)
{
	const float mouseRotationRate = 0.0001; //tweak me
	m_Pitch += mouseDeltaY * mouseRotationRate;
	m_Yaw += mouseDeltaX * mouseRotationRate;


	XMMATRIX rotation = XMMatrixMultiply(XMMatrixRotationY(-m_Yaw), XMMatrixRotationX(-m_Pitch));

	XMMATRIX inverseRotation = XMMatrixMultiply(XMMatrixRotationX(m_Pitch), XMMatrixRotationY(m_Yaw));

	XMVECTOR m = XMVectorSet(horizontalMovement, verticalMovement, 0.0f, 0.0f);

	XMVECTOR rotatedMovement = XMVector3Transform(m, inverseRotation);

	XMFLOAT3 rotatedM;
	XMStoreFloat3(&rotatedM, rotatedMovement);

	m_Position.x += rotatedM.x;
	m_Position.y += rotatedM.y;
	m_Position.z += rotatedM.z;


	XMMATRIX translation = XMMatrixTranslation(-m_Position.x, -m_Position.y, -m_Position.z);
	XMStoreFloat4x4(&m_View, XMMatrixMultiply(translation, rotation)); // could be backwards
}