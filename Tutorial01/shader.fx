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

Texture2D txDiffuse : register(t0);
Texture2D txNormalMap : register(t1);
SamplerState samLinear : register(s0);

#define MAX_LIGHTS 1
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct _Material
{
	float4  Emissive;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Ambient;        // 16 bytes
							//------------------------------------(16 byte boundary)
	float4  Diffuse;        // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Specular;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float   SpecularPower;  // 4 bytes
	bool    UseTexture;     // 4 bytes
	float2  Padding;        // 8 bytes
							//----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

cbuffer MaterialProperties : register(b1)
{
	_Material Material;
};

struct Light
{
	float4      Position;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Direction;              // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Color;                  // 16 bytes
										//----------------------------------- (16 byte boundary)
	float       SpotAngle;              // 4 bytes
	float       ConstantAttenuation;    // 4 bytes
	float       LinearAttenuation;      // 4 bytes
	float       QuadraticAttenuation;   // 4 bytes
										//----------------------------------- (16 byte boundary)
	int         LightType;              // 4 bytes
	bool        Enabled;                // 4 bytes
	int2        Padding;                // 8 bytes
										//----------------------------------- (16 byte boundary)
};  // Total:                           // 80 bytes (5 * 16)

cbuffer LightProperties : register(b2)
{
	float4 EyePosition;                 // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4 GlobalAmbient;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS];           // 80 * 8 = 640 bytes
}; 

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Tangent : TANGENT;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Tangent : TANGENT;
};

struct PS_OUTPUT
{
	float4 Diffuse : SV_TARGET0;
	float4 Specular : SV_TARGET1;
	float4 Emissive : SV_TARGET2;
	float4 Normal : SV_TARGET3;
};


float4 DoDiffuse(Light light, float3 L, float3 N)
{
	float NdotL = max(0, dot(N, L));
	return light.Color * NdotL;
}

float4 DoSpecular(Light lightObject, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal)
{
	float4 lightDir = float4(normalize(-lightDirectionToVertex),1);
	vertexToEye = normalize(vertexToEye);

	float lightIntensity = saturate(dot(Normal, lightDir));
	float4 specular = float4(0, 0, 0, 0);
	if (lightIntensity > 0.0f)
	{
		float3  reflection = normalize(2 * lightIntensity * Normal - lightDir);
		specular = pow(saturate(dot(reflection, vertexToEye)), Material.SpecularPower); // 32 = specular power
	}

	return specular;
}

float DoAttenuation(Light light, float d)
{
	return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d);
}

struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};

LightingResult DoPointLight(Light light, float3 vertexToEye, float4 vertexPos, float3 N)
{
	LightingResult result;

	float3 LightDirectionToVertex = (vertexPos - light.Position).xyz;
	float distance = length(LightDirectionToVertex);
	LightDirectionToVertex = LightDirectionToVertex  / distance;

	float3 vertexToLight = (light.Position - vertexPos).xyz;
	distance = length(vertexToLight);
	vertexToLight = vertexToLight / distance;

	float attenuation = DoAttenuation(light, distance);
	attenuation = 1;


	result.Diffuse = DoDiffuse(light, vertexToLight, N) * attenuation;
	result.Specular = DoSpecular(light, vertexToEye, LightDirectionToVertex, N) * attenuation;

	return result;
}

LightingResult ComputeLighting(float4 vertexPos, float3 N)
{
	float3 vertexToEye = normalize(EyePosition - vertexPos).xyz;

	LightingResult totalResult = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		LightingResult result = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };

		if (!Lights[i].Enabled) 
			continue;
		
		result = DoPointLight(Lights[i], vertexToEye, vertexPos, N);
		
		totalResult.Diffuse += result.Diffuse;
		totalResult.Specular += result.Specular;
	}

	totalResult.Diffuse = saturate(totalResult.Diffuse);
	totalResult.Specular = saturate(totalResult.Specular);

	return totalResult;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
	output.worldPos = output.Pos;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

	// multiply the normal by the world transform (to go from model space to world space)
	output.Norm = mul(float4(normalize(input.Norm), 1), World).xyz;
	output.Tex = input.Tex;
	output.Tangent = mul(float4(normalize(input.Tangent), 1), World).xyz;
    
    return output;
}

/***********************************************

MARKING SCHEME: Normal Mapping

DESCRIPTION: Map sampling, normal value decompression, transformation to tangent space

***********************************************/

float3 TangentToWorldSpace(float3 normalMapSample, float3 normal, float3 tangent)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f * normalMapSample - 1.0f;
	float3 N = normal;
	float3 T = normalize(tangent - dot(tangent, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = {T, B, N};

	float3 bumpNorm = mul(normalT, TBN);
	return bumpNorm;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

PS_OUTPUT PS(PS_INPUT IN)
{
	PS_OUTPUT output;
	float4 normalMapSample = { 0, 0, 1, 1 };
	if (Material.UseTexture)
	{
		normalMapSample = txNormalMap.Sample(samLinear, IN.Tex);
	}
	float3 tangent = normalize(IN.Tangent);
	float3 normal = normalize(IN.Norm);

	float3 worldSpaceNormal = TangentToWorldSpace(normalMapSample, normal, tangent);
	output.Normal = float4(worldSpaceNormal, 0);

	output.Emissive = Material.Emissive;
	output.Specular.w = Material.SpecularPower;
	output.Specular.rgb = Material.Specular.rgb;

	float4 texColor = { 1, 1, 1, 1 };
	if (Material.UseTexture)
	{
		texColor = txDiffuse.Sample(samLinear, IN.Tex);
	}
	output.Diffuse = texColor * Material.Diffuse;

	return output;
}

//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}


float4 ForwardPS(PS_INPUT IN) : SV_TARGET
{
	float4 normalMapSample = { 0, 0, 1, 1 };
	if (Material.UseTexture)
	{
		normalMapSample = txNormalMap.Sample(samLinear, IN.Tex);
	}
	float3 tangent = normalize(IN.Tangent);
	float3 normal = normalize(IN.Norm);

	float3 worldSpaceNormal = TangentToWorldSpace(normalMapSample, normal, tangent);
	LightingResult lit = ComputeLighting(IN.worldPos, worldSpaceNormal);

	float4 texColor = { 1, 1, 1, 1 };

	float4 emissive = Material.Emissive;
	float4 ambient = Material.Ambient * GlobalAmbient;
	float4 diffuse = Material.Diffuse * lit.Diffuse;
	float4 specular = Material.Specular * lit.Specular;

	if (Material.UseTexture)
	{
		texColor = txDiffuse.Sample(samLinear, IN.Tex);
	}

	float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;

	return finalColor;
}
