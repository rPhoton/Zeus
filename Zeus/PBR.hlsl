#include <Input.hlsli>

static const float PI = 3.141592;
static const float Epsilon = 0.00001;
static const float3 Fdielectric = 0.04;
static const float3 Irradiance = float3(1.0, 1.0, 1.0);

// Light Data Constants (Temprorary)
static const float3 lightPosition[4] =
{
    float3(8,  8,  8),
    float3(8,  8, -8),
    float3(8, -8,  8),
    float3(8, -8, -8)
};
static const float3 lightColor = float3(1, 1, 1);
static const float lightIntensity = 300;

cbuffer Material : register(b1)
{
    float3 AlbedoData;
    float MetalnessData;
    float RoughnessData;
}

Texture2D Albedo : register(t0);
Texture2D Normal : register(t1);
Texture2D Metalness : register(t2);
Texture2D Roughness : register(t3);

SamplerState Sampler : register(s0);

float D_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;
    
    float numerator = a2;
    float denominator = PI * pow(NdotH2 * (a2 - 1.0) + 1.0, 2.0);
    
    return numerator / denominator;
}

float G1_SchlickGGX(float3 N, float3 X, float roughness)
{
    float NdotX = saturate(dot(N, X));
    float k = pow((roughness + 1), 2.0) / 8;

    float numerator = NdotX;
    float denominator = NdotX * (1.0 - k) + k;

    return numerator / denominator;
}

float G_SmithGGX(float3 N, float3 V, float3 L, float roughness)
{
    return G1_SchlickGGX(N, V, roughness) * G1_SchlickGGX(N, L, roughness);
}

float3 F_Schlick(float3 F0, float3 V, float3 H)
{
    return F0 + (1.0 - F0) * pow(1 - saturate(dot(V, H)), 5.0);
}

float3 F_SchlickRoughness(float3 F0, float3 V, float3 H, float roughness)
{
    return F0 + (max((1.0 - roughness), F0) - F0) * pow(1 - saturate(dot(V, H)), 5.0);
}

float Fd_Lambert()
{
    return 1.0 / PI;
}

float3 BRDF(float3 albedo, float metalness, float roughness, float3 N, float3 V, float3 L, float3 H)
{
    float3 F0 = lerp(Fdielectric, albedo, metalness);
    float3 Ks = F_Schlick(F0, V, H);
    float3 Kd = (1.0 - Ks) * (1.0 - metalness);

    float3 diffuseBRDF = Kd * albedo * Fd_Lambert();

    float3 numerator = D_GGX(N, H, roughness) * G_SmithGGX(N, V, L, roughness) * F_Schlick(F0, V, H);
    
    float denominator = 4.0 * saturate(dot(V, N)) * saturate(dot(L, N));
    denominator = max(denominator, Epsilon);
    
    float3 specularBRDF = numerator / denominator;
   
    return diffuseBRDF + specularBRDF;
}

float4 main(PSInput input) : SV_TARGET
{
    float3 albedo = AlbedoData;
    float3 normal = input.Normal;
    float metalness = MetalnessData;
    float roughness = RoughnessData;
    
    float3 Lo = float3(0, 0, 0);
    
    //normal = 2.0 * normal - 1;
    //normal = mul(input.TBN, normal);
    
    float3 N = normalize(normal);
    float3 V = normalize(input.CamPos - input.PositionWS);
    
    for (int i = 0; i < 4; ++i)
    {
        float3 L = normalize(lightPosition[i]);
        float3 H = normalize(V + L);
    
        float dist = length(lightPosition[i] - input.PositionWS);
        float attenuation = 1.0 / (dist * dist);
        float3 radiance = lightColor * attenuation * lightIntensity;
    
        Lo += BRDF(albedo, metalness, roughness, N, V, L, H) * radiance * max(dot(L, N), 0.0);
    }
    
    float3 F0 = lerp(Fdielectric, albedo, metalness);
    float3 Ks = F_SchlickRoughness(F0, V, N, roughness);
    float3 Kd = 1.0 - Ks;
    float3 diffuse = Irradiance * albedo;
    
    float3 ambient = Kd * diffuse;
    float3 color = ambient + Lo;
    
    return float4(color, 1.0);
}