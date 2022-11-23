#include "Random.hpp"

#include <random>

static std::minstd_rand0 generator;
static std::uniform_real_distribution<float> floatD;
static std::uniform_int_distribution<int> intD;

void Random::Init()
{
    generator.seed(time(nullptr));
}

void Random::Shutdown()
{}

float Random::Float()
{
    return floatD(generator);
}

float Random::Float(float min, float max)
{
    float f = floatD(generator);
    return min + ((max - min) * f);
}