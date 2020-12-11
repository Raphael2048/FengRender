SamplerState linear_sampler : register(s0);

Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D t_gbuffer_roghness_metallic : register(t2);
Texture2D t_depth : register(t3);
Texture2D t_hzb_buffer : register(t4);
Texture2D t_scene_color : register(t5);

RWTexture2D<float3> t_ssr : register(u0);

#include "light_common.hlsl"
ConstantBuffer<PassConstant> PC : register(b0);

#include "pp_common.hlsl"

bool RayCast(float Roughness, float depth, uint Steps, float step, out float3 OutHitUVz, out float Level)
{
    
}


float4 PS(VertexOut pin) : SV_Target
{
    float2 RoughnessMetallic = t_gbuffer_roghness_metallic.Sample(linear_sampler, pin.uv).rg;
    float3 BaseColor = t_gbuffer_base_color.Sample(linear_sampler, pin.uv).rgb;
    float3 ScreenSpacePos = float3(pin.uv.x * 2.0f - 1.0f, 1.0f - pin.uv.y * 2.0f,  t_depth.Sample(linear_sampler, pin.uv).r);
    float4 WorldSpacePos = mul(float4(ScreenSpacePos, 1.0f), PC.inv_view_proj);
    WorldSpacePos /= WorldSpacePos.w;

    float3 N = t_gbuffer_normal.Sample(linear_sampler, pin.uv).rgb * 2.0 - 1.0;
    float3 V = normalize(PC.camera_pos - WorldSpacePos.xyz);

    float Roughness = RoughnessMetallic.x;

    float a = Roughness * Roughness;
    float a2 = a * a;

    float NoV = saturate( dot(N, V));
    float G_SmithV = 2 * NoV / (NoV + sqrt(NoV * (NoV - NoV * a2) + a2));

    float ClosestHitDistanceSqr = 1e10f;

    float3 R = reflect(-V, N);
    float3 ReflectPosWS = WorldSpacePos.xyz + R;
    float3 ReflectPosNDC = mul(PC.)
}
