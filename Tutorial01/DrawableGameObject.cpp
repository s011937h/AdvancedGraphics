#include "DrawableGameObject.h"

using namespace std;
using namespace DirectX;

#define NUM_VERTICES 36

DrawableGameObject::DrawableGameObject()
{
	pVertexBuffer = nullptr;
	pIndexBuffer = nullptr;
	pTextureRV = nullptr;
	pSamplerLinear = nullptr;
	isParallax = false;

	pVertexShader = nullptr;
    pParallaxVertexShader = nullptr;

    pPixelShader = nullptr;
    pPixelShaderSolid = nullptr;
    pParallaxPixelShader = nullptr;

    pVertexLayout = nullptr;
	pTextureRV = nullptr;
	pNormalTextureRV = nullptr;
	pParallaxTextureRV = nullptr;
	pParallaxColorRV = nullptr;
	pParallaxDisplacementMapRV = nullptr;

	// Initialize the world matrix
	XMStoreFloat4x4(&world, XMMatrixIdentity());
}


DrawableGameObject::~DrawableGameObject()
{
	CleaupGameObject();
}

HRESULT DrawableGameObject::InitGameObjectMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		//top
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f)}, //0
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //1
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //2

		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //3
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //4
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //5

		//bottom
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //6
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //7
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //8

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //9
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //10
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //11

		//left
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //12
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //13
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //14

		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //15
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //16
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //17

		//right
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, .0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //18
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //19
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //20

		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //21
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //22
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //23

		//front
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //24
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //25
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //26

		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //27
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //28
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //29

		//back
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //30
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //31
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //32

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //33
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //34
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.3f, -0.3f, -0.3f) }, //35
	};

	CalculateModelVectors(vertices, 36);

	HRESULT hr = CompileCreateShaders(pd3dDevice, pContext);

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 36;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, pVertexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	ID3D11Buffer* buffers[1] = {
		pVertexBuffer.Get()
	};
	pContext->IASetVertexBuffers(0, 1, buffers, &stride, &offset);

	// Create index buffer
	WORD indices[] =
	{
		0,1,2,
		3,4,5,
		6,7,8,
		9,10,11,
		12,13,14,
		15,16,17,
		18,19,20,
		21,22,23,
		24,25,26,
		27,28,29,
		30,31,32,
		33,34,35
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * NUM_VERTICES;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, pIndexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	// Set index buffer
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // load and setup textures
    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\stone.dds", nullptr, pTextureRV.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\conenormal.dds", nullptr, pNormalTextureRV.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\normals.dds", nullptr, pParallaxTextureRV.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\color.dds", nullptr, pParallaxColorRV.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pd3dDevice, L"Resources\\displacement.dds", nullptr, pParallaxDisplacementMapRV.ReleaseAndGetAddressOf());
    if (FAILED(hr))
        return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState(&sampDesc, pSamplerLinear.ReleaseAndGetAddressOf());

	material.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	material.Material.SpecularPower = 32.0f;
	material.Material.UseTexture = true;

	return hr;
}

void DrawableGameObject::SetPosition(XMFLOAT3 position)
{
	position = position;
}

void DrawableGameObject::Update(float t)
{
	// Cube:  Rotate around origin
	XMMATRIX mSpin = XMMatrixRotationY(-t);

	XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMMATRIX worldMatrix = mTranslate * mSpin;
	XMStoreFloat4x4(&world, worldMatrix);
}

void DrawableGameObject::Draw(ID3D11DeviceContext* pImmediateContext)
{
	if (isParallax)
	{
		pImmediateContext->VSSetShader(pParallaxVertexShader.Get(), nullptr, 0);
		pImmediateContext->PSSetShader(pParallaxPixelShader.Get(), nullptr, 0);
		ID3D11ShaderResourceView* psShaderResources [3] = {
			pParallaxColorRV.Get(),
			pParallaxTextureRV.Get(),
			pParallaxDisplacementMapRV.Get()
		};
		pImmediateContext->PSSetShaderResources(0, 3, psShaderResources);
	}
	else
	{
		pImmediateContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
		pImmediateContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
		ID3D11ShaderResourceView* psShaderResources[2] =
		{
			pTextureRV.Get(),
			pNormalTextureRV.Get()
		};
		pImmediateContext->PSSetShaderResources(0, 2, psShaderResources);
	}

	ID3D11SamplerState* psSamplers [1] = {
		pSamplerLinear.Get()
	};
    pImmediateContext->PSSetSamplers(0, 1, psSamplers);
    pImmediateContext->DrawIndexed(36, 0, 0);
}

HRESULT DrawableGameObject::CompileCreateShaders(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = ShaderManager::Get().CompileShaderFromFile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, pVertexShader.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), pVertexLayout.ReleaseAndGetAddressOf());
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	pImmediateContext->IASetInputLayout(pVertexLayout.Get());

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = ShaderManager::Get().CompileShaderFromFile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pPixelShader.ReleaseAndGetAddressOf());
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;


	// Compile the SOLID pixel shader
	pPSBlob = nullptr;
	hr = ShaderManager::Get().CompileShaderFromFile(L"shader.fx", "PSSolid", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the SOLID pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pPixelShaderSolid.ReleaseAndGetAddressOf());
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Compile the vertex shader
	pVSBlob = nullptr;
	hr = ShaderManager::Get().CompileShaderFromFile(L"parallaxShader.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, pParallaxVertexShader.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Compile the pixel shader
	pPSBlob = nullptr;
	hr = ShaderManager::Get().CompileShaderFromFile(L"parallaxShader.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pParallaxPixelShader.ReleaseAndGetAddressOf());
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;
}

void DrawableGameObject::CleaupGameObject()
{
	if (pVertexBuffer) pVertexBuffer.Reset();
	if (pIndexBuffer) pIndexBuffer.Reset();
	if (pVertexLayout) pVertexLayout.Reset();
	if (pVertexShader) pVertexShader.Reset();
	if (pParallaxVertexShader) pParallaxVertexShader.Reset();
	if (pPixelShader) pPixelShader.Reset();
	if (pPixelShaderSolid) pPixelShaderSolid.Reset();
	if (pParallaxPixelShader) pParallaxPixelShader.Reset();
	if (pTextureRV) pTextureRV.Reset();
	if (pParallaxTextureRV) pParallaxTextureRV.Reset();
	if (pNormalTextureRV) pNormalTextureRV.Reset();
	if (pParallaxColorRV) pParallaxColorRV.Reset();
	if (pParallaxDisplacementMapRV) pParallaxDisplacementMapRV.Reset();
	if (pSamplerLinear) pSamplerLinear.Reset();
}

