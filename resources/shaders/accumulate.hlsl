Texture2D t_accumulate : register(t0);

SamplerState linear_sampler : register(s0);

#include "pp_common.hlsl"

float4 PS(VertexOut pin) : SV_Target
{
    return t_accumulate.Sample(linear_sampler, pin.uv);
}