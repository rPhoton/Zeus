#include <Input.hlsli>

cbuffer Transform : register(b0)
{
	row_major matrix Model;
	row_major matrix iModel;
}

cbuffer Camera : register(b1)
{
    row_major matrix ViewProj;
    float3 Eye;
}

PSInput main(VSInput input)
{
    PSInput vsout;
    
    vsout.Position = mul(float4(input.Position, 1.0), mul(Model, ViewProj));
    vsout.PositionWS = mul(float4(input.Position, 1.0), Model).xyz;
    vsout.Normal = mul(input.Normal, (float3x3) iModel);
    
    vsout.TexCoord = input.TexCoord;
    vsout.CamPos = Eye;
    
    float3x3 TBN = float3x3(input.Tangent, input.BiTangent, input.Normal);
    vsout.TBN = mul((float3x3) Model, transpose(TBN));
    
    return vsout;
}