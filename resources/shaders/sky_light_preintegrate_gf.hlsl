RWTexture2D<float2> GF : register(u0);
#include "light_common.hlsl"
#include "specular_brdf_common.hlsl"

float2 IntegrateBRDF( float Roughness, float NoV )
{
    float3 V;
    V.x = sqrt( 1.0f - NoV * NoV ); // sin
    V.y = 0;
    V.z = NoV;
    // cos
    float A = 0;
    float B = 0;
    const uint NumSamples = 1024;
    float3 N = float3(0, 0, 1);
    for( uint i = 0; i < NumSamples; i++ )
    {
        float2 Xi = Hammersley( i, NumSamples );
        float3 H = ImportanceSampleGGX( Xi, Roughness, N );
        float3 L = 2 * dot( V, H ) * H - V;
        float NoL = saturate( L.z );
        float NoH = saturate( H.z );
        float VoH = saturate( dot( V, H ) );
        if( NoL > 0 )
        {
            float G = IBLGeometrySchlickGGX(NoL, Roughness) * IBLGeometrySchlickGGX(NoV, Roughness);
            float G_Vis = G * VoH / (NoH * NoV);
            float Fc = pow( 1 - VoH, 5 );
            A += (1 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return float2( A, B ) / NumSamples;
}

// 128*32
[numthreads(8, 8, 1)]
void CS(int3 dispatch_thread_id : SV_DispatchThreadID)
{
    int x = dispatch_thread_id.x;
    int y = dispatch_thread_id.y;
    float CosTheta = float(x + 0.5) / 128;
    float Roughness = float(y + 0.5) / 32;
    GF[dispatch_thread_id.xy] = IntegrateBRDF(Roughness, CosTheta);
}