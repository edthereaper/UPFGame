#ifndef UTILS_NUMBERS_H_
#define UTILS_NUMBERS_H_

namespace utils {

#define MACRO_EVAL(m)   m
#define M_PIf   MACRO_EVAL(M_PI)##f
#define M_PI_2f MACRO_EVAL(M_PI_2)##f
#define M_PI_4f MACRO_EVAL(M_PI_4)##f
#define M_TAU   (2*M_PI)
#define M_TAUf  (2*M_PIf)
#define M_2_PIf M_TAUf
#define M_4PI   (4*M_PI)
#define M_4PIf  (4*M_PIf)

inline float deg2rad(float deg) { return deg * (float)M_PI / 180.f; }
inline float rad2deg(float rad) { return rad * (180.f) / ((float)M_PI); }

template<class T>
inline T inRange(const T& min, const T& val, const T& max)
{
    return std::min(max, std::max(val, min));
}

inline float minAbs(float a, float b)
{
    if (a>=0) {return std::min(a,std::abs(b));}
    else {return std::max(a,-std::abs(b));}
}

template<class T>
inline T sign(T a) {return a > T(0) ? T(1) : T(-1);}

inline float accumulativeQ1Sin(float f)
{
    return floorf(f/M_PI_2f)+std::sin(fmodf(f, M_PI_2f));
}

inline float sinc(float x) {return x==0?1:std::sin(x)/x;}

}

#endif