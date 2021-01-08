SamplerState linear_sampler : register(s0);

Texture2D t_gbuffer_base_color : register(t0);
Texture2D t_gbuffer_normal : register(t1);
Texture2D t_gbuffer_roghness_metallic : register(t2);
Texture2D t_depth : register(t3);
Texture2D<float> t_hzb : register(t4);
Texture2D t_scene_color : register(t5);

RWTexture2D<float4> t_ssr : register(u0);

#include "light_common.hlsl"
ConstantBuffer<PassConstant> PC : register(b0);
cbuffer SSRBuffer : register(b1)
{
    float2 ScreenSize;
    float2 InvScreenSize;
    float2 HZBScreenSize;
    float2 InvHZBScreenSize;
};

#define MIPS_COUNT 10

float2 NDCToScreen(float2 ndc)
{
    float2 uv = float2((ndc.x + 1) * 0.5 , (1 - ndc.y) * 0.5);
    return uv * HZBScreenSize;
}

// 使用投影矩阵的逆矩阵, 算深度值
float NDCDepthToViewDepth(float z)
{
    return PC.inv_proj._43 / (PC.inv_proj._34 * z + PC.inv_proj._44);
}


[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID)
{
    t_ssr[DispatchThreadId] = float4(0, 0, 0, 0);
    float2 uv = InvHZBScreenSize * (float2(DispatchThreadId) + 0.5);
    float2 RoughnessMetallic = t_gbuffer_roghness_metallic.SampleLevel(linear_sampler, uv, 0).rg;
    float3 BaseColor = t_gbuffer_base_color.SampleLevel(linear_sampler, uv, 0).rgb;
    float NDCSpaceZ = t_hzb.mips[0][DispatchThreadId].r;
    if (NDCSpaceZ < 0.0001) return;
    float3 NDCSpacePos = float3(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, NDCSpaceZ);
    float4 WorldSpacePos = mul(float4(NDCSpacePos, 1.0f), PC.inv_view_proj);
    WorldSpacePos /= WorldSpacePos.w;

    float3 ViewSpacePos = mul(float4(WorldSpacePos.xyz, 1), PC.view).xyz;

    float3 N = t_gbuffer_normal.SampleLevel(linear_sampler, uv, 0).rgb * 2.0 - 1.0;
    float3 V = normalize(PC.camera_pos - WorldSpacePos.xyz);

    float Roughness = RoughnessMetallic.x;
    if (Roughness > 0.5) return;

    float3 R = reflect(-V, N);
    float3 ReflectPosWS = WorldSpacePos.xyz + R;
    float4 ReflectPosNDC = mul(float4(ReflectPosWS, 1), PC.view_proj);
    ReflectPosNDC /= ReflectPosNDC.w;

    float2 ScreenSpaceBegin = float2(DispatchThreadId) + 0.5;
    float2 ScreenSpaceDir = NDCToScreen(ReflectPosNDC.xy) - ScreenSpaceBegin;
   
    float3 ViewSpaceDir = mul(R, (float3x3)PC.view);
    float ViewSpaceBeginZ = ViewSpacePos.z;
    float ViewSpaceDirZ = ViewSpaceDir.z;

    float2 RcpDir = rcp(ScreenSpaceDir);
    float RcpDirZ = rcp(ViewSpaceDirZ);
    // 计算光线追踪的终点位置, t的最大值
    float tMax;
    {
        float2 bounds;
        bounds.x = RcpDir.x >= 0 ? HZBScreenSize.x - 0.5 : 0.5;
        bounds.y = RcpDir.y >= 0 ? HZBScreenSize.y - 0.5 : 0.5;
        float2 dist = (bounds - ScreenSpaceBegin) * RcpDir;
        
        // 朝屏幕外或者屏幕内
        float MaxDistance = NDCDepthToViewDepth(ViewSpaceDirZ <= 0 ? 0.00000024 : 0.999);
        float distZ = (MaxDistance - ViewSpaceBeginZ) * RcpDirZ;
        tMax = min(min(dist.x, dist.y), distZ);
    }
    const int MaxMipLevel = 9;

    float t = 0;

    float2 RayPos = ScreenSpaceBegin;
    int MipLevel = 0;
    int IterCount = 0;
    int2 CrossStep = int2(
        ScreenSpaceDir.x < 0 ? 0 : 1,
        ScreenSpaceDir.y < 0 ? 0 : 1
    );
    int2 CrossSign = int2(
        ScreenSpaceDir.x < 0 ? -1 : 1,
        ScreenSpaceDir.y < 0 ? -1 : 1
    );

    // t_ssr[DispatchThreadId] = float4(ViewSpaceBeginZ, NDCDepthToViewDepth(NDCSpaceZ) , 0, 0);

    // 下一采样点的两个潜在选择, 选择最近的那个
    // 左上角, 相交测试为左侧边和上侧边
    float t1 = (DispatchThreadId.x + CrossStep.x - ScreenSpaceBegin.x) * RcpDir.x;
    float t2 = (DispatchThreadId.y + CrossStep.y - ScreenSpaceBegin.y) * RcpDir.y;
    t = min(t1, t2);
    // 计算位置时向前移动一点,  计算出的所有坐标都是位于边界的点, 
    // 这样在转化为屏幕上格子坐标时, 可正好转化为当前射线求交判断的格子目标
    RayPos = ScreenSpaceBegin + t * ScreenSpaceDir + CrossSign * 0.0001;
    // t_ssr[DispatchThreadId] = float4(RayPos , ScreenSpaceDir);
    while((t < tMax) && (IterCount < 48))
    {
        IterCount++;
        int2 MipCoord = (int2)RayPos >> MipLevel;
        float SampleZ = t_hzb.mips[MipLevel][MipCoord].r;
        float ViewSpaceSampleZ = NDCDepthToViewDepth(SampleZ);
        float RayZ = ViewSpaceBeginZ + t * ViewSpaceDirZ;

        // Hit, 命中时采样点不动, 只减MIP
        if (ViewSpaceSampleZ > RayZ)
        {
            // 深度值差距过大, 认为是遮挡
            if (ViewSpaceSampleZ - RayZ > 20) return;
            if (MipLevel == 0)
            {
                // 此时视为成功找到
                float3 color = t_scene_color.SampleLevel(linear_sampler, RayPos * InvHZBScreenSize, 0).rgb;
                color *= 1 - Roughness;
                float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), BaseColor, RoughnessMetallic.y);
                float3 H = normalize(R + V);
                float HdotV = max(dot(H, V), 0.0);
                float3 F = FresnelSchlick(F0, HdotV);
                t_ssr[DispatchThreadId] = float4(color * F, 0);
                break;
            }
            else
            {
                --MipLevel; 
            }
        }
        else
        {
            //先按照当前Mip前进到下一个格子, 再将MipLevel++
            float t1 = (((MipCoord.x + CrossStep.x) << MipLevel) - ScreenSpaceBegin.x) * RcpDir.x;
            float t2 = (((MipCoord.y + CrossStep.y) << MipLevel) - ScreenSpaceBegin.y) * RcpDir.y;
            t = min(t1, t2);
            RayPos = ScreenSpaceBegin + t * ScreenSpaceDir + CrossSign * 0.0001;
            if (MipLevel != MaxMipLevel)
            {
                MipLevel++;
            }
        }
    }
    // t_ssr[DispatchThreadId] = float4(RayPos , IterCount, MipLevel);
    
    
}
