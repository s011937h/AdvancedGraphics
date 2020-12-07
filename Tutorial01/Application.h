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
	ComPtr<ID3D11Device>    m_pd3dDevice;
	ComPtr<ID3D11Device1>   m_pd3dDevice1;
	ComPtr<ID3D11Debug>		m_d3dDebug;
	ComPtr<ID3D11DeviceContext>  m_ImmediateContext; //replace p with m_
	ComPtr<ID3D11DeviceContext1> m_ImmediateContext1;
	ComPtr<IDXGISwapChain>		 m_SwapChain;
	ComPtr<IDXGISwapChain1>		 m_SwapChain1;
	ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
	ComPtr<ID3D11Texture2D>		 m_DepthStencil;
	ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	ComPtr<ID3D11Buffer> m_ConstantBuffer;
	ComPtr<ID3D11Buffer> m_MaterialConstantBuffer;
	ComPtr<ID3D11Buffer> m_ParallaxMaterialConstantBuffer;
	ComPtr<ID3D11Buffer> m_LightConstantBuffer;
	ComPtr<ID3D11Buffer> m_PostProcessBuffer; //reset me too

	ComPtr<ID3D11SamplerState> m_SamplerLinear;
	ComPtr<ID3D11SamplerState> m_SamplerNormal;

	ComPtr<ID3D11Texture2D> m_LightAccumulation; //reset all in cleanupdevice
	ComPtr<ID3D11RenderTargetView> m_LightAccumulationRTV;
	ComPtr<ID3D11ShaderResourceView> m_LightAccumulationSRV;
	ComPtr<ID3D11VertexShader> m_PostProcessVertexShader;
	ComPtr<ID3D11PixelShader> m_PostProcessPixelShader;

	bool colourInversion = false;


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

	void CharTyped(char charTyped);
};

