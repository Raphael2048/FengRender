
// [Filtering Approaches for Real-Time Anti-Aliasing, SIGGRAPH2011]
SamplerState linear_sampler : register(s0);

Texture2D<float2> t_edges : register(t0);

RWTexture2D<float4> t_weights : register(u0);

#include "pp_common.hlsl"
ConstantBuffer<ScreenSize> Screen : register(b0);

#define MAXSTEPS 3

float SearchXLeft(float2 coord)
{
    coord -= float2(1.5f, 0);
    float e = 0;
    for(int i = 0; i < MAXSTEPS; i++)
    {
        e = t_edges.SampleLevel(linear_sampler, coord * Screen.InvScreenSize, 0).g;
        [flatten]
        if (e < 0.9f)  break;
        coord -= float2(2, 0);
    }
    return min(2.0 * (i +  e), 2.0 * MAXSTEPS);
}

float SearchXRight(float2 coord)
{
    coord += float2(1.5f, 0);
    float e = 0;
    for(int i = 0; i < MAXSTEPS; i++)
    {
        e = t_edges.SampleLevel(linear_sampler, coord * Screen.InvScreenSize, 0).g;
        [flatten]
        if (e < 0.9f)  break;
        coord += float2(2, 0);
    }
    return min(2.0 * (i +  e), 2.0 * MAXSTEPS);
}

float SearchYUp(float2 coord)
{
    coord -= float2(0, 1.5f);
    float e = 0;
    for(int i = 0; i < MAXSTEPS; i++)
    {
        e = t_edges.SampleLevel(linear_sampler, coord * Screen.InvScreenSize, 0).r;
        [flatten]
        if (e < 0.9f)  break;
        coord -= float2(0, 2);
    }
    return min(2.0 * (i +  e), 2.0 * MAXSTEPS);
}

float SearchYDown(float2 coord)
{
    coord += float2(0, 1.5f);
    float e = 0;
    for (int i = 0; i < MAXSTEPS; i++)
    {
        e = t_edges.SampleLevel(linear_sampler, coord * Screen.InvScreenSize, 0).r;
        [flatten]
        if (e < 0.9f)  break;
        coord += float2(0, 2);
    }
    return min(2.0 * (i +  e), 2.0 * MAXSTEPS);
}

int ModeOf(float value)
{
    int mode = 3;
    if (value < 0.125)
        mode = 0;
    else if (value < 0.5)
        mode = 1;
    else if (value < 0.875)
        mode = 2;
    return mode;
}

//  单侧L型, 另一侧没有, d表示总间隔, m表示像素中心距边缘距离
//  |____
// 
float L_N_Shape(float d, float m)
{
    float l = d * 0.5;
    float s = 0;
    [flatten]
    if ( l > (m + 0.5))
    {
        // 梯形面积, 宽为1
        s = (l - m) * 0.5 / l;
    }
    else if (l > (m - 0.5))
    {
        // 三角形面积, a是宽, b是高
        float a = l - m + 0.5;
        // float b = a * 0.5 / l;
        // float s = a * b * 0.5;
        float s = a * a * 0.25 * rcp(l);
    }
    return s;
}

//  双侧L型, 且方向相同
//  |____|
// 
float L_L_S_Shape(float d1, float d2)
{
    float d = d1 + d2;
    float s1 = L_N_Shape(d, d1);
    float s2 = L_N_Shape(d, d2);
    return s1 + s2;
}

//  双侧L型/或一侧L, 一侧T, 且方向不同, 这里假设左侧向上, 来取正负
//  |____    |___|    
//       |       |
float L_L_D_Shape(float d1, float d2)
{
    float d = d1 + d2;
    float s1 = L_N_Shape(d, d1);
    float s2 = -L_N_Shape(d, d2);
    return s1 + s2;
}


float Area(float2 d, float2 e)
{
    int mode1 = ModeOf(e.x);
    int mode2 = ModeOf(e.y);
    float result = 0;
    // result为正, 表示将该像素点颜色扩散至上/左侧; result为负, 表示将上/左侧颜色扩散至该像素
    if (mode1 == 0)
    {
        if (mode2 == 1)
        {
            result = L_N_Shape(d.y + d.x + 1, d.y + 0.5);
        }
        else if (mode2 == 2)
        {
            result = -L_N_Shape(d.y + d.x + 1, d.y + 0.5);
        }
    }
    else if (mode1 == 1)
    {
        if (mode2 == 0)
        {
            result = L_N_Shape(d.y + d.x + 1, d.x + 0.5);
        }
        else if (mode2 == 1)
        {
            result = L_L_S_Shape(d.x + 0.5, d.y + 0.5);
        }
        else
        {
            result = L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
    }
    else if (mode1 == 2)
    {
        if (mode2 == 0)
        {
            result = -L_N_Shape(d.x + d.y + 1, d.x + 0.5);
        }
        else if (mode2 == 2)
        {
            result = -L_L_S_Shape(d.x + 0.5, d.y + 0.5);
        }
        else
        {
            result = -L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
    }
    else
    {
        if (mode2 == 1)
        {
            result = -L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
        else if (mode2 == 2)
        {
            result = L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
    }
    return result;

}

[numthreads(8, 8, 1)]
void CS(uint2 DispatchThreadId : SV_DISPATCHTHREADID)
{
    float2 ScreenPos = DispatchThreadId + float2(0.5, 0.5);
    float2 edge = t_edges.mips[0][DispatchThreadId].rg;
    float4 result = 0;
    if (edge.g > 0.1f) {
        float left = SearchXLeft(ScreenPos);
        float right = SearchXRight(ScreenPos);

        float2 left_uv = (ScreenPos + float2(-left, -0.25)) * Screen.InvScreenSize;
        float2 right_uv = (ScreenPos + float2(right + 1, -0.25)) * Screen.InvScreenSize;

        float left_value = t_edges.SampleLevel(linear_sampler, left_uv, 0).r;
        float right_value = t_edges.SampleLevel(linear_sampler, right_uv, 0).r;
        float value = Area(float2(left, right), float2(left_value, right_value));
        result.zw = float2(value, -value);
    }

    if (edge.r > 0.1f)
    {
        float up = SearchYUp(ScreenPos);
        float down = SearchYDown(ScreenPos);

        float2 up_uv = (ScreenPos + float2(-0.25, -up)) * Screen.InvScreenSize;
        float2 down_uv = (ScreenPos + float2(-0.25, down + 1)) * Screen.InvScreenSize;

        float up_value = t_edges.SampleLevel(linear_sampler, up_uv, 0).g;
        float down_value = t_edges.SampleLevel(linear_sampler, down_uv, 0).g;
        float value = Area(float2(up, down), float2(up_value, down_value));
        result.xy = float2(value, -value);
    }

    result = max(result, 0);
    t_weights[DispatchThreadId] = result;
}
