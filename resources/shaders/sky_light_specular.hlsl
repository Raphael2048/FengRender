#include "light_common.hlsl"
#include "sh_common.hlsl"
TextureCube t_cubemap : register(t0);
SamplerState linear_sampler : register(s0);
cbuffer C1 : register(b0)
{
    float4x4 ViewMatrix; 
};
cbuffer C2 : register(b1)
{
    float Roughness;
};

#include "pp_common.hlsl"

float4 PS(VertexOut pin) : SV_Target
{
    float3 dir = normalize(float3(pin.uv.x * 2.0f - 1.0f, 1.0f - pin.uv.y * 2.0f, -1.0f));
    float3 target_dir = mul(dir, (float3x3)ViewMatrix);
    float4 value = t_cubemap.Sample(linear_sampler, target_dir);
    return value;
    // return float4(target_dir * 0.5 + 1, 0);
    // return float4(dir, 0);
    // return float4(target_dir, 1);
    // return float4(1, 1, 1, 1);
}

