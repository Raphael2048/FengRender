Texture2D<float3> t_ssr : register(t0);

SamplerState linear_sampler : register(s0);

#include "pp_common.hlsl"

ConstantBuffer<ScreenSize> Screen:register(b0);

static const float offset = 0.3175426849681955;
static const float rate = 0.2987;

float4 PS(VertexOut pin) : SV_Target
{
    float2 uv = pin.uv;
    float3 v0 = t_ssr.Sample(linear_sampler, uv) * 0.4026;
#ifdef DIRECTION_H
    float3 v1 = t_ssr.Sample(linear_sampler, uv + float2(-1.5 + offset, 0) * Screen.InvScreenSize) * rate;
    float3 v2 = t_ssr.Sample(linear_sampler, uv + float2(1.5 - offset, 0) * Screen.InvScreenSize) * rate;
#else
    float3 v1 = t_ssr.Sample(linear_sampler, uv + float2(0, -1.5 + offset) * Screen.InvScreenSize) * rate;
    float3 v2 = t_ssr.Sample(linear_sampler, uv + float2(0, 1.5 - offset) * Screen.InvScreenSize) * rate;
#endif
    return float4(v0 + v1 + v2, 0);
}