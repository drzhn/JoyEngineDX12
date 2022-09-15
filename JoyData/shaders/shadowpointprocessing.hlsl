#include "CommonEngineStructs.h"

struct GS_OUTPUT
{
	float4 Pos : SV_POSITION;
	uint RTIndex : SV_RenderTargetArrayIndex;
};

ConstantBuffer<MVP> mvp : register(b0);
ConstantBuffer<LightData> lightData : register(b1);

float4 VSMain(float3 position : POSITION) : SV_POSITION
{
	return mul(mvp.model, float4(position, 1));
}

[maxvertexcount(18)]
void GSMain(triangle float4 InPos[3] : SV_Position, inout TriangleStream<GS_OUTPUT> OutStream)
{
	for (int iFace = 0; iFace < 6; iFace++)
	{
		GS_OUTPUT output;
		output.RTIndex = iFace;
		for (int v = 0; v < 3; v++)
		{
			output.Pos = mul(lightData.projection, mul(lightData.view[iFace], InPos[v]));
			OutStream.Append(output);
		}
		OutStream.RestartStrip();
	}
}

void PSMain() // : SV_TARGET
{
}
