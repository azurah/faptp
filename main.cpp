#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>
#include <vector>

#include "template/Algorithms.h"

#include "src/includes/parametros.h"

#include "src/Disciplina.h"
#include "src/Professor.h"
#include "src/Horario.h"
#include "src/Util.h"
#include "src/Resolucao.h"

int main(int argc, char** argv) {
    verbose = false;

    // Inicializa um objeto resolu��o com blocos tamanho 3, 2 camadas e 
    // 5 perfis de alunos
    Resolucao resolucaoGrasp(3, 2, 5);

    // Par�metro de inicializa��o do AG
    resolucaoGrasp.horarioPopulacaoInicial = 250;
    resolucaoGrasp.horarioTorneioPopulacao = 0.08;

    // Ir� gerar x pares de vencedores
    resolucaoGrasp.horarioTorneioPares = 0.2;

    // 25% dos filhos sofrer�o muta��o
    resolucaoGrasp.horarioMutacaoProbabilidade = .25;
    // Tentativa de mutar 3 genes
    resolucaoGrasp.horarioMutacaoTentativas = 3;

    // Escolhe o algoritmo de gera��o de vizinhos como aleat�rios, e o n�mero
    // de vizinhos gerados como 4 por itera��o, e o limite de parada como 0.8ms
    resolucaoGrasp.gradeGraspVizinhanca = RESOLUCAO_GRASP_VIZINHOS_ALEATORIOS;

    resolucaoGrasp.gradeGraspVizinhos = 4;
    resolucaoGrasp.gradeGraspTempoConstrucao = .8;

    resolucaoGrasp.gradeTipoConstrucao = RESOLUCAO_GERAR_GRADE_TIPO_GRASP;
    resolucaoGrasp.gradeAlfa = .9;

    auto inicioHorario = clock();

    std::cout << "\nMontando horarios [AG + Grasp]..." << std::endl;
    resolucaoGrasp.gerarHorarioAG();
    resolucaoGrasp.showResult();

    auto fimHorario = clock();
    double diff1 = ((float) (fimHorario - inicioHorario)) / (100.0 * 100.0 * 100.0);
    std::cout << "Tempo do horario: " << (diff1) << "s" << std::endl << std::endl;

    auto inicio = clock();

    // Inicia a execu��o do algoritmo
    std::cout << "\nMontando grades [File + Grasp]..." << std::endl;
    resolucaoGrasp.start();

    auto fim = clock();
    double diff2 = ((float) (fim - inicio)) / (100.0 * 100.0 * 100.0);
    std::cout << "Tempo: " << (diff2) << "s" << std::endl;
}
