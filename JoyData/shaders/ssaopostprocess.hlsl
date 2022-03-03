#include "common.hlsl"

struct PSInput
{
	float4 PosH : SV_POSITION;
	float3 PosV : POSITION;
	float2 TexC : TEXCOORD0;
};

ConstantBuffer<MVP> mvp : register(b0);
ConstantBuffer<EngineData> engineData : register(b1);
ConstantBuffer<SSAOData> ssaoData : register(b2);

Texture2D<float> depthTexture : register(t0);
Texture2D<float4> viewNormalTexture : register(t1);
Texture2D gRandomVecMap : register(t2);

SamplerState gsamDepthMap : register(s0);
SamplerState gsamLinearWrap : register(s1);
SamplerState gsamPointClamp : register(s2);


static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),

	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

static float gOcclusionRadius = 0.5f;
static float gOcclusionFadeStart = 0.2f;
static float gOcclusionFadeEnd = 2.0f;
static float gSurfaceEpsilon = 0.05f;

static const int gSampleCount = 14;

inline float LinearEyeDepth(float depth)
{
	// z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
	float viewZ = mvp.projection[2][3] / (depth - mvp.projection[2][2]);
	return viewZ;
}

// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;

		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}

	return occlusion;
}

PSInput VSMain(uint vid : SV_VertexID)
{
	PSInput vout;
	vout.TexC = gTexCoords[vid];

	// Quad covering screen in NDC space.
	vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);

	// Transform quad corners to view space near plane.
	float4 ph = mul(engineData.cameraInvProj, vout.PosH);
	vout.PosV = ph.xyz / ph.w;

	return vout;
}

float PSMain(PSInput v) : SV_Target
{
	//float4 n = viewNormalTexture.Load(float3(v.PosH.xy, 0));
	//float pz = depthTexture.Load(float3(v.PosH.xy, 0));

	// p -- the point we are computing the ambient occlusion for.
	// n -- normal vector at p.
	// q -- a random offset from p.
	// r -- a potential occluder that might occlude p.

	// Get viewspace normal and z-coord of this pixel.  
	float3 n = normalize(viewNormalTexture.SampleLevel(gsamPointClamp, v.TexC, 0.0f).xyz);

	float pz = depthTexture.SampleLevel(gsamDepthMap, v.TexC, 0.0f).r;
	pz = LinearEyeDepth(pz);

	//
	// Reconstruct full view space position (x,y,z).
	// Find t such that p = t*pin.PosV.
	// p.z = t*pin.PosV.z
	// t = p.z / pin.PosV.z
	//
	float3 p = (pz / v.PosV.z) * v.PosV;

	// Extract random vector and map from [0,1] --> [-1, +1].
	float3 randVec = 2.0f * gRandomVecMap.SampleLevel(gsamLinearWrap, 4.0f * v.TexC, 0.0f).rgb - 1.0f;

	float occlusionSum = 0.0f;

	// Sample neighboring points about p in the hemisphere oriented by n.
	for (int i = 0; i < gSampleCount; ++i)
	{
		// Are offset vectors are fixed and uniformly distributed (so that our offset vectors
		// do not clump in the same direction).  If we reflect them about a random vector
		// then we get a random uniform distribution of offset vectors.
		float3 offset = reflect(ssaoData.offsetVectors[i].xyz, randVec);

		// Flip offset vector if it is behind the plane defined by (p, n).
		float flip = sign(dot(offset, n));

		// Sample a point near p within the occlusion radius.
		float3 q = p + flip * gOcclusionRadius * offset;

		// Transform the world position to shadow projected space
		float4 posShadowMap = mul(mvp.projection, float4(q, 1.0));
		// Transform the position to shadow clip space
		float3 UVD = posShadowMap.xyz / posShadowMap.w;
		// Convert to shadow map UV values
		UVD.xy = 0.5 * UVD.xy + 0.5;
		UVD.y = 1.0 - UVD.y;

		// Find the nearest depth value along the ray from the eye to q (this is not
		// the depth of q, as q is just an arbitrary point near p and might
		// occupy empty space).  To find the nearest depth we look it up in the depthmap.

		float rz = depthTexture.SampleLevel(gsamDepthMap, UVD.xy, 0.0f).r;
		rz = LinearEyeDepth(rz);

		// Reconstruct full view space position r = (rx,ry,rz).  We know r
		// lies on the ray of q, so there exists a t such that r = t*q.
		// r.z = t*q.z ==> t = r.z / q.z

		float3 r = (rz / q.z) * q;

		//
		// Test whether r occludes p.
		//   * The product dot(n, normalize(r - p)) measures how much in front
		//     of the plane(p,n) the occluder point r is.  The more in front it is, the
		//     more occlusion weight we give it.  This also prevents self shadowing where 
		//     a point r on an angled plane (p,n) could give a false occlusion since they
		//     have different depth values with respect to the eye.
		//   * The weight of the occlusion is scaled based on how far the occluder is from
		//     the point we are computing the occlusion of.  If the occluder r is far away
		//     from p, then it does not occlude it.
		// 

		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);

		float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
	}

	occlusionSum /= gSampleCount;

	float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
	return saturate(pow(access, 6.0f));

}
