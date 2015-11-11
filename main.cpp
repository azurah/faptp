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

    // Par�metro de popula��o inicial
    resolucaoGrasp.horarioPopulacaoInicial = 500;

    // Escolhe o algoritmo de gera��o de vizinhos como aleat�rios, e o n�mero
    // de vizinhos gerados como 4 por itera��o, e o limite de parada como 0.8ms
    resolucaoGrasp.gradeGraspVizinhanca = RESOLUCAO_GRASP_VIZINHOS_ALEATORIOS;

    resolucaoGrasp.gradeGraspVizinhos = 4;
    resolucaoGrasp.gradeGraspTempoConstrucao = .8;

    resolucaoGrasp.gradeTipoConstrucao = RESOLUCAO_GERAR_GRADE_TIPO_GRASP;
    resolucaoGrasp.gradeAlfa = .9;

    auto inicioHorario = clock();
    
    resolucaoGrasp.gerarHorarioAGPopulacaoInicial();
    
    auto fimHorario = clock();
    std::cout << "Tempo do horario: " << (fimHorario - inicioHorario) << std::endl << std::endl;

    auto inicio = clock();

    // Inicia a execu��o do algoritmo
    resolucaoGrasp.start();

    auto fim = clock();
    std::cout << "Tempo: " << fim - inicio << "\n";
}
