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

using namespace std;

typedef vector<DrawableGameObject*> vecDrawables;

using namespace DirectX;
//make sure to change the input layout if you change this structure

class Application
{
private:
	HINSTANCE				g_hInst;
	HWND					g_hWnd;
	D3D_DRIVER_TYPE			g_driverType;
	D3D_FEATURE_LEVEL		g_featureLevel;
	ID3D11Device*			g_pd3dDevice;
	ID3D11Device1*			g_pd3dDevice1;
	ID3D11DeviceContext*	g_pImmediateContext;
	ID3D11DeviceContext1*	g_pImmediateContext1;
	IDXGISwapChain*			g_pSwapChain;
	IDXGISwapChain1*		g_pSwapChain1;
	ID3D11RenderTargetView* g_pRenderTargetView;
	ID3D11Texture2D*		g_pDepthStencil;
	ID3D11DepthStencilView* g_pDepthStencilView;
	ID3D11VertexShader*		g_pVertexShader;

	ID3D11PixelShader*		g_pPixelShader;
	ID3D11PixelShader*		g_pPixelShaderSolid;

	ID3D11InputLayout*		g_pVertexLayout;
	ID3D11Buffer*			g_pVertexBuffer;
	ID3D11Buffer*			g_pIndexBuffer;

	ID3D11Buffer*			g_pConstantBuffer;
	ID3D11Buffer*			g_pMaterialConstantBuffer;
	ID3D11Buffer*			g_pLightConstantBuffer;

	ID3D11ShaderResourceView* g_pTextureRV;
	ID3D11ShaderResourceView* g_pNormalTextureRV;
	ID3D11ShaderResourceView* g_pParallaxTextureRV;

	ID3D11SamplerState*		g_pSamplerLinear;
	ID3D11SamplerState*		g_pSamplerNormal;

	XMMATRIX                g_World1;
	XMMATRIX                g_Projection;

	int						g_viewWidth;
	int						g_viewHeight;

	DrawableGameObject		g_GameObject;

	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	HRESULT InitMesh();
	HRESULT	InitWorld(int width, int height);
	void	CleanupDevice();

	UINT _WindowHeight;
	UINT _WindowWidth;

	XMFLOAT4 g_EyePosition;
	//XMFLOAT4 g_LightPosition;

	float _XValue;
	float _YValue;
	float _ZValue;

	Camera* _camera;
	XMFLOAT3 uvf3 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 movementVector;
	XMFLOAT3 newCameraPos;
	XMFLOAT3 cameraPos;
	XMFLOAT4X4 _cameraPosM;



	/*my code from before below
	XMFLOAT4X4              _world, _world2, _world3, _world4, _plane, _verticalPlane, _car, _ball, _boost, _verticalPlane2, _verticalPlane3, _verticalPlane4; //add new world when you add new object

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solidObject;

	XMVECTOR lookToPos;

	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;
	XMFLOAT4 ambientMaterial;
	XMFLOAT4 ambientLight;

	XMFLOAT4 specularMaterial;
	XMFLOAT4 specularLight;
	float specularPower;
	XMFLOAT4 eyePositionW;
	ID3D11ShaderResourceView* _pTextureRV;
	ID3D11ShaderResourceView* _pTextureAlpha = nullptr;
	ID3D11ShaderResourceView* _pTextureCar = nullptr;
	ID3D11ShaderResourceView* _pTextureGround = nullptr;
	ID3D11SamplerState* _pSamplerLinear = nullptr;

	ID3D11BlendState* Transparency;


	bool firstLoad = true;
	*/

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Render();
};

