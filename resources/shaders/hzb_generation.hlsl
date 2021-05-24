SamplerState linear_sampler : register(s0);
Texture2D ParentDepthTexture : register(t0);
RWTexture2D<float> HZBOutputs : register(u0);

#define GROUP_TILE_SIZE 16

cbuffer ParentSize:register(b0)
{
    uint4 MipPosAndSize[9];
}

[numthreads(GROUP_TILE_SIZE, GROUP_TILE_SIZE, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID, uint GroupThreadIndex : SV_GROUPINDEX)
{
    HZBOutputs[MipPosAndSize[0].xy + DispatchThreadId] = ParentDepthTexture.Load(int3(DispatchThreadId.xy, 0)).r;

    uint2 OutputPixelPos = DispatchThreadId;

    for(uint MipLevel = 1; MipLevel < 9; MipLevel++)
    {
        DeviceMemoryBarrierWithGroupSync();
        [branch]
        if (OutputPixelPos.x % 2 == 0 && OutputPixelPos.y % 2 == 0)
        {
            OutputPixelPos = OutputPixelPos >> 1;
            float4 ParentValues;
            int2 BasePos = MipPosAndSize[MipLevel - 1].xy + OutputPixelPos * 2;
            ParentValues.x = HZBOutputs[BasePos];
            ParentValues.y = HZBOutputs[BasePos + uint2(1, 0)];
            ParentValues.z = HZBOutputs[BasePos + uint2(0, 1)];
            ParentValues.w = HZBOutputs[BasePos + uint2(1, 1)];
            HZBOutputs[MipPosAndSize[MipLevel].xy + OutputPixelPos] = max(max(ParentValues.x, ParentValues.y), max(ParentValues.z, ParentValues.w));
        }
    }
}