
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

bool4 ModeOfSingle(float value)
{
    bool4 ret = false;
    if (value > 0.875)
        ret.yz = bool2(true, true);
    else if(value > 0.5)
        ret.z = true;
    else if(value > 0.125)
        ret.y = true;
    return ret;
}

bool4 ModeOfDouble(float value1, float value2)
{
    bool4 ret;
    ret.xy = ModeOfSingle(value1).yz;
    ret.zw = ModeOfSingle(value2).yz;
    return ret;
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

// 圆角系数, 保留物体实际的边缘; 若为0 表示全保留, 为1表示不变
#define ROUNDING_FACTOR 0.25

float Area(float2 d, bool4 left, bool4 right)
{
    // result为正, 表示将该像素点颜色扩散至上/左侧; result为负, 表示将上/左侧颜色扩散至该像素
    float result = 0;
    [branch]
    if(!left.y && !left.z)
    {
        [branch]
        if(right.y && !right.z)
        {
            result = L_N_Shape(d.y + d.x + 1, d.y + 0.5);
        }
        else if (!right.y && right.z)
        {
            result = -L_N_Shape(d.y + d.x + 1, d.y + 0.5);
        }
    }
    else if (left.y && !left.z)
    {
        [branch]
        if(right.z)
        {
            result = L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
        else if (!right.y)
        {
            result = L_N_Shape(d.y + d.x + 1, d.x + 0.5);
        }
        else
        {
            result = L_L_S_Shape(d.x + 0.5, d.y + 0.5);
        }
    }
    else if (!left.y && left.z)
    {
        [branch]
        if (right.y)
        {
            result = -L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
        else if (!right.z)
        {
            result = -L_N_Shape(d.x + d.y + 1, d.x + 0.5);
        }
        else
        {
            result = -L_L_S_Shape(d.x + 0.5, d.y + 0.5);
        }
    }
    else
    {
        [branch]
        if(right.y && !right.z)
        {
            result = -L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
        else if (!right.y && right.z)
        {
            result = L_L_D_Shape(d.x + 0.5, d.y + 0.5);
        }
    }

#ifdef ROUNDING_FACTOR
    bool apply = false;
    if (result > 0)
    {
        if(d.x < d.y && left.x)
        {
            apply = true;
        }
        else if(d.x >= d.y && right.x)
        {
            apply = true;
        }
    }
    else if (result < 0)
    {
        if(d.x < d.y && left.w)
        {
            apply = true;
        }
        else if(d.x >= d.y && right.w)
        {
            apply = true;
        }
    }
    if (apply)
    {
        result = result * ROUNDING_FACTOR;
    }
#endif

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

        bool4 l, r;
#ifdef ROUNDING_FACTOR
        float left1 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(-left, -1.25)) * Screen.InvScreenSize, 0).r;
        float left2 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(-left, 0.75)) * Screen.InvScreenSize, 0).r;
        l = ModeOfDouble(left1, left2);
        float right1 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(right + 1, -1.25)) * Screen.InvScreenSize, 0).r;
        float right2 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(right + 1, 0.75)) * Screen.InvScreenSize, 0).r;
        r = ModeOfDouble(right1, right2);
#else
        float left_value = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(-left, -0.25)) * Screen.InvScreenSize, 0).r;
        float right_value = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(right + 1, -0.25)) * Screen.InvScreenSize, 0).r;
        l = ModeOfSingle(left_value);
        r = ModeOfSingle(right_value);
#endif
        float value = Area(float2(left, right), l, r);
        result.xy = float2(-value, value);
    }

    if (edge.r > 0.1f)
    {
        float up = SearchYUp(ScreenPos);
        float down = SearchYDown(ScreenPos);

        bool4 u, d;
#ifdef ROUNDING_FACTOR
        float up1 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(-1.25, -up)) * Screen.InvScreenSize, 0).g;
        float up2 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(0.75, -up)) * Screen.InvScreenSize, 0).g;
        float down1 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(-1.25, down + 1)) * Screen.InvScreenSize, 0).g;
        float down2 = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(0.75, down + 1)) * Screen.InvScreenSize, 0).g;
        u = ModeOfDouble(up1, up2);
        d = ModeOfDouble(down1, down2);
#else
        float up_value = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(-0.25, -up)) * Screen.InvScreenSize, 0).g;
        float down_value = t_edges.SampleLevel(linear_sampler, (ScreenPos + float2(-0.25, down + 1)) * Screen.InvScreenSize, 0).g;
        u = ModeOfSingle(up_value);
        d = ModeOfSingle(down_value);
#endif
        float value = Area(float2(up, down), u, d);
        result.zw = float2(-value, value);
    }
    result = max(result, 0);
    t_weights[DispatchThreadId] = result;
}
