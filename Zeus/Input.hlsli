struct VSInput
{
    float3 Position  : POSITION;
    float2 TexCoord  : TEXCOORD;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 BiTangent : BITANGENT;
};

struct PSInput
{
    float4 Position   : SV_POSITION;
    float2 TexCoord   : TEXCOORD;
    float3 Normal     : NORMAL;
    float3 CamPos     : EYE;
    float3 PositionWS : POSITIONWS;
    float3x3 TBN      : TBN;
};