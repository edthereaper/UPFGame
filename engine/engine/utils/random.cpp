#include "mcv_platform.h"
#include "random.h"

#include <random>
#include <array>
#include <chrono>

namespace utils {

std::mt19937 _rEngine;

void seedRand()
{
#if defined(_DEBUG) && defined(DEFAULT_SEED)
    _rEngine.seed(std::mt19937::default_seed);
#else
    std::array<int, std::mt19937::state_size> seed_data;
    std::random_device r;
    std::generate_n(seed_data.begin(), seed_data.size(), std::ref(r));
    std::seed_seq seq(seed_data.begin(), seed_data.end());
    _rEngine.seed(seq);
#endif
    srand(NULL);
}

float rand_normal(float mean, float deviation)
{
    return std::normal_distribution<float>(mean, deviation)(_rEngine);
}

float rand_uniform(float max, float min)
{
    return std::uniform_real_distribution<float>(min, max)(_rEngine);
}

int rand_uniform(int max, int min)
{
    return std::uniform_int_distribution<>(min, max)(_rEngine);
}

bool phi(float time, float mean, float deviation)
{
    const float pdf = (float)(0.5 * (2 - std::erfc((time - mean)/deviation*std::sqrt(2))));
    return rand_uniform(1.f) < pdf;
}

}