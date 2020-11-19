#include "light_common.hlsl"
#include "sh_common.hlsl"
TextureCube t_cubemap : register(t0);
SamplerState linear_sampler : register(s0);
cbuffer C1 : register(b0)
{
    float4x4 InvMatrix; 
};
cbuffer C2 : register(b1)
{
    float Roughness;
};

#include "pp_common.hlsl"

float4 PS(VertexOut pin) : SV_Target
{
    return float4(0, 0, 0, 0);
}

