struct PassConstant
{
    float4x4 view;
    float4x4 inv_view;
    float4x4 proj;
    float4x4 inv_proj;
    float4x4 view_proj;
    float4x4 inv_view_proj;
    float3 camera_pos;
};

static const float PI = 3.14159265359;
// D 法线分布项
float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// G 几何遮挡项
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// F 菲涅尔反射项
float3 FresnelSchlick (float3 f0, float cosTheta)
{
    return f0 + (1.0f - f0) * pow(1.0f - cosTheta, 5.0f);
}

float3 PBRLight(float3 N, float3 V, float3 L, float3 BaseColor, float Roughness, float Metallic)
{
    float3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), BaseColor, Metallic);
    float NDF = DistributionGGX(NdotH, Roughness);
    float G = GeometrySchlickGGX(NdotV, Roughness) * GeometrySchlickGGX(NdotL, Roughness);
    float3 F = FresnelSchlick(F0, HdotV);

    float3 nominator = NDF * G * F;
    float denominator = 4 * NdotV * NdotL + 0.001;
    float3 specular = nominator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= (1.0 - Metallic);

    return (kD * BaseColor / PI + specular) * NdotL;
}

// [ Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion" ]
float3 AOMultiBounce( float3 BaseColor, float AO )
{
	float3 a =  2.0404 * BaseColor - 0.3324;
	float3 b = -4.7951 * BaseColor + 0.6417;
	float3 c =  2.7552 * BaseColor + 0.6903;
	return max( AO, ( ( AO * a + b ) * AO + c ) * AO );
}

float Illumination(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}
