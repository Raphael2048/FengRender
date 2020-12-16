SamplerState linear_sampler : register(s0);
Texture2D ParentDepthTexture : register(t0);
RWTexture2D<float> HZBOutputs[5] : register(u0);

#define GROUP_TILE_SIZE 16

cbuffer ParentSize:register(b0)
{
    float2 DispatchIdToUV;
}

// groupshared float SharedClosestDeviceZ[GROUP_TILE_SIZE * GROUP_TILE_SIZE];

[numthreads(GROUP_TILE_SIZE, GROUP_TILE_SIZE, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID, uint GroupThreadIndex : SV_GROUPINDEX)
{
    float2 BufferUV = (DispatchThreadId + 0.5) * DispatchIdToUV;
    float4 DeviceZ = ParentDepthTexture.GatherRed(linear_sampler, BufferUV, 0);
    float ClosestZ = max(max(DeviceZ.x, DeviceZ.y), max(DeviceZ.z, DeviceZ.w));
   
    uint2 OutputPixelPos = DispatchThreadId;
    HZBOutputs[0][OutputPixelPos] = ClosestZ;
    //写入到共享内存, 加速访问
    // SharedClosestDeviceZ[GroupThreadIndex] = ClosestZ;

    [unroll]
    for(uint MipLevel = 1; MipLevel < 5; MipLevel++)
    {
        GroupMemoryBarrierWithGroupSync();
        [branch]
        if (OutputPixelPos.x % 2 == 0 && OutputPixelPos.y % 2 == 0)
        {
            OutputPixelPos = OutputPixelPos >> 1;
            float4 ParentValues;
            ParentValues.x = HZBOutputs[MipLevel-1][OutputPixelPos * 2];
            ParentValues.y = HZBOutputs[MipLevel-1][OutputPixelPos * 2 + uint2(1, 0)];
            ParentValues.z = HZBOutputs[MipLevel-1][OutputPixelPos * 2 + uint2(0, 1)];
            ParentValues.w = HZBOutputs[MipLevel-1][OutputPixelPos * 2 + uint2(1, 1)];
            HZBOutputs[MipLevel][OutputPixelPos] = max(max(ParentValues.x, ParentValues.y), max(ParentValues.z, ParentValues.w));
        }
    }
}