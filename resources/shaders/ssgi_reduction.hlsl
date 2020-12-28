SamplerState linear_sampler : register(s0);
Texture2D t_scene_color_last_frame : register(t0);
Texture2D t_depth : register(t1);

#include "light_common.hlsl"
ConstantBuffer<PassConstant> PC : register(b0);
ConstantBuffer<PassConstant> LastPC : register(b1);
RWTexture2D<float4> ReducedSceneColor0 : register(u0);
RWTexture2D<float4> ReducedSceneColor1 : register(u1);
RWTexture2D<float4> ReducedSceneColor2 : register(u1);

cbuffer SSGIBuffer:register(b2)
{
    float2 ScreenSize;
    float2 InvScreenSize;
};

[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadID : SV_DISPATCHTHREADID)
{
    float2 uv = DispatchThreadID * InvScreenSize;
    float3 ScreenSpacePos = float3(
        uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f,  t_depth.Sample(linear_sampler, uv).r);
    float4 WorldSpacePos = mul(float4(ScreenSpacePos, 1.0f), PC.inv_view_proj);
    WorldSpacePos /= WorldSpacePos.w;

    float4 LastFrameNDC = mul(float4(WorldSpacePos.xyz, 1), LastPC.view_proj);
    LastFrameNDC /= LastFrameNDC.w;

    float2 lastuv = float2(LastFrameNDC.x * 0.5f + 0.5f, 0.5f - LastFrameNDC.y * 0.5f);
    float3 LastColor = t_scene_color_last_frame.SampleLevel(linear_sampler, lastuv, 0);

    ReducedSceneColor0[DispatchThreadID] = LastColor;
    ReducedSceneColor1[DispatchThreadID] = LastColor;
}