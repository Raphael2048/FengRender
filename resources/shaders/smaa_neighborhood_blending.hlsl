SamplerState linear_sampler : register(s0);

Texture2D t_scene_color : register(t0);
Texture2D t_weights : register(t1);

RWTexture2D<float4> t_output : register(u0);

[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID)
{
    float4 TL = t_weights.mips[0][DispatchThreadId];
    float B = t_weights.mips[0][DispatchThreadId + int2(0, 1)].g;
    float R = t_weights.mips[0][DispatchThreadId + int2(1, 0)].a;

    float4 a = float4(TL.r, B, TL.b, R);
}
