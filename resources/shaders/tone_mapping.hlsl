Texture2D t_source : register(t0);

SamplerState linear_sampler : register(s0);

struct VertexIn
{
    float2 pos : POSITION;
};

struct VertexOut
{
    float4 pos : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.pos = float4(vin.pos, 0.5f, 1.0f);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 color =  t_base_color.Sample(linear_sampler, pin.uv).rgb;
    return color;
}