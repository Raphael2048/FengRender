#include "light_common.hlsl"

Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D t_gbuffer_roghness_metallic : register(t2);
Texture2D t_depth : register(t3);
Texture2D t_shadowmap : register(t4);

SamplerState linear_sampler : register(s0);
SamplerComparisonState shadow_sampler : register(s1);

cbuffer light_constant:register(b0)
{
    float4x4 shadow_matrix;
    float3 light_position;
    float light_radius;
    float3 light_direction;
    float shadowmap_size;
    float4 light_color;
    float inner_falloff;
    float outer_falloff;
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

float Square(float value)
{
    return value * value;
}

float4 PS(VertexOut pin) : SV_Target
{
    float2 RoughnessMetallic = t_gbuffer_roghness_metallic.Sample(linear_sampler, pin.uv).rg;
    float3 BaseColor = t_gbuffer_base_color.Sample(linear_sampler, pin.uv).rgb;

    float3 ScreenSpacePos = float3(
        pin.uv.x * 2.0f - 1.0f, 1.0f - pin.uv.y * 2.0f,  t_depth.Sample(linear_sampler, pin.uv).r);
    float4 WorldSpacePos = mul(float4(ScreenSpacePos, 1.0f), inv_view_proj);
    WorldSpacePos /= WorldSpacePos.w;

    float3 N = t_gbuffer_normal.Sample(linear_sampler, pin.uv).rgb * 2.0 - 1.0;
    float3 V = normalize(camera_pos - WorldSpacePos.xyz);

    float4 ndc = mul(WorldSpacePos, shadow_matrix);
    ndc /= ndc.w;
    float u = (ndc.x + 1.0f) * 0.5f;
    float v = (1.0f - ndc.y) * 0.5f;
    float sum = 0.0f;
    float dx = 1.0f / shadowmap_size;
    sum +=  t_shadowmap.SampleCmpLevelZero(shadow_sampler, float2(u + dx, v + dx) , ndc.z + 0.001f).r;
    sum +=  t_shadowmap.SampleCmpLevelZero(shadow_sampler, float2(u + dx, v - dx) , ndc.z + 0.001f).r;
    sum +=  t_shadowmap.SampleCmpLevelZero(shadow_sampler, float2(u - dx, v + dx) , ndc.z + 0.001f).r;
    sum +=  t_shadowmap.SampleCmpLevelZero(shadow_sampler, float2(u - dx, v - dx) , ndc.z + 0.001f).r;
    float multiper = sum / 4.0f;

    float3 ToLight = light_position - WorldSpacePos.xyz;
    float3 L = normalize(ToLight);

    float intensity = Square(saturate(1 - Square(dot(ToLight, ToLight) / Square(light_radius))));
    float delta_cosine = dot(normalize(-light_direction), L);
    float falloff_k = 0.0f;
    [branch]
    if (delta_cosine > inner_falloff)
    {
        falloff_k = 1.0f;
    }
    else if(delta_cosine > outer_falloff)
    {
        falloff_k = (delta_cosine - outer_falloff) / (inner_falloff - outer_falloff);
    }
    intensity *= falloff_k;

    float3 output = PBRLight(N, V, L, BaseColor, RoughnessMetallic.x, RoughnessMetallic.y) * light_color.xyz ;
    
    return float4(output * intensity * multiper, 0.0f);
}