#include "system/CpuInfo.h"
#include "Random.h"

using sese::CpuInfo;
using sese::Random;

int main() {
    for (int i = 0; i < 20; i++) {
        printf("%" PRId64 "\n", Random::next());
    }
    return 0;
}