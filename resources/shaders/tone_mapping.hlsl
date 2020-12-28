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

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESFilm(float3 color, float adapted_lum)
{
    const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 color =  t_source.Sample(linear_sampler, pin.uv).rgb;
    color = ACESFilm(color, 1);
    return float4(LinearToSRGB(color), 0.0f);
}