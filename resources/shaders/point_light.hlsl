#include "light_common.hlsl"

Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D t_gbuffer_roghness_metallic : register(t2);
Texture2D t_depth : register(t3);
Texture2D t_shadowmap[6] : register(t4);

SamplerState linear_sampler : register(s0);
SamplerComparisonState shadow_sampler : register(s1);

#include "pp_common.hlsl"

cbuffer light_constant:register(t0)
{
    float4x4 shadow_matrix[6];
    float3 light_position;
    float light_radius;
    float4 light_color;
    float shadowmap_size;
}