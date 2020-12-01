#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"
#include "SimpleVertex.h"
#include "ComPtr.h"
#include "ShaderManager.h"

using namespace DirectX;


class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();

	HRESULT								InitGameObjectMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void								Update(float t);
	void								Draw(ID3D11DeviceContext* pContext);
	ComPtr<ID3D11Buffer>				GetVertexBuffer() { return pVertexBuffer; }
	ComPtr<ID3D11Buffer>				GetIndexBuffer() { return pIndexBuffer; }
	ComPtr<ID3D11ShaderResourceView>	GetTextureResourceView() { return pTextureRV; 	}
	XMFLOAT4X4*							GetTransform() { return &world; }
	ComPtr<ID3D11SamplerState>			GetTextureSamplerState() { return pSamplerLinear; }
	MaterialPropertiesConstantBuffer	GetMaterial() { return material;}
	void								SetPosition(XMFLOAT3 position);
	bool								isParallax;
	void								CleaupGameObject();

private:
	
	XMFLOAT4X4							world;

	ComPtr<ID3D11Buffer>				pVertexBuffer;
	ComPtr<ID3D11Buffer>				pIndexBuffer;
	ComPtr<ID3D11ShaderResourceView>	pTextureRV;
	ComPtr<ID3D11SamplerState>			pSamplerLinear;
	MaterialPropertiesConstantBuffer	material;
	XMFLOAT3							position;

	ComPtr<ID3D11ShaderResourceView>	pNormalTextureRV;
	ComPtr<ID3D11ShaderResourceView>	pParallaxTextureRV;
	ComPtr<ID3D11ShaderResourceView>	pParallaxColorRV;
	ComPtr<ID3D11ShaderResourceView>	pParallaxDisplacementMapRV;

	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11VertexShader> pParallaxVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3D11PixelShader> pPixelShaderSolid;
	ComPtr<ID3D11PixelShader> pParallaxPixelShader;

	ComPtr<ID3D11InputLayout> pVertexLayout;

	HRESULT CompileCreateShaders(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	
};

