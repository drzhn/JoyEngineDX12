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

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};

struct RayGenConstantBuffer
{
	Viewport viewport;
	Viewport stencil;
};

// global
RaytracingAccelerationStructure g_SceneAccelerationStructure : register(t0, space0);
RWTexture2D<float4> g_OutputRenderTarget : register(u0);

// local
ConstantBuffer<RayGenConstantBuffer> screenParams : register(b0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

struct [raypayload] RayPayload
{
    float4 color : read(caller) : write(caller, closesthit, miss);
};

bool IsInsideViewport(float2 p, Viewport viewport)
{
	return (p.x >= viewport.left && p.x <= viewport.right)
		&& (p.y >= viewport.top && p.y <= viewport.bottom);
}

[shader("raygeneration")]
void MyRaygenShader()
{
	float2 lerpValues = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();

	// Orthographic projection since we're raytracing in screen space.
	float3 rayDir = float3(0, 0, 1);
	float3 origin = float3(
		lerp(screenParams.viewport.left, screenParams.viewport.right, lerpValues.x),
		lerp(screenParams.viewport.top, screenParams.viewport.bottom, lerpValues.y),
		0.0f);

	if (IsInsideViewport(origin.xy, screenParams.stencil))
	{
		// Trace the ray.
		// Set the ray's extents.
		RayDesc ray;
		ray.Origin = origin;
		ray.Direction = rayDir;
		// Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
		// TMin should be kept small to prevent missing geometry at close contact areas.
		ray.TMin = 0.001;
		ray.TMax = 10000.0;
		RayPayload payload = {float4(0, 0, 0, 0)};
		TraceRay(g_SceneAccelerationStructure, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

		// Write the raytraced color to the output texture.
		g_OutputRenderTarget[DispatchRaysIndex().xy] = payload.color;
	}
	else
	{
		// Render interpolated DispatchRaysIndex outside the stencil window
		g_OutputRenderTarget[DispatchRaysIndex().xy] = float4(lerpValues, 0, 1);
	}
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
	float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
	payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
	payload.color = float4(0, 0, 0, 1);
}

#endif // RAYTRACING_HLSL
