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
Texture2D txDisplacementMap : register(t2);
SamplerState samLinear : register(s0);

#define MAX_LIGHTS 1
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct _ParallaxMaterial
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
	float   Bias;			// 4 bytes
	float	ScaleFactor;	// 4 bytes
							//----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

cbuffer ParallaxMaterialProperties : register(b1)
{
	_ParallaxMaterial ParallaxMaterial;
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
		specular = pow(saturate(dot(reflection, vertexToEye)), ParallaxMaterial.SpecularPower); // 32 = specular power
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

float2 parallaxOcclusionMapping(in float3 V, in float2 textureCoords, out float parallaxHeight)
{
	// determine optimal number of layers
	const float minLayers = 10;
	const float maxLayers = 15;
	float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0, 0, 1), V)));

	// Calculate height of each layer
	// current depth of the layer
	float curLayerHeight = 0;
	// shift of texture coordinates for each layer
	float2 dtex = parallaxScale * V.xy / V.z / numLayers;

	// current texture coordinates
	float2 currentTextureCoords = textureCoords;

	// get depth from heightmap
	float heightFromTexture = txDisplacementMap.Sample(samLinear, currentTextureCoords).r; //getting red channel of texture (first one)

	// while point is above the surface
	while (heightFromTexture > curLayerHeight)
	{
		// to the next layer
		curLayerHeight += layerHeight;
		// shift of texture coordinates
		currentTextureCoords -= dtex;
		// get new depth from heightmap
		heightFromTexture = txDisplacementMap.Sample(samLinear, currentTextureCoords).r;
	}

	   // previous texture coordinates
	float2 prevTCoords = currentTextureCoords + texStep;

	// heights for linear interpolation
	float nextH = heightFromTexture - curLayerHeight;
	float prevH = txDisplacementMap.Sample(heightFromTexture, prevTCoords).r - curLayerHeight + layerHeight;

	// proportions for linear interpolation
	float weight = nextH / (nextH - prevH);

	// interpolation of texture coordinates
	float2 finalTexCoords = prevTCoords * weight + currentTextureCoords * (1.0 - weight);

	// interpolation of depth values
	parallaxHeight = curLayerHeight + prevH * weight + nextH * (1.0 - weight);

	// return result
	return finalTexCoords;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(PS_INPUT IN) : SV_TARGET
{
	float4 normalMapSample = { 0, 0, 1, 1 };
	if (ParallaxMaterial.UseTexture)
	{
		normalMapSample = txNormalMap.Sample(samLinear, IN.Tex);
	}
	float3 tangent = normalize(IN.Tangent);
	float3 normal = normalize(IN.Norm);

	float3 worldSpaceNormal = TangentToWorldSpace(normalMapSample, normal, tangent);
	LightingResult lit = ComputeLighting(IN.worldPos, worldSpaceNormal);

	float4 texColor = { 1, 1, 1, 1 };

	float4 emissive = ParallaxMaterial.Emissive;
	float4 ambient = ParallaxMaterial.Ambient * GlobalAmbient;
	float4 diffuse = ParallaxMaterial.Diffuse * lit.Diffuse;
	float4 specular = ParallaxMaterial.Specular * lit.Specular;

	if (ParallaxMaterial.UseTexture)
	{
		texColor = txDiffuse.Sample(samLinear, IN.Tex);
	}

	float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;

	return finalColor;
}

//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}