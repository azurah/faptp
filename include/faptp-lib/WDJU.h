#pragma once

#include <memory>

#include <faptp-lib/Solucao.h>
#include <faptp-lib/Resolucao.h>

class WDJU
{
public:
  /// Constr�i uma configura��o do WDJU.
  /// 
  /// @param res Objeto de onde os operadores de vizinhan�a ser�o chamados
  /// @param timeout Tempo limite de execu��o do algoritmo.
  /// @param stagnation_limit Limite de itera��es sem melhoria.
  /// @param jump_factor Fator de altera��o da altura do salto.
  WDJU(const Resolucao& res, long long timeout, int stagnation_limit,
       double jump_factor);

  /// Aplica o WDJU em uma solu��o inicial, buscando melhorar a solu��o.
  /// A melhor solu��o � retornada quando o tempo se esgota.
  ///
  /// @param solucao Solu��o inicial para o algoritmo.
  /// @return Melhor solu��o encontrada.
  std::unique_ptr<Solucao> gerar_horario(const Solucao& solucao);

  /// Obt�m o tempo em milissegundos do come�o da execu��o at� encontrar
  /// a primeira solu��o com a melhor FO.
  ///
  /// @return Tempo em milissegundos at� encontrar solu��o com a melhor FO.
  long long tempo_fo() const;

  /// Obt�m FO da melhor solu��o encontrada.
  ///
  /// @return FO da melhor solu��o encontrada.
  Solucao::FO_t maior_fo() const;

private:
  double next_jump(const std::vector<double>& history) const;
  std::unique_ptr<Solucao> gerar_vizinho(const Solucao& solucao) const;

  const Resolucao& res_;
  long long timeout_;
  int stagnation_limit_;
  double max_jump_factor_;
  long long tempo_fo_;
  Solucao::FO_t maior_fo_;
};
