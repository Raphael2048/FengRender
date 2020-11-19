#include "light_common.hlsl"
#include "sh_common.hlsl"
TextureCube t_cubemap : register(t0);
SamplerState linear_sampler : register(s0);
cbuffer Constants : register(b0)
{
    float Roughness;
}


