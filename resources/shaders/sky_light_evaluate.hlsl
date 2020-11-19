#include "light_common.hlsl"
#include "sh_common.hlsl"
Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D<float2> t_gbuffer_roghness_metallic : register(t2);
Texture2D<float> t_depth : register(t3);
StructuredBuffer<ThreeOrderSH> SH : register(t4);

cbuffer LightIntensity : register(b0)
{
    float Intensity;
}
ConstantBuffer<PassConstant> PC : register(b1);

SamplerState linear_sampler : register(s0);

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

float4 PS(VertexOut pin) : SV_Target
{
    float2 RoughnessMetallic = t_gbuffer_roghness_metallic.Sample(linear_sampler, pin.uv).rg;
    float3 BaseColor = t_gbuffer_base_color.Sample(linear_sampler, pin.uv).rgb;

    float3 ScreenSpacePos = float3(
        pin.uv.x * 2.0f - 1.0f, 1.0f - pin.uv.y * 2.0f,  t_depth.Sample(linear_sampler, pin.uv).r);
    float4 WorldSpacePos = mul(float4(ScreenSpacePos, 1.0f), PC.inv_view_proj);
    WorldSpacePos.xyz /= WorldSpacePos.w;

    float3 N = t_gbuffer_normal.Sample(linear_sampler, pin.uv).rgb * 2.0 - 1.0;

    //一种优化方案是存储预计算后的系数, 可参考StupiedSH AppendixA10
    ThreeOrderSH basis =  GetThreeOrderSHBasis(N);
    float DiffuseR = DotSH3(basis, SH[0]);
    float DiffuseG = DotSH3(basis, SH[1]);
    float DiffuseB = DotSH3(basis, SH[2]);
    float3 diffuse = float3(DiffuseR, DiffuseG, DiffuseB) * Intensity * BaseColor;
    return float4(diffuse, 1);
}




