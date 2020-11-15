#include "light_common.hlsl"

Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D t_gbuffer_roghness_metallic : register(t2);
Texture2D t_depth : register(t3);
Texture2D t_shadowmap_splits[3] : register(t4);

SamplerState linear_sampler : register(s0);
SamplerComparisonState shadow_sampler : register(s1);

cbuffer light_constant:register(b0)
{
    float4x4 shadowmap_splits[3];
    float3 light_direction;
    float shadowmap_size;
    float3 light_color;
}

cbuffer pass_constant : register(b1)
{
    float4x4 view;
    float4x4 inv_view;
    float4x4 proj;
    float4x4 inv_proj;
    float4x4 view_proj;
    float4x4 inv_view_proj;
    float3 camera_pos;
};

struct VertexIn
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VertexOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.pos = float4(vin.pos, 0.5f, 1.0f);
    vout.uv = vin.uv;
    return vout;
}


float GetAt(float3 ndc, int index)
{
    float dx = 1.0f /shadowmap_size;
    // const float2 offset[9] =
    // {
    //     float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
    //     float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
    //     float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    // };

    const float2 offset[4] =
    {
        float2(-dx,  -dx),  float2(dx,  -dx),
        float2(-dx,  +dx),  float2(dx,  +dx)
    };
    float u = (ndc.x + 1.0f) * 0.5f;
    float v = (1.0f - ndc.y) * 0.5f;
    float2 pos = float2(u, v);
    float sum = 0.0f;
    sum +=  t_shadowmap_splits[index].SampleCmpLevelZero(shadow_sampler, float2(u + dx, v + dx) , ndc.z + 0.001f).r;
    sum +=  t_shadowmap_splits[index].SampleCmpLevelZero(shadow_sampler, float2(u + dx, v - dx) , ndc.z + 0.001f).r;
    sum +=  t_shadowmap_splits[index].SampleCmpLevelZero(shadow_sampler, float2(u - dx, v + dx) , ndc.z + 0.001f).r;
    sum +=  t_shadowmap_splits[index].SampleCmpLevelZero(shadow_sampler, float2(u - dx, v - dx) , ndc.z + 0.001f).r;
    return sum / 4.0f;
}

bool InsideBox(float3 ndc)
{
    return ndc.x > -0.999f && ndc.x < 0.999f
        && ndc.y > -0.999f && ndc.y < 0.999f
        && ndc.z > 0.001f && ndc.z <= 0.999f;
}
float4 PS(VertexOut pin) : SV_Target
{
    float3 N = t_gbuffer_normal.Sample(linear_sampler, pin.uv).rgb * 2.0 - 1.0;
    float2 RoughnessMetallic = t_gbuffer_roghness_metallic.Sample(linear_sampler, pin.uv).rg;
    float3 BaseColor = t_gbuffer_base_color.Sample(linear_sampler, pin.uv).rgb;

    float3 ScreenSpacePos = float3(
        pin.uv.x * 2.0f - 1.0f, 1.0f - pin.uv.y * 2.0f,  t_depth.Sample(linear_sampler, pin.uv).r);
    float4 WorldSpacePos = mul(float4(ScreenSpacePos, 1.0f), inv_view_proj);
    WorldSpacePos.xyz /= WorldSpacePos.w;

    float3 V = normalize(camera_pos - WorldSpacePos.xyz);
    float3 L = normalize(-light_direction);
    float3 H = normalize(L + V);

    WorldSpacePos.w = 1.0f;
    float3 ndc[3];
    ndc[0] = mul(WorldSpacePos, shadowmap_splits[0]).xyz;
    ndc[1] = mul(WorldSpacePos, shadowmap_splits[1]).xyz;
    ndc[2] = mul(WorldSpacePos, shadowmap_splits[2]).xyz;

    float multiper = 1.0f;
    [branch]
    if (InsideBox(ndc[0]))
    {
#ifdef DEBUG
        ndc[1].z = ndc[2].z = 0;
#else
        multiper = GetAt(ndc[0], 0);
#endif
    }
    else if (InsideBox(ndc[1]))
    {
#ifdef DEBUG
        ndc[0].z = ndc[2].z = 0;
#else
        multiper = GetAt(ndc[1], 1);
#endif
    }
    else
    {
#ifdef DEBUG
        ndc[0].z = ndc[1].z = 0;
#else
        multiper = GetAt(ndc[2], 2);
#endif
    }

    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, BaseColor, RoughnessMetallic.y);

    float NDF = DistributionGGX(NdotH, RoughnessMetallic.x);
    float G = GeometrySchlickGGX(NdotV, RoughnessMetallic.x) * GeometrySchlickGGX(NdotL, RoughnessMetallic.x);
    float3 F = FresnelSchlick(F0, HdotV);

    float3 nominator = NDF * G * F;
    float denominator = 4 * NdotV * NdotL + 0.001;
    float3 specular = nominator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= (1.0 - RoughnessMetallic.y);

    float3 output = (kD * BaseColor / PI + specular) * light_color * NdotL;

#ifdef DEBUG
    return float4(ndc[0].z, ndc[1].z, ndc[2].z, 0.0f)* multiper ;
#else
    return float4(output * multiper, 0.0f);
#endif
}