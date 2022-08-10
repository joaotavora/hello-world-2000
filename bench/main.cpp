#include <benchmark/benchmark.h>

int main()
{
  ::benchmark::Initialize(0, nullptr);
  ::benchmark::RunSpecifiedBenchmarks();
}
