#include "light_common.hlsl"

Texture3D t_accumulate : register(t0);
Texture2D t_depth : register(t1);
SamplerState linear_sampler : register(s0);

#include "pp_common.hlsl"

cbuffer VolomueParameter : register(b0)
{
    float3 InvVolumeTextureSize;
};

float4 PS(VertexOut pin) : SV_Target
{
    float z = t_depth.SampleLevel(linear_sampler, pin.uv, 0).r;
    float4 color = t_accumulate.Sample(linear_sampler, float3(pin.uv, z + InvVolumeTextureSize.z * 0.5f));
    return color;
    // return float4(1.0f, 0.0f, 0.0f, 1.0f);
}


