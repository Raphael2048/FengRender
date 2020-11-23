#include "light_common.hlsl"
#include "sh_common.hlsl"
TextureCube t_cubemap : register(t0);
SamplerState linear_sampler : register(s0);
cbuffer C1 : register(b0)
{
    float4x4 ViewMatrix; 
};
cbuffer C2 : register(b1)
{
    float ConstantRoughness;
};

#include "pp_common.hlsl"

// EPIC GAMES s2013_PBR_Epic_Note_V2
float RadicalInverse_Vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_Vdc(i));
}

float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
    float a = Roughness * Roughness;
    float Phi = 2.0 * PI * Xi.x;

    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y) );
    float SinTheta = sqrt(1 - CosTheta * CosTheta);

    float3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;

    float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 TangentX = normalize( cross(UpVector, N) );
    float3 TangentY = cross(N, TangentX);

    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 PrefilterEnvMap(float Roughness, float3 R)
{
    float3 N = R;
    float3 V = R;
    float3 PrefilteredColor = 0;
    float TotalWeight = 0;
    const uint NumSamples = 1024;
    for(uint i = 0; i < NumSamples; i++)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, N);
        float3 L = 2 * dot( V, H ) * H - V;
        float NoL = saturate( dot( N, L ) );
        if( NoL > 0 )
        {
            PrefilteredColor += t_cubemap.Sample(linear_sampler, L).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 dir = normalize(float3(pin.uv.x * 2.0f - 1.0f, 1.0f - pin.uv.y * 2.0f, 1.0f));
    float3 target_dir = mul(dir, (float3x3)ViewMatrix);
    // float4 value = t_cubemap.Sample(linear_sampler, target_dir);
    return float4(PrefilterEnvMap(ConstantRoughness, target_dir), 1);
    // return float4(target_dir * 0.5 + 1, 0);
    // return float4(dir, 0);
    // return float4(target_dir, 1);
    // return float4(1, 1, 1, 1);
}

