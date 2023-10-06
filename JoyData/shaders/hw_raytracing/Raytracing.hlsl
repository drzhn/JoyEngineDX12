//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "CommonEngineStructs.h"
#include "RaytracingInclude.hlsl"

// global
RaytracingAccelerationStructure g_SceneAccelerationStructure : register(t0, space0);
RWTexture2D<float4> g_OutputRenderTarget : register(u0, space0);

ConstantBuffer<EngineData> g_engineData : register(b0, space0);
ConstantBuffer<RaytracedProbesData> raytracedProbesData : register(b1, space0);

//StructuredBuffer<Triangle> triangleData; // size = THREADS_PER_BLOCK * BLOCK_SIZE

// using of multiple spaces is the hack to bind bindless srv of different types
//StructuredBuffer<Vertex> objectVertices[] : register(t0, space1);
//StructuredBuffer<UINT1> objectIndices[] : register(t0, space2);

//ConstantBuffer<StandardMaterialData> materials;
Texture2D textures[] : register(t0, space3);
SamplerState linearClampSampler : register(s0, space0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

struct RayPayload
{
	float4 color;
};

// TODO Should be in lib_6_8 and hlsl 2021
//struct [raypayload] RayPayload
//{
//    float4 color : read(caller) : write(caller, closesthit, miss);
//};

[shader("raygeneration")]
void MyRaygenShader()
{
	uint3 id = DispatchRaysIndex();

	const float near = g_engineData.cameraNear;
	const float fov = tan(g_engineData.cameraFovRadians / 2);
	const float height = 2 * near * fov;
	const float width = g_engineData.screenWidth * height / g_engineData.screenHeight;

	const float3 originCamera = float3(0, 0, 0);
	const float3 dirCamera = float3(
		-width / 2 + width / g_engineData.screenWidth * (id.x + 0.5),
		height / 2 - height / g_engineData.screenHeight * (id.y + 0.5),
		near
	);

	const float3 origin = mul(g_engineData.cameraInvView, float4(originCamera, 1)).xyz;
	const float3 dir = mul(g_engineData.cameraInvView, float4(dirCamera, 0)).xyz;

	// Trace the ray.
	// Set the ray's extents.
	RayDesc ray;
	ray.Origin = origin;
    ray.Direction = dir;
	// Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
	// TMin should be kept small to prevent missing geometry at close contact areas.
	ray.TMin = 0.001;
	ray.TMax = 10000.0;
	RayPayload payload = {float4(0, 0, 0, 0)};
	TraceRay(g_SceneAccelerationStructure, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 0, 0, ray, payload);

	// Write the raytraced color to the output texture.
	g_OutputRenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

    payload.color = float4(1, 0.1, 0.1, 1); //float4(barycentrics, 1);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = textures[raytracedProbesData.skyboxTextureIndex].SampleLevel(linearClampSampler, SampleSphericalMap(-normalize(WorldRayDirection())), 2);
}

#endif // RAYTRACING_HLSL
