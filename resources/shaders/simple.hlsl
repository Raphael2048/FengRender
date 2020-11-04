
cbuffer object_constant : register(b0)
{
    float4x4 world;
    float4x4 inv_world;
};

cbuffer pass_constant : register(b1)
{
    float4x4 view;
    float4x4 inv_view;
    float4x4 proj;
    float4x4 inv_proj;
    float4x4 view_proj;
    float4x4 inv_view_proj;
};

struct VertexIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct VertexOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    float4x4 MVP = mul(view, proj);
    MVP = mul(world, MVP);
    vout.pos = mul(float4(vin.pos, 1.0f), MVP);
    // vout.pos = float4(vin.pos.xy, 0.5f, 1.0f);
    vout.color = float4(vin.uv, 1.0f, 1.0f);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // float4 collll = float4(0.0f, 0.0f, 1.0f, 1.0f);
    // return view[1];
    return pin.color;
    // return collll;
}