#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <chrono>

#include "template/Algorithms.h"

#include "src/parametros.h"

#include "src/Disciplina.h"
#include "src/Professor.h"
#include "src/Horario.h"
#include "src/Resolucao.h"

#include "src/Output.h"

void comArgumentos(char** argv)
{
	experimento = true;

	int numVizinhos;
	double tempoConstr, alfaGrasp;

	std::ifstream arquivoConf(argv[1]);
	std::string arquivoEntrada = argv[2];

	arquivoConf >> numVizinhos >> tempoConstr >> alfaGrasp;

	Resolucao resolucaoGrasp(3, 2, 5, arquivoEntrada);

	resolucaoGrasp.gradeGraspVizinhanca = RESOLUCAO_GRASP_VIZINHOS_ALEATORIOS;

	resolucaoGrasp.gradeGraspVizinhos = numVizinhos;
	resolucaoGrasp.gradeGraspTempoConstrucao = tempoConstr;

	resolucaoGrasp.gradeTipoConstrucao = RESOLUCAO_GERAR_GRADE_TIPO_GRASP;
	resolucaoGrasp.gradeAlfa = alfaGrasp;

	auto inicioTempo = std::chrono::steady_clock::now();
	auto fo = resolucaoGrasp.start();
	auto fimTempo = std::chrono::steady_clock::now();

	std::cout << fo << " " << std::chrono::duration_cast<std::chrono::milliseconds>
			(fimTempo - inicioTempo).count() << "\n";
}

void calibracao(int tipo)
{
	Output o;

	experimento = false;

	double fo;
	int p = 0;
	int params[][5] = {
		{50, 30, 120, 4, 30},
		{100, 30, 120, 4, 30},

		{50, 60, 120, 4, 30},
		{100, 60, 120, 4, 30},

		{50, 30, 120, 6, 30},
		{100, 30, 120, 6, 30},

		{50, 60, 120, 6, 30},
		{100, 60, 120, 6, 30},

		{50, 30, 120, 4, 60},
		{100, 30, 120, 4, 60},

		{50, 60, 120, 4, 60},
		{100, 60, 120, 4, 60},

		{50, 30, 120, 6, 60},
		{100, 30, 120, 6, 60},

		{50, 60, 120, 6, 60},
		{100, 60, 120, 6, 60}
	};

	for (int j = 0; j < 16; j++) {
		p++;

		if (j != 8) {
			continue;
		}

		for (int i = 0; i < 10; i++) {

			//Resolucao resolucaoGrasp(4, (50 - 15), 1413);
			Resolucao resolucaoGrasp(2, 2, 5);

			resolucaoGrasp.horarioPopulacaoInicial = params[j][0];

			resolucaoGrasp.horarioIteracao = 100;
			resolucaoGrasp.horarioTorneioPares = 0;
			resolucaoGrasp.horarioTorneioPopulacao = 1;

			double mutacao = (double) params[j][1] / 100;
			resolucaoGrasp.horarioMutacaoProbabilidade = mutacao;
			resolucaoGrasp.horarioMutacaoTentativas = 2;

			resolucaoGrasp.gradeTipoConstrucao = RESOLUCAO_GERAR_GRADE_TIPO_GRASP;
			//resolucaoGrasp.gradeTipoConstrucao = RESOLUCAO_GERAR_GRADE_TIPO_GULOSO;

			resolucaoGrasp.gradeGraspVizinhanca = RESOLUCAO_GRASP_VIZINHOS_ALEATORIOS;

			double tempo = (double) params[j][2] / 100;
			resolucaoGrasp.gradeGraspTempoConstrucao = tempo;
			resolucaoGrasp.gradeGraspVizinhos = params[j][3];
			double alfa = (double) params[j][4] / 100;
			resolucaoGrasp.gradeAlfa = alfa;

			std::cout << "Inicio: Execucao " << (i + 1) << " da configuracao " << p << std::endl;

			auto inicioHorario = clock();
			resolucaoGrasp.start(false);
			auto fimHorario = clock();

			fo = resolucaoGrasp.getSolucao()->getObjectiveFunction();
			std::cout << "FO da solucao: " << fo << std::endl;

			double diff = ((double) (fimHorario - inicioHorario) / 1000 / 1000);
			std::cout << "Tempo do horario: " << (diff) << "s" << std::endl << std::endl;

			std::stringstream s;
			s << "experimento/p" << j << "e" << i << "fo" << fo;
			std::string savePath = s.str();
			o.write(resolucaoGrasp.getSolucao(), savePath);
		}
	}
}

void semArgumentos()
{
	Output o;

	experimento = false;

	Resolucao resolucaoGrasp(4, (50 - 15), 1413);

	resolucaoGrasp.horarioPopulacaoInicial = 1;


	resolucaoGrasp.horarioIteracao = 0;
	resolucaoGrasp.horarioTorneioPares = 0;
	resolucaoGrasp.horarioTorneioPopulacao = 1;

	resolucaoGrasp.horarioMutacaoProbabilidade = 0;
	resolucaoGrasp.horarioMutacaoTentativas = 2;

	resolucaoGrasp.gradeTipoConstrucao = RESOLUCAO_GERAR_GRADE_TIPO_GRASP;
	resolucaoGrasp.gradeTipoConstrucao = RESOLUCAO_GERAR_GRADE_TIPO_GULOSO;

	resolucaoGrasp.gradeGraspVizinhanca = RESOLUCAO_GRASP_VIZINHOS_ALEATORIOS;

	resolucaoGrasp.gradeGraspVizinhos = 2;
	resolucaoGrasp.gradeGraspTempoConstrucao = .2;

	resolucaoGrasp.gradeAlfa = .9;

	std::cout << "Montando horarios [AG + Grasp]..." << std::endl;

	auto inicioHorario = clock();
	resolucaoGrasp.start(false);
	auto fimHorario = clock();

	double diff1 = ((double) (fimHorario - inicioHorario) / 1000 / 1000);
	std::cout << "Tempo do horario: " << (diff1) << "s" << std::endl << std::endl;

	double fo = resolucaoGrasp.getSolucao()->getObjectiveFunction();
	std::cout << "Resultado:" << fo << std::endl;
	resolucaoGrasp.showResult();

#if defined(_WIN32)
	std::string folder = "teste\\";
#else
	std::string folder = "teste/";
#endif
	std::stringstream s;
	s << folder + "fo" << fo;
	std::string savePath = s.str();
	o.write(resolucaoGrasp.getSolucao(), savePath);
}

int main(int argc, char** argv)
{
	verbose = false;

	if (argc == 3) {
		comArgumentos(argv);
	} else if (argc == 2) {
		calibracao(atoi(argv[1]));
	} else {
		//calibracao(0);
		semArgumentos();
	}
}

