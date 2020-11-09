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

	XMFLOAT4 GetPosition() const { return position; }
	//XMFLOAT4 GetLookAt() const { return _lookAt; }

	XMFLOAT4X4 GetView() const { return view; }
	XMFLOAT4X4 GetProjection() const { return projection; }

	void Update(XMVECTOR eye, float mouseDeltaX, float mouseDeltaY);
	void MoveCamera(float x, float y, float z, float roll, float pitch, float yaw);

private:
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;

	XMFLOAT4 position;
	//XMFLOAT4 _up;
	//XMFLOAT4 _lookAt;

	//XMFLOAT4 _eyeVec;
	//XMFLOAT4 _atVec;
	//XMFLOAT4 _upVec;
	//bool _isLookAt;

	float roll;
	float pitch;
	float yaw;

	const float PI = 3.14159265358979f;

};

