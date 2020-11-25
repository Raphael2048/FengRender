#include "light_common.hlsl"

cbuffer object_constant : register(b0)
{
    float4x4 world;
    float4x4 inv_world;
};

cbuffer pass_constant : register(b1)
{
    float4x4 ViewMatrix[6];
    float4x4 ProjMatrix;
    float3 LightPos;
    float Radius;
    float4 Color;
    float ShadowmapSize;
};

struct VertexIn
{
    float3 pos : POSITION;
};

struct VertexOut
{
    float3 pos : POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.pos = mul(float4(vin.pos, 1.0f), world).xyz;
    return vout;
}

struct GeoOut
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    uint index : SV_RENDERTARGETARRAYINDEX;
};

[maxvertexcount(18)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> triStream)
{
    for (uint f = 0; f < 6; ++f)
    {
        GeoOut output;
        output.index = f;
        for(int v = 0; v < 3; ++v)
        {
            output.worldPos = gin[v].pos; 
            output.pos = mul(mul(float4(output.worldPos, 1), ViewMatrix[f]), ProjMatrix);
            triStream.Append(output);
        }
        triStream.RestartStrip();
    }
}

float4 PS(GeoOut pin, out float depth : SV_Depth) : SV_Target
{
    //写入线性深度
    float dis = distance(pin.worldPos, LightPos);
    depth = dis / Radius;
    return float4(0, 0, 0, 0);
}