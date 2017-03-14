#pragma once

#include <chrono>
#include <ostream>

/// Implementa um cron�metro, facilitando a medi��o da passagem
/// do tempo na condi��o de parada dos algoritmos.
class Timer
{
public:
  /// Cria um novo timer, com tempo inicial igual ao momento de cria��o.
  Timer()
  {
    reset();
  }

  /// Reseta o tempo inicial para o momento da chamada.
  void
  reset()
  {
    start_ = std::chrono::high_resolution_clock::now();
  }

  /// Calcula o tempo decorrido (em ms) do momento inicial ao momento da chamada.
  /// 
  /// @return Tempo em milissegundos entre o momento inicial e o "agora".
  long long
  elapsed() const
  {
    auto e = std::chrono::high_resolution_clock::now() - start_;
    return std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
  }

  /// Imprime a representa��o textual do tempo decorrido para a `os`,
  /// exemplo: `150ms`.
  /// 
  /// @param os Stream de sa�da onde ser� impressa a representa��o textual.
  /// @param t Timer que ser� representado.
  /// @return A stream de sa�da `os`, ap�s ser modificada.
  friend std::ostream&
  operator<<(std::ostream& os, const Timer& t)
  {
    os << t.elapsed() << "ms";
    return os;
  }

private:
  /// Momento inicial, de onde os c�lculos de tempo decorrido ser�o derivados.
  std::chrono::high_resolution_clock::time_point start_;
};
