SamplerState linear_sampler : register(s0);

Texture2D t_depth : register(t0);
Texture2D t_gbuffer_normal : register(t1);
// Texture2D t_hzb_buffer : register(t2);

RWTexture2D<float> t_ao : register(u0);

#include "light_common.hlsl"

ConstantBuffer<PassConstant> PC : register(b0);

cbuffer GTAOBuffer : register(b1)
{
    uint2 ScreenSize;
    float2 InvScreenSize;
    float InvRadiusSq;
};

#include "random_common.hlsl"

float2 GetDirection(float2 PixelPos, uint offset)
{
    float noise = InterleaveGradientNoise(PixelPos, offset);
    float rotations[] = {60, 300, 180, 240, 120, 0};
    float rotation = rotations[offset] / 360;

    noise += rotation;
    noise *= PI;
    return float2(cos(noise), sin(noise));
}


float HorizonLoop(float3 ViewSpaceBegin, float3 ViewSpaveV, float2 begin, float2 dir)
{
    float MaxHorizon = -1.0f; // cos(pi)
    // float LastHozrion = -1.0f;
    // 在临近像素中寻找最大水平角
    for(uint i = 1; i < 10; i++)
    {
        float2 SampleUV = (begin + dir * i) * InvScreenSize;
        if (SampleUV.x < 0 || SampleUV.x > 1 || SampleUV.y < 0 || SampleUV.y > 1) break;
        float d = t_depth.SampleLevel(linear_sampler, SampleUV, 0).r;
        float4 ScreenSpacePos = float4(SampleUV.x * 2 - 1, 1 - SampleUV.y * 2, d, 1);
        float4 ViewSpacePos = mul(ScreenSpacePos, PC.inv_proj);
        ViewSpacePos /= ViewSpacePos.w;
        float3 delta = ViewSpacePos.xyz - ViewSpaceBegin;

        float sqrDistance = dot(delta, delta);
        float currentHorizon = dot(delta, ViewSpaveV) * rsqrt(sqrDistance);
        float falloff = saturate(1.0 - (sqrDistance * InvRadiusSq));
        if (currentHorizon > MaxHorizon)
        {
            MaxHorizon = lerp(MaxHorizon, currentHorizon, falloff);
        }
        else
        {
            MaxHorizon = lerp(MaxHorizon, currentHorizon, 0.03f);
        }
    }

    return MaxHorizon;
}

float acosFast(float inX) 
{
    float x = abs(inX);
    float res = -0.156583f * x + (0.5 * PI);
    res *= sqrt(1.0f - x);
    return (inX >= 0) ? res : PI - res;
}

bool AnyIsNan(float2 value)
{
    return isnan(value.x) || isnan(value.y);
}

[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadID : SV_DISPATCHTHREADID)
{
    [branch]
    if (DispatchThreadID.x < ScreenSize.x && DispatchThreadID.y < ScreenSize.y)
    {
        float2 PixelPos = float2(DispatchThreadID) + 0.5;
        float2 UV = PixelPos * InvScreenSize;
        float CurrentZ = t_depth.SampleLevel(linear_sampler, UV, 0).r;
        float3 ScreenSpacePos =  float3(UV.x * 2.0f - 1.0f, 1.0f - UV.y * 2.0f,  CurrentZ);
        float4 ViewSpacePos = mul(float4(ScreenSpacePos, 1.0f), PC.inv_proj);
        ViewSpacePos.xyz /= ViewSpacePos.w;
        if (ViewSpacePos.z > 100 || CurrentZ == 0)
        {
            t_ao[DispatchThreadID] = 1;
            return;
        }

        float3 ViewSpaceV = normalize(-ViewSpacePos.xyz);
        float3 Normal = t_gbuffer_normal.SampleLevel(linear_sampler, UV, 0).rgb * 2.0 - 1.0;
        float3 ViewSpaceN = mul(Normal, (float3x3)PC.view);

        float SUM = 0;
        uint COUNT = 3;
        for(uint i = 0; i < COUNT; i++)
        {
            float2 dir = GetDirection(PixelPos, i);

            float2 MaxHorizons;
            MaxHorizons.x = HorizonLoop(ViewSpacePos.xyz, ViewSpaceV, PixelPos, dir);
            MaxHorizons.y = HorizonLoop(ViewSpacePos.xyz, ViewSpaceV, PixelPos, -dir);

            float3 NDCSpaceDir = float3(dir.x * 2 - 1, 1 - dir.y * 2, 0);
            // Slice切面的法线
            float3 PlaneNormal = normalize(cross(NDCSpaceDir, ViewSpaceV));
            // Slice切面内, 和View方向互为垂直的方向
            float3 Perp = cross(ViewSpaceV, PlaneNormal);
            // 视图空间的法线, 在Slice切面上的投影
            float3 ProjNormal = ViewSpaceN - PlaneNormal * dot(ViewSpaceN, PlaneNormal);

            float LenProjNormal = length(ProjNormal);
            float CosN = dot(ProjNormal / LenProjNormal, ViewSpaceV);

            // 投影法线和观察方向的夹角, 公式中的GAMMA值
            float N = -sign(dot(ProjNormal, Perp)) * acosFast(CosN);

            MaxHorizons.x = -acosFast(MaxHorizons.x);
            MaxHorizons.y = acosFast(MaxHorizons.y);
            MaxHorizons.x = N + max(MaxHorizons.x - N, -PI * 0.5);
            MaxHorizons.y = N + min(MaxHorizons.y - N, PI * 0.5);
            if (isnan(MaxHorizons.x) || isnan(MaxHorizons.y))
            {
                SUM += 1;
            }
            else
            {
                float h1 = MaxHorizons.x * 2;
                float h2 = MaxHorizons.y * 2;
                float SinN = sin(N);
                float value = 0.25 * ((-cos(h1 - N) + CosN + h1 * SinN) + (-cos(h2 - N) + CosN + h2 * SinN));
                SUM += value;
            }
        }
        float AO = SUM / COUNT;
        AO *= 2/PI;
        AO = lerp(AO, 1, saturate(ScreenSpacePos.z * 0.01 + 0.1));
        t_ao[DispatchThreadID] = AO;
    }
}
