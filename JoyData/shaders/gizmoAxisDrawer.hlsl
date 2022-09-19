#include "CommonEngineStructs.h"

ConstantBuffer<MVP> mvp : register(b0);

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(uint id : SV_VertexID)
{
	PSInput input;

	// position:
	// 0 -- 0 0 0
	// 1 -- 1 0 0 
	// 2 -- 0 0 0
	// 3 -- 0 1 0
	// 4 -- 0 0 0
	// 5 -- 0 0 1

	//color
	// 0 -- 1 0 0
	// 1 -- 1 0 0 
	// 2 -- 0 1 0
	// 3 -- 0 1 0
	// 4 -- 0 0 1
	// 5 -- 0 0 1


	float4 position = float4(id == 1, id == 3, id == 5, 1);

	float4x4 invView = mvp.view;
	invView[0][3] = 0;
	invView[1][3] = 0;
	invView[2][3] = 0;

	position = mul(invView, position);

	position += float4(0, 0, 3, 0);

	input.position = mul(mvp.proj, position);
	input.color = float4((id / 2) % 3 == 0, (id / 2) % 3 == 1, (id / 2) % 3 == 2, 1);
	return input;
}

float4 PSMain(PSInput input) : SV_Target
{
	return input.color;
}
