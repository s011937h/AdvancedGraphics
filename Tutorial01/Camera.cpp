#define _XM_NO_INTRINSICS_
#include "Camera.h"


Camera::Camera(UINT windowWidth, UINT windowHeight, XMVECTOR eye, XMVECTOR at, XMVECTOR up, bool isLookAt)
{
	// Initialize the projection matrix
	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	roll = 0;
	pitch = 0;
	yaw = PI;
	Update(eye, 0, 0);
}


Camera::~Camera()
{

}


void Camera::Update(XMVECTOR eye, float mouseDeltaX, float mouseDeltaY)
{
	const float mouseRotationRate = 0.001; //tweak me
	pitch += mouseDeltaY * mouseRotationRate;
	yaw += mouseDeltaX * mouseRotationRate;

	XMStoreFloat4(&position, eye);
	//XMStoreFloat4(&_lookAt, at);
	//XMStoreFloat4(&_up, up);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(roll, pitch, yaw);
	XMMATRIX translation = XMMatrixTranslation(-position.x, -position.y, -position.z);
	XMStoreFloat4x4(&view, XMMatrixMultiply(rotation, translation)); // could be backwards
}

void Camera::MoveCamera(float x, float y, float z, float roll, float pitch, float yaw)
{

}