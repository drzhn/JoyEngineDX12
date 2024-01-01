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

ConstantBuffer<EngineData> g_engineData : register(b0, space0);
ConstantBuffer<RaytracedProbesData> raytracedProbesData : register(b1, space0);

StructuredBuffer<MeshData> meshData : register(t1, space0); // size = THREADS_PER_BLOCK * BLOCK_SIZE
StructuredBuffer<Vertex> objectVertices : register(t0, space1);
StructuredBuffer<UINT1> objectIndices : register(t0, space2);

ConstantBuffer<StandardMaterialData> materials : register(b2, space0);
Texture2D textures[] : register(t0, space3);
SamplerState linearClampSampler : register(s0, space0);

RWTexture2D<float4> colorTexture : register(u0, space0);
RWTexture2D<float4> normalsTexture : register(u1, space0);
RWTexture2D<float4> positionTexture : register(u2, space0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

// TODO Should be in lib_6_8 and hlsl 2021
//struct [raypayload] RayPayload
//{
//    float4 color : read(caller) : write(caller, closesthit, miss);
//};

[shader("raygeneration")]
void MyRaygenShader()
{
#if defined(HW_CAMERA_TRACE)
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
#else

	const uint3 id = DispatchRaysIndex();
	const float3 probeId = float3(
		(id.x / (raytracedProbesData.gridY * raytracedProbesData.gridZ)),
		((id.x / raytracedProbesData.gridZ) % raytracedProbesData.gridY),
		(id.x % raytracedProbesData.gridZ)
	);
	const float3 origin = raytracedProbesData.gridMin + probeId * raytracedProbesData.cellSize;
	const float3 dir = sphericalFibonacci(id.y, DDGI_RAYS_COUNT);

#endif

	// Trace the ray.
	// Set the ray's extents.
	RayDesc ray;
	ray.Origin = origin;
	ray.Direction = dir;
	// Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
	// TMin should be kept small to prevent missing geometry at close contact areas.
	ray.TMin = 0.001;
	ray.TMax = 10000.0;
	HardwareRayPayload payload =
	{
		float4(0, 0, 0, 0),
		float4(0, 0, 0, 0),
		float4(0, 0, 0, 0)
	};
	TraceRay(g_SceneAccelerationStructure, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 0, 0, ray, payload);

	// Write the raytraced color to the output texture.
	colorTexture[id.xy] = payload.color;
	normalsTexture[id.xy] = payload.normals;
	positionTexture[id.xy] = payload.position;
}

[shader("closesthit")]
void MyClosestHitShader(inout HardwareRayPayload payload, in MyAttributes attr)
{
	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

	const MeshData md = meshData[GeometryIndex()];

	const Vertex v0 = objectVertices[md.verticesIndex + objectIndices[md.indicesIndex + PrimitiveIndex() * 3 + 0]];
	const Vertex v1 = objectVertices[md.verticesIndex + objectIndices[md.indicesIndex + PrimitiveIndex() * 3 + 1]];
	const Vertex v2 = objectVertices[md.verticesIndex + objectIndices[md.indicesIndex + PrimitiveIndex() * 3 + 2]];

	const float2 uv = barycentrics.x * v0.texCoord + barycentrics.y * v1.texCoord + barycentrics.z * v2.texCoord;
	const float3 normal =
		barycentrics.x * unpackRGB10A2Unorm(v0.normal).xyz +
		barycentrics.y * unpackRGB10A2Unorm(v1.normal).xyz +
		barycentrics.z * unpackRGB10A2Unorm(v2.normal).xyz;
	const uint materialIndex = md.materialIndex;

	float4 color = textures[materials.data[materialIndex].DiffuseMap].SampleLevel(linearClampSampler, uv, 2);

	payload.color = float4(color.rgb, 1);
	payload.normals = float4(normalize(normal), 1);
	payload.position = float4(WorldRayOrigin() + WorldRayDirection() * RayTCurrent(), 1);
}

[shader("miss")]
void MyMissShader(inout HardwareRayPayload payload)
{
	float4 skyboxColor = textures[raytracedProbesData.skyboxTextureIndex].SampleLevel(linearClampSampler, SampleSphericalMap(-normalize(WorldRayDirection())), 2);
	payload.color = float4(skyboxColor.rgb, 1);
	payload.normals = float4(0, 0, 0, 0);
	payload.position = float4(0, 0, 0, 0);
}

#endif // RAYTRACING_HLSL
