#include "light_common.hlsl"

Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D t_gbuffer_roghness_metallic : register(t2);
Texture2D t_depth : register(t3);
TextureCube t_shadowmap : register(t4);

SamplerState linear_sampler : register(s0);
SamplerComparisonState shadow_sampler : register(s1);

cbuffer light_constant : register(b0)
{
    float4x4 ViewMatrix[6];
    float4x4 ProjMatrix;
    float3 LightPos;
    float Radius;
    float4 Color;
    float ShadowmapSize;
};
ConstantBuffer<PassConstant> PC:register(b1);

#include "pp_common.hlsl"

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
    float4 WorldSpacePos = mul(float4(ScreenSpacePos, 1.0f), PC.inv_view_proj);
    WorldSpacePos /= WorldSpacePos.w;

    float3 N = t_gbuffer_normal.Sample(linear_sampler, pin.uv).rgb * 2.0 - 1.0;
    float3 V = normalize(PC.camera_pos - WorldSpacePos.xyz);

    float3 ToLight = LightPos - WorldSpacePos.xyz;
    float intensity = Square(saturate(1 - Square(dot(ToLight, ToLight) / Square(Radius))));
    float3 L = normalize(ToLight);

    float linear_depth = distance(WorldSpacePos.xyz, LightPos) / Radius;

    float3 Output = PBRLight(N, V, L, BaseColor, RoughnessMetallic.x, RoughnessMetallic.y) * Color.xyz;
    float3 dir = float3(L.x, L.y, -L.z);
    float k  = t_shadowmap.SampleCmpLevelZero(shadow_sampler, dir, linear_depth + 0.001f).r;

    return float4(Output * intensity, 0);
}