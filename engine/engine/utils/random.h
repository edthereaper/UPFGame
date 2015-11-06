#ifndef UTILS_RANDOM_H_
#define UTILS_RANDOM_H_

namespace utils {
    
//#define DEFAULT_SEED

//Seed rand
void seedRand();

//Generate random numbers
float rand_normal(float mean = 0.f, float deviation = 1.f);
float rand_uniform(float max, float min = 0.f);
int rand_uniform(int max, int min = 0);
XMVECTOR rand_vector3(float max, float min, float w = 0);
XMVECTOR rand_vectorXZ(float max, float min, float y=0, float w = 0);
XMVECTOR rand_vector4(float max, float min);

/* Throw an n-D */
inline unsigned die(unsigned n) { return rand_uniform((int)n-1); }

/* Random uniform chance*/
inline bool chance(unsigned x, unsigned outOf) { return die(outOf) < x;}

/* probability based on the PDF of a normal distribution (increases with time) */
bool phi(float time, float mean = 0.f, float deviation = 1.f);
};

#endif