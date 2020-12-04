#include "light_common.hlsl"
#include "sh_common.hlsl"
Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D<float2> t_gbuffer_roghness_metallic : register(t2);
Texture2D<float> t_depth : register(t3);
StructuredBuffer<ThreeOrderSH> SH : register(t4);
TextureCube t_specular_prefilter : register(t5);
Texture2D t_specular_lut : register(t6);
Texture2D t_ao : register(t7);

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

#include "specular_brdf_common.hlsl"

float4 PS(VertexOut pin) : SV_Target
{
    float2 RoughnessMetallic = t_gbuffer_roghness_metallic.Sample(linear_sampler, pin.uv).rg;
    float3 BaseColor = t_gbuffer_base_color.Sample(linear_sampler, pin.uv).rgb;
    float AO = t_ao.Sample(linear_sampler, pin.uv).r;
    BaseColor = AOMultiBounce(BaseColor, AO);
    float Roughness = RoughnessMetallic.x;
    float Metallic = RoughnessMetallic.y;

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
    float3 Diffuse = float3(DiffuseR, DiffuseG, DiffuseB) * BaseColor;

    float3 V = normalize(PC.camera_pos - WorldSpacePos.xyz);
    float NdotV = max(0, dot(N, V));
    float3 L = reflect(-V, N);

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), BaseColor, Metallic);
    float3 F = FresnelSchlickRoughness(NdotV, F0, Roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1 - Metallic;

    const float MAX_REFLECTION_LOD = 4.0;
    float3 PrefilterColor = t_specular_prefilter.SampleLevel(linear_sampler, L, Roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = t_specular_lut.Sample(linear_sampler, float2(NdotV, Roughness)).rg;
    float3 Specular = PrefilterColor * (F * brdf.x + brdf.y);

    float3  Color = kD * Diffuse + Specular;

    return float4(Color * Intensity, 1);
}




