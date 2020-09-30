
cbuffer mtx : register(b0)
{
    float4x4 view;
    float4x4 inv_view;
    float4x4 proj;
    float4x4 inv_proj;
}

struct VertexIn
{
    float2 pos : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.pos = float4(vin.pos, 0.1, 1);
    vout.color = vin.color;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // float4 collll = float4(0.0f, 0.0f, 1.0f, 1.0f);
    return view[1];
    // return collll;
}