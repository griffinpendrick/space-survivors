#if !defined(SS_H)
#define SS_H

INLINE f32 RandomFloat(f32 Min, f32 Max)
{
    return(Min + ((f32)GetRandomValue(0, INT32_MAX) / (f32)INT32_MAX) * (Max - Min));
}

#endif