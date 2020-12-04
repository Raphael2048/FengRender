//From  Next Generation Post Processing in Call of Duty: Advanced Warfare [Jimenez 2014]
// http://advances.realtimerendering.com/s2014/index.html

float InterleaveGradientNoise(float2 pixCoord, int frameCount)
{
    const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
    float2 frameMagicScale = float2(2.083f, 4.867f);
    pixCoord += frameCount * frameMagicScale;
    return frac(magic.z * frac(dot(pixCoord, magic.xy)));
}