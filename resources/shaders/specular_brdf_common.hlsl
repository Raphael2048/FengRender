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

// IBL的G项和Direct的不同
float IBLGeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness;
    float k = (r*r) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float3 FresnelSchlickRoughness(float CosTheta, float3 F0, float Roughness)
{
    return F0 + (max(float3(1 - Roughness, 1 - Roughness, 1 - Roughness), F0) - F0) * pow(1.0 - CosTheta, 5.0);
}