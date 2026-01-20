#ifndef HCGMATH_HCGMATH_H
#define HCGMATH_HCGMATH_H

#if (defined(__GNUC__) || defined(__clang__))

#define HMATH_ALL

#ifdef HMATH_ALL
#define H_VECTOR
#define H_MATRIX
#endif
#include <complex.h>
#include <math.h>

#ifndef H_TYPES
#include <stdint.h>
typedef float       f32;
typedef double      f64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
#endif

#define lerp(a, b, t) ({typeof(a) _a = (a); typeof(b) _b = (b);typeof(t)_t=(t); _a + (_b - _a) * (_t);})

#ifdef H_VECTOR

#define H_SWIZZLE(Vec, ...) __builtin_shufflevector(Vec, Vec, __VA_ARGS__)

    #ifdef __clang__
#define H_VECTOR_TYPE(Type, Size) Type __attribute__((ext_vector_type(Size)))
    #elif  defined(__GNUC__)
#define H_VECTOR_TYPE(Type, Size) Type __attribute__((vector_size(Size * sizeof(Type))))
    #endif

    // int vectors
    typedef H_VECTOR_TYPE(i32, 2) ivec2;
    typedef H_VECTOR_TYPE(i32, 4) ivec4;
    typedef H_VECTOR_TYPE(i32, 8) ivec8;

    // float vectors
    typedef H_VECTOR_TYPE(f32, 2) vec2;
    typedef H_VECTOR_TYPE(f32, 4) vec4;
    typedef H_VECTOR_TYPE(f32, 8) vec8;

    // double vectors (f64)
    typedef H_VECTOR_TYPE(f64, 2) dvec2;
    typedef H_VECTOR_TYPE(f64, 4) dvec4;
    typedef H_VECTOR_TYPE(f64, 8) dvec8;

    // to Complex (vec2 only)

#define H_DECLARE_TO_COMPLEX(Type, ComplexType) static inline ComplexType complex Type##_to_complex(Type a) {return a[0] + a[1]*I;}

    H_DECLARE_TO_COMPLEX(ivec2, int)
    H_DECLARE_TO_COMPLEX(vec2, float)
    H_DECLARE_TO_COMPLEX(dvec2, double)
#define to_complex(a) _Generic((a),ivec2:ivec2_to_complex,vec2:vec2_to_complex,dvec2:dvec2_to_complex)(a)

#undef H_DECLARE_TO_COMPLEX

    // to Vector

#define H_DECLARE_TO_VECTOR(Type, VectorType) static inline VectorType Type##_complex_to_vector(Type complex a) {return (VectorType){creal(a), cimag(a)};}

    H_DECLARE_TO_VECTOR(int, ivec2)
    H_DECLARE_TO_VECTOR(float, vec2)
    H_DECLARE_TO_VECTOR(double, dvec2)
#define complex_to_vector(a) _Generic((a),int complex:int_complex_to_vector,float complex:float_complex_to_vector,double complex:double_complex_to_vector)(a)

#undef H_DECLARE_TO_VECTOR

    // DOT operator

#define H_DECLARE_DOT2(Type) static inline f64 Type##_dot2(Type a, Type b) {Type c = a*b;return c[0]+c[1]; }
#define H_DECLARE_DOT3(Type) static inline f64 Type##_dot3(Type a, Type b) {Type c = a*b;return c[0]+c[1]+c[2]; }
#define H_DECLARE_DOT4(Type) static inline f64 Type##_dot4(Type a, Type b) {Type c = a*b;return c[0]+c[1]+c[2]+c[3]; }
#define H_DECLARE_DOT8(Type) static inline f64 Type##_dot8(Type a, Type b) {Type c = a*b;return c[0]+c[1]+c[2]+c[3]+c[4]+c[5]+c[6]+c[7]; }

    H_DECLARE_DOT2(ivec2)
    H_DECLARE_DOT2(vec2)
    H_DECLARE_DOT2(dvec2)
    #define dot2(a, b) _Generic((a),ivec2:ivec2_dot2,vec2:vec2_dot2,dvec2:dvec2_dot2)(a, (typeof(a))b)

    H_DECLARE_DOT3(ivec4)
    H_DECLARE_DOT3(vec4)
    H_DECLARE_DOT3(dvec4)
    #define dot3(a, b) _Generic((a),ivec4:ivec4_dot3,vec4:vec4_dot3,dvec4:dvec4_dot3)(a, (typeof(a))b)

    H_DECLARE_DOT4(ivec4)
    H_DECLARE_DOT4(vec4)
    H_DECLARE_DOT4(dvec4)
    #define dot4(a, b) _Generic((a),ivec4:ivec4_dot4,vec4:vec4_dot4,dvec4:dvec4_dot4)(a, (typeof(a))b)

    H_DECLARE_DOT8(ivec8)
    H_DECLARE_DOT8(vec8)
    H_DECLARE_DOT8(dvec8)
    #define dot8(a, b) _Generic((a),ivec8:ivec8_dot8,vec8:vec8_dot8,dvec8:dvec8_dot8)(a, (typeof(a))b)

    #define dot(a, b) _Generic((a), \
            ivec2:ivec2_dot2,\
            vec2:vec2_dot2,\
            dvec2:dvec2_dot2,\
            ivec4:ivec4_dot4,\
            vec4:vec4_dot4,\
            dvec4:dvec4_dot4,\
            ivec8:ivec8_dot8,\
            vec8:vec8_dot8,\
            dvec8:dvec8_dot8 \
    )(a, b)

