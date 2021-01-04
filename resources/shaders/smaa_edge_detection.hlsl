SamplerState linear_sampler : register(s0);

Texture2D t_scene_color : register(t0);

RWTexture2D<float2> t_edges : register(u0);

#include "light_common.hlsl"

groupshared float illuminations[20 * 12];

float GetIllumination(int x, int y)
{
    return illuminations[(y + 2) * 20 + x + 2];
}

// 亮度值差, 边缘判定阈值
#define THRESHOLD 0.1

[numthreads(16, 8, 1)]
void CS(uint GroupIndex:SV_GROUPINDEX, uint2 GroupId:SV_GROUPID,
        int2 DispatchThreadId : SV_DISPATCHTHREADID, int2 GroupThreadID : SV_GROUPTHREADID)
{
    uint pixIdx = GroupIndex * 2;
    int2 FullGroupOrigin = int2(GroupId.x * 16, GroupId.y * 8);
    int2 FullGroupOriginM2	= FullGroupOrigin.xy - int2(2,2);
    if (pixIdx < 20 * 12)
    {
        uint XPos = pixIdx % 20;
		uint YPos = pixIdx / 20;
        int LDSPos = (YPos * 20) + XPos;
        int2 ReadXY = FullGroupOriginM2 + int2(XPos,YPos);
        float ill = Illumination(t_scene_color.Load(int3(ReadXY, 0)).rgb);
        illuminations[LDSPos] = ill;
        LDSPos++;
        ReadXY.x ++;
        ill = Illumination(t_scene_color.Load(int3(ReadXY, 0)).rgb);
        illuminations[LDSPos] = ill;
    }
    GroupMemoryBarrierWithGroupSync();


    float IThis = GetIllumination(GroupThreadID.x, GroupThreadID.y);
    float IL  = abs(GetIllumination(GroupThreadID.x - 1, GroupThreadID.y) - IThis);
    float IL2 = abs(GetIllumination(GroupThreadID.x - 2, GroupThreadID.y) - IThis);
    float IR  = abs(GetIllumination(GroupThreadID.x + 1, GroupThreadID.y) - IThis);
    float IT  = abs(GetIllumination(GroupThreadID.x, GroupThreadID.y - 1) - IThis);
    float IT2 = abs(GetIllumination(GroupThreadID.x, GroupThreadID.y - 2) - IThis);
    float IB  = abs(GetIllumination(GroupThreadID.x, GroupThreadID.y + 1) - IThis);

    float CMAX = max(max(IL, IR), max(IT, IB));

    bool EL = IL > THRESHOLD;
    EL = EL && IL > (max(CMAX, IL2) * 0.5);

    bool ET = IT > THRESHOLD;
    ET = ET && IT > (max(CMAX, IT2) * 0.5);

    t_edges[DispatchThreadId] = float2( EL ? 1 : 0, ET ? 1 : 0 );
}
