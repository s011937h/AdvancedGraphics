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

using namespace DirectX;


class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();

	HRESULT								InitGameObjectMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void								Update(float t);
	void								Draw(ID3D11DeviceContext* pContext);
	ID3D11Buffer*						GetVertexBuffer() { return pVertexBuffer; }
	ID3D11Buffer*						GetIndexBuffer() { return pIndexBuffer; }
	ID3D11ShaderResourceView**			GetTextureResourceView() { return &pTextureRV; 	}
	XMFLOAT4X4*							GetTransform() { return &world; }
	ID3D11SamplerState**				GetTextureSamplerState() { return &pSamplerLinear; }
	MaterialPropertiesConstantBuffer	GetMaterial() { return material;}
	void								SetPosition(XMFLOAT3 position);
	bool								isParallax;
	void								CleaupGameObject();

private:
	
	XMFLOAT4X4							world;

	ID3D11Buffer*						pVertexBuffer;
	ID3D11Buffer*						pIndexBuffer;
	ID3D11ShaderResourceView*			pTextureRV;
	ID3D11SamplerState *				pSamplerLinear;
	MaterialPropertiesConstantBuffer	material;
	XMFLOAT3							position;

	ID3D11ShaderResourceView*			pNormalTextureRV;
	ID3D11ShaderResourceView*			pParallaxTextureRV;
	ID3D11ShaderResourceView*			pParallaxColorRV;
	ID3D11ShaderResourceView*			pParallaxDisplacementMapRV;

	ID3D11VertexShader* pVertexShader;
	ID3D11VertexShader* pParallaxVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11PixelShader* pPixelShaderSolid;
	ID3D11PixelShader* pParallaxPixelShader;

	ID3D11InputLayout* pVertexLayout;

	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT CompileCreateShaders(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	
};

