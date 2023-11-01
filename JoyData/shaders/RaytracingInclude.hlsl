#ifndef RAYTRACING_INCLUDE
#define RAYTRACING_INCLUDE

#include "CommonEngineStructs.h"

float2 SampleSphericalMap(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv *= float2(-1, 1);

	//float r = length(v);
	//float theta = acos(-v.y);
	//float phi = atan2(v.x, -v.z);
	//float2 uv = float2(
	//	0.5 + phi / 2.0/ PI,
	//	theta / PI
	//	);
    return uv;
}

inline float madfrac(float A, float B)
{
    return ((A) * (B) - floor((A) * (B)));
}

inline float3 sphericalFibonacci(float i, float n)
{
    float PHI = sqrt(5) * 0.5 + 0.5;
    float phi = 2.0 * PI * madfrac(i, PHI - 1);
    float cosTheta = 1.0 - (2.0 * i + 1.0) * (1.0 / n);
    float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));

    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

inline float4 unpackVertexPosition(PACKED_HALF4 position)
{
    return float4(
        f16tof32((position.x) & 0xFFFF),
        f16tof32((position.x >> 16) & 0xFFFF),
        f16tof32((position.y) & 0xFFFF),
        f16tof32((position.y >> 16) & 0xFFFF)
    );
}

inline float4 unpackRGB10A2Unorm(PACKED_RGB10A2_UNORM value)
{
    return float4(
     (((value >> 0) & (0x3FF)) / 1023.0) * 2.0 - 1.0,
     (((value >> 10) & (0x3FF)) / 1023.0) * 2.0 - 1.0,
     (((value >> 20) & (0x3FF)) / 1023.0) * 2.0 - 1.0,
     (((value >> 30) & (3)) / 3.0) * 2.0 - 1.0
    );
}

#endif
