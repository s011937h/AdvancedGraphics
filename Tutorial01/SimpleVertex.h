#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Tangent;
	//XMFLOAT3 biTangent;
};
void CalculateTangent(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& tangent);
void CalculateModelVectors(SimpleVertex* vertices, int vertexCount);