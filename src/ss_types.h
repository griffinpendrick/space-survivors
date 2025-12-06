#if !defined(SS_TYPES_H)
#define SS_TYPES_H

#include <stdint.h>

#if defined(_MSC_VER)
#define INLINE __inline
#else
#define INLINE static inline
#endif

#define internal static
#define local_persist static
#define global static
#define local_const static const
#define global_const static const

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32_t bool32;

typedef float f32;
typedef double f64;

typedef Vector2 v2;

#endif