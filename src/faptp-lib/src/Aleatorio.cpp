#include <vector>
#include <random>
#include <chrono>
#include <omp.h>
#include <cstdint>

#include <faptp-lib/Aleatorio.h>

class Aleatorio
{
    //! Alias para a classe que representa um relogio de alta precisao
    using Relogio = std::chrono::high_resolution_clock;
public:
    Aleatorio();
    Aleatorio& operator=(const Aleatorio&) = delete;
    Aleatorio(const Aleatorio&) = delete;

    //! \brief Gera um numero aleatorio de 0 a 2^31 - 1 a partir de uma distribuicao uniforme
    //! \return Int na faixa 0 <= x <= 2^31 - 1.
    int randomInt();
    uint32_t randomUInt();

private:
    uint32_t state_;
};

//! Constante para maior valor a ser gerado. Igual 2^31 - 1.
constexpr uint32_t max_random = 2147483647;

Aleatorio::Aleatorio() 
{
  const auto now = Relogio::now().time_since_epoch().count();
  const auto thread_number = omp_get_thread_num();
  state_ = static_cast<uint32_t>(now + thread_number);
}

int Aleatorio::randomInt()
{
  return randomUInt() & max_random;
}

uint32_t
Aleatorio::randomUInt()
{
  state_ ^= state_ << 13;
  state_ ^= state_ >> 17;
  state_ ^= state_ << 5;
  return state_;
}

static std::vector<Aleatorio> generators;

void aleatorio::initRandom(int numThreads)
{
  generators = std::vector<Aleatorio>(numThreads);
}

int aleatorio::randomInt()
{
  thread_local const auto thread_number = omp_get_thread_num();
  return randomInt(thread_number);
}

uint32_t
aleatorio::randomUInt()
{
  thread_local const auto thread_number = omp_get_thread_num();
  return randomUInt(thread_number);
}

uint32_t
aleatorio::randomUInt(int thread_id)
{
  return generators[thread_id].randomUInt();
}

int
aleatorio::randomInt(int thread_id)
{
  return generators[thread_id].randomInt();
}

