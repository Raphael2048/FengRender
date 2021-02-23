Texture3D<float4> t_volume_density : register(t0);
RWTexture3D<float4> t_volume_accum : register(u0);

#include "light_common.hlsl"
SamplerState linear_sampler : register(s0);

ConstantBuffer<PassConstant> PC :register(b0);

cbuffer VolomueParameter : register(b1)
{
    float3 InvVolumeTextureSize;
};

// 使用投影矩阵的逆矩阵, 算深度值
float NDCDepthToViewDepth(float z)
{
    return PC.inv_proj._43 / (PC.inv_proj._34 * z + PC.inv_proj._44);
}

static const float GParameter = 0.5f;
static const float SigmaT = 0.1f;
static const float SigmaS = 0.08f;
#define DEPTH 64
[numthreads(1, 1, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID)
{
    float2 uv = InvVolumeTextureSize.xy * (float2(DispatchThreadId) + 0.5);
    float z = 1.0f;
    float depth = NDCDepthToViewDepth(z);

    // Alpah is Transmittance
    float4 AccumLight = float4(0.0f, 0.0f, 0.0f, 1.0f);

    // Accumulate From Near to Far
    for(int i = DEPTH - 1; i >= 0; i--)
    {
        float4 density = t_volume_density.Load(int4(DispatchThreadId, i, 0));
        z-= InvVolumeTextureSize.z;
        float ThisDepth = NDCDepthToViewDepth(z);
        float DeltaDepth = abs(ThisDepth - depth);
        depth = ThisDepth;

        float T = exp(-DeltaDepth * SigmaT * density.a);

        // AccumLight.rgb += density.rgb * SigmaS * DeltaDepth * AccumLight.a;

        // 穿过一个切片的积分值, 解析解, 更加精确
        float3 scat = density.rgb * SigmaS;
        float3 intergScatt = (scat - scat * T) / SigmaT;
        AccumLight.rgb += intergScatt * AccumLight.a;
        AccumLight.a *= T;
        
        t_volume_accum[int3(DispatchThreadId, i)] = AccumLight;
    }
}