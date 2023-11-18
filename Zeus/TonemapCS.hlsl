RWTexture2D<float4> RenderTarget : register(u0);
RWTexture2D<float4> CopyBuffer : register(u1);

float3 reinhard_jodie(float3 v)
{
    float l = dot(v, float3(0.2126f, 0.7152f, 0.0722f));
    float3 tv = v / (1.0f + v);
    return lerp(v / (1.0f + l), tv, tv);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{   
    float3 color = RenderTarget[DTid.xy].xyz;
    
    float3 mapped = pow(reinhard_jodie(color), 1.0 / 2.2);
    
    CopyBuffer[DTid.xy] = float4(mapped, 1.0);
}