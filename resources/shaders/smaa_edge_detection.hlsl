SamplerState linear_sampler : register(s0);

Texture2D t_scene_color : register(t0);

RWTexture2D<float2> t_edges : register(u0);

#include "light_common.hlsl"

[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID)
{
    float color = Illumination(t_scene_color.mips[0][DispatchThreadId].rgb);
    float colorL = Illumination(t_scene_color.mips[0][DispatchThreadId + int2(-1, 0)].rgb);
    // float colorR = Illumination(t_scene_color.mips[0][DispatchThreadId + int2(1, 0)].rgb);
    float colorT = Illumination(t_scene_color.mips[0][DispatchThreadId + int2(0, -1)].rgb);
    // float colorB = Illumination(t_scene_color.mips[0][DispatchThreadId + int2(0, 1)].rgb);

    //左, 上
    float2 delta = abs(color.xx - float2(colorL, colorT));
    float2 edges = step(0.4, delta);
    t_edges[DispatchThreadId] = edges;
}
