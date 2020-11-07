Texture2D t_source : register(t0);

SamplerState linear_sampler : register(s0);

struct VertexIn
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VertexOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.pos = float4(vin.pos, 0.5f, 1.0f);
    vout.uv = vin.uv;
    return vout;
}

float Li2S(float lin)
{
    	if (lin <= 0.0031308)
		return lin * 12.92;
	else
		return 1.055 * pow(lin, 1.0 / 2.4) - 0.055;
}

float3 LinearToSRGB(float3 lin)
{
    return float3(
        Li2S(lin.r), Li2S(lin.g), Li2S(lin.b)
    );
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 color =  t_source.Sample(linear_sampler, pin.uv).rgb;
    return float4(LinearToSRGB(color), 0.0f);
}