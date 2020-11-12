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
	HINSTANCE				hInst;
	HWND					hWnd;
	D3D_DRIVER_TYPE			driverType;
	D3D_FEATURE_LEVEL		featureLevel;
	ID3D11Device* pd3dDevice;
	ID3D11Device1* pd3dDevice1;
	ID3D11DeviceContext* pImmediateContext;
	ID3D11DeviceContext1* pImmediateContext1;
	IDXGISwapChain* pSwapChain;
	IDXGISwapChain1* pSwapChain1;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11Texture2D* pDepthStencil;
	ID3D11DepthStencilView* pDepthStencilView;
	ID3D11VertexShader* pVertexShader;
	ID3D11VertexShader* pParallaxVertexShader;

	ID3D11PixelShader* pPixelShader;
	ID3D11PixelShader* pPixelShaderSolid;
	ID3D11PixelShader* pParallaxPixelShader;

	ID3D11InputLayout* pVertexLayout;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;

	ID3D11Buffer* pConstantBuffer;
	ID3D11Buffer* pMaterialConstantBuffer;
	ID3D11Buffer* pLightConstantBuffer;

	ID3D11ShaderResourceView* pTextureRV;
	ID3D11ShaderResourceView* pNormalTextureRV;
	ID3D11ShaderResourceView* pParallaxTextureRV;
	ID3D11ShaderResourceView* pParallaxColorRV;
	ID3D11ShaderResourceView* pParallaxDisplacementMapRV;

	ID3D11SamplerState* pSamplerLinear;
	ID3D11SamplerState* pSamplerNormal;

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

	HRESULT CompileAndCreateVertexShader(const WCHAR* shaderFilename, const LPCSTR shaderName, ID3D11VertexShader* vertexShader);
	HRESULT CompileAndCreatePixelShader(const WCHAR* shaderFilename, const LPCSTR shaderName, ID3D11PixelShader* pixelShader);

	UINT windowHeight;
	UINT windowWidth;

	XMFLOAT4 eyePosition;

	float _XValue;
	float _YValue;
	float _ZValue;

	Camera* currentCamera;
	XMFLOAT3 uvf3 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 movementVector;
	XMFLOAT3 newCameraPos;
	XMFLOAT3 cameraPos;
	XMFLOAT4X4 _cameraPosM;



	/*my code from before below

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

