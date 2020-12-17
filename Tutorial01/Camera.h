#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

using namespace DirectX;

class Camera
{
public:
	Camera(UINT windowWidth, UINT windowHeight, XMVECTOR eye, XMVECTOR at, XMVECTOR up, bool isLookAt);
	~Camera();

	XMFLOAT4 GetPosition() const { return m_Position; }

	XMFLOAT4X4 GetView() const { return m_View; }
	XMFLOAT4X4 GetProjection() const { return m_Projection; }

	void MoveCamera(float verticalMovement, float horizontalMovement, float mouseDeltaX, float mouseDeltaY);

private:
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Projection;

	XMFLOAT4 m_Position;

	float m_Roll;
	float m_Pitch;
	float m_Yaw;

	const float PI = 3.14159265358979f;

};

