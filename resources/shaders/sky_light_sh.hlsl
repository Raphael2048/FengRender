#include "light_common.hlsl"

TextureCube<float3> t_cubemap : register(t0);
struct SH
{
    float3 Coefficient[9];
};
RWStructuredBuffer<SH> buffer : register(u0); 
SamplerState linear_sampler : register(s0);

[numthreads(9, 1, 1)]
void CS(int3 dispatch_thread_id : SV_DispatchThreadID)
{
    const int D1 = 50;
    const int D2 = 50;
    int index = dispatch_thread_id.x;
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < D1; i++)
    {
        for (int j = 0; j < D2; j++)
        {
            float eta1 = ((float)i) / D1;
            float eta2 = ((float)j) / D2;
            float a = sqrt(eta1 * (1.0f - eta1));
            float3 dir = float3(cos(2 * PI * eta2) * a, sin(2 * PI * eta2) * a, 1 - 2 * eta1);
            float3 value = t_cubemap.Sample(linear_sampler, dir).xyz;
            float k = GetSHCoffient(index, dir);
            result += k * value;
        }
    }
    result /= (D1 * D2);
    SH.Coefficient[index] = result;
}


