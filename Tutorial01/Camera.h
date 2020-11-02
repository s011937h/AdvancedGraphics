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

	XMFLOAT4 GetPosition() const { return _position; }
	XMFLOAT4 GetLookAt() const { return _lookAt; }

	XMFLOAT4X4 GetView() const { return _view; }
	XMFLOAT4X4 GetProjection() const { return _projection; }

	void Update(XMVECTOR eye, XMVECTOR at, XMVECTOR up, bool isLookAt);

private:
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

	XMFLOAT4 _position;
	XMFLOAT4 _up;
	XMFLOAT4 _lookAt;

	XMFLOAT4 _eyeVec;
	XMFLOAT4 _atVec;
	XMFLOAT4 _upVec;
	bool _isLookAt;

	const float PI = 3.14159265358979f;

};

