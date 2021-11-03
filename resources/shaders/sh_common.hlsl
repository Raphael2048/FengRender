struct ThreeOrderSH
{
    float4 V0;
    float4 V1;
    float V2;
};

//球谐系数 https://en.wikipedia.org/wiki/Table_of_spherical_harmonics
ThreeOrderSH GetThreeOrderSHBasis(float3 direction)
{
    float x = direction.x;
    float y = direction.y;
    float z = direction.z;
    ThreeOrderSH result;
    // 1/(2 * sqrt(PI))
    result.V0.x = 0.28209479177387814f;
    // sqrt(3 / (4*PI)) * y
    result.V0.y = 0.4886025119029199 * y;
    result.V0.z = 0.4886025119029199 * z;
    result.V0.w = 0.4886025119029199 * x;

    // 1/2 * sqrt(15/PI) * xy
    result.V1.x = 1.0925484305920792 * x * y;
    result.V1.y = 1.0925484305920792 * y * z;
    // 1/4 * sqrt(5/PI) * (-x^2 - y^2 + 2z^2)
    result.V1.z = 0.31539156525252005 * (3*z*z - 1);
    result.V1.w = 1.0925484305920792 * z * x;
    // 1/4 * sqrt(15/PI) * (x^2 - y^2)
    result.V2 = 0.5462742152960396 * (x*x - y*y);
    return result;
}

//余弦函数的ZH系数,来自UE4源码, 自己计算亦可
ThreeOrderSH GetCosineThreeOrderSH(float3 direction)
{
    ThreeOrderSH result = GetThreeOrderSHBasis(direction);
    result.V0.x *= PI;
    result.V0.yzw *= (PI * 2 / 3);
    float L2 = PI / 4;
    result.V1.xyzw *= L2;
    result.V2 *= L2;
    return result;
}

float3 GetCosineZH()
{
    return float3(
        PI, PI * 2 / 3, PI / 4
    );
}

float DotSH3(ThreeOrderSH A, ThreeOrderSH B)
{
    return dot(A.V0, B.V0) + dot(A.V1, B.V1) + A.V2 * B.V2;
}

