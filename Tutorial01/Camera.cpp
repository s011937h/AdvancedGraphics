#define _XM_NO_INTRINSICS_
#include "Camera.h"


Camera::Camera(UINT windowWidth, UINT windowHeight, XMVECTOR eye, XMVECTOR at, XMVECTOR up, bool isLookAt)
{
	XMStoreFloat4(&_eyeVec, eye);
	XMStoreFloat4(&_atVec, at);
	XMStoreFloat4(&_upVec, up);
	_isLookAt = isLookAt;
	// Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	Update(eye, at, up, _isLookAt);
}


Camera::~Camera()
{

}


void Camera::Update(XMVECTOR eye, XMVECTOR at, XMVECTOR up, bool isLookAt)
{
	// Initialize the view matrix
	if (isLookAt)
	{
		XMStoreFloat4x4(&_view, XMMatrixLookAtLH(eye, at, up));
	}
	else
	{
		XMStoreFloat4x4(&_view, XMMatrixLookToLH(eye, at, up));
	}

	XMStoreFloat4(&_position, eye);
	XMStoreFloat4(&_lookAt, at);
	XMStoreFloat4(&_up, up);
}