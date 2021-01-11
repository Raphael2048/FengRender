SamplerState linear_sampler : register(s0);

Texture2D t_scene_color : register(t0);
Texture2D t_weights : register(t1);

RWTexture2D<float4> t_output : register(u0);
#include "pp_common.hlsl"
ConstantBuffer<ScreenSize> Screen : register(b0);
[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID)
{
    float2 uv = (DispatchThreadId + float2(0.5, 0.5)) * Screen.InvScreenSize;

    float4 TL = t_weights.mips[0][DispatchThreadId];
    float R = t_weights.mips[0][DispatchThreadId + int2(1, 0)].a;
    float B = t_weights.mips[0][DispatchThreadId + int2(0, 1)].g;

    float4 a = float4(TL.r, B, TL.b, R);
    float4 w = a * a * a;
    float sum = dot(w, 1.0);
    [branch]
    if (sum > 0) {
        float4 o = a * Screen.InvScreenSize.yyxx;
        float4 color = 0;

        color = mad(t_scene_color.SampleLevel(linear_sampler, uv + float2(0.0, -o.r), 0), w.r, color);
        color = mad(t_scene_color.SampleLevel(linear_sampler, uv + float2( 0.0, o.g), 0), w.g, color);
        color = mad(t_scene_color.SampleLevel(linear_sampler, uv + float2(-o.b, 0.0), 0), w.b, color);
        color = mad(t_scene_color.SampleLevel(linear_sampler, uv + float2( o.a, 0.0), 0), w.a, color);
        t_output[DispatchThreadId] = color/sum;
    } else
    {
        t_output[DispatchThreadId] = t_scene_color.SampleLevel(linear_sampler, uv, 0);
    }
}
