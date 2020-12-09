SamplerState linear_sampler : register(s0);

Texture2D t_depth : register(t0);
Texture2D t_ao_filter : register(t1);

RWTexture2D<float> t_ao : register(u0);

#include "light_common.hlsl"

ConstantBuffer<PassConstant> PC : register(b0);

cbuffer GTAOBuffer : register(b1)
{
    uint2 ScreenSize;
    float2 InvScreenSize;
    float InvRadiusSq;
};

// 16 * 8 周围扩展2像素
groupshared float AOData[20 * 12];
groupshared float ZData[20 * 12];

int GetLDSLocation(int x, int y)
{
    return (y + 2) * 20 + x + 2;
}

float GetAOLin(int loc)
{
	return AOData [loc];
}

float GetZLin(int loc)
{
	return ZData[loc];
}

float GetAO(int x, int y)
{
    return AOData[(y + 2) * 20 + x + 2];
}

float GetZ(int x, int y)
{
    return ZData[(y + 2) * 20 + x + 2];
}


[numthreads(16, 8, 1)]
void CS(uint GroupIndex:SV_GROUPINDEX, uint2 GroupId:SV_GROUPID,
            uint2 DispatchThreadID : SV_DISPATCHTHREADID, uint2 GroupThreadID : SV_GROUPTHREADID)
{
    int2 GTId = int2(GroupThreadID);
    int2 FullGroupOrigin = int2(GroupId.x * 16, GroupId.y * 8);
    int2 FullGroupOriginM2	= FullGroupOrigin.xy - int2(2,2);
    uint pixIdx = GroupIndex * 2;

    if (pixIdx < (20 * 12))
    {
        uint XPos = pixIdx % 20;
		uint YPos = pixIdx / 20;
        int LDSPos = (YPos * 20) + XPos;
        int2 ReadXYAO = FullGroupOriginM2 + int2(XPos,YPos);
		int2 ReadXYZ  = ReadXYAO;

		float AO = t_ao_filter.Load(int3(ReadXYAO, 0)).r;
		float  Z = t_depth.Load(int3(ReadXYZ, 0)).r;
		AOData[ LDSPos ] = AO;
		ZData[  LDSPos ] = Z;

        LDSPos++;

		ReadXYAO.x	+=1;
		ReadXYZ.x	+=1;
		AO = t_ao_filter.Load(int3(ReadXYAO, 0)).r;
		Z  = t_depth.Load(int3(ReadXYZ, 0)).r;
		AOData[ LDSPos ] = AO;
		ZData[  LDSPos ] = Z;
    }
    GroupMemoryBarrierWithGroupSync();

    float ThisZ		= GetZ(GTId.x, GTId.y);
	// float ThisZLin	= ConvertFromDeviceZ( ThisZ);
    float2 ZDiff;

    int FilterMin = -2;
    int FilterMax = 2;
    int LDSBase = GetLDSLocation(GTId.x + FilterMin, GTId.y + FilterMin);

    //X方向 最大深度差
    int LDSCenter = GetLDSLocation(GTId.x, GTId.y);
    {
        float XM2Z	= GetZLin(LDSCenter-2);
		float XM1Z	= GetZLin(LDSCenter-1);
		float XP1Z	= GetZLin(LDSCenter+1);
		float XP2Z	= GetZLin(LDSCenter+2);
		float C1 = abs((XM1Z + (XM1Z - XM2Z)) - ThisZ);
		float C2 = abs((XP1Z + (XP1Z - XP2Z)) - ThisZ);
		if(C1 < C2)
		{
			ZDiff.x = XM1Z - XM2Z;
		}
		else
		{
			ZDiff.x = XP2Z - XP1Z;
		}
    }
    //Y方向 最大深度差
    {
        float YM2Z	= GetZLin(LDSCenter-(2*20));
		float YM1Z	= GetZLin(LDSCenter-(1*20));
		float YP1Z	= GetZLin(LDSCenter+(1*20));
		float YP2Z	= GetZLin(LDSCenter+(2*20));
										 
		// Get extrapolated point either side
		float C1 = abs((YM1Z + (YM1Z - YM2Z)) - ThisZ);
		float C2 = abs((YP1Z + (YP1Z - YP2Z)) - ThisZ);

		if(C1 < C2)
		{
			ZDiff.y = YM1Z - YM2Z;
		}
		else
		{
			ZDiff.y = YP2Z - YP1Z;
		}
    }

    float SumAO		= 0;
	float SumWeight = 0;
	float DepthBase = ThisZ  +(ZDiff.x*FilterMin) + (ZDiff.y*FilterMin);
	float SimpleBlur=0.0;
    for (int y = FilterMin; y <= FilterMax; y++)
    {
		float PlaneZ = DepthBase;
		int LDSLineBase = LDSBase;
		LDSBase += 20;
		for(int x=FilterMin; x<=FilterMax; x++)
		{
			float Sample_AO = GetAOLin(LDSLineBase);
			float SampleZ   = GetZLin( LDSLineBase);
			LDSLineBase++;

			float SampleZDiff = abs(PlaneZ - SampleZ);
			
			const float SpatialFilterWeight = 20000;
			float Weight =  1.0f  - saturate(SampleZDiff*SpatialFilterWeight );
			SumAO += Sample_AO * Weight;
			SumWeight += Weight;
			PlaneZ += ZDiff.x;
		}
		DepthBase += ZDiff.y;
    }
	SumAO /= SumWeight;

    t_ao[DispatchThreadID] = SumAO;
}
