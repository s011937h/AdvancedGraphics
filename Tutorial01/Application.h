#pragma once

#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>

#include "DrawableGameObject.h"
#include "structures.h"
#include "Camera.h"

#include <vector>
#include <memory>

using namespace std;

typedef vector<DrawableGameObject*> vecDrawables;

using namespace DirectX;
//make sure to change the input layout if you change this structure


void SetDebugName(ID3D11DeviceChild* object, const std::string& name);
void SetDebugName(const ComPtr<ID3D11DeviceChild> & object, const std::string& name);

class Application
{
private:
	HINSTANCE				hInst;
	HWND					hWnd;
	D3D_DRIVER_TYPE			driverType;
	D3D_FEATURE_LEVEL		featureLevel;
	ComPtr<ID3D11Device>    pd3dDevice;
	ComPtr<ID3D11Device1>   pd3dDevice1;
	ComPtr<ID3D11Debug>		d3dDebug;
	ComPtr<ID3D11DeviceContext>  pImmediateContext;
	ComPtr<ID3D11DeviceContext1> pImmediateContext1;
	ComPtr<IDXGISwapChain>		 pSwapChain;
	ComPtr<IDXGISwapChain1>		 pSwapChain1;
	ComPtr<ID3D11RenderTargetView> pRenderTargetView;
	ComPtr<ID3D11Texture2D>		 pDepthStencil;
	ComPtr<ID3D11DepthStencilView> pDepthStencilView;

	ComPtr<ID3D11Buffer> pConstantBuffer;
	ComPtr<ID3D11Buffer> pMaterialConstantBuffer;
	ComPtr<ID3D11Buffer> pLightConstantBuffer;


	ComPtr<ID3D11SamplerState> pSamplerLinear;
	ComPtr<ID3D11SamplerState> pSamplerNormal;

	XMMATRIX                world1; //add new world when you add new object
	XMMATRIX                projection;

	int						viewWidth;
	int						viewHeight;

	DrawableGameObject		gameObject;

	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	HRESULT InitMesh();
	HRESULT	InitWorld(int width, int height);
	void	CleanupDevice();

	UINT windowHeight;
	UINT windowWidth;

	XMFLOAT4 eyePosition;

	float _XValue;
	float _YValue;
	float _ZValue;

	std::unique_ptr<Camera> currentCamera;
	XMFLOAT3 uvf3 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 movementVector;
	XMFLOAT3 newCameraPos;
	XMFLOAT3 cameraPos;
	XMFLOAT4X4 cameraPosM;


public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Render();
};

