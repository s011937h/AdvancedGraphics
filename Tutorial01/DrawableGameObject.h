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
	ComPtr<ID3D11Buffer>				GetVertexBuffer() { return m_VertexBuffer; }
	ComPtr<ID3D11Buffer>				GetIndexBuffer() { return m_IndexBuffer; }
	ComPtr<ID3D11ShaderResourceView>	GetTextureResourceView() { return m_TextureRV; 	}
	XMFLOAT4X4*							GetTransform() { return &m_World; }
	ComPtr<ID3D11SamplerState>			GetTextureSamplerState() { return m_SamplerLinear; }
	MaterialPropertiesConstantBuffer	GetMaterial() { return m_Material;}
	void								SetPosition(XMFLOAT3 position);
	void								CleaupGameObject();

	enum MaterialType {
		material_NormalMapped,
		material_StandardParallax,
		material_ParallaxOcclusion
	};

	MaterialType GetMaterialType() { return m_MaterialType; }
	void SetMaterialType(MaterialType materialType) { m_MaterialType = materialType; }

	bool GetIsSpinning() { return m_IsSpinning; }
	void SetIsSpinning(bool isSpinning) { m_IsSpinning = isSpinning; }

private:
	//TODO: m_ all of these
	XMFLOAT4X4							m_World;

	ComPtr<ID3D11Buffer>				m_VertexBuffer;
	ComPtr<ID3D11Buffer>				m_IndexBuffer;
	ComPtr<ID3D11ShaderResourceView>	m_TextureRV;
	ComPtr<ID3D11SamplerState>			m_SamplerLinear;
	MaterialPropertiesConstantBuffer	m_Material;
	XMFLOAT3							m_Position;

	ComPtr<ID3D11ShaderResourceView>	m_NormalTextureRV;
	ComPtr<ID3D11ShaderResourceView>	m_ParallaxTextureRV;
	ComPtr<ID3D11ShaderResourceView>	m_ParallaxColorRV;
	ComPtr<ID3D11ShaderResourceView>	m_ParallaxDisplacementMapRV;

	ComPtr<ID3D11VertexShader> m_VertexShader;
	ComPtr<ID3D11VertexShader> m_ParallaxVertexShader;
	ComPtr<ID3D11PixelShader> m_PixelShader;
	ComPtr<ID3D11PixelShader> m_PixelShaderSolid;
	ComPtr<ID3D11PixelShader> m_ParallaxPixelShader;
	ComPtr<ID3D11VertexShader> m_StandardParallaxVertexShader;
	ComPtr<ID3D11PixelShader> m_StandardParallaxPixelShader;

	ComPtr<ID3D11InputLayout> m_VertexLayout;

	HRESULT CompileCreateShaders(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);

	MaterialType m_MaterialType;
	bool m_IsSpinning;
	
};

