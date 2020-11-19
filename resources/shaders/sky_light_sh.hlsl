#include "light_common.hlsl"
#include "sh_common.hlsl"
TextureCube t_cubemap : register(t0);
RWStructuredBuffer<ThreeOrderSH> SH : register(u0); 
SamplerState linear_sampler : register(s0);

[numthreads(1, 1, 1)]
void CS(int3 dispatch_thread_id : SV_DispatchThreadID)
{
    const int D1 = 50;
    const int D2 = 50;
    int index = dispatch_thread_id.x;

    ThreeOrderSH R, G, B;
    R.V0 = R.V1 = G.V0 = G.V1 = B.V0 = B.V1 = 0;
    R.V2 = G.V2 = B.V2  = 0;
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < D1; i++)
    {
        float eta1 = ((float)i) / (D1 - 1);
        float a = sqrt(eta1 * (1.0f - eta1));
        for (int j = 0; j < D2; j++)
        {
            float eta2 = ((float)j) / (D2 - 1);
            float3 dir = float3(cos(2 * PI * eta2) * a, sin(2 * PI * eta2) * a, 1 - 2 * eta1);
            float3 value = t_cubemap.SampleLevel(linear_sampler, dir, 0).xyz;
            ThreeOrderSH basis = GetThreeOrderSHBasis(dir);
            R.V0 += basis.V0 * value.x;
            R.V1 += basis.V1 * value.x;
            R.V2 += basis.V2 * value.x;
            G.V0 += basis.V0 * value.y;
            G.V1 += basis.V1 * value.y;
            G.V2 += basis.V2 * value.y;
            B.V0 += basis.V0 * value.z;
            B.V1 += basis.V1 * value.z;
            B.V2 += basis.V2 * value.z;
        }
    }
    float3 CosineZH = GetCosineZH();
    float k = 2 * PI / PI / (D1 * D2);
    float3 KS = CosineZH * k;
    R.V0.x *= KS.x;
    R.V0.yzw *= KS.y;
    R.V1.xyzw *= KS.z;
    R.V2 *= KS.z;
    G.V0.x *= KS.x;
    G.V0.yzw *= KS.y;
    G.V1.xyzw *= KS.z;
    G.V2 *= KS.z;
    B.V0.x *= KS.x;
    B.V0.yzw *= KS.y;
    B.V1.xyzw *= KS.z;
    B.V2 *= KS.z;
    SH[0] = R;
    SH[1] = G;
    SH[2] = B;
}