#undef H_DECLARE_DOT2
#undef H_DECLARE_DOT3
#undef H_DECLARE_DOT4
#undef H_DECLARE_DOT8

    // CROSS operator

#define H_DECLARE_CROSS(Type) static inline Type Type##_cross(Type a, Type b) {\
    Type aa = H_SWIZZLE(a,1,2,0,3); \
    Type ba = H_SWIZZLE(b,2,0,1,3); \
    Type ab = H_SWIZZLE(a,2,0,1,3); \
    Type bb = H_SWIZZLE(b,1,2,0,3); \
    return aa*ba - ab*bb ;}

H_DECLARE_CROSS(ivec4)
H_DECLARE_CROSS(vec4)
H_DECLARE_CROSS(dvec4)
#define cross(a, b) _Generic((a),ivec4:ivec4_cross,vec4:vec4_cross,dvec4:dvec4_cross)(a, (typeof(a))b)

#undef H_DECLARE_CROSS

// magnitude function

#define H_DECLARE_MAGNITUDE2(Type) static inline f64 Type##_magnitude2(Type a) {return sqrt(dot2(a, a));}
#define H_DECLARE_MAGNITUDE3(Type) static inline f64 Type##_magnitude3(Type a) {return sqrt(dot3(a, a));}
#define H_DECLARE_MAGNITUDE4(Type) static inline f64 Type##_magnitude4(Type a) {return sqrt(dot4(a, a));}
#define H_DECLARE_MAGNITUDE8(Type) static inline f64 Type##_magnitude8(Type a) {return sqrt(dot8(a, a));}

H_DECLARE_MAGNITUDE2(ivec2)
H_DECLARE_MAGNITUDE2(vec2)
H_DECLARE_MAGNITUDE2(dvec2)
#define magnitude2(a) _Generic((a),ivec2:ivec2_magnitude2,vec2:vec2_magnitude2,dvec2:dvec2_magnitude2)(a)

H_DECLARE_MAGNITUDE3(ivec4)
H_DECLARE_MAGNITUDE3(vec4)
H_DECLARE_MAGNITUDE3(dvec4)
#define magnitude3(a) _Generic((a),ivec4:ivec4_magnitude3,vec4:vec4_magnitude3,dvec4:dvec4_magnitude3)(a)

H_DECLARE_MAGNITUDE4(ivec4)
H_DECLARE_MAGNITUDE4(vec4)
H_DECLARE_MAGNITUDE4(dvec4)
#define magnitude4(a) _Generic((a),ivec4:ivec4_magnitude4,vec4:vec4_magnitude4,dvec4:dvec4_magnitude4)(a)

H_DECLARE_MAGNITUDE8(ivec8)
H_DECLARE_MAGNITUDE8(vec8)
H_DECLARE_MAGNITUDE8(dvec8)
#define magnitude8(a) _Generic((a),ivec8:ivec8_magnitude8,vec8:vec8_magnitude8,dvec8:dvec8_magnitude8)(a)

#define magnitude(a) _Generic((a), \
ivec2:ivec2_magnitude2,\
vec2:vec2_magnitude2,\
dvec2:dvec2_magnitude2,\
ivec4:ivec4_magnitude4,\
vec4:vec4_magnitude4,\
dvec4:dvec4_magnitude4,\
ivec8:ivec8_magnitude8,\
vec8:vec8_magnitude8,\
dvec8:dvec8_magnitude8 \
)(a)

#undef H_DECLARE_MAGNITUDE2
#undef H_DECLARE_MAGNITUDE3
#undef H_DECLARE_MAGNITUDE4
#undef H_DECLARE_MAGNITUDE8

#define normalize(a) ({typeof(a) _a = (a); _a / (f32)magnitude(_a);})
#define normalize3(a) ({typeof(a) _a = (a); _a / magnitude3(_a);})

#undef H_VECTOR_TYPE

#endif

#ifdef H_MATRIX
#endif

#else
#error HCGMath only supports GCC and Clang compilers.
#endif

#endif //HCGMATH_HCGMATH_H