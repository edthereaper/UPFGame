#include "mcv_platform.h"
#include "random.h"
#include "app.h"

#include <random>
#include <array>
#include <chrono>

namespace utils {

std::mt19937 _rEngine;

std::map<std::pair<float, float>, std::uniform_real_distribution<float>> _m_Uf;
std::map<std::pair<float, float>, std::uniform_real_distribution<float>> _m_Nf;
std::map<std::pair<int, int>, std::uniform_int_distribution<int>> _m_Ui;

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

XMVECTOR rand_vectorXZ(float max, float min, float y, float w)
{
    auto gen =std::uniform_real_distribution<float>(min, max);
    return DirectX::XMVectorSet(gen(_rEngine), y, gen(_rEngine), w);
}

XMVECTOR rand_vector3(float max, float min, float w)
{
    auto gen =std::uniform_real_distribution<float>(min, max);
    return DirectX::XMVectorSet(gen(_rEngine), gen(_rEngine), gen(_rEngine), w);
}

XMVECTOR rand_vector4(float max, float min)
{
    auto gen =std::uniform_real_distribution<float>(min, max);
    return DirectX::XMVectorSet(gen(_rEngine), gen(_rEngine), gen(_rEngine), gen(_rEngine));
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