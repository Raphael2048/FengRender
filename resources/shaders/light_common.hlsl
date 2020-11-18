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

// N 法线分布项
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
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), BaseColor, Roughness);
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


//球谐系数 https://en.wikipedia.org/wiki/Table_of_spherical_harmonics
float GetSHCoffient(int index, float3 direction)
{
    float x = direction.x;
    float y = direction.y;
    float z = direction.z;
    switch(index){
        // 1/2 * sqrt(1/PI)
        case 0:
            return 0.28209479177387814;
            break;
        // sqrt(3/(4PI)) * z
        case 1:
            return 0.4886025119029199 * z;
            break;
        case 2:
            return 0.4886025119029199 * y;
            break;
        case 3:
            return 0.4886025119029199 * x;
            break;
        // 1/2 * sqrt(15/PI) * xz
        case 4:
            return 1.0925484305920792 * x * z;
            break;
        case 5:
            return 1.0925484305920792 * y * z;
            break;
        // 1/4 * (5/PI) * (-x^2 - y^2 + 2z^2)
        case 6:
            return 0.31539156525252005 * (3*y*y - 1);
            break;
        case 7:
            return 1.0925484305920792 * y * x;
            break;
        // 1/4 * sqrt(15/PI) * (x^2 - z^2)
        case 8:
            return 0.5462742152960396 * (x*x - z*z);
            break;
    }
    return 0.0f;
}