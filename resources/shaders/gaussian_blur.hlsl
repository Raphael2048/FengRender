Texture2D<float3> t_origin : register(t0);

SamplerState linear_sampler : register(s0);

#include "pp_common.hlsl"

cbuffer Screen : register(b0)
{
    float2 InvScreenSize;
    float kernel;
}
static const float offset = 0.3175426849681955;
static const float rate = 0.2987;

float4 PS(VertexOut pin) : SV_Target
{
    float2 Inv = InvScreenSize * kernel;
    float2 uv = pin.uv;
    float3 v0 = t_origin.Sample(linear_sampler, uv) * 0.4026;
#ifdef DIRECTION_H
    float3 v1 = t_origin.Sample(linear_sampler, uv + float2(-1.5 + offset, 0) * Inv) * rate;
    float3 v2 = t_origin.Sample(linear_sampler, uv + float2(1.5 - offset, 0) * Inv) * rate;
#else
    float3 v1 = t_origin.Sample(linear_sampler, uv + float2(0, -1.5 + offset) * Inv) * rate;
    float3 v2 = t_origin.Sample(linear_sampler, uv + float2(0, 1.5 - offset) * Inv) * rate;
#endif
    return float4(v0 + v1 + v2, 0);
}