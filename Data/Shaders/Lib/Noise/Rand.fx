#ifndef LIB_NOISE_RAND_FX_INCLUDED_
#define LIB_NOISE_RAND_FX_INCLUDED_

float RandNoise(float2 seed) {
    return sin(dot(seed, float2(12.9898, 78.233)*2.0)) * 43758.5453;
}

float2 Rand2(float2 xy) {
    float noise = RandNoise(xy);

    float2 result;
    result.x = frac(noise)*2.0 - 1.0;
    result.y = frac(noise*1.2154)*2.0 - 1.0;
    return result;
}

float3 Rand3(float2 xy) {
    float noise = RandNoise(xy);

    float3 result;
    result.x = frac(noise)*2.0 - 1.0;
    result.y = frac(noise*1.2154)*2.0 - 1.0;
    result.z = frac(noise*1.3453)*2.0 - 1.0;
    return result;
}

#endif //!LIB_NOISE_RAND_FX_INCLUDED_
