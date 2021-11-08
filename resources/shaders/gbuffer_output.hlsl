Texture2D t_albedo_metallic : register(t0);
Texture2D t_normal_roughness : register(t1);

SamplerState linear_sampler : register(s0);

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
    float3 camera_pos;
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
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    float4x4 MVP = mul(view, proj);
    MVP = mul(world, MVP);
    vout.pos = mul(float4(vin.pos, 1.0f), MVP);
    float3x3 local2view = transpose((float3x3)inv_world);
    vout.normal = mul(vin.normal, local2view);
    vout.tangent = mul(vin.tangent, local2view);
    vout.uv = vin.uv;
    return vout;
}

struct PixelOutput
{
    float4 base_color;
    float4 world_normal;
    float2 roughness_metallic;
};

PixelOutput PS(VertexOut pin) : SV_Target
{
    PixelOutput pout;
    float4 Sample1 = t_albedo_metallic.Sample(linear_sampler, pin.uv);
    pout.base_color.rgb = Sample1.rgb;
    pout.base_color.a = 0.0f;

    float4 Sample2 = t_normal_roughness.Sample(linear_sampler, pin.uv);
    float3 normalT = 2.0f * Sample2.rgb - 1.0f;

    float3 N = normalize(pin.normal);
    float3 T = normalize(pin.tangent - dot(pin.tangent, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    pout.world_normal.rgb = (mul(normalT, TBN) + 1.0) * 0.5;
    pout.world_normal.a = 0.0f;
    pout.roughness_metallic.x = Sample2.a;
    pout.roughness_metallic.y = Sample1.a;
    return pout;
}