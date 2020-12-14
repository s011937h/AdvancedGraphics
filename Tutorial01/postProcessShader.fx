//--------------------------------------------------------------------------------------
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// the lighting equations in this code have been taken from https://www.3dgep.com/texturing-lighting-directx-11/
// with some modifications by David White

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
}

cbuffer PostProcessBuffer : register(b1)
{
	int enableColourInversion;
}

Texture2D txLightAccumulation : register(t0);
SamplerState samLinear : register(s0);

#define MAX_LIGHTS 1
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	uint VertexID : SV_VertexID;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	switch (input.VertexID)
	{
	default:
	case 0:
		output.Pos = float4(-1, -1, 0, 1);
		break;
	case 1:
		output.Pos = float4(-1, 3, 0, 1);
		break;
	case 2:
		output.Pos = float4(3, -1, 0, 1);
		break;
	}
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(PS_INPUT IN) : SV_TARGET
{
	float2 coordinates = IN.Pos.xy;
	float2 screenSize;
	txLightAccumulation.GetDimensions(screenSize.x, screenSize.y);
	coordinates /= screenSize;

	float4 lightAccumulationSample = { 0, 0, 1, 1 };
	lightAccumulationSample = txLightAccumulation.Sample(samLinear, coordinates);


	/***********************************************

	MARKING SCHEME: Simple screen space effect

	DESCRIPTION: Colour inversion
	
	***********************************************/
	if (enableColourInversion)
	{
		return float4(float3(1, 1, 1) - lightAccumulationSample.rgb, 1); //colour inversion
	}
	else
	{
		return lightAccumulationSample;
	}
}

