SamplerState linear_sampler : register(s0);
Texture2D t_scene_color : register(t0);

#include "light_common.hlsl"
RWTexture2D<float4> ReducedSceneColor[4] : register(u0);
cbuffer SSGIBuffer:register(b0)
{
    float2 InvScreenSize;
};

[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadID : SV_DISPATCHTHREADID)
{
    //简易版, 较好的版本应该使用上一帧的数据, 且根据相对距离算混合权重
    uint2 BasePos = DispatchThreadID * 2;
    float2 uv = (float2(DispatchThreadID.x, DispatchThreadID.y) + float2(0.5, 0.5)) * InvScreenSize;
    float3 color = t_scene_color.SampleLevel(linear_sampler, uv, 0).rgb;
    ReducedSceneColor[0][DispatchThreadID] = float4(color, 1);

    uint2 OutputPixelPos = DispatchThreadID;

    for(int i = 1; i < 4; i++)
    {
        GroupMemoryBarrierWithGroupSync();
        if (OutputPixelPos.x % 2 == 0 && OutputPixelPos.y % 2 == 0)
        {
            OutputPixelPos = OutputPixelPos >> 1;
            float4 ParentValues;
            int2 BasePos = OutputPixelPos * 2;
            float4 color1 = ReducedSceneColor[i - 1][BasePos];
            float4 color2 = ReducedSceneColor[i - 1][BasePos + uint2(1, 0)];
            float4 color3 = ReducedSceneColor[i - 1][BasePos + uint2(0, 1)];
            float4 color4 = ReducedSceneColor[i - 1][BasePos + uint2(1, 1)];
            ReducedSceneColor[i][OutputPixelPos] = (color1 + color2 + color3 + color4) * 0.25;
        }
    }
}