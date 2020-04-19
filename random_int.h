#include <random>

class Rand {
private:
    static std::default_random_engine generator;
    static std::uniform_int_distribution<int> distribution;

public:
    // initialize random engine
    static void Init() {
        std::random_device rd;
        generator = std::default_random_engine(rd());
        distribution = std::uniform_int_distribution<int>(0, INT32_MAX);
    }

    // get non-negative random number in INT32 bounds
    static int Next() {
        return distribution(generator);
    }
};

std::default_random_engine Rand::generator;
std::uniform_int_distribution<int> Rand::distribution;
