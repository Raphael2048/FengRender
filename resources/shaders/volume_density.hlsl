Texture2D<float> t_shadowmap_splits[3] : register(t0);
RWTexture3D<float4> t_volume_density : register(u0);

#include "light_common.hlsl"
SamplerState linear_sampler : register(s0);
SamplerComparisonState shadow_sampler : register(s1);

ConstantBuffer<PassConstant> PC :register(b0);
cbuffer ShadowSplits : register(b1)
{
    float4x4 shadowmap_splits[3];
    float3 light_direction;
    float shadowmap_size;
    float4 light_color;
};
cbuffer VolomueParameter : register(b2)
{
    float3 InvVolumeTextureSize;
    float Time;
};

float HGPhase(float CosineTheta, float G)
{
    float G2 = G * G;
    float b = 1.0 + G2 - 2 * G * CosineTheta;
    float b3 = b * b * b;
    float c = sqrt(b3) * 4 * PI;
    return (1.0 - G2) / c;
}

bool InsideBox(float3 ndc)
{
    return ndc.x > -0.999f && ndc.x < 0.999f
        && ndc.y > -0.999f && ndc.y < 0.999f
        && ndc.z > 0.001f && ndc.z <= 0.999f;
}

float GetAt(float3 ndc, int index)
{
    float u = (ndc.x + 1.0f) * 0.5f;
    float v = (1.0f - ndc.y) * 0.5f;
    return  t_shadowmap_splits[index].SampleCmpLevelZero(shadow_sampler, float2(u, v) , ndc.z + 0.001f).r;
}


#include "random_common.hlsl"
// Media Phase Function G Value
static const float GParameter = 0.9f;

[numthreads(1, 1, 1)]
void CS(uint3 DispatchThreadId : SV_DISPATCHTHREADID)
{
    float3 uvz = InvVolumeTextureSize * (float3(DispatchThreadId) + 0.5);
    float3 ScreenSpacePos = float3(uvz.x * 2.0f - 1.0f, 1.0f - uvz.y * 2.0f, uvz.z);
    float4 WorldSpacePos = mul(float4(ScreenSpacePos, 1.0f), PC.inv_view_proj);
    WorldSpacePos.xyz /= WorldSpacePos.w;
    WorldSpacePos.w = 1;

    float3 V = normalize(PC.camera_pos - WorldSpacePos.xyz);
    float3 L = normalize(-light_direction);

    // Is Inside Shadow Volume?
    float3 ndc[3];
    ndc[0] = mul(WorldSpacePos, shadowmap_splits[0]).xyz;
    ndc[1] = mul(WorldSpacePos, shadowmap_splits[1]).xyz;
    ndc[2] = mul(WorldSpacePos, shadowmap_splits[2]).xyz;
    float multiper = 0.0f;
    [branch]
    if (InsideBox(ndc[0]))
    {
        multiper = GetAt(ndc[0], 0);
    }
    else if (InsideBox(ndc[1]))
    {
        multiper = GetAt(ndc[1], 1);
    }
    else
    {
        multiper = GetAt(ndc[2], 2);
    }

    float CosineTheta = dot(V, -L);

    float d = HGPhase(CosineTheta, GParameter) * multiper;

    // TODO:: 
    float v = GradientNoise3D_ALU((WorldSpacePos.xyz + float3(0, 0, Time))* 0.05f, false, 0);

    // Alpha: density of volume. sigma t value.
    t_volume_density[DispatchThreadId] = float4(d * light_color.rgb, clamp(abs(v), 0.1, 1));
    // t_volume_density[DispatchThreadId] = float4(WorldSpacePos.xyz, 1);
    // t_volume_density[DispatchThreadId] = float4(uvz, 1.0f);
    // t_volume_density[DispatchThreadId] = float4(color, 1.0);
}