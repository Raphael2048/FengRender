//From  Next Generation Post Processing in Call of Duty: Advanced Warfare [Jimenez 2014]
// http://advances.realtimerendering.com/s2014/index.html

float InterleaveGradientNoise(float2 pixCoord, int frameCount)
{
    const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
    float2 frameMagicScale = float2(2.083f, 4.867f);
    pixCoord += frameCount * frameMagicScale;
    return frac(magic.z * frac(dot(pixCoord, magic.xy)));
}

// 3D random number generator inspired by PCGs (permuted congruential generator)
// Using a **simple** Feistel cipher in place of the usual xor shift permutation step
// @param v = 3D integer coordinate
// @return three elements w/ 16 random bits each (0-0xffff).
// ~8 ALU operations for result.x    (7 mad, 1 >>)
// ~10 ALU operations for result.xy  (8 mad, 2 >>)
// ~12 ALU operations for result.xyz (9 mad, 3 >>)
uint3 Rand3DPCG16(int3 p)
{
	// taking a signed int then reinterpreting as unsigned gives good behavior for negatives
	uint3 v = uint3(p);

	// Linear congruential step. These LCG constants are from Numerical Recipies
	// For additional #'s, PCG would do multiple LCG steps and scramble each on output
	// So v here is the RNG state
	v = v * 1664525u + 1013904223u;

	// PCG uses xorshift for the final shuffle, but it is expensive (and cheap
	// versions of xorshift have visible artifacts). Instead, use simple MAD Feistel steps
	//
	// Feistel ciphers divide the state into separate parts (usually by bits)
	// then apply a series of permutation steps one part at a time. The permutations
	// use a reversible operation (usually ^) to part being updated with the result of
	// a permutation function on the other parts and the key.
	//
	// In this case, I'm using v.x, v.y and v.z as the parts, using + instead of ^ for
	// the combination function, and just multiplying the other two parts (no key) for 
	// the permutation function.
	//
	// That gives a simple mad per round.
	v.x += v.y*v.z;
	v.y += v.z*v.x;
	v.z += v.x*v.y;
	v.x += v.y*v.z;
	v.y += v.z*v.x;
	v.z += v.x*v.y;

	// only top 16 bits are well shuffled
	return v >> 16u;
}

// Wraps noise for tiling texture creation
// @param v = unwrapped texture parameter
// @param bTiling = true to tile, false to not tile
// @param RepeatSize = number of units before repeating
// @return either original or wrapped coord
float3 NoiseTileWrap(float3 v,  bool bTiling, float RepeatSize)
{
	return bTiling ? (frac(v / RepeatSize) * RepeatSize) : v;
}

// Evaluate polynomial to get smooth transitions for Perlin noise
// only needed by Perlin functions in this file
// scalar(per component): 2 add, 5 mul
float4 PerlinRamp(float4 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10); 
}

#define MGradientMask int3(0x8000, 0x4000, 0x2000)
#define MGradientScale float3(1. / 0x4000, 1. / 0x2000, 1. / 0x1000)
// Modified noise gradient term
// @param seed - random seed for integer lattice position
// @param offset - [-1,1] offset of evaluation point from lattice point
// @return gradient direction (xyz) and contribution (w) from this lattice point
float4 MGradient(int seed, float3 offset)
{
	uint rand = Rand3DPCG16(int3(seed,0,0)).x;
	float3 direction = float3(rand.xxx & MGradientMask) * MGradientScale - 1;
	return float4(direction, dot(direction, offset));
}

// compute Perlin and related noise corner seed values
// @param v = 3D noise argument, use float3(x,y,0) for 2D or float3(x,0,0) for 1D
// @param bTiling = true to return seed values for a repeating noise pattern
// @param RepeatSize = integer units before tiling in each dimension
// @param seed000-seed111 = hash function seeds for the eight corners
// @return fractional part of v
float3 NoiseSeeds(float3 v, bool bTiling, float RepeatSize,
	out float seed000, out float seed001, out float seed010, out float seed011,
	out float seed100, out float seed101, out float seed110, out float seed111)
{
	float3 fv = frac(v);
	float3 iv = floor(v);

	const float3 primes = float3(19, 47, 101);

	if (bTiling)
	{	// can't algebraically combine with primes
		seed000 = dot(primes, NoiseTileWrap(iv, true, RepeatSize));
		seed100 = dot(primes, NoiseTileWrap(iv + float3(1, 0, 0), true, RepeatSize));
		seed010 = dot(primes, NoiseTileWrap(iv + float3(0, 1, 0), true, RepeatSize));
		seed110 = dot(primes, NoiseTileWrap(iv + float3(1, 1, 0), true, RepeatSize));
		seed001 = dot(primes, NoiseTileWrap(iv + float3(0, 0, 1), true, RepeatSize));
		seed101 = dot(primes, NoiseTileWrap(iv + float3(1, 0, 1), true, RepeatSize));
		seed011 = dot(primes, NoiseTileWrap(iv + float3(0, 1, 1), true, RepeatSize));
		seed111 = dot(primes, NoiseTileWrap(iv + float3(1, 1, 1), true, RepeatSize));
	}
	else
	{	// get to combine offsets with multiplication by primes in this case
		seed000 = dot(iv, primes);
		seed100 = seed000 + primes.x;
		seed010 = seed000 + primes.y;
		seed110 = seed100 + primes.y;
		seed001 = seed000 + primes.z;
		seed101 = seed100 + primes.z;
		seed011 = seed010 + primes.z;
		seed111 = seed110 + primes.z;
	}

	return fv;
}

// Perlin-style "Modified Noise"
// http://www.umbc.edu/~olano/papers/index.html#mNoise
// @param v = 3D noise argument, use float3(x,y,0) for 2D or float3(x,0,0) for 1D
// @param bTiling = repeat noise pattern
// @param RepeatSize = integer units before tiling in each dimension
// @return random number in the range -1 .. 1
float GradientNoise3D_ALU(float3 v, bool bTiling, float RepeatSize)
{
	float seed000, seed001, seed010, seed011, seed100, seed101, seed110, seed111;
	float3 fv = NoiseSeeds(v, bTiling, RepeatSize, seed000, seed001, seed010, seed011, seed100, seed101, seed110, seed111);

	float rand000 = MGradient(int(seed000), fv - float3(0, 0, 0)).w;
	float rand100 = MGradient(int(seed100), fv - float3(1, 0, 0)).w;
	float rand010 = MGradient(int(seed010), fv - float3(0, 1, 0)).w;
	float rand110 = MGradient(int(seed110), fv - float3(1, 1, 0)).w;
	float rand001 = MGradient(int(seed001), fv - float3(0, 0, 1)).w;
	float rand101 = MGradient(int(seed101), fv - float3(1, 0, 1)).w;
	float rand011 = MGradient(int(seed011), fv - float3(0, 1, 1)).w;
	float rand111 = MGradient(int(seed111), fv - float3(1, 1, 1)).w;

	float3 Weights = PerlinRamp(float4(fv, 0)).xyz;

	float i = lerp(lerp(rand000, rand100, Weights.x), lerp(rand010, rand110, Weights.x), Weights.y);
	float j = lerp(lerp(rand001, rand101, Weights.x), lerp(rand011, rand111, Weights.x), Weights.y);
	return lerp(i, j, Weights.z).x;
}

